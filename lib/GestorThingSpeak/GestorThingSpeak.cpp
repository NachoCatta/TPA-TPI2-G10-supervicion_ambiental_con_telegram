#include "GestorThingSpeak.h"

#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include "CertificadoThingSpeak.h"

namespace {
constexpr unsigned long INTERVALO_MINIMO_THINGSPEAK_MS = 15500UL;
constexpr uint16_t TIMEOUT_THINGSPEAK_MS = 10000U;
constexpr unsigned long TIMEOUT_TLS_SEG = 10UL;
constexpr time_t EPOCH_MINIMO_VALIDO = 1700000000;
const char URL_THINGSPEAK[] = "https://api.thingspeak.com/update";

String camposClima(float temperatura, float humedad) {
    String campos = "field";
    campos += FIELD_TEMPERATURA;
    campos += "=";
    campos += String(temperatura, 2);
    campos += "&field";
    campos += FIELD_HUMEDAD;
    campos += "=";
    campos += String(humedad, 2);
    return campos;
}
}

GestorThingSpeak::GestorThingSpeak()
    : _apiKey(SECRET_THINGSPEAK_WRITE_KEY),
      _channelId(SECRET_THINGSPEAK_CHANNEL_ID),
      _ultimoEnvioMs(0),
      _ultimaActualizacionClimaMs(0),
      _huboEnvio(false),
      _huboActualizacionClima(false),
      _voltajePendiente(false), _voltajeProgramado(0),
      _resultadoVoltajeDisponible(false), _resultadoVoltajeExito(false),
      _resultadoVoltaje(0) {}

void GestorThingSpeak::iniciar() {
    _ultimoEnvioMs = 0;
    _ultimaActualizacionClimaMs = millis();
    _huboEnvio = false;
    _huboActualizacionClima = false;
    _voltajePendiente = false;
    _resultadoVoltajeDisponible = false;
    Serial.printf("[ThingSpeak] Canal configurado: %lu\n", _channelId);
}

void GestorThingSpeak::actualizar(const GestorSensores& sensores) {
    unsigned long ahoraMs = millis();
    if (_voltajePendiente) {
        if (WiFi.status() == WL_CONNECTED &&
            time(nullptr) >= EPOCH_MINIMO_VALIDO && puedeEnviar()) {
            const float voltaje = _voltajeProgramado;
            bool exito;
            if (sensores.datosDhtValidos()) {
                String campos = camposClima(sensores.obtenerTemperatura(),
                                             sensores.obtenerHumedad());
                campos += "&field";
                campos += FIELD_VOLTAJE_POT;
                campos += "=";
                campos += String(voltaje, 3);
                _ultimaActualizacionClimaMs = ahoraMs;
                _huboActualizacionClima = true;
                exito = realizarPeticion(campos);
            } else {
                exito = enviarVoltajePotenciometro(voltaje);
            }
            _resultadoVoltaje = voltaje;
            _resultadoVoltajeExito = exito;
            _resultadoVoltajeDisponible = true;
            if (exito) {
                _voltajePendiente = false;
            }
        }
        return;
    }
    if (_huboActualizacionClima &&
        ahoraMs - _ultimaActualizacionClimaMs < INTERVALO_THINGSPEAK_MS) {
        return;
    }

    if (!sensores.datosDhtValidos()) {
        if (ahoraMs - _ultimaActualizacionClimaMs <
            INTERVALO_LECTURA_DHT_MS) {
            return;
        }
        _ultimaActualizacionClimaMs = ahoraMs;
        _huboActualizacionClima = false;
        Serial.println(
            F("[ThingSpeak] Se omite clima: la ultima lectura DHT22 no es valida."));
        return;
    }

    if (WiFi.status() != WL_CONNECTED ||
        time(nullptr) < EPOCH_MINIMO_VALIDO || !puedeEnviar()) {
        return;
    }

    _ultimaActualizacionClimaMs = ahoraMs;
    _huboActualizacionClima = true;
    enviarClima(sensores.obtenerTemperatura(), sensores.obtenerHumedad());
}

bool GestorThingSpeak::enviarClima(float temperatura, float humedad) {
    return realizarPeticion(camposClima(temperatura, humedad));
}

bool GestorThingSpeak::enviarVoltajePotenciometro(float voltaje) {
    String queryParams = "field";
    queryParams += FIELD_VOLTAJE_POT;
    queryParams += "=";
    queryParams += String(voltaje, 3);
    return realizarPeticion(queryParams);
}

void GestorThingSpeak::programarEnvioVoltaje(float voltaje) {
    _voltajeProgramado = voltaje;
    _voltajePendiente = true;
    _resultadoVoltajeDisponible = false;
}

bool GestorThingSpeak::consumirResultadoEnvioVoltaje(bool& exito,
                                                     float& voltaje) {
    if (!_resultadoVoltajeDisponible) {
        return false;
    }
    exito = _resultadoVoltajeExito;
    voltaje = _resultadoVoltaje;
    _resultadoVoltajeDisponible = false;
    return true;
}

bool GestorThingSpeak::puedeEnviar() const {
    return !_huboEnvio ||
           millis() - _ultimoEnvioMs >= INTERVALO_MINIMO_THINGSPEAK_MS;
}

unsigned long GestorThingSpeak::tiempoRestanteSegundos() const {
    if (!_huboEnvio) {
        return 0;
    }

    unsigned long transcurridoMs = millis() - _ultimoEnvioMs;
    if (transcurridoMs >= INTERVALO_MINIMO_THINGSPEAK_MS) {
        return 0;
    }

    unsigned long restanteMs =
        INTERVALO_MINIMO_THINGSPEAK_MS - transcurridoMs;
    return restanteMs / 1000UL + (restanteMs % 1000UL != 0);
}

bool GestorThingSpeak::realizarPeticion(const String& queryParams) {
    if (!puedeEnviar()) {
        Serial.printf(
            "[ThingSpeak] Escritura rechazada por limite local; espere %lu s.\n",
            tiempoRestanteSegundos());
        return false;
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println(F("[ThingSpeak] No se envia: Wi-Fi desconectado."));
        return false;
    }
    if (time(nullptr) < EPOCH_MINIMO_VALIDO) {
        Serial.println(F("[ThingSpeak] No se envia: hora NTP aun no valida."));
        return false;
    }

    String url = URL_THINGSPEAK;
    url += "?api_key=";
    url += _apiKey;
    url += "&";
    url += queryParams;

    WiFiClientSecure cliente;
    cliente.setCACert(THINGSPEAK_CERTIFICATE_ROOT);
    cliente.setTimeout(TIMEOUT_TLS_SEG);
    cliente.setHandshakeTimeout(TIMEOUT_TLS_SEG);
    static_cast<Stream &>(cliente).setTimeout(TIMEOUT_THINGSPEAK_MS);
    HTTPClient http;
    http.setConnectTimeout(TIMEOUT_THINGSPEAK_MS);
    http.setTimeout(TIMEOUT_THINGSPEAK_MS);
    if (!http.begin(cliente, url)) {
        Serial.println(F("[ThingSpeak] No se pudo iniciar la peticion HTTP."));
        _ultimoEnvioMs = millis();
        _huboEnvio = true;
        return false;
    }

    int codigoHttp = http.GET();

    String payload;
    if (codigoHttp == HTTP_CODE_OK) {
        payload = http.getString();
        payload.trim();
    }
    http.end();
    // Una escritura puede llegar al servicio aunque falle la respuesta.
    _ultimoEnvioMs = millis();
    _huboEnvio = true;

    bool entryIdValido = payload.length() > 0;
    for (unsigned int i = 0; i < payload.length(); ++i) {
        if (payload[i] < '0' || payload[i] > '9') {
            entryIdValido = false;
            break;
        }
    }

    bool exito = codigoHttp == HTTP_CODE_OK && entryIdValido &&
                 payload.toInt() > 0;
    if (exito) {
        Serial.printf("[ThingSpeak] Escritura aceptada. Entry ID: %s\n",
                      payload.c_str());
    } else {
        Serial.printf(
            "[ThingSpeak] Escritura fallida. HTTP: %d, respuesta: %s\n",
            codigoHttp, payload.c_str());
    }
    return exito;
}

#include "GestorSensores.h"

GestorSensores::GestorSensores(uint8_t pinDht, uint8_t pinPot)
    : _pinDht(pinDht),
      _pinPot(pinPot),
      _dht(pinDht, DHT22),
      _temperatura(0.0f),
      _humedad(0.0f),
      _dhtValido(false),
      _primeraLecturaDht(false),
      _adcPot(0),
      _voltajePot(0.0f),
      _enEstadoCritico(false),
      _alertaPendiente(false),
      _ultimaLecturaDhtMs(0),
      _ultimaLecturaPotMs(0),
      _ultimaAlertaMs(0) {}

void GestorSensores::iniciar() {
  _dht.begin();
  pinMode(_pinPot, INPUT);
  analogReadResolution(12); // Configura el ADC del ESP32 en 12 bits (0 - 4095)

  // Lectura inicial al arrancar el sistema
  unsigned long ahoraMs = millis();
  leerPotenciometro(ahoraMs);
  leerDht();
  _ultimaLecturaPotMs = ahoraMs;
  _ultimaLecturaDhtMs = ahoraMs;
}

void GestorSensores::actualizar() {
  unsigned long ahoraMs = millis();

  // 1. Muestreo periódico no bloqueante del potenciómetro (cada 250 ms)
  if (ahoraMs - _ultimaLecturaPotMs >= INTERVALO_LECTURA_POT_MS) {
    _ultimaLecturaPotMs = ahoraMs;
    leerPotenciometro(ahoraMs);
  }

  // 2. Muestreo periódico no bloqueante del sensor DHT22 (cada 2500 ms)
  if (!_primeraLecturaDht ||
      (ahoraMs - _ultimaLecturaDhtMs >= INTERVALO_LECTURA_DHT_MS)) {
    _ultimaLecturaDhtMs = ahoraMs;
    _primeraLecturaDht = true;
    leerDht();
  }
}

void GestorSensores::forzarLecturaPotenciometro() {
  unsigned long ahoraMs = millis();
  _ultimaLecturaPotMs = ahoraMs;
  leerPotenciometro(ahoraMs);
}

void GestorSensores::leerDht() {
  float nuevaHumedad = _dht.readHumidity();
  float nuevaTemperatura = _dht.readTemperature();

  // Validar que la lectura no sea NaN (Not a Number) por fallo de comunicación
  if (!isnan(nuevaHumedad) && !isnan(nuevaTemperatura)) {
    _humedad = nuevaHumedad;
    _temperatura = nuevaTemperatura;
    _dhtValido = true;
  } else {
    Serial.println(
        F("[Sensores] Advertencia: Error al leer el sensor DHT22."));
  }
}

void GestorSensores::leerPotenciometro(unsigned long ahoraMs) {
  _adcPot = analogRead(_pinPot);
  _voltajePot =
      (static_cast<float>(_adcPot) / ADC_RESOLUCION) * VOLTAJE_REFERENCIA;

  // Evaluación de umbral crítico con histéresis y cooldown no bloqueante
  if (_voltajePot >= UMBRAL_CRITICO_VOLTAJE) {
    bool cooldownCumplido =
        (_ultimaAlertaMs == 0) ||
        (ahoraMs - _ultimaAlertaMs >= COOLDOWN_ALERTA_VOLTAJE_MS);

    if (!_enEstadoCritico && cooldownCumplido) {
      _alertaPendiente = true;
      _ultimaAlertaMs = ahoraMs;
      Serial.printf(
          "[Sensores] ALERTA CRITICA: Voltaje %.2f V supera umbral (%.2f V)\n",
          _voltajePot, UMBRAL_CRITICO_VOLTAJE);
    }
    _enEstadoCritico = true;
  } else if (_voltajePot <= (UMBRAL_CRITICO_VOLTAJE - HISTERESIS_VOLTAJE)) {
    // Rearma la detección únicamente cuando cae por debajo del margen de histéresis
    _enEstadoCritico = false;
  }
}

float GestorSensores::obtenerTemperatura() const { return _temperatura; }

float GestorSensores::obtenerHumedad() const { return _humedad; }

bool GestorSensores::datosDhtValidos() const { return _dhtValido; }

uint16_t GestorSensores::obtenerValorAdcPot() const { return _adcPot; }

float GestorSensores::obtenerVoltajePot() const { return _voltajePot; }

float GestorSensores::obtenerPorcentajePot() const {
  return (static_cast<float>(_adcPot) / ADC_RESOLUCION) * 100.0f;
}

bool GestorSensores::superaUmbralCritico() const { return _enEstadoCritico; }

bool GestorSensores::consumirEventoAlertaCritica() {
  if (_alertaPendiente) {
    _alertaPendiente = false;
    return true;
  }
  return false;
}

#include "GestorSensores.h"

GestorSensores::GestorSensores(uint8_t pinDht, uint8_t pinPot)
    : _pinDht(pinDht),
      _pinPot(pinPot),
      _dht(pinDht, DHT22),
      _temperatura(0.0f),
      _humedad(0.0f),
      _dhtValido(false),
      _adcPot(0),
      _voltajePot(0.0f),
      _enEstadoCritico(false),
      _alertaGeneradaEnEstadoCritico(false),
      _alertaPendiente(false),
      _ultimaLecturaDhtMs(0),
      _ultimaLecturaPotMs(0),
      _ultimaAlertaMs(0) {}

void GestorSensores::iniciar() {
  _dht.begin();
  pinMode(_pinPot, INPUT);
  analogReadResolution(12); // Configura el ADC del ESP32 en 12 bits (0 - 4095)
  analogSetPinAttenuation(_pinPot, ADC_11db);

  // Lectura inicial al arrancar el sistema
  unsigned long ahoraMs = millis();
  // Permite alertar en el arranque, incluso cuando millis() vale cero.
  _ultimaAlertaMs = ahoraMs - COOLDOWN_ALERTA_VOLTAJE_MS;
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
  if (ahoraMs - _ultimaLecturaDhtMs >= INTERVALO_LECTURA_DHT_MS) {
    _ultimaLecturaDhtMs = ahoraMs;
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
  _dhtValido = !isnan(nuevaHumedad) && !isnan(nuevaTemperatura);
  if (_dhtValido) {
    _humedad = nuevaHumedad;
    _temperatura = nuevaTemperatura;
  } else {
    Serial.println(
        F("[Sensores] Advertencia: Error al leer el sensor DHT22."));
  }
}

void GestorSensores::leerPotenciometro(unsigned long ahoraMs) {
  _adcPot = analogRead(_pinPot);
  _voltajePot = analogReadMilliVolts(_pinPot) / 1000.0f;

  // La histéresis define el episodio; el cooldown solo retrasa su alerta.
  if (_voltajePot >= UMBRAL_CRITICO_VOLTAJE) {
    _enEstadoCritico = true;
  } else if (_voltajePot <= (UMBRAL_CRITICO_VOLTAJE - HISTERESIS_VOLTAJE)) {
    _enEstadoCritico = false;
    _alertaGeneradaEnEstadoCritico = false;
  }

  bool cooldownCumplido = ahoraMs - _ultimaAlertaMs >= COOLDOWN_ALERTA_VOLTAJE_MS;
  if (_enEstadoCritico && !_alertaGeneradaEnEstadoCritico &&
      !_alertaPendiente && cooldownCumplido) {
    _alertaPendiente = true;
    _alertaGeneradaEnEstadoCritico = true;
    _ultimaAlertaMs = ahoraMs;
    Serial.printf(
        "[Sensores] ALERTA CRITICA: Voltaje %.2f V supera umbral (%.2f V)\n",
        _voltajePot, UMBRAL_CRITICO_VOLTAJE);
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

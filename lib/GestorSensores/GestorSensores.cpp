#include "GestorSensores.h"

GestorSensores::GestorSensores(uint8_t pinDht, uint8_t pinPot)
    : _pinDht(pinDht), _pinPot(pinPot), _dht(pinDht, DHT22), _temperatura(0.0f),
      _humedad(0.0f), _dhtValido(false), _adcPot(0), _voltajePot(0.0f),
      _enEstadoCritico(false), _alertaPendiente(false), _ultimaLecturaDhtMs(0),
      _ultimaLecturaPotMs(0), _ultimaAlertaMs(0) {}

void GestorSensores::iniciar() {
  _dht.begin();
  pinMode(_pinPot, INPUT);
}

void GestorSensores::actualizar() {
  // Se implementará la lógica completa de lectura en la Etapa 2
}

float GestorSensores::obtenerTemperatura() const { return _temperatura; }
float GestorSensores::obtenerHumedad() const { return _humedad; }
bool GestorSensores::datosDhtValidos() const { return _dhtValido; }

uint16_t GestorSensores::obtenerValorAdcPot() const { return _adcPot; }
float GestorSensores::obtenerVoltajePot() const { return _voltajePot; }
bool GestorSensores::superaUmbralCritico() const { return _enEstadoCritico; }

bool GestorSensores::consumirEventoAlertaCritica() {
  if (_alertaPendiente) {
    _alertaPendiente = false;
    return true;
  }
  return false;
}

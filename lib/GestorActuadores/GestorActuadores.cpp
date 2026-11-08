#include "GestorActuadores.h"

GestorActuadores::GestorActuadores(uint8_t pinReleVerde, uint8_t pinLedAzul)
    : _pinReleVerde(pinReleVerde), _pinLedAzul(pinLedAzul), _estadoRele(false),
      _estadoLedAzul(false), _temporizadorActivo(false),
      _inicioTemporizadorMs(0), _duracionTemporizadorMs(0) {}

void GestorActuadores::iniciar() {
  pinMode(_pinReleVerde, OUTPUT);
  pinMode(_pinLedAzul, OUTPUT);
  digitalWrite(_pinReleVerde, LOW);
  digitalWrite(_pinLedAzul, LOW);
}

void GestorActuadores::actualizar() {
  // Se implementará el apagado automático no bloqueante en la Etapa 2
}

void GestorActuadores::encenderRele() {
  _estadoRele = true;
  _temporizadorActivo = false;
  digitalWrite(_pinReleVerde, HIGH);
}

void GestorActuadores::encenderReleTemporizado(unsigned long segundos) {
  // Se completará en la Etapa 2
  (void)segundos;
}

void GestorActuadores::apagarRele() {
  _estadoRele = false;
  _temporizadorActivo = false;
  digitalWrite(_pinReleVerde, LOW);
}

void GestorActuadores::fijarLedAzul(bool encendido) {
  _estadoLedAzul = encendido;
  digitalWrite(_pinLedAzul, encendido ? HIGH : LOW);
}

bool GestorActuadores::estaReleEncendido() const { return _estadoRele; }
bool GestorActuadores::estaLedAzulEncendido() const { return _estadoLedAzul; }
unsigned long GestorActuadores::segundosRestantesRele() const { return 0; }

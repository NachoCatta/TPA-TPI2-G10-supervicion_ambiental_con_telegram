#include "GestorActuadores.h"

GestorActuadores::GestorActuadores(uint8_t pinReleVerde, uint8_t pinLedAzul)
    : _pinReleVerde(pinReleVerde),
      _pinLedAzul(pinLedAzul),
      _estadoRele(false),
      _estadoLedAzul(false),
      _temporizadorActivo(false),
      _eventoApagadoAuto(false),
      _inicioTemporizadorMs(0),
      _duracionTemporizadorMs(0) {}

void GestorActuadores::iniciar() {
  pinMode(_pinReleVerde, OUTPUT);
  pinMode(_pinLedAzul, OUTPUT);
  digitalWrite(_pinReleVerde, LOW);
  digitalWrite(_pinLedAzul, LOW);
}

void GestorActuadores::actualizar() {
  // Verificación no bloqueante del temporizador de apagado automático del relé
  if (_estadoRele && _temporizadorActivo) {
    unsigned long ahoraMs = millis();
    if (ahoraMs - _inicioTemporizadorMs >= _duracionTemporizadorMs) {
      _estadoRele = false;
      _temporizadorActivo = false;
      _eventoApagadoAuto = true;
      digitalWrite(_pinReleVerde, LOW);
      Serial.println(
          F("[Actuadores] Tiempo cumplido: Rele (LED Verde) apagado automaticamente."));
    }
  }
}

void GestorActuadores::encenderRele() {
  _estadoRele = true;
  _temporizadorActivo = false;
  _eventoApagadoAuto = false;
  digitalWrite(_pinReleVerde, HIGH);
  Serial.println(F("[Actuadores] Rele (LED Verde) encendido de forma continua."));
}

void GestorActuadores::encenderReleTemporizado(unsigned long segundos) {
  if (segundos == 0) {
    encenderRele();
    return;
  }

  _estadoRele = true;
  _temporizadorActivo = true;
  _eventoApagadoAuto = false;
  _inicioTemporizadorMs = millis();
  _duracionTemporizadorMs = segundos * 1000UL;
  digitalWrite(_pinReleVerde, HIGH);

  Serial.printf(
      "[Actuadores] Rele (LED Verde) encendido por %lu segundos.\n", segundos);
}

void GestorActuadores::apagarRele() {
  _estadoRele = false;
  _temporizadorActivo = false;
  _eventoApagadoAuto = false;
  digitalWrite(_pinReleVerde, LOW);
  Serial.println(F("[Actuadores] Rele (LED Verde) apagado."));
}

void GestorActuadores::fijarLedAzul(bool encendido) {
  if (_estadoLedAzul != encendido) {
    _estadoLedAzul = encendido;
    digitalWrite(_pinLedAzul, encendido ? HIGH : LOW);
  }
}

bool GestorActuadores::estaReleEncendido() const { return _estadoRele; }

bool GestorActuadores::estaLedAzulEncendido() const { return _estadoLedAzul; }

bool GestorActuadores::tieneTemporizadorActivo() const {
  return _estadoRele && _temporizadorActivo;
}

unsigned long GestorActuadores::segundosRestantesRele() const {
  if (!_estadoRele || !_temporizadorActivo) {
    return 0;
  }
  unsigned long transcurridoMs = millis() - _inicioTemporizadorMs;
  if (transcurridoMs >= _duracionTemporizadorMs) {
    return 0;
  }
  // Redondeo hacia arriba al segundo entero más cercano
  return (_duracionTemporizadorMs - transcurridoMs + 999UL) / 1000UL;
}

bool GestorActuadores::consumirEventoApagadoAutomatico() {
  if (_eventoApagadoAuto) {
    _eventoApagadoAuto = false;
    return true;
  }
  return false;
}

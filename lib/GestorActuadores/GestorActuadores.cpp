#include "GestorActuadores.h"

GestorActuadores::GestorActuadores(uint8_t pinRele, uint8_t pinLedVerde, uint8_t pinLedAzul)
    : _pinRele(pinRele), _pinLedVerde(pinLedVerde), _pinLedAzul(pinLedAzul), _estadoRele(false),
      _estadoLedAzul(false), _temporizadorActivo(false), _eventoApagadoAuto(false),
      _inicioTemporizadorMs(0), _duracionTemporizadorMs(0), _temporizador(nullptr) {}

GestorActuadores::~GestorActuadores() {
  if (_temporizador) {
    esp_timer_stop(_temporizador);
    esp_timer_delete(_temporizador);
  }
}

void GestorActuadores::bloquear() const {
#ifdef ARDUINO_ARCH_ESP32
  portENTER_CRITICAL(&_proteccion);
#endif
}

void GestorActuadores::desbloquear() const {
#ifdef ARDUINO_ARCH_ESP32
  portEXIT_CRITICAL(&_proteccion);
#endif
}

void GestorActuadores::iniciar() {
  if (_temporizador) esp_timer_stop(_temporizador);
  bloquear();
  _estadoRele = false;
  _temporizadorActivo = false;
  _eventoApagadoAuto = false;
  fijarEstadoRele(false);
  desbloquear();
  _estadoLedAzul = false;
  digitalWrite(_pinLedAzul, LOW);
  pinMode(_pinRele, OUTPUT);
  pinMode(_pinLedVerde, OUTPUT);
  pinMode(_pinLedAzul, OUTPUT);
  if (!_temporizador) {
    esp_timer_create_args_t argumentos = {};
    argumentos.callback = alVencerTemporizador;
    argumentos.arg = this;
    argumentos.name = "rele";
    if (esp_timer_create(&argumentos, &_temporizador) != ESP_OK) {
      Serial.println(F("[Actuadores] Error al crear temporizador."));
    }
  }
}

void GestorActuadores::fijarEstadoRele(bool encendido) {
  digitalWrite(_pinRele, encendido ? NIVEL_RELE_ACTIVO : (NIVEL_RELE_ACTIVO == HIGH ? LOW : HIGH));
  digitalWrite(_pinLedVerde, encendido ? HIGH : LOW);
}

void GestorActuadores::actualizar() {
  // El apagado lo ejecuta esp_timer, incluso cuando el loop espera a la red.
}

void GestorActuadores::alVencerTemporizador(void *contexto) {
  static_cast<GestorActuadores *>(contexto)->apagarSiVencido();
}

void GestorActuadores::apagarSiVencido() {
  bloquear();
  // Un callback anterior puede haber quedado pendiente al reprogramar.
  if (_estadoRele && _temporizadorActivo &&
      millis() - _inicioTemporizadorMs >= _duracionTemporizadorMs) {
    fijarEstadoRele(false);
    _estadoRele = false;
    _temporizadorActivo = false;
    _eventoApagadoAuto = true;
  }
  desbloquear();
}

void GestorActuadores::encenderRele() {
  if (_temporizador) esp_timer_stop(_temporizador);
  bloquear();
  _estadoRele = true;
  _temporizadorActivo = false;
  _eventoApagadoAuto = false;
  fijarEstadoRele(true);
  desbloquear();
}

bool GestorActuadores::encenderReleTemporizado(unsigned long segundos) {
  if (segundos == 0 || segundos > MAX_TIEMPO_RELE_SEGUNDOS || !_temporizador) {
    return false;
  }

  esp_timer_stop(_temporizador);
  bloquear();
  _estadoRele = true;
  _temporizadorActivo = true;
  _eventoApagadoAuto = false;
  _inicioTemporizadorMs = millis();
  _duracionTemporizadorMs = segundos * 1000UL;
  fijarEstadoRele(true);
  desbloquear();

  if (esp_timer_start_once(_temporizador, static_cast<uint64_t>(segundos) * 1000000ULL) != ESP_OK) {
    apagarRele();
    Serial.println(F("[Actuadores] Error al iniciar temporizador; salida apagada."));
    return false;
  }
  return true;
}

void GestorActuadores::apagarRele() {
  if (_temporizador) esp_timer_stop(_temporizador);
  bloquear();
  _estadoRele = false;
  _temporizadorActivo = false;
  _eventoApagadoAuto = false;
  fijarEstadoRele(false);
  desbloquear();
}

void GestorActuadores::fijarLedAzul(bool encendido) {
  if (_estadoLedAzul != encendido) {
    _estadoLedAzul = encendido;
    digitalWrite(_pinLedAzul, encendido ? HIGH : LOW);
  }
}

bool GestorActuadores::estaReleEncendido() const {
  bloquear();
  bool estado = _estadoRele;
  desbloquear();
  return estado;
}
bool GestorActuadores::estaLedAzulEncendido() const { return _estadoLedAzul; }

bool GestorActuadores::tieneTemporizadorActivo() const {
  bloquear();
  bool activo = _estadoRele && _temporizadorActivo;
  desbloquear();
  return activo;
}

unsigned long GestorActuadores::segundosRestantesRele() const {
  bloquear();
  unsigned long restanteMs = 0;
  if (_estadoRele && _temporizadorActivo) {
    unsigned long transcurridoMs = millis() - _inicioTemporizadorMs;
    if (transcurridoMs < _duracionTemporizadorMs) {
      restanteMs = _duracionTemporizadorMs - transcurridoMs;
    }
  }
  desbloquear();
  return restanteMs / 1000UL + (restanteMs % 1000UL != 0);
}

bool GestorActuadores::consumirEventoApagadoAutomatico() {
  bloquear();
  bool evento = _eventoApagadoAuto;
  _eventoApagadoAuto = false;
  desbloquear();
  return evento;
}

#include "GestorTelegram.h"

GestorTelegram::GestorTelegram()
    : _bot(SECRET_BOT_TOKEN, _clienteSeguro), _ultimaConsultaMs(0),
      _ultimoChatIdActivo(SECRET_CHAT_ID) {}

void GestorTelegram::iniciar() {
  // Se implementará la configuración TLS y manejo de comandos en la Etapa 4
  _clienteSeguro.setCACert(TELEGRAM_CERTIFICATE_ROOT);
}

void GestorTelegram::actualizar(GestorSensores &sensores,
                                GestorActuadores &actuadores,
                                GestorDisplay &display,
                                GestorThingSpeak &thingSpeak) {
  (void)sensores;
  (void)actuadores;
  (void)display;
  (void)thingSpeak;
}

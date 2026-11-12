#include "GestorTelegram.h"
#include "ComandoRele.h"

#include <WiFi.h>
#include <time.h>

namespace {
constexpr unsigned long TELEGRAM_CLIENT_TIMEOUT_SEG = 10UL;
constexpr unsigned int TELEGRAM_RESPUESTA_TIMEOUT_MS = 10000U;
constexpr unsigned long INTERVALO_REINTENTO_ALERTA_MS = 5000UL;
constexpr time_t EPOCH_MINIMO_VALIDO = 1700000000;

bool horaValida() {
  return time(nullptr) >= EPOCH_MINIMO_VALIDO;
}

String tecladoPrincipal() {
  return F("[[{\"text\":\"Ver clima\",\"callback_data\":\"/clima\"},"
           "{\"text\":\"Encender 10 s\",\"callback_data\":\"/releon 10\"}],"
           "[{\"text\":\"Apagar rele\",\"callback_data\":\"/releoff\"},"
           "{\"text\":\"Enviar pote\",\"callback_data\":\"/thingup\"}],"
           "[{\"text\":\"Display LED\",\"callback_data\":\"/displayled\"},"
           "{\"text\":\"Display pote\",\"callback_data\":\"/displaypot\"}],"
           "[{\"text\":\"Display DHT\",\"callback_data\":\"/displaydht\"}]]");
}
} // namespace

GestorTelegram::GestorTelegram()
    : _bot(SECRET_BOT_TOKEN, _clienteSeguro), _ultimaConsultaMs(0),
      _ultimoIntentoAlertaMs(0), _ultimoChatIdActivo(SECRET_CHAT_ID),
      _mensajeAlertaPendiente(), _hayAlertaPendiente(false),
      _huboConsultaTelegram(false) {}

void GestorTelegram::iniciar() {
  _clienteSeguro.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  _clienteSeguro.setTimeout(TELEGRAM_CLIENT_TIMEOUT_SEG);
  static_cast<Stream &>(_clienteSeguro).setTimeout(TELEGRAM_RESPUESTA_TIMEOUT_MS);
  _clienteSeguro.setHandshakeTimeout(TELEGRAM_CLIENT_TIMEOUT_SEG);
  _bot.longPoll = 0;
  _bot.waitForResponse = TELEGRAM_RESPUESTA_TIMEOUT_MS;
}

bool GestorTelegram::chatAutorizado(const String &chatId) const {
  return chatId == String(SECRET_CHAT_ID);
}

void GestorTelegram::actualizar(GestorSensores &sensores,
                                GestorActuadores &actuadores,
                                GestorDisplay &display,
                                GestorThingSpeak &thingSpeak) {
  actualizarAlerta(sensores);

  if (WiFi.status() != WL_CONNECTED || !horaValida()) {
    return;
  }

  bool exito = false;
  float voltaje = 0;
  if (thingSpeak.consumirResultadoEnvioVoltaje(exito, voltaje)) {
    String mensaje = exito ? F("Voltaje enviado a ThingSpeak: ")
                           : F("Fallo el envio de voltaje; se reintentara: ");
    mensaje += String(voltaje, 3);
    mensaje += F(" V.");
    _bot.sendMessage(String(SECRET_CHAT_ID), mensaje, "");
  }

  unsigned long ahoraMs = millis();
  if (!_huboConsultaTelegram ||
      ahoraMs - _ultimaConsultaMs >= INTERVALO_TELEGRAM_POLL_MS) {
    _ultimaConsultaMs = ahoraMs;
    _huboConsultaTelegram = true;

    int mensajes = _bot.getUpdates(_bot.last_message_received + 1);
    for (int i = 0; i < mensajes; ++i) {
      if (_bot.messages[i].type == F("callback_query")) {
        procesarCallback(_bot.messages[i].chat_id,
                         _bot.messages[i].query_id, _bot.messages[i].text,
                         sensores, actuadores, display, thingSpeak);
      } else {
        procesarComando(_bot.messages[i].chat_id, _bot.messages[i].text,
                        sensores, actuadores, display, thingSpeak);
      }
    }
  }

  enviarAlertaPendiente();
}

void GestorTelegram::procesarCallback(
    const String &chatId, const String &callbackId, const String &comando,
    GestorSensores &sensores, GestorActuadores &actuadores,
    GestorDisplay &display, GestorThingSpeak &thingSpeak) {
  if (!chatAutorizado(chatId)) {
    _bot.answerCallbackQuery(callbackId, F("Chat no autorizado"), true);
    return;
  }

  _bot.answerCallbackQuery(callbackId, F("Orden recibida"));
  procesarComando(chatId, comando, sensores, actuadores, display, thingSpeak);
}

void GestorTelegram::procesarComando(
    const String &chatId, const String &comando, GestorSensores &sensores,
    GestorActuadores &actuadores, GestorDisplay &display,
    GestorThingSpeak &thingSpeak) {
  if (!chatAutorizado(chatId)) {
    _bot.sendMessage(chatId, F("Chat no autorizado."), "");
    return;
  }

  _ultimoChatIdActivo = String(SECRET_CHAT_ID);
  if (comando == F("/start")) {
    _bot.sendMessageWithInlineKeyboard(
        chatId,
        F("Bienvenido. Selecciona una accion para supervisar el sistema."),
        "", tecladoPrincipal());
    return;
  }

  if (comando == F("/clima")) {
    if (!sensores.datosDhtValidos()) {
      _bot.sendMessage(chatId, F("No hay una lectura DHT22 valida."), "");
      return;
    }
    String respuesta = F("Temperatura: ");
    respuesta += String(sensores.obtenerTemperatura(), 1);
    respuesta += F(" C\nHumedad: ");
    respuesta += String(sensores.obtenerHumedad(), 1);
    respuesta += F(" %");
    _bot.sendMessage(chatId, respuesta, "");
    return;
  }

  if (comando.startsWith(F("/releon"))) {
    unsigned long segundos = 0;
    if (!parsearComandoRele(comando.c_str(), MAX_TIEMPO_RELE_SEGUNDOS,
                            segundos) ||
        !actuadores.encenderReleTemporizado(segundos)) {
      _bot.sendMessage(chatId, F("Uso: /releon <segundos>, entre 1 y 3600."),
                       "");
      return;
    }
    _bot.sendMessage(chatId, F("Rele encendido por el tiempo indicado."), "");
    return;
  }

  if (comando == F("/releoff")) {
    actuadores.apagarRele();
    _bot.sendMessage(chatId, F("Rele apagado."), "");
    return;
  }

  if (comando == F("/thingup")) {
    sensores.forzarLecturaPotenciometro();
    thingSpeak.programarEnvioVoltaje(sensores.obtenerVoltajePot());
    _bot.sendMessage(chatId, F("Envio de voltaje programado"), "");
    return;
  }

  if (comando == F("/displayled")) {
    if (!display.estaInicializado()) {
      _bot.sendMessage(chatId, F("No se pudo inicializar el display."), "");
      return;
    }
    display.mostrarEstadoLed(actuadores.estaReleEncendido(),
                             actuadores.estaLedAzulEncendido(),
                             actuadores.segundosRestantesRele());
    _bot.sendMessage(chatId, F("Display actualizado con el estado del LED."),
                     "");
    return;
  }

  if (comando == F("/displaypot")) {
    if (!display.estaInicializado()) {
      _bot.sendMessage(chatId, F("No se pudo inicializar el display."), "");
      return;
    }
    sensores.forzarLecturaPotenciometro();
    display.mostrarEstadoPotenciometro(
        sensores.obtenerVoltajePot(), sensores.obtenerValorAdcPot(),
        sensores.superaUmbralCritico());
    _bot.sendMessage(chatId, F("Display actualizado con el potenciometro."),
                     "");
    return;
  }

  if (comando == F("/displaydht")) {
    if (!display.estaInicializado()) {
      _bot.sendMessage(chatId, F("No se pudo inicializar el display."), "");
      return;
    }
    display.mostrarEstadoDht(sensores.obtenerTemperatura(),
                             sensores.obtenerHumedad(),
                             sensores.datosDhtValidos());
    _bot.sendMessage(chatId, F("Display actualizado con el DHT22."), "");
    return;
  }

  if (comando.startsWith(F("/display"))) {
    display.mostrarComandoNoIdentificado(comando);
  }
  _bot.sendMessage(chatId, F("Comando no reconocido."), "");
}

void GestorTelegram::actualizarAlerta(GestorSensores &sensores) {
  float voltaje = 0;
  if (_hayAlertaPendiente || !sensores.consumirEventoAlertaCritica(voltaje)) {
    return;
  }

  _mensajeAlertaPendiente = F("ALERTA: el voltaje del potenciometro supero "
                              "el umbral critico: ");
  _mensajeAlertaPendiente += String(voltaje, 2);
  _mensajeAlertaPendiente += F(" V.");
  _hayAlertaPendiente = true;
}

bool GestorTelegram::enviarAlertaPendiente() {
  if (!_hayAlertaPendiente || WiFi.status() != WL_CONNECTED || !horaValida()) {
    return false;
  }

  unsigned long ahoraMs = millis();
  if (_ultimoIntentoAlertaMs != 0 &&
      ahoraMs - _ultimoIntentoAlertaMs < INTERVALO_REINTENTO_ALERTA_MS) {
    return false;
  }

  _ultimoIntentoAlertaMs = ahoraMs;
  if (!_bot.sendMessage(_ultimoChatIdActivo, _mensajeAlertaPendiente, "")) {
    return false;
  }

  _mensajeAlertaPendiente = "";
  _hayAlertaPendiente = false;
  _ultimoIntentoAlertaMs = 0;
  return true;
}

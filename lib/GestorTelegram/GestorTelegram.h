#ifndef GESTOR_TELEGRAM_H
#define GESTOR_TELEGRAM_H

#include "Configuracion.h"
#include "GestorActuadores.h"
#include "GestorDisplay.h"
#include "GestorSensores.h"
#include "GestorHTTPClient.h"
#include <Arduino.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>

class GestorTelegram {
public:
  GestorTelegram();

  void iniciar();
  void actualizar(GestorSensores &sensores, GestorActuadores &actuadores,
                  GestorDisplay &display, GestorHTTPClient &httpClient);

private:
  WiFiClientSecure _clienteSeguro;
  UniversalTelegramBot _bot;
  unsigned long _ultimaConsultaMs;
  unsigned long _ultimoIntentoAlertaMs;
  String _ultimoChatIdActivo;
  String _mensajeAlertaPendiente;
  bool _hayAlertaPendiente;
  bool _huboConsultaTelegram;

  bool chatAutorizado(const String &chatId) const;
  void procesarComando(const String &chatId, const String &comando,
                       GestorSensores &sensores, GestorActuadores &actuadores,
                       GestorDisplay &display, GestorHTTPClient &httpClient);
  void procesarCallback(const String &chatId, const String &callbackId,
                        const String &comando, GestorSensores &sensores,
                        GestorActuadores &actuadores, GestorDisplay &display,
                        GestorHTTPClient &httpClient);
  void actualizarAlerta(GestorSensores &sensores);
  bool enviarAlertaPendiente();
};

#endif // GESTOR_TELEGRAM_H

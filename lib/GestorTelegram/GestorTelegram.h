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
  String _ultimoChatIdActivo;
};

#endif // GESTOR_TELEGRAM_H

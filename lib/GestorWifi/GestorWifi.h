#ifndef GESTOR_WIFI_H
#define GESTOR_WIFI_H

#include "Configuracion.h"
#include <WiFi.h>

class GestorWifi {
public:
  GestorWifi();

  void iniciar();
  void actualizar();
  bool estaConectado() const;

private:
  void intentarConexion();

  bool _conectado;
  unsigned long _ultimoIntentoMs;
};

#endif

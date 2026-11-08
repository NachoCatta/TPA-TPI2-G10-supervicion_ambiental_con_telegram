#ifndef GESTOR_SENSORES_H
#define GESTOR_SENSORES_H

#include "Configuracion.h"
#include <Arduino.h>
#include <DHT.h>

class GestorSensores {
public:
  GestorSensores(uint8_t pinDht, uint8_t pinPot);

  void iniciar();
  void actualizar();

  float obtenerTemperatura() const;
  float obtenerHumedad() const;
  bool datosDhtValidos() const;

  uint16_t obtenerValorAdcPot() const;
  float obtenerVoltajePot() const;
  bool superaUmbralCritico() const;
  bool consumirEventoAlertaCritica();

private:
  uint8_t _pinDht;
  uint8_t _pinPot;
  DHT _dht;

  float _temperatura;
  float _humedad;
  bool _dhtValido;

  uint16_t _adcPot;
  float _voltajePot;
  bool _enEstadoCritico;
  bool _alertaPendiente;

  unsigned long _ultimaLecturaDhtMs;
  unsigned long _ultimaLecturaPotMs;
  unsigned long _ultimaAlertaMs;
};

#endif // GESTOR_SENSORES_H

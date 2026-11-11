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

  // --- Métodos del Sensor DHT22 ---
  float obtenerTemperatura() const;
  float obtenerHumedad() const;
  bool datosDhtValidos() const; // Indica si la última lectura fue exitosa.

  // --- Métodos del Potenciómetro (ADC -> Voltaje) ---
  uint16_t obtenerValorAdcPot() const;
  float obtenerVoltajePot() const;
  float obtenerPorcentajePot() const;
  bool superaUmbralCritico() const;

  // Retorna true una única vez cuando se dispara una nueva alerta de umbral crítico
  bool consumirEventoAlertaCritica();
  bool consumirEventoAlertaCritica(float &voltaje);

  // Fuerza una lectura inmediata del potenciómetro (útil al ejecutar /thingup o /displaypot)
  void forzarLecturaPotenciometro();

private:
  void leerDht();
  void leerPotenciometro(unsigned long ahoraMs);

  uint8_t _pinDht;
  uint8_t _pinPot;
  DHT _dht;

  float _temperatura;
  float _humedad;
  bool _dhtValido;

  uint16_t _adcPot;
  float _voltajePot;
  bool _enEstadoCritico;
  bool _alertaGeneradaEnEstadoCritico;
  bool _alertaPendiente;
  float _voltajeAlertaPendiente;

  unsigned long _ultimaLecturaDhtMs;
  unsigned long _ultimaLecturaPotMs;
  unsigned long _ultimaAlertaMs;
};

#endif // GESTOR_SENSORES_H

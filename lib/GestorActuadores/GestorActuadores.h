#ifndef GESTOR_ACTUADORES_H
#define GESTOR_ACTUADORES_H

#include "Configuracion.h"
#include <Arduino.h>

class GestorActuadores {
public:
  GestorActuadores(uint8_t pinReleVerde, uint8_t pinLedAzul);

  void iniciar();
  void actualizar();

  void encenderRele();
  void encenderReleTemporizado(unsigned long segundos);
  void apagarRele();

  void fijarLedAzul(bool encendido);

  bool estaReleEncendido() const;
  bool estaLedAzulEncendido() const;
  unsigned long segundosRestantesRele() const;

private:
  uint8_t _pinReleVerde;
  uint8_t _pinLedAzul;
  bool _estadoRele;
  bool _estadoLedAzul;
  bool _temporizadorActivo;
  unsigned long _inicioTemporizadorMs;
  unsigned long _duracionTemporizadorMs;
};

#endif // GESTOR_ACTUADORES_H

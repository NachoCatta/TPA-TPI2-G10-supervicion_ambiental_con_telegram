#ifndef GESTOR_ACTUADORES_H
#define GESTOR_ACTUADORES_H

#include "Configuracion.h"
#include <Arduino.h>

class GestorActuadores {
public:
  GestorActuadores(uint8_t pinReleVerde, uint8_t pinLedAzul);

  void iniciar();
  void actualizar();

  // --- Control del Relé / Actuador Principal (LED Verde - GPIO 23) ---
  void encenderRele();
  void encenderReleTemporizado(unsigned long segundos);
  void apagarRele();

  // --- Control del LED Indicador de Conexión (LED Azul - GPIO 2) ---
  void fijarLedAzul(bool encendido);

  // --- Consultas de Estado ---
  bool estaReleEncendido() const;
  bool estaLedAzulEncendido() const;
  bool tieneTemporizadorActivo() const;
  unsigned long segundosRestantesRele() const;

  // Retorna true una única vez cuando el temporizador finaliza y apaga el relé
  bool consumirEventoApagadoAutomatico();

private:
  uint8_t _pinReleVerde;
  uint8_t _pinLedAzul;
  bool _estadoRele;
  bool _estadoLedAzul;
  bool _temporizadorActivo;
  bool _eventoApagadoAuto;
  unsigned long _inicioTemporizadorMs;
  unsigned long _duracionTemporizadorMs;
};

#endif // GESTOR_ACTUADORES_H

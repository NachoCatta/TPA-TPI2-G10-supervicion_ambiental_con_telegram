#ifndef GESTOR_ACTUADORES_H
#define GESTOR_ACTUADORES_H

#include "Configuracion.h"
#include <Arduino.h>
#include <esp_timer.h>

class GestorActuadores {
public:
  GestorActuadores(uint8_t pinRele, uint8_t pinLedVerde, uint8_t pinLedAzul);
  ~GestorActuadores();
  GestorActuadores(const GestorActuadores &) = delete;
  GestorActuadores &operator=(const GestorActuadores &) = delete;

  void iniciar();
  void actualizar();

  // Controla el relé y su indicador verde con la misma orden.
  void encenderRele();
  // Rechaza cero o tiempos mayores al máximo sin cambiar la salida actual.
  bool encenderReleTemporizado(unsigned long segundos);
  void apagarRele();

  // --- Control del LED Indicador de Conexión (LED Azul - GPIO 2) ---
  void fijarLedAzul(bool encendido);

  // --- Consultas de Estado ---
  // Estado ordenado, no confirma la posición del contacto físico.
  bool estaReleEncendido() const;
  bool estaLedAzulEncendido() const;
  bool tieneTemporizadorActivo() const;
  unsigned long segundosRestantesRele() const;

  // Retorna true una única vez cuando el temporizador finaliza y apaga el relé
  bool consumirEventoApagadoAutomatico();

private:
  static void alVencerTemporizador(void *contexto);
  void apagarSiVencido();
  void fijarEstadoRele(bool encendido);
  void bloquear() const;
  void desbloquear() const;
  uint8_t _pinRele;
  uint8_t _pinLedVerde;
  uint8_t _pinLedAzul;
  bool _estadoRele;
  bool _estadoLedAzul;
  bool _temporizadorActivo;
  bool _eventoApagadoAuto;
  unsigned long _inicioTemporizadorMs;
  unsigned long _duracionTemporizadorMs;
  esp_timer_handle_t _temporizador;
#ifdef ARDUINO_ARCH_ESP32
  mutable portMUX_TYPE _proteccion = portMUX_INITIALIZER_UNLOCKED;
#endif
};

#endif // GESTOR_ACTUADORES_H

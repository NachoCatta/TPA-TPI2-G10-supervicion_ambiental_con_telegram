#ifndef ESP_TIMER_PRUEBA_H
#define ESP_TIMER_PRUEBA_H

#include <cstdint>

typedef void (*esp_timer_cb_t)(void *);
struct TemporizadorPrueba {
  esp_timer_cb_t callback;
  void *arg;
  bool activo;
};
typedef TemporizadorPrueba *esp_timer_handle_t;
struct esp_timer_create_args_t {
  esp_timer_cb_t callback;
  void *arg;
  int dispatch_method;
  const char *name;
};
constexpr int ESP_OK = 0;
inline esp_timer_handle_t &ultimoTemporizadorPrueba() {
  static esp_timer_handle_t temporizador = nullptr;
  return temporizador;
}
inline bool &fallarInicioTemporizadorPrueba() {
  static bool fallar = false;
  return fallar;
}

inline int esp_timer_create(const esp_timer_create_args_t *args, esp_timer_handle_t *timer) {
  *timer = new TemporizadorPrueba{args->callback, args->arg, false};
  ultimoTemporizadorPrueba() = *timer;
  return ESP_OK;
}
inline int esp_timer_start_once(esp_timer_handle_t timer, uint64_t) {
  if (fallarInicioTemporizadorPrueba()) return -1;
  timer->activo = true;
  return ESP_OK;
}
inline int esp_timer_stop(esp_timer_handle_t timer) {
  timer->activo = false;
  return ESP_OK;
}
inline int esp_timer_delete(esp_timer_handle_t timer) {
  if (ultimoTemporizadorPrueba() == timer) ultimoTemporizadorPrueba() = nullptr;
  delete timer;
  return ESP_OK;
}
// Simula la tarea de esp_timer, sin ejecutar el loop del programa.
inline void vencerTemporizadorPrueba() {
  if (ultimoTemporizadorPrueba() && ultimoTemporizadorPrueba()->activo) {
    ultimoTemporizadorPrueba()->activo = false;
    ultimoTemporizadorPrueba()->callback(ultimoTemporizadorPrueba()->arg);
  }
}
#endif

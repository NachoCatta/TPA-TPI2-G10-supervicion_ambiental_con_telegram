#pragma once
#include_next <time.h>
extern time_t horaPrueba;
inline time_t tiempoSimulado(time_t* destino) {
  if (destino) *destino = horaPrueba;
  return horaPrueba;
}
#define time tiempoSimulado

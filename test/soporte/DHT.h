#ifndef DHT_PRUEBA_H
#define DHT_PRUEBA_H

#include <cstdint>

constexpr uint8_t DHT22 = 22;
extern float temperaturaPrueba;
extern float humedadPrueba;
extern unsigned int lecturasDhtPrueba;

class DHT {
public:
  DHT(uint8_t, uint8_t) {}
  void begin() {}
  float readHumidity() {
    ++lecturasDhtPrueba;
    return humedadPrueba;
  }
  float readTemperature() { return temperaturaPrueba; }
};

#endif

#ifndef ARDUINO_PRUEBA_H
#define ARDUINO_PRUEBA_H

// Sustituye el hardware y la configuración NTP usados en las pruebas nativas.
#include <cmath>
#include <cstdint>

using std::isnan;

#define F(texto) texto
constexpr int INPUT = 0;
constexpr int OUTPUT = 1;
constexpr int LOW = 0;
constexpr int HIGH = 1;
constexpr int ADC_11db = 3;

extern unsigned long tiempoPrueba;
extern uint16_t adcPrueba;
extern uint32_t milivoltiosPrueba;
extern int salidasPrueba[40];

inline unsigned long millis() { return tiempoPrueba; }
inline void pinMode(uint8_t, int) {}
inline void analogReadResolution(uint8_t) {}
inline void analogSetPinAttenuation(uint8_t, int) {}
inline uint16_t analogRead(uint8_t) { return adcPrueba; }
inline uint32_t analogReadMilliVolts(uint8_t) { return milivoltiosPrueba; }
inline void digitalWrite(uint8_t pin, int estado) { salidasPrueba[pin] = estado; }

struct SerialPrueba {
  void println(const char *) {}
  void printf(const char *, ...) {}
};
extern SerialPrueba Serial;

void configTime(long gmtOffset, int daylightOffset, const char *servidor1,
                const char *servidor2 = nullptr, const char *servidor3 = nullptr);

#endif

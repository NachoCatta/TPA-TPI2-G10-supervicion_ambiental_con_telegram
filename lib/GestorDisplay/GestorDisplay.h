#ifndef GESTOR_DISPLAY_H
#define GESTOR_DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <Wire.h>

#include "Configuracion.h"

enum class TipoDisplay { SSD1306, SH1106 };

class GestorDisplay {
public:
  explicit GestorDisplay(TipoDisplay tipo);

  bool iniciar();
  void mostrarBienvenida();
  void mostrarEstadoWifi(bool conectado, const String& ssid, const String& ip);

  // Vistas invocadas por los comandos /display<cmd> desde Telegram
  void mostrarEstadoLed(bool releEncendido, bool ledAzulEncendido,
                        unsigned long segundosRestantes = 0);
  void mostrarEstadoPotenciometro(float voltaje, uint16_t valorAdc, bool superaUmbral = false);
  void mostrarEstadoDht(float temperatura, float humedad, bool valido);
  void mostrarComandoNoIdentificado(const String& comandoRecibido);

private:
  void dibujarEncabezado(const char* titulo);
  void actualizarPantalla();

  TipoDisplay _tipo;
  Adafruit_SSD1306 _ssd1306;
  Adafruit_SH1106G _sh1106;
  Adafruit_GFX &_oled; // Los dos controladores comparten las funciones de dibujo.
  bool _inicializado;
};

#endif  // GESTOR_DISPLAY_H

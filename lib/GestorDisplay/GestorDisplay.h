#ifndef GESTOR_DISPLAY_H
#define GESTOR_DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <Wire.h>

#include "Configuracion.h"

class GestorDisplay {
public:
  GestorDisplay();

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

  Adafruit_SSD1306 _oled;
  bool _inicializado;
};

#endif  // GESTOR_DISPLAY_H

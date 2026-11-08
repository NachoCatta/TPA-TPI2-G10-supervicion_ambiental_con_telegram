#ifndef GESTOR_DISPLAY_H
#define GESTOR_DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Configuracion.h"

class GestorDisplay {
public:
    GestorDisplay();

    bool iniciar();
    void mostrarBienvenida();
    void mostrarEstadoLed(bool releEncendido, bool ledAzulEncendido);
    void mostrarEstadoPotenciometro(float voltaje, uint16_t valorAdc);
    void mostrarEstadoDht(float temperatura, float humedad, bool valido);
    void mostrarComandoNoIdentificado(const String& comandoRecibido);

private:
    Adafruit_SSD1306 _oled;
    bool _inicializado;
};

#endif // GESTOR_DISPLAY_H


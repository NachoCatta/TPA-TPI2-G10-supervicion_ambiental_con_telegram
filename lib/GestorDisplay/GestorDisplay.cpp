#include "GestorDisplay.h"

GestorDisplay::GestorDisplay()
    : _oled(OLED_ANCHO, OLED_ALTO, &Wire, -1),
      _inicializado(false) {}

bool GestorDisplay::iniciar() {
    // Se implementará la inicialización I2C y renderizado OLED en la Etapa 2
    return _inicializado;
}

void GestorDisplay::mostrarBienvenida() {}

void GestorDisplay::mostrarEstadoLed(bool releEncendido, bool ledAzulEncendido) {
    (void)releEncendido;
    (void)ledAzulEncendido;
}

void GestorDisplay::mostrarEstadoPotenciometro(float voltaje, uint16_t valorAdc) {
    (void)voltaje;
    (void)valorAdc;
}

void GestorDisplay::mostrarEstadoDht(float temperatura, float humedad, bool valido) {
    (void)temperatura;
    (void)humedad;
    (void)valido;
}

void GestorDisplay::mostrarComandoNoIdentificado(const String& comandoRecibido) {
    (void)comandoRecibido;
}


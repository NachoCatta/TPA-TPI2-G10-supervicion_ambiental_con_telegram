#include "Configuracion.h"
#include "GestorActuadores.h"
#include "GestorDisplay.h"
#include "GestorSensores.h"
#include "GestorTelegram.h"
#include "GestorThingSpeak.h"
#include "GestorWifi.h"
#include <Arduino.h>

// Instancias globales de los módulos del sistema
GestorSensores sensores(PIN_DHT22, PIN_POTENCIOMETRO);
GestorActuadores actuadores(PIN_RELE, PIN_LED_VERDE, PIN_LED_AZUL);
// Wokwi usa SSD1306; para una pantalla SH1106, cambiar solo el tipo de esta línea.
GestorDisplay display(TipoDisplay::SSD1306);
GestorThingSpeak thingSpeak;
GestorTelegram telegram;
GestorWifi wifi;

void setup() {
  Serial.begin(115200);
  sensores.iniciar();
  actuadores.iniciar();
  display.iniciar();
  wifi.iniciar();
  thingSpeak.iniciar();
  telegram.iniciar();
}

void loop() {
  sensores.actualizar();
  actuadores.actualizar();
  wifi.actualizar();
  actuadores.fijarLedAzul(wifi.estaConectado());
  telegram.actualizar(sensores, actuadores, display, thingSpeak);
  thingSpeak.actualizar(sensores);
}

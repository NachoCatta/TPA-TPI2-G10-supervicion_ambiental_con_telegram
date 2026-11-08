#include "GestorThingSpeak.h"

GestorThingSpeak::GestorThingSpeak()
    : _ultimoEnvioMs(0) {}

void GestorThingSpeak::iniciar() {
    // Se implementará la conexión HTTP a ThingSpeak en la Etapa 3
}

void GestorThingSpeak::actualizar(const GestorSensores& sensores) {
    (void)sensores;
}

bool GestorThingSpeak::enviarVoltajePotenciometro(float voltaje) {
    (void)voltaje;
    return false;
}


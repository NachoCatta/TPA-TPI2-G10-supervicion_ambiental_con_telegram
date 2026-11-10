#include "GestorHTTPClient.h"

GestorHTTPClient::GestorHTTPClient()
    : _ultimoEnvioMs(0) {}

void GestorHTTPClient::iniciar() {
    // Pendiente (tarea 2): envíos a ThingSpeak utilizando HTTPClient.h del ESP32.
}

void GestorHTTPClient::actualizar(const GestorSensores& sensores) {
    (void)sensores;
}

bool GestorHTTPClient::enviarVoltajePotenciometro(float voltaje) {
    (void)voltaje;
    return false;
}

#ifndef GESTOR_THINGSPEAK_H
#define GESTOR_THINGSPEAK_H

#include <Arduino.h>
#include "Configuracion.h"
#include "GestorSensores.h"

class GestorThingSpeak {
public:
    GestorThingSpeak();

    void iniciar();
    void actualizar(const GestorSensores& sensores);
    bool enviarClima(float temperatura, float humedad);
    bool enviarVoltajePotenciometro(float voltaje);
    bool puedeEnviar() const;
    unsigned long tiempoRestanteSegundos() const;

private:
    bool realizarPeticion(const String& queryParams);

    const char* _apiKey;
    unsigned long _channelId;
    unsigned long _ultimoEnvioMs;
    unsigned long _ultimaActualizacionClimaMs;
    bool _huboEnvio;
};

#endif // GESTOR_THINGSPEAK_H

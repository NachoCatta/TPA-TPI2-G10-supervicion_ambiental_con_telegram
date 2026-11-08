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
    bool enviarVoltajePotenciometro(float voltaje);

private:
    unsigned long _ultimoEnvioMs;
};

#endif // GESTOR_THINGSPEAK_H


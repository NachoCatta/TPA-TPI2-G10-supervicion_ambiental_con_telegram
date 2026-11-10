#ifndef GESTOR_HTTP_CLIENT_H
#define GESTOR_HTTP_CLIENT_H

#include <Arduino.h>
#include "Configuracion.h"
#include "GestorSensores.h"

class GestorHTTPClient {
public:
    GestorHTTPClient();

    void iniciar();
    void actualizar(const GestorSensores& sensores);
    bool enviarVoltajePotenciometro(float voltaje);

private:
    unsigned long _ultimoEnvioMs;
};

#endif // GESTOR_HTTP_CLIENT_H


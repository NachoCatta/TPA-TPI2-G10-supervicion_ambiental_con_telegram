#ifndef WIFI_PRUEBA_H
#define WIFI_PRUEBA_H

constexpr int WIFI_STA = 1;
enum wl_status_t { WL_IDLE_STATUS, WL_NO_SSID_AVAIL, WL_CONNECTED,
                   WL_CONNECT_FAILED, WL_DISCONNECTED };

// Simula resultados del SDK. No establece conexiones reales.
struct WiFiPrueba {
  wl_status_t estado = WL_DISCONNECTED;
  int modo = 0;
  bool persistente = true;
  bool reconexionAutomatica = true;
  unsigned int intentos = 0;
  unsigned int desconexiones = 0;
  const char *ssidRecibido = nullptr;
  const char *passwordRecibido = nullptr;

  void persistent(bool valor) { persistente = valor; }
  bool mode(int valor) {
    modo = valor;
    return true;
  }
  bool setAutoReconnect(bool valor) {
    reconexionAutomatica = valor;
    return true;
  }
  wl_status_t status() const { return estado; }
  bool disconnect() {
    ++desconexiones;
    estado = WL_DISCONNECTED;
    return true;
  }
  wl_status_t begin(const char *ssid, const char *password) {
    ++intentos;
    ssidRecibido = ssid;
    passwordRecibido = password;
    return estado;
  }
};

extern WiFiPrueba WiFi;

#endif

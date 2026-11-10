#include "GestorWifi.h"

GestorWifi::GestorWifi() : _conectado(false), _ultimoIntentoMs(0) {}

void GestorWifi::iniciar() {
  _conectado = false;
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  // Esta clase administra los reintentos mediante millis().
  WiFi.setAutoReconnect(false);
  intentarConexion();
}

void GestorWifi::actualizar() {
  bool conectado = WiFi.status() == WL_CONNECTED;

  if (conectado != _conectado) {
    _conectado = conectado;
    if (_conectado) {
      Serial.println(F("[WiFi] Conectado. Solicitando hora por NTP."));
      // Inicia la sincronización en segundo plano; no espera a recibir la hora.
      configTime(0, 0, SERVIDOR_NTP_PRIMARIO, SERVIDOR_NTP_SECUNDARIO);
    } else {
      Serial.println(F("[WiFi] Conexion perdida. Se reintentara la conexion."));
    }
  }

  if (!_conectado && millis() - _ultimoIntentoMs >= INTERVALO_REINTENTO_WIFI_MS) {
    intentarConexion();
  }
}

bool GestorWifi::estaConectado() const { return _conectado; }

void GestorWifi::intentarConexion() {
  _ultimoIntentoMs = millis();
  WiFi.disconnect();
  WiFi.begin(SECRET_WIFI_SSID, SECRET_WIFI_PASSWORD);
  Serial.println(F("[WiFi] Iniciando intento de conexion."));
}

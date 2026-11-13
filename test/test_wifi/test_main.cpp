#include <climits>
#include <cstring>
#include <unity.h>
#include "GestorActuadores.h"
#include "GestorWifi.h"

unsigned long tiempoPrueba;
int salidasPrueba[40];
SerialPrueba Serial;
WiFiPrueba WiFi;

unsigned int solicitudesNtp;
long offsetNtp;
int horarioVeranoNtp;
const char *servidorNtp1;
const char *servidorNtp2;

void configTime(long offset, int horarioVerano, const char *servidor1,
                const char *servidor2, const char *) {
  ++solicitudesNtp;
  offsetNtp = offset;
  horarioVeranoNtp = horarioVerano;
  servidorNtp1 = servidor1;
  servidorNtp2 = servidor2;
}

void setUp() {
  tiempoPrueba = 0;
  WiFi = WiFiPrueba();
  solicitudesNtp = 0;
  offsetNtp = 0;
  horarioVeranoNtp = 0;
  servidorNtp1 = nullptr;
  servidorNtp2 = nullptr;
  for (int &salida : salidasPrueba) salida = LOW;
}

void tearDown() {}

void test_inicio_solicita_conexion_con_credenciales_sin_esperar() {
  GestorWifi wifi;
  wifi.iniciar();
  TEST_ASSERT_EQUAL(WIFI_STA, WiFi.modo);
  TEST_ASSERT_FALSE(WiFi.persistente);
  TEST_ASSERT_FALSE(WiFi.reconexionAutomatica);
  TEST_ASSERT_EQUAL(1, WiFi.intentos);
  TEST_ASSERT_TRUE(std::strcmp(SECRET_WIFI_SSID, WiFi.ssidRecibido) == 0);
  TEST_ASSERT_TRUE(std::strcmp(SECRET_WIFI_PASSWORD, WiFi.passwordRecibido) == 0);
  TEST_ASSERT_FALSE(wifi.estaConectado());
  TEST_ASSERT_EQUAL(0, solicitudesNtp);
}

void test_reintentos_respetan_intervalo_y_no_se_repetiten_en_cada_ciclo() {
  GestorWifi wifi;
  wifi.iniciar();
  tiempoPrueba = INTERVALO_REINTENTO_WIFI_MS - 1;
  wifi.actualizar();
  TEST_ASSERT_EQUAL(1, WiFi.intentos);
  tiempoPrueba = INTERVALO_REINTENTO_WIFI_MS;
  wifi.actualizar();
  TEST_ASSERT_EQUAL(2, WiFi.intentos);
  wifi.actualizar();
  TEST_ASSERT_EQUAL(2, WiFi.intentos);
}

void test_conexion_solicita_ntp_una_vez_y_suspende_reintentos() {
  GestorWifi wifi;
  wifi.iniciar();
  WiFi.estado = WL_CONNECTED;
  wifi.actualizar();
  TEST_ASSERT_TRUE(wifi.estaConectado());
  TEST_ASSERT_EQUAL(1, solicitudesNtp);
  TEST_ASSERT_EQUAL(0, offsetNtp);
  TEST_ASSERT_EQUAL(0, horarioVeranoNtp);
  TEST_ASSERT_EQUAL_STRING(SERVIDOR_NTP_PRIMARIO, servidorNtp1);
  TEST_ASSERT_EQUAL_STRING(SERVIDOR_NTP_SECUNDARIO, servidorNtp2);
  tiempoPrueba = INTERVALO_REINTENTO_WIFI_MS * 10;
  wifi.actualizar();
  TEST_ASSERT_EQUAL(1, solicitudesNtp);
  TEST_ASSERT_EQUAL(1, WiFi.intentos);
}

void test_caida_y_recuperacion_actualizan_led_y_vuelven_a_solicitar_hora() {
  GestorWifi wifi;
  GestorActuadores actuadores(PIN_RELE, PIN_LED_VERDE, PIN_LED_AZUL);
  actuadores.iniciar();
  wifi.iniciar();
  WiFi.estado = WL_CONNECTED;
  wifi.actualizar();
  actuadores.fijarLedAzul(wifi.estaConectado());
  TEST_ASSERT_EQUAL(HIGH, salidasPrueba[PIN_LED_AZUL]);
  TEST_ASSERT_EQUAL(LOW, salidasPrueba[PIN_RELE]);
  TEST_ASSERT_EQUAL(LOW, salidasPrueba[PIN_LED_VERDE]);
  actuadores.encenderRele();
  WiFi.estado = WL_DISCONNECTED;
  tiempoPrueba = 1000;
  wifi.actualizar();
  actuadores.fijarLedAzul(wifi.estaConectado());
  TEST_ASSERT_EQUAL(LOW, salidasPrueba[PIN_LED_AZUL]);
  TEST_ASSERT_EQUAL(HIGH, salidasPrueba[PIN_RELE]);
  TEST_ASSERT_EQUAL(HIGH, salidasPrueba[PIN_LED_VERDE]);
  TEST_ASSERT_FALSE(wifi.estaConectado());
  tiempoPrueba = INTERVALO_REINTENTO_WIFI_MS;
  wifi.actualizar();
  TEST_ASSERT_EQUAL(2, WiFi.intentos);
  WiFi.estado = WL_CONNECTED;
  wifi.actualizar();
  actuadores.fijarLedAzul(wifi.estaConectado());
  TEST_ASSERT_EQUAL(HIGH, salidasPrueba[PIN_LED_AZUL]);
  TEST_ASSERT_EQUAL(HIGH, salidasPrueba[PIN_RELE]);
  TEST_ASSERT_EQUAL(HIGH, salidasPrueba[PIN_LED_VERDE]);
  actuadores.apagarRele();
  TEST_ASSERT_EQUAL(HIGH, salidasPrueba[PIN_LED_AZUL]);
  TEST_ASSERT_EQUAL(2, solicitudesNtp);
}

void test_red_ausente_o_clave_incorrecta_no_generan_reintentos_continuos() {
  GestorWifi wifi;
  wifi.iniciar();
  for (unsigned long segundo = 1; segundo < 10; ++segundo) {
    tiempoPrueba = segundo * 1000UL;
    WiFi.estado = segundo % 2 ? WL_NO_SSID_AVAIL : WL_CONNECT_FAILED;
    wifi.actualizar();
    TEST_ASSERT_FALSE(wifi.estaConectado());
  }
  TEST_ASSERT_EQUAL(1, WiFi.intentos);
  TEST_ASSERT_EQUAL(0, solicitudesNtp);
  tiempoPrueba = INTERVALO_REINTENTO_WIFI_MS;
  wifi.actualizar();
  TEST_ASSERT_EQUAL(2, WiFi.intentos);
}

void test_reintento_al_desbordar_millis() {
  GestorWifi wifi;
  tiempoPrueba = ULONG_MAX - 5000;
  wifi.iniciar();
  tiempoPrueba = 4998;
  wifi.actualizar();
  TEST_ASSERT_EQUAL(1, WiFi.intentos);
  tiempoPrueba = 4999;
  wifi.actualizar();
  TEST_ASSERT_EQUAL(2, WiFi.intentos);
}

void test_temporizador_sigue_funcionando_durante_reintentos_wifi() {
  GestorWifi wifi;
  GestorActuadores actuadores(PIN_RELE, PIN_LED_VERDE, PIN_LED_AZUL);
  actuadores.iniciar();
  actuadores.encenderReleTemporizado(15);
  wifi.iniciar();
  for (tiempoPrueba = 0; tiempoPrueba < 15000; tiempoPrueba += 250) {
    actuadores.actualizar();
    wifi.actualizar();
    actuadores.fijarLedAzul(wifi.estaConectado());
    TEST_ASSERT_TRUE(actuadores.estaReleEncendido());
  }
  vencerTemporizadorPrueba();
  TEST_ASSERT_FALSE(actuadores.estaReleEncendido());
  TEST_ASSERT_EQUAL(LOW, salidasPrueba[PIN_RELE]);
  TEST_ASSERT_EQUAL(LOW, salidasPrueba[PIN_LED_VERDE]);
  TEST_ASSERT_EQUAL(2, WiFi.intentos);
  TEST_ASSERT_EQUAL(LOW, salidasPrueba[PIN_LED_AZUL]);
}

void test_no_hay_rafaga_de_reintentos_si_el_loop_se_demora() {
  GestorWifi wifi;
  wifi.iniciar();
  tiempoPrueba = INTERVALO_REINTENTO_WIFI_MS * 3;
  wifi.actualizar();
  wifi.actualizar();
  TEST_ASSERT_EQUAL(2, WiFi.intentos);
  tiempoPrueba += INTERVALO_REINTENTO_WIFI_MS;
  wifi.actualizar();
  TEST_ASSERT_EQUAL(3, WiFi.intentos);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_inicio_solicita_conexion_con_credenciales_sin_esperar);
  RUN_TEST(test_reintentos_respetan_intervalo_y_no_se_repetiten_en_cada_ciclo);
  RUN_TEST(test_conexion_solicita_ntp_una_vez_y_suspende_reintentos);
  RUN_TEST(test_caida_y_recuperacion_actualizan_led_y_vuelven_a_solicitar_hora);
  RUN_TEST(test_red_ausente_o_clave_incorrecta_no_generan_reintentos_continuos);
  RUN_TEST(test_reintento_al_desbordar_millis);
  RUN_TEST(test_temporizador_sigue_funcionando_durante_reintentos_wifi);
  RUN_TEST(test_no_hay_rafaga_de_reintentos_si_el_loop_se_demora);
  return UNITY_END();
}

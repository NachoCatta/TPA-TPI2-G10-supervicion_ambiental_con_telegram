#include <climits>
#include <cmath>
#include <ctime>
#include <string>
#include <vector>
#include <unity.h>
#include <WiFi.h>
#include "../../lib/GestorSensores/GestorSensores.cpp"
#define time tiempoSimulado
#include "../../lib/GestorThingSpeak/GestorThingSpeak.cpp"
#undef time

constexpr unsigned long MIN_ESCRITURA_PRUEBA = 15500UL;

unsigned long tiempoPrueba;
uint16_t adcPrueba;
uint32_t milivoltiosPrueba;
int salidasPrueba[40];
float temperaturaPrueba, humedadPrueba;
unsigned int lecturasDhtPrueba;
SerialPrueba Serial;
WiFiPrueba WiFi;
time_t horaPrueba;
std::vector<std::string> urlsPrueba;
int codigoPrueba;
const char* payloadPrueba;
bool inicioHttpPrueba;
unsigned int iniciosHttpPrueba;
unsigned long duracionHttpPrueba;

void setUp() {
  tiempoPrueba = 0; adcPrueba = 0; milivoltiosPrueba = 0;
  temperaturaPrueba = 24; humedadPrueba = 60; lecturasDhtPrueba = 0;
  WiFi = WiFiPrueba(); WiFi.estado = WL_CONNECTED;
  horaPrueba = 1700000000; urlsPrueba.clear();
  codigoPrueba = 200; payloadPrueba = "42"; inicioHttpPrueba = true;
  iniciosHttpPrueba = 0;
  duracionHttpPrueba = 0;
}
void tearDown() {}

void assertCampo(unsigned int indice, const char* campo) {
  TEST_ASSERT_TRUE(indice < urlsPrueba.size());
  TEST_ASSERT_NOT_EQUAL(std::string::npos, urlsPrueba[indice].find(campo));
}
void assertSinCampo(unsigned int indice, const char* campo) {
  TEST_ASSERT_TRUE(indice < urlsPrueba.size());
  TEST_ASSERT_EQUAL(std::string::npos, urlsPrueba[indice].find(campo));
}
void assertResultado(GestorThingSpeak& gestor, bool esperado, float voltajeEsperado) {
  bool exito = !esperado; float voltaje = -1;
  TEST_ASSERT_TRUE(gestor.consumirResultadoEnvioVoltaje(exito, voltaje));
  TEST_ASSERT_EQUAL(esperado, exito);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, voltajeEsperado, voltaje);
  TEST_ASSERT_FALSE(gestor.consumirResultadoEnvioVoltaje(exito, voltaje));
}

void test_pendiente_tiene_prioridad_al_vencer_cooldown() {
  GestorSensores sensores(PIN_DHT22, PIN_POTENCIOMETRO); sensores.iniciar();
  GestorThingSpeak gestor; gestor.iniciar(); gestor.actualizar(sensores);
  TEST_ASSERT_EQUAL(1, urlsPrueba.size()); assertCampo(0, "field1=24.00&field2=60.00");
  assertSinCampo(0, "field3=");
  gestor.programarEnvioVoltaje(1.25f);
  temperaturaPrueba = 28; humedadPrueba = 55;
  tiempoPrueba = MIN_ESCRITURA_PRUEBA - 1; gestor.actualizar(sensores);
  TEST_ASSERT_EQUAL(1, urlsPrueba.size());
  sensores.actualizar(); tiempoPrueba++; gestor.actualizar(sensores);
  TEST_ASSERT_EQUAL(2, urlsPrueba.size());
  assertCampo(1, "field1=28.00&field2=55.00&field3=1.250");
  assertResultado(gestor, true, 1.25f);
  gestor.actualizar(sensores); TEST_ASSERT_EQUAL(2, urlsPrueba.size());
  tiempoPrueba += MIN_ESCRITURA_PRUEBA; gestor.actualizar(sensores);
  TEST_ASSERT_EQUAL(3, urlsPrueba.size()); assertCampo(2, "field1=");
  assertSinCampo(2, "field3=");
}

void test_dht_invalido_no_bloquea_y_ultimo_valor_reemplaza_anterior() {
  GestorSensores sensores(PIN_DHT22, PIN_POTENCIOMETRO); sensores.iniciar();
  temperaturaPrueba = NAN; tiempoPrueba = INTERVALO_LECTURA_DHT_MS;
  sensores.actualizar();
  GestorThingSpeak gestor; gestor.iniciar();
  gestor.programarEnvioVoltaje(1.0f); gestor.programarEnvioVoltaje(2.75f);
  gestor.actualizar(sensores);
  TEST_ASSERT_EQUAL(1, urlsPrueba.size()); assertCampo(0, "field3=2.750");
  assertSinCampo(0, "field1="); assertSinCampo(0, "field2=");
  assertResultado(gestor, true, 2.75f);
}

void test_sin_wifi_o_hora_conserva_pendiente() {
  GestorSensores sensores(PIN_DHT22, PIN_POTENCIOMETRO); sensores.iniciar();
  GestorThingSpeak gestor; gestor.iniciar(); gestor.programarEnvioVoltaje(0.5f);
  WiFi.estado = WL_DISCONNECTED; gestor.actualizar(sensores);
  WiFi.estado = WL_CONNECTED; horaPrueba = 0; gestor.actualizar(sensores);
  TEST_ASSERT_EQUAL(0, urlsPrueba.size());
  bool exito; float valor; TEST_ASSERT_FALSE(gestor.consumirResultadoEnvioVoltaje(exito, valor));
  horaPrueba = 1700000000; gestor.actualizar(sensores);
  TEST_ASSERT_EQUAL(1, urlsPrueba.size()); assertCampo(0, "field3=0.500");
  assertResultado(gestor, true, 0.5f);
}

void test_fallo_reintenta_desde_fin_y_notifica_cada_resultado() {
  GestorSensores sensores(PIN_DHT22, PIN_POTENCIOMETRO); sensores.iniciar();
  GestorThingSpeak gestor; gestor.iniciar(); gestor.programarEnvioVoltaje(2.0f);
  codigoPrueba = -1; duracionHttpPrueba = 3000; gestor.actualizar(sensores);
  TEST_ASSERT_EQUAL(1, urlsPrueba.size());
  assertCampo(0, "field1=24.00&field2=60.00&field3=2.000");
  assertResultado(gestor, false, 2.0f);
  codigoPrueba = 200; duracionHttpPrueba = 0;
  tiempoPrueba = 3000 + MIN_ESCRITURA_PRUEBA - 1; gestor.actualizar(sensores);
  TEST_ASSERT_EQUAL(1, urlsPrueba.size());
  tiempoPrueba++; gestor.actualizar(sensores);
  TEST_ASSERT_EQUAL(2, urlsPrueba.size());
  assertCampo(1, "field1=24.00&field2=60.00&field3=2.000");
  assertResultado(gestor, true, 2.0f);
}

void test_solo_http_200_con_id_positivo_es_exito() {
  GestorSensores sensores(PIN_DHT22, PIN_POTENCIOMETRO); sensores.iniciar();
  GestorThingSpeak gestor; gestor.iniciar(); gestor.programarEnvioVoltaje(3.0f);
  payloadPrueba = "0"; gestor.actualizar(sensores); assertResultado(gestor, false, 3.0f);
  tiempoPrueba += MIN_ESCRITURA_PRUEBA; codigoPrueba = 503; payloadPrueba = "10";
  gestor.actualizar(sensores); assertResultado(gestor, false, 3.0f);
  tiempoPrueba += MIN_ESCRITURA_PRUEBA; codigoPrueba = 200; payloadPrueba = " 12 \n";
  gestor.actualizar(sensores); assertResultado(gestor, true, 3.0f);
  TEST_ASSERT_EQUAL(3, urlsPrueba.size());
}

void test_cooldown_al_desbordar_millis() {
  tiempoPrueba = ULONG_MAX - 1000;
  GestorSensores sensores(PIN_DHT22, PIN_POTENCIOMETRO); sensores.iniciar();
  GestorThingSpeak gestor; gestor.iniciar(); gestor.actualizar(sensores);
  gestor.programarEnvioVoltaje(1.0f);
  tiempoPrueba = MIN_ESCRITURA_PRUEBA - 1002; gestor.actualizar(sensores);
  TEST_ASSERT_EQUAL(1, urlsPrueba.size());
  tiempoPrueba++; gestor.actualizar(sensores);
  TEST_ASSERT_EQUAL(2, urlsPrueba.size()); assertCampo(1, "field3=1.000");
}

void test_inicio_http_fallido_tambien_espera_antes_de_reintentar() {
  GestorSensores sensores(PIN_DHT22, PIN_POTENCIOMETRO); sensores.iniciar();
  GestorThingSpeak gestor; gestor.iniciar(); gestor.programarEnvioVoltaje(1.5f);
  inicioHttpPrueba = false; duracionHttpPrueba = 1000; gestor.actualizar(sensores);
  TEST_ASSERT_EQUAL(1, iniciosHttpPrueba); assertResultado(gestor, false, 1.5f);
  inicioHttpPrueba = true; duracionHttpPrueba = 0;
  tiempoPrueba = 1000 + MIN_ESCRITURA_PRUEBA - 1; gestor.actualizar(sensores);
  TEST_ASSERT_EQUAL(1, iniciosHttpPrueba); TEST_ASSERT_EQUAL(0, urlsPrueba.size());
  tiempoPrueba++; gestor.actualizar(sensores);
  TEST_ASSERT_EQUAL(2, iniciosHttpPrueba); assertCampo(0, "field3=1.500");
  assertResultado(gestor, true, 1.5f);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_pendiente_tiene_prioridad_al_vencer_cooldown);
  RUN_TEST(test_dht_invalido_no_bloquea_y_ultimo_valor_reemplaza_anterior);
  RUN_TEST(test_sin_wifi_o_hora_conserva_pendiente);
  RUN_TEST(test_fallo_reintenta_desde_fin_y_notifica_cada_resultado);
  RUN_TEST(test_solo_http_200_con_id_positivo_es_exito);
  RUN_TEST(test_cooldown_al_desbordar_millis);
  RUN_TEST(test_inicio_http_fallido_tambien_espera_antes_de_reintentar);
  return UNITY_END();
}

#include <climits>
#include <unity.h>
#include "GestorActuadores.h"
#include "GestorSensores.h"

unsigned long tiempoPrueba;
uint16_t adcPrueba;
uint32_t milivoltiosPrueba;
int salidasPrueba[40];
float temperaturaPrueba;
float humedadPrueba;
unsigned int lecturasDhtPrueba;
SerialPrueba Serial;

void setUp() {
  tiempoPrueba = 0;
  adcPrueba = 0;
  milivoltiosPrueba = 0;
  temperaturaPrueba = 24.0f;
  humedadPrueba = 60.0f;
  lecturasDhtPrueba = 0;
  fallarInicioTemporizadorPrueba() = false;
  for (int &salida : salidasPrueba) salida = LOW;
}

void tearDown() {}

void muestrear(GestorSensores &sensores, unsigned long tiempo, uint32_t milivoltios) {
  tiempoPrueba = tiempo;
  milivoltiosPrueba = milivoltios;
  sensores.forzarLecturaPotenciometro();
}

void test_apagado_automatico_y_evento_unico() {
  GestorActuadores actuadores(PIN_LED_VERDE, PIN_LED_AZUL);
  actuadores.iniciar();
  TEST_ASSERT_TRUE(actuadores.encenderReleTemporizado(5));
  TEST_ASSERT_EQUAL(HIGH, salidasPrueba[PIN_LED_VERDE]);
  tiempoPrueba = 4999;
  actuadores.actualizar();
  TEST_ASSERT_TRUE(actuadores.estaReleEncendido());
  TEST_ASSERT_EQUAL(1, actuadores.segundosRestantesRele());
  tiempoPrueba = 5000;
  vencerTemporizadorPrueba(); // La red podría seguir bloqueando el loop.
  TEST_ASSERT_FALSE(actuadores.estaReleEncendido());
  TEST_ASSERT_EQUAL(LOW, salidasPrueba[PIN_LED_VERDE]);
  TEST_ASSERT_TRUE(actuadores.consumirEventoApagadoAutomatico());
  TEST_ASSERT_FALSE(actuadores.consumirEventoApagadoAutomatico());
}

void test_tiempo_invalido_no_enciende_ni_reemplaza_temporizador() {
  GestorActuadores actuadores(PIN_LED_VERDE, PIN_LED_AZUL);
  actuadores.iniciar();
  TEST_ASSERT_FALSE(actuadores.encenderReleTemporizado(0));
  TEST_ASSERT_FALSE(actuadores.encenderReleTemporizado(ULONG_MAX));
  TEST_ASSERT_FALSE(actuadores.estaReleEncendido());
  TEST_ASSERT_TRUE(actuadores.encenderReleTemporizado(5));
  tiempoPrueba = 1000;
  TEST_ASSERT_FALSE(actuadores.encenderReleTemporizado(MAX_TIEMPO_RELE_SEGUNDOS + 1));
  TEST_ASSERT_EQUAL(4, actuadores.segundosRestantesRele());
}

void test_limite_maximo_del_temporizador() {
  GestorActuadores actuadores(PIN_LED_VERDE, PIN_LED_AZUL);
  actuadores.iniciar();
  TEST_ASSERT_TRUE(actuadores.encenderReleTemporizado(MAX_TIEMPO_RELE_SEGUNDOS));
  TEST_ASSERT_EQUAL(MAX_TIEMPO_RELE_SEGUNDOS, actuadores.segundosRestantesRele());
}

void test_error_al_programar_temporizador_apaga_salida() {
  GestorActuadores actuadores(PIN_LED_VERDE, PIN_LED_AZUL);
  actuadores.iniciar();
  fallarInicioTemporizadorPrueba() = true;
  TEST_ASSERT_FALSE(actuadores.encenderReleTemporizado(5));
  TEST_ASSERT_FALSE(actuadores.estaReleEncendido());
  TEST_ASSERT_EQUAL(LOW, salidasPrueba[PIN_LED_VERDE]);
  TEST_ASSERT_FALSE(actuadores.consumirEventoApagadoAutomatico());
}

void test_apagado_manual_cancela_temporizador() {
  GestorActuadores actuadores(PIN_LED_VERDE, PIN_LED_AZUL);
  actuadores.iniciar();
  actuadores.encenderReleTemporizado(5);
  actuadores.apagarRele();
  tiempoPrueba = 6000;
  ultimoTemporizadorPrueba()->callback(ultimoTemporizadorPrueba()->arg);
  TEST_ASSERT_FALSE(actuadores.tieneTemporizadorActivo());
  TEST_ASSERT_FALSE(actuadores.consumirEventoApagadoAutomatico());
}

void test_nueva_orden_reinicia_temporizador() {
  GestorActuadores actuadores(PIN_LED_VERDE, PIN_LED_AZUL);
  actuadores.iniciar();
  actuadores.encenderReleTemporizado(5);
  tiempoPrueba = 3000;
  actuadores.encenderReleTemporizado(10);
  tiempoPrueba = 5000;
  // Callback ya despachado para el temporizador anterior.
  ultimoTemporizadorPrueba()->callback(ultimoTemporizadorPrueba()->arg);
  TEST_ASSERT_TRUE(actuadores.estaReleEncendido());
  TEST_ASSERT_EQUAL(8, actuadores.segundosRestantesRele());
  tiempoPrueba = 13000;
  vencerTemporizadorPrueba();
  TEST_ASSERT_FALSE(actuadores.estaReleEncendido());
}

void test_temporizador_al_desbordar_millis() {
  GestorActuadores actuadores(PIN_LED_VERDE, PIN_LED_AZUL);
  actuadores.iniciar();
  tiempoPrueba = ULONG_MAX - 500;
  actuadores.encenderReleTemporizado(2);
  tiempoPrueba = 1498;
  actuadores.actualizar();
  TEST_ASSERT_TRUE(actuadores.estaReleEncendido());
  TEST_ASSERT_EQUAL(1, actuadores.segundosRestantesRele());
  tiempoPrueba = 1499;
  vencerTemporizadorPrueba();
  TEST_ASSERT_FALSE(actuadores.estaReleEncendido());
}

void test_dht_respeta_intervalo_desde_inicio() {
  GestorSensores sensores(PIN_DHT22, PIN_POTENCIOMETRO);
  sensores.iniciar();
  sensores.actualizar();
  tiempoPrueba = INTERVALO_LECTURA_DHT_MS - 1;
  sensores.actualizar();
  TEST_ASSERT_EQUAL(1, lecturasDhtPrueba);
  tiempoPrueba = INTERVALO_LECTURA_DHT_MS;
  sensores.actualizar();
  TEST_ASSERT_EQUAL(2, lecturasDhtPrueba);
}

void test_dht_informa_error_conserva_dato_y_se_recupera() {
  GestorSensores sensores(PIN_DHT22, PIN_POTENCIOMETRO);
  sensores.iniciar();
  temperaturaPrueba = NAN;
  tiempoPrueba = INTERVALO_LECTURA_DHT_MS;
  sensores.actualizar();
  TEST_ASSERT_FALSE(sensores.datosDhtValidos());
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 24.0f, sensores.obtenerTemperatura());
  temperaturaPrueba = 26.0f;
  tiempoPrueba += INTERVALO_LECTURA_DHT_MS;
  sensores.actualizar();
  TEST_ASSERT_TRUE(sensores.datosDhtValidos());
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 26.0f, sensores.obtenerTemperatura());
}

void test_voltaje_usa_milivoltios_y_no_escala_adc() {
  GestorSensores sensores(PIN_DHT22, PIN_POTENCIOMETRO);
  adcPrueba = 2000;
  milivoltiosPrueba = 2600;
  sensores.iniciar();
  TEST_ASSERT_EQUAL(2000, sensores.obtenerValorAdcPot());
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 2.6f, sensores.obtenerVoltajePot());
  TEST_ASSERT_TRUE(sensores.superaUmbralCritico());
}

void test_alerta_conserva_voltaje_que_la_genero() {
  GestorSensores sensores(PIN_DHT22, PIN_POTENCIOMETRO);
  sensores.iniciar();
  muestrear(sensores, 1000, 3000);
  muestrear(sensores, 2000, 1000);
  float voltaje = 0;
  TEST_ASSERT_TRUE(sensores.consumirEventoAlertaCritica(voltaje));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 3.0f, voltaje);
  TEST_ASSERT_FALSE(sensores.consumirEventoAlertaCritica(voltaje));
}

void test_alerta_al_arrancar_en_cero_respeta_cooldown() {
  GestorSensores sensores(PIN_DHT22, PIN_POTENCIOMETRO);
  milivoltiosPrueba = 2600;
  sensores.iniciar();
  TEST_ASSERT_TRUE(sensores.consumirEventoAlertaCritica());
  muestrear(sensores, 1000, 2300);
  muestrear(sensores, 2000, 2600);
  TEST_ASSERT_FALSE(sensores.consumirEventoAlertaCritica());
  muestrear(sensores, COOLDOWN_ALERTA_VOLTAJE_MS, 2600);
  TEST_ASSERT_TRUE(sensores.consumirEventoAlertaCritica());
}

void test_segundo_episodio_sostenido_alerta_al_vencer_espera() {
  GestorSensores sensores(PIN_DHT22, PIN_POTENCIOMETRO);
  sensores.iniciar();
  muestrear(sensores, 5000, 2600);
  TEST_ASSERT_TRUE(sensores.consumirEventoAlertaCritica());
  muestrear(sensores, 10000, 2300);
  muestrear(sensores, 15000, 2600);
  TEST_ASSERT_FALSE(sensores.consumirEventoAlertaCritica());
  muestrear(sensores, 34999, 2600);
  TEST_ASSERT_FALSE(sensores.consumirEventoAlertaCritica());
  muestrear(sensores, 35000, 2600);
  TEST_ASSERT_TRUE(sensores.consumirEventoAlertaCritica());
  muestrear(sensores, 70000, 2600);
  TEST_ASSERT_FALSE(sensores.consumirEventoAlertaCritica());
}

void test_histeresis_evitar_rearme_en_la_banda() {
  GestorSensores sensores(PIN_DHT22, PIN_POTENCIOMETRO);
  sensores.iniciar();
  muestrear(sensores, 5000, 2600);
  TEST_ASSERT_TRUE(sensores.consumirEventoAlertaCritica());
  muestrear(sensores, 40000, 2400);
  TEST_ASSERT_TRUE(sensores.superaUmbralCritico());
  muestrear(sensores, 41000, 2600);
  TEST_ASSERT_FALSE(sensores.consumirEventoAlertaCritica());
  muestrear(sensores, 42000, 2300);
  TEST_ASSERT_FALSE(sensores.superaUmbralCritico());
  muestrear(sensores, 43000, 2600);
  TEST_ASSERT_TRUE(sensores.consumirEventoAlertaCritica());
}

void test_episodio_terminado_durante_espera_no_alerta_despues() {
  GestorSensores sensores(PIN_DHT22, PIN_POTENCIOMETRO);
  sensores.iniciar();
  muestrear(sensores, 5000, 2600);
  sensores.consumirEventoAlertaCritica();
  muestrear(sensores, 10000, 2300);
  muestrear(sensores, 15000, 2600);
  muestrear(sensores, 20000, 2300);
  muestrear(sensores, 35000, 2300);
  TEST_ASSERT_FALSE(sensores.consumirEventoAlertaCritica());
}

void test_cooldown_al_desbordar_millis() {
  GestorSensores sensores(PIN_DHT22, PIN_POTENCIOMETRO);
  tiempoPrueba = ULONG_MAX - 5000;
  milivoltiosPrueba = 2600;
  sensores.iniciar();
  TEST_ASSERT_TRUE(sensores.consumirEventoAlertaCritica());
  muestrear(sensores, ULONG_MAX - 4000, 2300);
  muestrear(sensores, ULONG_MAX - 3000, 2600);
  muestrear(sensores, 24998, 2600);
  TEST_ASSERT_FALSE(sensores.consumirEventoAlertaCritica());
  muestrear(sensores, 24999, 2600);
  TEST_ASSERT_TRUE(sensores.consumirEventoAlertaCritica());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_apagado_automatico_y_evento_unico);
  RUN_TEST(test_tiempo_invalido_no_enciende_ni_reemplaza_temporizador);
  RUN_TEST(test_limite_maximo_del_temporizador);
  RUN_TEST(test_error_al_programar_temporizador_apaga_salida);
  RUN_TEST(test_apagado_manual_cancela_temporizador);
  RUN_TEST(test_nueva_orden_reinicia_temporizador);
  RUN_TEST(test_temporizador_al_desbordar_millis);
  RUN_TEST(test_dht_respeta_intervalo_desde_inicio);
  RUN_TEST(test_dht_informa_error_conserva_dato_y_se_recupera);
  RUN_TEST(test_voltaje_usa_milivoltios_y_no_escala_adc);
  RUN_TEST(test_alerta_conserva_voltaje_que_la_genero);
  RUN_TEST(test_alerta_al_arrancar_en_cero_respeta_cooldown);
  RUN_TEST(test_segundo_episodio_sostenido_alerta_al_vencer_espera);
  RUN_TEST(test_histeresis_evitar_rearme_en_la_banda);
  RUN_TEST(test_episodio_terminado_durante_espera_no_alerta_despues);
  RUN_TEST(test_cooldown_al_desbordar_millis);
  return UNITY_END();
}

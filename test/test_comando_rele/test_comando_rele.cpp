#include "ComandoRele.h"
#include <unity.h>

void setUp() {}
void tearDown() {}

void test_comandos_validos() {
  unsigned long segundos = 42;
  TEST_ASSERT_TRUE(parsearComandoRele("/releon 1", 3600, segundos));
  TEST_ASSERT_EQUAL_UINT32(1, segundos);
  TEST_ASSERT_TRUE(parsearComandoRele("/releon 3600", 3600, segundos));
  TEST_ASSERT_EQUAL_UINT32(3600, segundos);
  TEST_ASSERT_TRUE(parsearComandoRele("/releon   10 ", 3600, segundos));
  TEST_ASSERT_EQUAL_UINT32(10, segundos);
}

void test_comandos_invalidos_no_cambian_salida() {
  const char *invalidos[] = {"/releon", "/releon10", "/releonX10",
    "/releon 0", "/releon 3601", "/releon -1", "/releon 1.5",
    "/releon 12 mas", "/releon 999999999999999999999", "/releon "};
  for (const char *texto : invalidos) {
    unsigned long segundos = 42;
    TEST_ASSERT_FALSE(parsearComandoRele(texto, 3600, segundos));
    TEST_ASSERT_EQUAL_UINT32(42, segundos);
  }
}

void test_limite_configurable() {
  unsigned long segundos = 42;
  TEST_ASSERT_TRUE(parsearComandoRele("/releon 8", 8, segundos));
  TEST_ASSERT_EQUAL_UINT32(8, segundos);
  TEST_ASSERT_FALSE(parsearComandoRele("/releon 9", 8, segundos));
  TEST_ASSERT_FALSE(parsearComandoRele("/releon 1", 0, segundos));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_comandos_validos);
  RUN_TEST(test_comandos_invalidos_no_cambian_salida);
  RUN_TEST(test_limite_configurable);
  return UNITY_END();
}

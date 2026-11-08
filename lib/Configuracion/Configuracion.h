#ifndef CONFIGURACION_H
#define CONFIGURACION_H

#include <Arduino.h>

// Si existe secrets.h se carga; de lo contrario se usa secrets.example.h
#if __has_include("secrets.h")
#include "secrets.h"
#else
#include "secrets.example.h"
#endif

// 1. DEFINICIÓN DE PINES DE HARDWARE
constexpr uint8_t PIN_DHT22 = 33; // Sensor de Temperatura y Humedad DHT22
constexpr uint8_t PIN_POTENCIOMETRO = 32; // Entrada Analógica ADC1_CH4
constexpr uint8_t PIN_LED_VERDE = 23;     // Actuador principal / Relé
constexpr uint8_t PIN_LED_AZUL = 2;  // LED indicador de estado / secundario
constexpr uint8_t PIN_OLED_SDA = 21; // I2C SDA estándar
constexpr uint8_t PIN_OLED_SCL = 22; // I2C SCL estándar

// 2. PARÁMETROS DE PERIFÉRICOS Y UMBRALES
constexpr uint8_t OLED_ANCHO = 128;
constexpr uint8_t OLED_ALTO = 64;
constexpr uint8_t OLED_DIRECCION_I2C = 0x3C;

constexpr float ADC_RESOLUCION = 4095.0f;
constexpr float VOLTAJE_REFERENCIA = 3.30f;
constexpr float UMBRAL_CRITICO_VOLTAJE =
    2.50f; // Umbral crítico para alerta automática
constexpr float HISTERESIS_VOLTAJE = 0.15f; // Margen para rearmar la alerta

// 3. INTERVALOS DE TIEMPO NO BLOQUEANTES (millis)
constexpr unsigned long INTERVALO_LECTURA_DHT_MS = 2500UL; // 2.5 segundos
constexpr unsigned long INTERVALO_LECTURA_POT_MS = 250UL;  // 250 ms
constexpr unsigned long INTERVALO_THINGSPEAK_MS =
    20000UL; // 20 segundos (mínimo 15s en ThingSpeak Free)
constexpr unsigned long INTERVALO_TELEGRAM_POLL_MS = 1000UL; // 1 segundo
constexpr unsigned long COOLDOWN_ALERTA_VOLTAJE_MS =
    30000UL; // 30 segundos mínimo entre alertas

// 4. CAMPOS DE THINGSPEAK
constexpr uint8_t FIELD_TEMPERATURA = 1;
constexpr uint8_t FIELD_HUMEDAD = 2;
constexpr uint8_t FIELD_VOLTAJE_POT = 3;

#endif // CONFIGURACION_H

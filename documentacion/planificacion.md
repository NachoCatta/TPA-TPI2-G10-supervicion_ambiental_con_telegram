# Plan de Trabajo - Trabajo Práctico Integrador N°2 (TA4k01/26)
**Proyecto:** Supervisión Ambiental y Actuación Remota con ESP32, ThingSpeak y Bot de Telegram

---

## 1. Análisis General del Proyecto y Requerimientos

El objetivo del proyecto es desarrollar un sistema embebido basado en el microcontrolador **ESP32** (simulado en **Wokwi con PlatformIO** y desplegable en la **placa de desarrollo física Evaluation Board V1.0 de la UTN**) capaz de:
1. **Monitorear variables ambientales y analógicas:** Temperatura y humedad relativa mediante un sensor **DHT22**, y nivel de tensión mediante un **potenciómetro**.
2. **Visualizar estados localmente:** Mostrar información de los componentes y diagnósticos en una pantalla **OLED I²C (SSD1306)** e indicadores luminosos (**LED verde y LED azul**).
3. **Reportar telemetría a la nube (IoT):** Enviar periódicamente temperatura y humedad, así como lecturas de tensión bajo demanda, hacia un canal de **ThingSpeak** mediante solicitudes HTTP.
4. **Interactuar remotamente vía Telegram:** Integrar un **Bot de Telegram** con menú interactivo (*inline keyboard*), consulta de variables, control temporizado no bloqueante de salidas digitales, visualización en pantalla OLED y envío de alertas automáticas por umbral crítico.

### Requerimientos Arquitectónicos Clave
* **Modularidad y Librerías Propias:** El archivo principal (`main.cpp` / sketch `.ino`) debe mantener la función `loop()` lo más limpia posible, delegando toda la lógica de negocio a módulos/librerías propias bien estructuradas.
* **Ejecución 100% No Bloqueante:** Queda prohibido el uso de `delay()` en el flujo principal. Todas las tareas periódicas (lectura de sensores, envío a ThingSpeak, consulta de mensajes de Telegram y temporización del relé/LEDs) se gestionarán con `millis()`.
* **Portabilidad a Placa UTN (.ino):** El código debe poder ejecutarse tanto en el entorno PlatformIO + Wokwi como exportarse en formato de sketch `.ino` compatible con la placa física de la cátedra.

---

## 2. Asignación de Hardware y Pines (Pinout)

| Componente | Pin ESP32 | Tipo / Protocolo | Función en el Sistema |
| :--- | :--- | :--- | :--- |
| **Sensor DHT22** | `GPIO 33` | Digital (OneWire propio) | Medición de Temperatura (°C) y Humedad Relativa (%) |
| **Potenciómetro** | `GPIO 32` | Entrada Analógica (`ADC1_CH4`) | Medición de nivel analógico (0–4095) convertido a Voltaje (0.00V – 3.30V) |
| **LED Verde** | `GPIO 23` | Salida Digital | Actuador principal / Relé o indicador de estado |
| **LED Azul** | `GPIO 2` | Salida Digital | Indicador de estado / conectividad o segundo actuador |
| **Display OLED (SDA)** | `GPIO 21` | I²C Datos (`SDA`) | Pantalla OLED SSD1306 (128x64, dir. `0x3C`) |
| **Display OLED (SCL)** | `GPIO 22` | I²C Reloj (`SCL`) | Pantalla OLED SSD1306 (128x64, dir. `0x3C`) |

---

## 3. Plan de Trabajo Dividido por Etapas

### Etapa 1: Configuración del Entorno, Dependencias y Estructura Modular
1. **Corrección y ajuste de archivos de configuración:**
   * Corregir la ruta del firmware en `wokwi.toml` (`firmware = '.pio/build/esp32doit-devkit-v1/firmware.bin'`).
   * Configurar `platformio.ini` con la velocidad del monitor serie (`monitor_speed = 115200`) y las librerías externas necesarias (`lib_deps`):
     * `adafruit/DHT sensor library` y `adafruit/Adafruit Unified Sensor` (o `beegee-tokyo/DHT sensor library for ESPx`) para el DHT22.
     * `adafruit/Adafruit SSD1306` y `adafruit/Adafruit GFX Library` para el display OLED I²C.
     * `witnessmenow/UniversalTelegramBot` y `bblanchon/ArduinoJson` para la integración con Telegram.
     * `mathworks/ThingSpeak` (o uso nativo de `HTTPClient.h` para peticiones HTTP GET/POST a ThingSpeak).
2. **Diseño de la Arquitectura de Librerías Propias (en carpeta `lib/`):**
   * `Configuracion`: Archivo de cabeceras con constantes de pines, intervalos de tiempo, umbrales y credenciales (Wi-Fi, Token Telegram, API Key ThingSpeak).
   * `GestorSensores`: Encapsula la inicialización y lectura no bloqueante del DHT22 y del potenciómetro (conversión ADC a Voltaje y detección de umbral crítico).
   * `GestorActuadores`: Administra los LEDs / Relé y la lógica de encendido temporizado no bloqueante con `millis()`.
   * `GestorDisplay`: Controla la pantalla OLED SSD1306 (pantallas de inicio, actualización de estado de LED, potenciómetro, DHT22 y avisos de comando no reconocido).
   * `GestorThingSpeak`: Maneja la conexión HTTP con ThingSpeak para el envío periódico de clima (Temp/Hum) y el envío del voltaje del potenciómetro (`/thingup`).
   * `GestorTelegram`: Administra la cola de mensajes, el teclado inline (`/start`), el parseo de comandos (`/clima`, `/rele...`, `/thingup`, `/display...`) y el disparo de alertas automáticas.

---

### Etapa 2: Desarrollo de Capa de Hardware (Sensores, Actuadores y OLED)
1. **Módulo de Sensores (`GestorSensores`):**
   * Implementar lectura periódica del DHT22 (cada ~2-5 segundos) validando errores de lectura (`isnan`).
   * Implementar lectura del ADC en `GPIO 32` y conversión a tensión real ($V = \frac{ADC}{4095.0} \times 3.3\text{ V}$).
   * Implementar detector de umbral crítico con **histéresis / enfriamiento (cooldown)** para disparar la alerta automática a Telegram una única vez por evento de exceso de voltaje (evitando saturar el chat con mensajes repetidos cada ciclo de `loop()`).
2. **Módulo de Actuadores (`GestorActuadores`):**
   * Configurar `GPIO 23` (LED verde) y `GPIO 2` (LED azul) como salidas digitales.
   * Implementar máquina de estados no bloqueante para el comando `/rele<on/off> <time>`:
     * Al activar con tiempo $t$ segundos, guardar `tiempoInicio = millis()` y `duracionMs = t * 1000`.
     * En cada iteración de `actualizar()`, verificar si `millis() - tiempoInicio >= duracionMs` para apagar automáticamente la salida sin detener el microcontrolador.
3. **Módulo de Pantalla OLED (`GestorDisplay`):**
   * Inicializar bus I²C (`GPIO 21` / `GPIO 22`) y pantalla SSD1306 en dirección `0x3C`.
   * Crear vistas claras y formateadas para responder a `/display<cmd>`:
     * Estado de LEDs/Relé (`"Led encendido"` / `"Led apagado"`).
     * Estado del Potenciómetro (valor en Voltios y porcentaje/ADC).
     * Estado del sensor DHT22 (Temperatura en °C y Humedad en %).
     * Mensaje de error ante comando inválido (`"Comando no identificado"`).

---

### Etapa 3: Conectividad Wi-Fi e Integración con ThingSpeak
1. **Conexión Wi-Fi (`WiFi.h`):**
   * Configurar conexión automática a la red Wi-Fi (soportando `Wokwi-GUEST` para el simulador y credenciales reales para la placa física UTN).
   * Implementar verificación de reconexión no bloqueante en caso de caída de red.
2. **Cliente HTTP para ThingSpeak (`GestorThingSpeak`):**
   * Configurar temporizador no bloqueante (ej. cada 20 o 30 segundos, respetando el límite mínimo de 15 s de ThingSpeak gratuito) para enviar Temperatura (`field1`) y Humedad (`field2`) mediante HTTP GET/POST usando la **Write API Key**.
   * Implementar función dedicada para `/thingup` que envíe el voltaje actual del potenciómetro (`field3`) hacia ThingSpeak y retorne el resultado de la operación.

---

### Etapa 4: Integración del Bot de Telegram y Comandos Remotos
1. **Configuración de Cliente Seguro (`WiFiClientSecure`):**
   * Configurar el cliente TLS con el certificado raíz de Telegram (`TELEGRAM_CERTIFICATE_ROOT` o `setInsecure()` según compatibilidad de memoria) y el **Bot Token**.
2. **Implementación de Comandos y Teclado Inline (`GestorTelegram`):**
   * **`/start`**: Envía mensaje de bienvenida junto con un teclado inline (`sendMessageWithInlineKeyboard`) con botones de acceso rápido (por ejemplo: *"🌡️ Ver Clima"*, *"⚡ Encender Relé"*, *"🔌 Apagar Relé"*, *"📈 Enviar Pote (/thingup)"*, *"🖥️ Display LED"*, *"🖥️ Display Pote"*, *"🖥️ Display DHT"*).
   * **`/clima`**: Responde con los últimos valores registrados de temperatura y humedad del DHT22.
   * **`/rele<on/off> <time>`**: Parsea el comando y el parámetro de tiempo (ej. `/releon 10` o `/releoff`), activa el actuador de forma no bloqueante y confirma la acción por Telegram.
   * **`/thingup`**: Envía el voltaje del potenciómetro a ThingSpeak e informa el valor en el chat.
   * **Alerta Automática de Umbral Crítico**: En el ciclo principal, si el voltaje del potenciómetro supera el umbral crítico definido, el ESP32 envía proactivamente un mensaje de `"⚠️ ALERTA DE SEGURIDAD: Voltaje crítico superado (X.XX V)"` al `chat_id` registrado.
   * **`/display<cmd>`**: Analiza el sufijo del comando (`/displayled`, `/displaypot`, `/displaydht`, etc.), actualiza la pantalla OLED con la información solicitada (o muestra `"Comando no identificado"` en el OLED si el sufijo no es válido) y confirma al usuario por Telegram.

---

### Etapa 5: Integración en `main.cpp`, Exportación a `.ino` y Pruebas
1. **Estructuración limpia de `main.cpp`:**
   * `setup()` únicamente con llamadas de inicialización (`sistema.iniciar()`).
   * `loop()` minimalista con llamadas de actualización no bloqueantes:
     ```cpp
     void loop() {
         sensores.actualizar();
         actuadores.actualizar();
         thingSpeak.actualizar(sensores);
         telegram.actualizar(sensores, actuadores, display, thingSpeak);
     }
     ```
2. **Pruebas integrales en Wokwi:**
   * Verificar lectura de DHT22, variación del potenciómetro, disparo de alerta automática, respuesta de botones inline en Telegram, recepción de datos en ThingSpeak y visualización en el OLED.
3. **Preparación de entregable para la cátedra (`.ino` + Wokwi):**
   * Generar/verificar el archivo `.ino` compatible con el IDE de Arduino / placa UTN para cumplir con las condiciones de entrega de la tarea `4k01TP02/26`.

---

## 4. Definiciones y Credenciales Confirmadas

### A. Credenciales y Servicios Externos
1. **Telegram Bot Token y Chat ID:** Configurados en `include/secrets.h`. Se utiliza el `chat_id` personal como destinatario por defecto para las alertas automáticas de seguridad (además, cuando cualquier usuario interactúe con el bot, el sistema responderá a ese chat).
2. **ThingSpeak Channel ID y Write API Key:** Configurados en `include/secrets.h` con el siguiente orden de campos confirmado:
   * `Field 1`: Temperatura (°C)
   * `Field 2`: Humedad Relativa (%)
   * `Field 3`: Voltaje del Potenciómetro (V)
3. **Credenciales Wi-Fi:** Configuradas inicialmente con `Wokwi-GUEST` para simulación. Para la prueba física en la placa UTN solo se deberá actualizar `SECRET_WIFI_SSID` y `SECRET_WIFI_PASSWORD` en `include/secrets.h` (con el Wi-Fi del laboratorio o datos móviles/hotspot del celular).

### B. Definiciones Funcionales Confirmadas
1. **Rol de los LEDs:**
   * **LED Verde (`GPIO 23`):** Actuador principal / Relé controlado mediante Telegram.
   * **LED Azul (`GPIO 2`):** Indicador del estado de la conexión Wi-Fi (encendido cuando está conectado).
2. **Umbral Crítico del Potenciómetro:** Confirmado en **`2.50 V`** (con histéresis de `0.15 V` y cooldown de `30 s`).
3. **Subcomandos de `/display<cmd>`:**
   * `/displayled`: Muestra el estado del LED/Relé en la pantalla OLED (`"Led encendido"` / `"Led apagado"`).
   * `/displaypot`: Muestra el voltaje y valor ADC del potenciómetro en la pantalla OLED.
   * `/displaydht`: Muestra la temperatura y humedad del sensor DHT22 en la pantalla OLED.
   * Cualquier otra variante de `/display...` mostrará `"Comando no identificado"` en la pantalla OLED.
4. **Formato del comando del Relé:** Formato pegado: `/releon <time>` y `/releoff <time>` (o `/releoff`).


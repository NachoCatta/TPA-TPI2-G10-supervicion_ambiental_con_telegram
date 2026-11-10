# Plan de Trabajo - Trabajo Práctico Integrador N°2 (TA4k01/26)
**Proyecto:** Supervisión Ambiental y Actuación Remota con ESP32, ThingSpeak y Bot de Telegram

**Estado al 10/11/2026:** compilación corregida, sensores/actuadores y selección OLED preparados. Wi-Fi ya está implementado con reintentos, LED azul y solicitud NTP. Las 22 pruebas nativas pasan. ThingSpeak y Telegram quedan pendientes para aportes de otros integrantes. Ver [las dos tareas, archivos y criterios de aceptación](tareas_pendientes.md). La ejecución en Wokwi y en placa UTN sigue pendiente de comprobación real.

---

## 1. Análisis General del Proyecto y Requerimientos

El objetivo del proyecto es desarrollar un sistema embebido basado en el microcontrolador **ESP32** (simulado en **Wokwi con PlatformIO** y desplegable en la **placa de desarrollo física Evaluation Board V1.0 de la UTN**) capaz de:
1. **Monitorear variables ambientales y analógicas:** Temperatura y humedad relativa mediante un sensor **DHT22**, y nivel de tensión mediante un **potenciómetro**.
2. **Visualizar estados localmente:** Mostrar información de los componentes y diagnósticos en una pantalla **OLED I²C (SSD1306 o SH1106, seleccionable desde main)** e indicadores luminosos (**LED verde y LED azul**).
3. **Reportar telemetría a la nube (IoT):** Enviar periódicamente temperatura y humedad, así como lecturas de tensión bajo demanda, hacia un canal de **ThingSpeak** mediante solicitudes HTTP.
4. **Interactuar remotamente vía Telegram:** Integrar un **Bot de Telegram** con menú interactivo (*inline keyboard*), consulta de variables, control temporizado no bloqueante de salidas digitales, visualización en pantalla OLED y envío de alertas automáticas por umbral crítico.

### Requerimientos Arquitectónicos Clave
* **Modularidad y Librerías Propias:** El archivo principal (`main.cpp` / sketch `.ino`) debe mantener la función `loop()` lo más limpia posible, delegando toda la lógica de negocio a módulos/librerías propias bien estructuradas.
* **Temporización sin esperas deliberadas:** Gestionar las tareas periódicas y el apagado del actuador mediante `millis()`, sin agregar `delay()` al flujo principal. Las lecturas y solicitudes de red pueden ser síncronas: acotar sus tiempos de espera y comprobar el apagado del actuador también cuando falla la conexión. Usar `millis()` no garantiza por sí solo una ejecución 100% no bloqueante.
* **Portabilidad a Placa UTN (.ino):** El código debe poder ejecutarse tanto en el entorno PlatformIO + Wokwi como exportarse en formato de sketch `.ino` compatible con la placa física de la cátedra.

---

## 2. Asignación de Hardware y Pines (Pinout)

| Componente | Pin ESP32 | Tipo / Protocolo | Función en el Sistema |
| :--- | :--- | :--- | :--- |
| **Sensor DHT22** | `GPIO 33` | Digital (protocolo propio del DHT) | Medición de Temperatura (°C) y Humedad Relativa (%) |
| **Potenciómetro** | `GPIO 32` | Entrada Analógica (`ADC1_CH4`) | ADC bruto para porcentaje y milivoltios calibrados para voltaje; verificar rango real |
| **LED Verde** | `GPIO 23` | Salida Digital | Actuador de demostración del comando `/rele`; no es el relé físico |
| **LED Azul** | `GPIO 2` | Salida Digital | Indicador de estado / conectividad o segundo actuador |
| **Display OLED (SDA)** | `GPIO 21` | I²C Datos (`SDA`) | OLED SSD1306 o SH1106 (128x64, dir. `0x3C`) |
| **Display OLED (SCL)** | `GPIO 22` | I²C Reloj (`SCL`) | OLED SSD1306 o SH1106 (128x64, dir. `0x3C`) |

El pinout del kit ubica el **relé físico en GPIO 14**. El código actual utiliza el LED verde como actuador, permitido por el enunciado. Antes de optar por el relé físico, verificar su polaridad y adaptar el control. GPIO 2 proviene de la consigna; confirmar el LED azul en la placa. El controlador OLED, su geometría y dirección I²C requieren validación física. El proyecto admite SSD1306 y SH1106; seleccionar uno en la instancia de `GestorDisplay` en main, según explica el README.

---

## 3. Plan de Trabajo Dividido por Etapas

### Etapa 1: Configuración del Entorno, Dependencias y Estructura Modular
**Estado:** configuración base corregida y firmware compilado; conservar una compilación exitosa al integrar cada aporte.

1. **Corrección y ajuste de archivos de configuración:**
   * Corregir la ruta del firmware en `wokwi.toml` (`firmware = '.pio/build/esp32doit-devkit-v1/firmware.bin'`).
   * Configurar `platformio.ini` con la velocidad del monitor serie (`monitor_speed = 115200`) y las librerías externas necesarias (`lib_deps`):
     * `adafruit/DHT sensor library` y `adafruit/Adafruit Unified Sensor` (o `beegee-tokyo/DHT sensor library for ESPx`) para el DHT22.
     * `adafruit/Adafruit SSD1306`, `adafruit/Adafruit SH110X` y `adafruit/Adafruit GFX Library` para los dos controladores OLED I²C admitidos.
     * `witnessmenow/UniversalTelegramBot` y `bblanchon/ArduinoJson` para la integración con Telegram.
     * `HTTPClient.h`, incluido en Arduino ESP32, para peticiones HTTP GET/POST a ThingSpeak. No se utiliza la dependencia externa `mathworks/ThingSpeak`.
   * Mantener la ruta común de `include/` en `build_flags` para que las bibliotecas encuentren la configuración. El entorno predeterminado es ESP32; `native` se utiliza solo para pruebas de lógica.
2. **Diseño de la Arquitectura de Librerías Propias (en carpeta `lib/`):**
   * `Configuracion`: Archivo de cabeceras con constantes de pines, intervalos de tiempo, umbrales y credenciales (Wi-Fi, Token Telegram, API Key ThingSpeak).
   * `GestorWifi` (implementado): conexión en modo estación, consulta de estado, reintentos espaciados y solicitud de sincronización NTP; main solo invoca su interfaz y actualiza el indicador azul.
   * `GestorSensores`: Encapsula la inicialización y lectura no bloqueante del DHT22 y del potenciómetro (conversión ADC a Voltaje y detección de umbral crítico).
   * `GestorActuadores`: Administra los LEDs / Relé y la lógica de encendido temporizado no bloqueante con `millis()`.
   * `GestorDisplay`: Controla la pantalla OLED SSD1306 o SH1106 seleccionada en main. Las vistas de inicio, LED, potenciómetro, DHT22 y comando desconocido comparten el mismo código.
   * `GestorHTTPClient`: Módulo propio que implementará envíos a ThingSpeak mediante `HTTPClient.h`: clima periódico (Temp/Hum) y voltaje del potenciómetro (`/thingup`). La clase del framework se llama `HTTPClient`; el gestor propio tiene otro nombre para distinguirlas.
   * `GestorTelegram`: Administra la cola de mensajes, el teclado inline (`/start`), el parseo de comandos (`/clima`, `/rele...`, `/thingup`, `/display...`) y el disparo de alertas automáticas.

---

### Etapa 2: Desarrollo de Capa de Hardware (Sensores, Actuadores y OLED)
**Estado:** bibliotecas implementadas y lógica cubierta por pruebas nativas; la comprobación de periféricos reales queda incluida en el cierre de integración de la entrega.

1. **Módulo de Sensores (`GestorSensores`):**
   * Implementar lectura periódica del DHT22 (cada ~2-5 segundos) validando errores de lectura (`isnan`).
   * Leer ADC bruto en `GPIO 32` para porcentaje/barra y usar `analogReadMilliVolts() / 1000.0f` para voltaje, con atenuación de 11 dB. Comparar con multímetro antes de validar físicamente el umbral; la escala ideal `ADC / 4095 * 3.3` no es una calibración.
   * Mantener el último valor DHT bueno, pero marcar como inválida la última lectura si falla. Consumidores como display y ThingSpeak deben consultar `datosDhtValidos()`.
   * Generar un evento local por episodio crítico con histéresis y cooldown. Un segundo episodio que siga activo al vencer la espera debe producir su evento. Si termina antes, no generar un aviso tardío. El envío efectivo a Telegram pertenece a la tarea 3.
2. **Módulo de Actuadores (`GestorActuadores`):**
   * Configurar `GPIO 23` (LED verde) y `GPIO 2` (LED azul) como salidas digitales.
   * Implementar máquina de estados no bloqueante para el comando `/rele<on/off> <time>`:
     * Al activar con tiempo $t$ segundos, guardar `tiempoInicio = millis()` y `duracionMs = t * 1000`.
     * En cada iteración de `actualizar()`, verificar si `millis() - tiempoInicio >= duracionMs` para apagar automáticamente la salida sin detener el microcontrolador.
     * `encenderReleTemporizado()` acepta 1 a 3600 segundos y retorna `bool`; un valor inválido no modifica el estado anterior. `/releoff` deberá apagar inmediatamente y rechazar argumentos adicionales. Validar texto y rango antes de activar desde Telegram.
3. **Módulo de Pantalla OLED (`GestorDisplay`):**
   * Inicializar bus I²C (`GPIO 21` / `GPIO 22`) y el controlador elegido en dirección `0x3C`: `GestorDisplay display(TipoDisplay::SSD1306)` para el diagrama actual, o `TipoDisplay::SH1106` para una pantalla física de ese modelo. Solo se inicializa el controlador seleccionado.
   * Crear vistas claras y formateadas para responder a `/display<cmd>`:
     * Estado de LEDs/Relé (`"Led encendido"` / `"Led apagado"`).
     * Estado del Potenciómetro (valor en Voltios y porcentaje/ADC).
     * Estado del sensor DHT22 (Temperatura en °C y Humedad en %).
     * Mensaje de error ante comando inválido (`"Comando no identificado"`).

---

### Etapa 3: Conectividad Wi-Fi e Integración con ThingSpeak
**Estado:** Wi-Fi implementado (antigua tarea 1); ThingSpeak pendiente (tarea 2). Las pruebas reales de red y servicios se completan junto con la entrega.

1. **Conexión Wi-Fi (`WiFi.h`):**
   * Configurar conexión automática a la red Wi-Fi (soportando `Wokwi-GUEST` para el simulador y credenciales reales para la placa física UTN).
   * Implementar verificación de reconexión no bloqueante en caso de caída de red.
   * Encapsularla en `GestorWifi`; actualizar el LED azul mediante llamadas desde main y preparar hora válida para TLS.
   * Implementado: reintentar cada 10 s mediante `millis()`, sin bucle esperando conexión; SDK sin reconexión automática y sin persistir credenciales. `configTime()` solicita hora UTC al conectarse, pero no espera su recepción ni prueba que ya sea válida.
2. **Cliente HTTP para ThingSpeak (`GestorHTTPClient`):**
   * Configurar temporizador no bloqueante (ej. cada 20 o 30 segundos, respetando el límite mínimo de 15 s de ThingSpeak gratuito) para enviar Temperatura (`field1`) y Humedad (`field2`) mediante HTTP GET/POST usando la **Write API Key**.
   * Implementar función dedicada para `/thingup` que envíe el voltaje actual del potenciómetro (`field3`) hacia ThingSpeak y retorne el resultado de la operación.
   * Compartir el control de frecuencia entre envíos periódicos y manuales. Si una solicitud manual no puede enviarse ahora, retornar `false` e informar al usuario; retornar `true` solo cuando el servicio confirme la escritura. No publicar DHT inválido.

---

### Etapa 4: Integración del Bot de Telegram y Comandos Remotos
**Estado:** pendiente, tarea 3. La configuración del certificado existe, pero no hay consultas de mensajes ni comandos implementados.

1. **Configuración de Cliente Seguro (`WiFiClientSecure`):**
   * Configurar el cliente TLS con certificado raíz, hora válida y el **Bot Token**. Mantener la verificación del servidor; no utilizar `setInsecure()` como alternativa habitual.
2. **Implementación de Comandos y Teclado Inline (`GestorTelegram`):**
   * **`/start`**: Envía mensaje de bienvenida junto con un teclado inline (`sendMessageWithInlineKeyboard`) con botones de acceso rápido (por ejemplo: *"🌡️ Ver Clima"*, *"⚡ Encender Relé"*, *"🔌 Apagar Relé"*, *"📈 Enviar Pote (/thingup)"*, *"🖥️ Display LED"*, *"🖥️ Display Pote"*, *"🖥️ Display DHT"*).
   * **`/clima`**: Responde con los últimos valores registrados de temperatura y humedad del DHT22.
   * **`/rele<on/off> <time>`**: Parsea el comando y el parámetro de tiempo (ej. `/releon 10` o `/releoff`), activa el actuador de forma no bloqueante y confirma la acción por Telegram.
   * **`/thingup`**: Envía el voltaje del potenciómetro a ThingSpeak e informa el valor en el chat.
   * **Alerta Automática de Umbral Crítico**: En el ciclo principal, si el voltaje del potenciómetro supera el umbral crítico definido, el ESP32 envía proactivamente un mensaje de `"⚠️ ALERTA DE SEGURIDAD: Voltaje crítico superado (X.XX V)"` al `chat_id` registrado.
   * **`/display<cmd>`**: Analiza el sufijo del comando (`/displayled`, `/displaypot`, `/displaydht`, etc.), actualiza la pantalla OLED con la información solicitada (o muestra `"Comando no identificado"` en el OLED si el sufijo no es válido) y confirma al usuario por Telegram.
   * Procesar los callbacks de los botones inline, autorizar el chat configurado y validar completamente los argumentos. Mantener un aviso pendiente si falla el envío de una alarma: consumir el evento de sensores no equivale a entregarlo.

---

### Etapa 5: Integración en `main.cpp`, Exportación a `.ino` y Pruebas
**Estado:** main integrado con Wi-Fi, indicador azul y vistas preparadas. Pruebas integrales, exportación `.ino` y cierre de entrega pendientes dentro de la tarea 3, con apoyo de la tarea 2.

1. **Estructuración limpia de `main.cpp`:**
   * `setup()` únicamente con llamadas de inicialización (`sistema.iniciar()`).
   * `loop()` minimalista con llamadas de actualización no bloqueantes:
     ```cpp
     void loop() {
         sensores.actualizar();
         actuadores.actualizar();
         wifi.actualizar();
         actuadores.fijarLedAzul(wifi.estaConectado());
         httpClient.actualizar(sensores);
         telegram.actualizar(sensores, actuadores, display, httpClient);
     }
     ```
   * Las llamadas a inicio/actualización de `GestorWifi` ya están incorporadas. Mantener su lógica dentro de la biblioteca.
2. **Pruebas integrales en Wokwi:**
   * Verificar lectura de DHT22, variación del potenciómetro, disparo de alerta automática, respuesta de botones inline en Telegram, recepción de datos en ThingSpeak y visualización en el OLED.
3. **Preparación de entregable para la cátedra (`.ino` + Wokwi):**
   * Generar/verificar el archivo `.ino` compatible con el IDE de Arduino / placa UTN para cumplir con las condiciones de entrega de la tarea `4k01TP02/26`.
   * Entregar también las bibliotecas e instrucciones necesarias para compilar el `.ino`; no basta con renombrar `main.cpp`. Conservar una sola fuente de verdad y documentar cómo se genera la exportación.
4. **Criterios de cierre:**
   * Ejecutar `pio run` y `pio test -e native` (este último requiere compilador C/C++ en la computadora; ver README).
   * Probar entradas inválidas, DHT desconectado, segundo episodio crítico durante la espera, cortes de red y apagado temporizado con solicitudes de red activas.
   * Probar físicamente la placa UTN y registrar lo observado. Una compilación o prueba nativa exitosa no acredita funcionamiento de los periféricos ni de los servicios.

---

## 4. Decisiones del proyecto y configuración local

### A. Credenciales y Servicios Externos
1. **Telegram Bot Token y Chat ID:** Cada integrante debe verificar su `include/secrets.h` local, creado a partir de la plantilla e ignorado por Git. El checkout público no acredita credenciales reales. Usar el chat configurado para autorizar comandos y recibir alertas; no reemplazarlo automáticamente por el último usuario que interactúe.
2. **ThingSpeak Channel ID y Write API Key:** Completar y comprobar en el archivo privado local. El orden de campos adoptado por el proyecto es:
   * `Field 1`: Temperatura (°C)
   * `Field 2`: Humedad Relativa (%)
   * `Field 3`: Voltaje del Potenciómetro (V)
3. **Credenciales Wi-Fi:** La plantilla usa `Wokwi-GUEST` para simulación. Para la placa, completar `SECRET_WIFI_SSID` y `SECRET_WIFI_PASSWORD` con la red real y comprobar conexión y periféricos. Cambiar las credenciales por sí solo no sustituye la prueba física.

### B. Decisiones funcionales del grupo
1. **Rol de los LEDs:**
   * **LED Verde (`GPIO 23`):** Actuador de demostración para el futuro comando `/rele`. El relé físico en GPIO 14 es otro periférico.
   * **LED Azul (`GPIO 2`):** Indicador del estado de la conexión Wi-Fi (encendido cuando está conectado).
2. **Umbral Crítico del Potenciómetro:** Adoptado en **`2.50 V`**, con histéresis de `0.15 V` y cooldown de `30 s`. Son decisiones del proyecto, no valores numéricos fijados por el enunciado; verificar su comportamiento en la placa.
3. **Subcomandos de `/display<cmd>`:**
   * `/displayled`: Muestra el estado del LED/Relé en la pantalla OLED (`"Led encendido"` / `"Led apagado"`).
   * `/displaypot`: Muestra el voltaje y valor ADC del potenciómetro en la pantalla OLED.
   * `/displaydht`: Muestra la temperatura y humedad del sensor DHT22 en la pantalla OLED.
   * Cualquier otra variante de `/display...` mostrará `"Comando no identificado"` en la pantalla OLED.
4. **Formato del comando del Relé:** `/releon <segundos>` con entero entre 1 y 3600; `/releoff` apaga inmediatamente y no acepta parámetros. Documentar esta interpretación del formato del enunciado al implementar los comandos.


# Supervisión ambiental con ESP32 — TPI 2, grupo 10

Proyecto PlatformIO + Wokwi con DHT22, potenciómetro, OLED y control remoto previsto mediante Telegram y ThingSpeak.

## Estado del trabajo

La base local está implementada: sensores, alarma por umbral, actuador temporizado, vistas OLED, ThingSpeak y conexión/reconexión Wi-Fi con indicador azul y solicitud de hora NTP. `setup()` y `loop()` conservan solo llamadas a funciones/métodos. **Telegram ya está integrado** con comandos, callbacks inline y alertas pendientes. Para el sketch de Arduino IDE, consultar [sus instrucciones](documentacion/arduino_ide.md).

El actuador de demostración es el **LED verde en GPIO 23**. Las funciones se llaman `encenderRele`/`apagarRele` por el comando del enunciado; no accionan el relé físico del kit, que está en GPIO 14. Confirmar el montaje antes de cambiar pines.

## Compilar y preparar credenciales

Desde la terminal de PlatformIO, ubicada en esta carpeta:

```sh
pio run
```

Este comando compila únicamente el entorno ESP32 definido como predeterminado. Las dependencias se descargan la primera vez.

Para usar los servicios, copiar `include/secrets.example.h` a `include/secrets.h` y completar los valores reales. `secrets.h` está excluido de Git. La plantilla permite compilar sin credenciales reales, pero no configura cuentas ni proporciona conectividad.

Para Wokwi se usan `diagram.json` y `wokwi.toml`. La compilación no sustituye una prueba del simulador o de la placa UTN.

## Funcionamiento de Wi-Fi

`GestorWifi` utiliza `WiFi.h`, como la Práctica 06, con reintentos espaciados en lugar de esperar en un `while`. En `setup()` se llama a `wifi.iniciar()` y en cada `loop()` a `wifi.actualizar()`. El LED azul se actualiza con `actuadores.fijarLedAzul(wifi.estaConectado())`.

- Se usa modo estación (`WIFI_STA`), con las credenciales de `secrets.h` o la plantilla Wokwi. No se imprimen contraseñas ni se guardan cambios de credenciales en la memoria persistente del SDK.
- Si no hay conexión, se inicia un nuevo intento como máximo cada 10 s, según `INTERVALO_REINTENTO_WIFI_MS`. La clase gestiona esos reintentos, con la reconexión automática del SDK desactivada.
- Al observar cada conexión solicita la hora UTC a `pool.ntp.org` y `time.nist.gov` mediante `configTime()`. Esto inicia NTP en segundo plano; no confirma que la hora ya sea válida. Las tareas de servicios deben comprobarla antes de usar TLS.
- `estaConectado()` devuelve el último estado observado en `actualizar()`. Estar conectado al punto de acceso no demuestra acceso a Internet ni éxito de Telegram o ThingSpeak.
- Los cambios de conexión y los intentos se informan por Serial. No se reemplazan las vistas OLED con avisos de red en cada ciclo.

El código propio no agrega `delay()` ni un bucle esperando Wi-Fi o NTP. Los tiempos reales del SDK y del acceso a la red deben comprobarse en Wokwi y en la placa; las pruebas nativas simulan sus respuestas.

## ThingSpeak

`GestorThingSpeak` publica temperatura y humedad en los campos 1 y 2 cada 20 s, y ofrece el envío de voltaje en el campo 3. Usa HTTP por el puerto 80 y la clave/canal de `secrets.h`. Una ventana compartida de 15,5 s evita escrituras prematuras. La escritura solo se confirma si ThingSpeak devuelve HTTP 200 y un Entry ID numérico mayor que cero.

Las peticiones HTTP tienen un timeout de conexión y lectura de 2 s, pero `HTTPClient` es síncrono: estos límites no vuelven no bloqueante al `loop()` ni limitan necesariamente la resolución DNS. Hay que medir la operación en la placa y verificar el temporizador del actuador bajo carga de red.

## Elegir la pantalla desde main

En `src/main.cpp`, usar **una sola** de estas declaraciones:

```cpp
GestorDisplay display(TipoDisplay::SSD1306); // Pantalla del diagrama Wokwi actual.
```

```cpp
GestorDisplay display(TipoDisplay::SH1106); // Si el kit físico utiliza SH1106.
```

La selección predeterminada es SSD1306. En ambos casos `setup()` llama a `display.iniciar()` y las funciones de las vistas son las mismas. Las dos dependencias están declaradas en PlatformIO; `GestorDisplay` inicializa y actualiza únicamente el controlador elegido, compartiendo el código de dibujo.

La selección es manual, antes de compilar: no se intenta detectar automáticamente el controlador por su dirección I²C. Confirmar el modelo físico. Resolución 128×64, dirección `0x3C` y pines SDA/SCL se configuran en `Configuracion.h`. Tener ambos controladores preparados no sustituye la prueba de la pantalla real.

Para estudiar la implementación y explicarla en la defensa, ver [cómo funciona el gestor de pantalla](documentacion/explicacion_display.md), con recorrido del código, diagrama, explicación de referencias y un guion breve.

## Probar la lógica sin placa

```sh
pio test -e native
```

Se necesita un compilador C/C++ (`gcc` y `g++`) en PATH. En Windows se puede instalar el de PlatformIO:

```powershell
pio pkg install --global --tool platformio/toolchain-gccmingw32
$env:PATH = "$env:USERPROFILE\.platformio\packages\toolchain-gccmingw32\bin;$env:PATH"
pio test -e native
```

Las 22 pruebas compilan las bibliotecas reales: 14 de sensores/actuadores y 8 de Wi-Fi. Estas últimas comprueban intervalos de reintento, caída/recuperación, LED azul, solicitud NTP, desbordamiento de `millis()` y continuidad del temporizador mientras se reintenta. Los archivos de `test/soporte/` sustituyen reloj, GPIO, lecturas, Wi-Fi y configuración NTP; solo se incluyen en el entorno `native`. No verifican radio real, recepción de hora, calibración eléctrica, OLED ni servicios de Internet.

## Reglas de funcionamiento acordadas

- `/releon <segundos>` deberá aceptar enteros entre 1 y 3600 (máximo configurable en `Configuracion.h`). `encenderReleTemporizado()` devuelve `false` si el valor es inválido y conserva la salida/temporización anterior. `/releoff` deberá apagar inmediatamente.
- El potenciómetro se informa en voltios usando `analogReadMilliVolts()`. El ADC bruto se conserva para porcentaje y barra. Validar las medidas con multímetro en la placa.
- En Wokwi el DHT22, el potenciómetro y el OLED se alimentan desde `3V3` para mantener las señales conectadas al ESP32 dentro de niveles seguros. Verificar el cableado y la tensión real antes de conectar la placa UTN.
- La alarma se activa desde 2,50 V y se rearma al bajar a 2,35 V. Genera un evento por episodio y espera al menos 30 s entre eventos. Si un segundo episodio sigue activo al vencer esa espera, genera su evento. Si termina antes, no genera un aviso tardío de ese episodio. El muestreo normal es cada 250 ms.
- `consumirEventoAlertaCritica()` retira un evento local, no confirma un envío a Telegram. Telegram conserva un único aviso pendiente y lo reintenta si falla la red.
- `datosDhtValidos()` refleja el resultado de la última lectura. Los getters conservan el último dato bueno; antes de publicarlo o mostrarlo como actual hay que comprobar esa bandera.

La [planificación](documentacion/planificacion.md) describe las etapas. Los archivos y criterios de aceptación de cada aporte están en [tareas pendientes](documentacion/tareas_pendientes.md).

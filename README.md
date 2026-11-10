# Supervisión ambiental con ESP32 — TPI 2, grupo 10

Proyecto PlatformIO + Wokwi con DHT22, potenciómetro, OLED y control remoto previsto mediante Telegram y ThingSpeak.

## Estado del trabajo

La base local está implementada: sensores, alarma por umbral, actuador temporizado y vistas OLED. `setup()` y `loop()` conservan solo llamadas a funciones/métodos. **Wi-Fi, ThingSpeak y los comandos de Telegram están pendientes**, repartidos en [tres tareas para completar en equipo](documentacion/tareas_pendientes.md).

El actuador de demostración es el **LED verde en GPIO 23**. Las funciones se llaman `encenderRele`/`apagarRele` por el comando del enunciado; no accionan el relé físico del kit, que está en GPIO 14. Confirmar el montaje antes de cambiar pines.

## Compilar y preparar credenciales

Desde la terminal de PlatformIO, ubicada en esta carpeta:

```sh
pio run
```

Este comando compila únicamente el entorno ESP32 definido como predeterminado. Las dependencias se descargan la primera vez.

Para usar los servicios, copiar `include/secrets.example.h` a `include/secrets.h` y completar los valores reales. `secrets.h` está excluido de Git. La plantilla permite compilar sin credenciales reales, pero no configura cuentas ni proporciona conectividad.

Para Wokwi se usan `diagram.json` y `wokwi.toml`. La compilación no sustituye una prueba del simulador o de la placa UTN.

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

Las pruebas compilan las bibliotecas reales de sensores y actuadores. Los archivos de `test/soporte/` sustituyen reloj, GPIO y lecturas; solo se incluyen en el entorno `native`. No verifican sensores reales, calibración eléctrica, OLED ni servicios de Internet.

## Reglas de funcionamiento acordadas

- `/releon <segundos>` deberá aceptar enteros entre 1 y 3600 (máximo configurable en `Configuracion.h`). `encenderReleTemporizado()` devuelve `false` si el valor es inválido y conserva la salida/temporización anterior. `/releoff` deberá apagar inmediatamente.
- El potenciómetro se informa en voltios usando `analogReadMilliVolts()`. El ADC bruto se conserva para porcentaje y barra. Validar las medidas con multímetro en la placa.
- La alarma se activa desde 2,50 V y se rearma al bajar a 2,35 V. Genera un evento por episodio y espera al menos 30 s entre eventos. Si un segundo episodio sigue activo al vencer esa espera, genera su evento. Si termina antes, no genera un aviso tardío de ese episodio. El muestreo normal es cada 250 ms.
- `consumirEventoAlertaCritica()` retira un evento local, no confirma un envío a Telegram. El módulo pendiente de Telegram deberá conservar un aviso por enviar si falla la red.
- `datosDhtValidos()` refleja el resultado de la última lectura. Los getters conservan el último dato bueno; antes de publicarlo o mostrarlo como actual hay que comprobar esa bandera.

La [planificación](documentacion/planificacion.md) describe las etapas. Los archivos y criterios de aceptación de cada aporte están en [tareas pendientes](documentacion/tareas_pendientes.md).

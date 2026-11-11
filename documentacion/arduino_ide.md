# Sketch para Arduino IDE

El archivo [`supervision_ambiental.ino`](../supervision_ambiental.ino) es un
adaptador para abrir el proyecto desde Arduino IDE. Incluye
[`src/main.cpp`](../src/main.cpp), que es la única fuente de `setup()` y
`loop()`; no se debe copiar ni modificar esa lógica en el `.ino`.

## Preparación

1. Abrir la carpeta del proyecto desde Arduino IDE y seleccionar una placa
   ESP32 Dev Module.
2. Instalar las bibliotecas `DHT sensor library`, `Adafruit Unified Sensor`,
   `Adafruit GFX Library`, `Adafruit SSD1306`, `Adafruit SH110X`,
   `UniversalTelegramBot` y `ArduinoJson` desde el gestor de bibliotecas.
3. Mantener las carpetas de [`lib/`](../lib/) disponibles como bibliotecas del
   sketch, o agregar sus rutas al include path de la configuración de Arduino
   CLI/IDE. No se deben duplicar ni editar por separado.
4. Copiar [`include/secrets.example.h`](../include/secrets.example.h) como
   `include/secrets.h` y completar las credenciales. El archivo real no debe
   versionarse.

PlatformIO sigue siendo el entorno reproducible recomendado porque ya configura
el include path de `include/` y compila automáticamente las bibliotecas de
`lib/`. Si se modifica una biblioteca, el `.ino` no requiere regeneración:
vuelve a incluir los mismos archivos del proyecto.

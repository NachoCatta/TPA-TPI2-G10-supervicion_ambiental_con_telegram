# Exportar y compilar en Arduino IDE

La fuente del proyecto está en `src/`, `lib/` e `include/`. El archivo
`supervision_ambiental.ino` de la raíz es una plantilla para el exportador:
no se abre directamente, porque el nombre de la carpeta del repositorio no
coincide con el del sketch y Arduino necesita encontrar las bibliotecas locales.

1. Instalar Python 3 y ejecutar desde la raíz del repositorio:

   ```sh
   python scripts/exportar_arduino.py
   ```

2. Abrir `build/arduino/supervision_ambiental/supervision_ambiental.ino`
   en Arduino IDE. La carpeta generada está excluida de Git y contiene copias
   de las fuentes vigentes al exportar. Volver a generarla después de cada
   cambio: primero retirar esa carpeta y luego ejecutar el comando otra vez.
   El script no sobrescribe una exportación existente.
3. En el Gestor de placas instalar **esp32 by Espressif Systems**, versión
   2.0.17, y seleccionar **DOIT ESP32 DEVKIT V1**. Si falta el índice, agregar
   `https://espressif.github.io/arduino-esp32/package_esp32_index.json` a las
   URL adicionales del Gestor de placas.
4. En el Gestor de bibliotecas instalar DHT sensor library, Adafruit Unified
   Sensor, Adafruit SSD1306, Adafruit SH110X, Adafruit GFX Library,
   Adafruit BusIO, UniversalTelegramBot y ArduinoJson **6.x** (no 7.x).
   WiFi, WiFiClientSecure, HTTPClient y Wire vienen con el core ESP32.
5. Compilar con Verificar. Para cargar en una placa, seleccionar además el puerto
   serie apropiado y usar Subir. Monitor serie: 115200 baudios.

Por defecto se exporta solo `secrets.example.h`, sin las credenciales reales.
Para uso privado local, crear `include/secrets.h` a partir del ejemplo y ejecutar
`python scripts/exportar_arduino.py --con-secretos` en una exportación nueva.
Esa carpeta contiene credenciales: no compartirla ni subirla. No editar las
copias generadas como fuente principal; editar el repositorio y volver a exportar.

Arduino compila recursivamente `src/`; por eso el `.ino` no incluye
`main.cpp`. Las cabeceras de las bibliotecas propias se copian junto a los
fuentes para que sus inclusiones se resuelvan sin opciones adicionales.
Ver [especificación oficial del sketch](https://docs.arduino.cc/arduino-cli/sketch-specification/).

## Verificación de compilación

El 11 de noviembre de 2026 se compiló la exportación sin credenciales con
Arduino CLI 1.6.0, core ESP32 2.0.17, placa
`esp32:esp32:esp32doit-devkit-v1` y ArduinoJson 6.21.6. La compilación terminó
sin errores: 972949 de 1310720 bytes de programa (74 %) y 48056 de 327680
bytes de memoria global (14 %). Esta verificación no incluye carga ni prueba
en una placa física.

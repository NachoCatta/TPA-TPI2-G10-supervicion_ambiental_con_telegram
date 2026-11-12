# Estado y pruebas pendientes

Actualizado: 12/11/2026. Wi-Fi, ThingSpeak y Telegram están implementados. `/thingup` ahora conserva un pedido pendiente con prioridad y reintentos. El programa principal mantiene las llamadas a los gestores. Consultar [planificación](planificacion.md), [README](../README.md) e [instrucciones Arduino IDE](arduino_ide.md).

ThingSpeak usa HTTPS, validación de certificado y hora NTP válida; el intervalo mínimo compartido es 15,5 s desde el final del intento, incluso si falla el inicio HTTP. El único pedido `/thingup` pendiente tiene prioridad sobre clima, espera red y hora, se reintenta ante fallos y se reemplaza con una nueva pulsación. Telegram informa el resultado real y el valor intentado. Los límites de conexión, TLS y lectura son de 10 s cada uno, sin garantizar un tiempo total ni acotar necesariamente DNS. El certificado raíz DigiCert Global Root G2 se obtuvo del almacén de confianza y su huella SHA-256 es `CB:3C:CB:B7:60:31:E5:E0:13:8F:8D:D3:9A:23:F9:DE:47:FF:C3:5E:43:C1:14:4C:EA:27:D4:6A:5A:B1:CB:5F`. [DigiCert publica la raíz e identifica el emisor del intermediario](https://knowledge.digicert.com/general-information/digicert-trusted-root-authority-certificates). El 11/11/2026 se validaron desde la computadora el nombre de host y la cadena del servidor usando solo esta raíz, con TLS 1.2 y 1.3, sin credenciales ni escrituras. Esto no comprueba la conexión desde ESP32 ni la publicación real. Si el proveedor cambia la cadena, actualizar la raíz después de verificarla.

Telegram valida el chat autorizado, comandos y duración; conserva un aviso pendiente con el voltaje capturado al dispararse el evento. Una única alerta puede esperar envío; los eventos adicionales no constituyen una cola ilimitada. El 11/11/2026 se verificaron desde la computadora el nombre de host y la cadena de `api.telegram.org` mediante TLS 1.2 usando solo la raíz incluida en UniversalTelegramBot, sin token, solicitudes HTTP ni mensajes. Esto no prueba la conexión del ESP32 ni la ejecución de comandos. El display informa error si no inicializa. Las operaciones de red son síncronas: sus límites no garantizan una duración total ni acotan necesariamente DNS.

El 11/11/2026 pasaron 27 pruebas nativas (3 del parser, 16 de lógica y 8 de Wi-Fi) y compiló el firmware ESP32 en PlatformIO (RAM 48.128/327.680 bytes; flash 997.221/1.310.720 bytes). También compiló el sketch exportado con Arduino CLI, core ESP32 2.0.17 y placa `esp32doit-devkit-v1` (RAM 48.056/327.680 bytes; flash 972.949/1.310.720 bytes). Cppcheck no emitió diagnósticos sobre el código propio analizado. Estas verificaciones no prueban la ejecución integral del bot ni los servicios reales.

El 12/11/2026 pasaron 34 pruebas nativas: 27 existentes y 7 del gestor ThingSpeak, incluida la falla de inicio HTTP y su reintento. Compiló el firmware actualizado en PlatformIO (RAM 48.136 bytes; flash 997.665 bytes) y el sketch exportado con Arduino CLI (RAM 48.064/327.680 bytes; flash 973.373/1.310.720 bytes). Cppcheck no emitió diagnósticos. Las comprobaciones del 11/11 corresponden a la versión anterior.

## Pendiente de validación externa

- [ ] Probar TLS de ThingSpeak y verificar los campos 1, 2 y 3, límite de frecuencia y errores de red/API Key.
- [ ] Probar Telegram con chat autorizado y no autorizado, mensajes, botones inline, entradas inválidas y recuperación de un aviso pendiente.
- [ ] Comprobar encendido, apagado manual y vencimiento del actuador durante operaciones de red; medir los tiempos reales.
- [ ] Probar conexión/reconexión, LED azul y recepción de hora NTP en Wokwi y placa UTN.
- [ ] Verificar DHT22, voltaje con multímetro, OLED y selección SSD1306/SH1106 en la placa. Confirmar cableado y niveles eléctricos.
- [ ] Verificar GPIO 14, polaridad y alimentación antes de sustituir el LED verde GPIO 23 por el relé físico.
- [x] Compilar el entregable Arduino con sus bibliotecas mediante Arduino CLI.
- [ ] Ejecutar el entregable Arduino y verificar su comportamiento.

No se dispone de la placa para estas comprobaciones. Las pruebas nativas y la compilación no prueban periféricos, radio ni servicios reales.

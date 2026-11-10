# Dos tareas pendientes para aportar al repositorio

Actualizado: 10/11/2026. Wi-Fi ya está implementado. **ThingSpeak y Telegram quedan sin implementar intencionalmente** para que otros integrantes completen el trabajo. Conservamos sus números originales: tareas 2 y 3. No hay que rehacer los módulos ya existentes ni agregar lógica de control a `main.cpp`.

## Base preparada

- Compilación con la carpeta `include/` accesible desde las librerías.
- Sensores con muestreo periódico, validez de la última lectura DHT y voltaje en milivoltios calibrados por el core ESP32.
- Alarma con histéresis y espera entre episodios, incluyendo el segundo episodio sostenido durante la espera.
- Actuador de demostración (LED verde, GPIO 23): temporización, apagado manual y validación de 1 a 3600 s. Cero no enciende indefinidamente desde la función temporizada.
- Vistas de OLED disponibles para LED, potenciómetro, DHT y comando desconocido; todavía no son invocadas por Telegram. Soporte preparado para SSD1306 y SH1106, elegido en main mediante `GestorDisplay display(TipoDisplay::SSD1306)` o `TipoDisplay::SH1106`. Se mantiene una sola instancia y las mismas funciones de vistas.
- Wi-Fi con conexión, reintentos cada 10 s, actualización del LED azul y solicitud de hora NTP al conectarse.
- 22 pruebas de lógica en `test/test_logica/` y `test/test_wifi/`, ejecutables con `pio test -e native` y un compilador C/C++ instalado.

**Pines:** DHT22 33, potenciómetro 32, LED verde 23, LED azul 2 (según consigna), OLED SDA 21 / SCL 22. El relé físico del kit es GPIO 14 y no está controlado por el código actual. La polaridad del relé y la alimentación del potenciómetro deben verificarse en la placa, no deducirse del dibujo Wokwi.

La [guía del gestor de pantalla](explicacion_display.md) explica la selección entre controladores y cómo deberá utilizarlo el futuro manejador de Telegram. Incluye un guion para la defensa y distingue lo implementado de lo pendiente.

## Wi-Fi implementado — antigua tarea 1

**Archivos:** `lib/GestorWifi/GestorWifi.h` y `.cpp`, `Configuracion.h` y las llamadas de integración de `src/main.cpp`.

La interfaz es `iniciar()`, `actualizar()` y `estaConectado()`. La conexión se inicia con `WiFi.begin()` y no hay un bucle esperando a que termine. Los reintentos usan `millis()` y `INTERVALO_REINTENTO_WIFI_MS` (10 s). Se desactiva la reconexión automática del SDK para administrar los intentos en un único lugar.

Al observar una conexión se llama a `configTime()` con los servidores de `Configuracion.h`. La sincronización continúa en segundo plano. **Solicitar NTP no demuestra que ya llegó una hora válida**: comprobar la hora antes de las operaciones TLS de los servicios pendientes. Las credenciales reales siguen en `include/secrets.h`, ignorado por Git.

La integración mantiene el main como coordinador: actualiza sensores y actuadores, llama a `wifi.actualizar()` y refleja el estado con `actuadores.fijarLedAzul(wifi.estaConectado())`. Luego conserva las llamadas a los dos servicios todavía pendientes.

Las 8 pruebas nuevas simulan conexión, red ausente/clave incorrecta, caída y recuperación, NTP solicitado una vez por conexión, LED azul, desbordamiento de `millis()` y funcionamiento del temporizador durante reintentos. No establecen conexiones ni reciben hora reales.

La puesta en marcha real se verifica junto con el cierre de la entrega, no se marca realizada por estas pruebas. Ver las comprobaciones comunes al final del documento.

## Tarea 2 — Envíos a ThingSpeak mediante HTTPClient

**Responsable:** por asignar. **Dependencias:** usar el módulo Wi-Fi ya integrado; requiere red/canal reales para pruebas de servicio.

**Archivos:** `lib/GestorHTTPClient/GestorHTTPClient.h` y `.cpp`. La clase propia se llama `GestorHTTPClient`; `HTTPClient` es la clase proporcionada por el framework y se incluye con `<HTTPClient.h>`. El servicio de destino sigue siendo ThingSpeak. Mantener las funciones públicas:

```cpp
void iniciar();
void actualizar(const GestorSensores &sensores);
bool enviarVoltajePotenciometro(float voltaje);
```

### Implementar

1. Implementar las solicitudes con `<HTTPClient.h>`, incluido en Arduino ESP32, a `https://api.thingspeak.com/update` mediante GET o POST. Utilizar `SECRET_THINGSPEAK_WRITE_KEY`; esa clave identifica el canal para este endpoint. `SECRET_THINGSPEAK_CHANNEL_ID` se conserva como identificación del canal, pero no es un parámetro requerido de `/update`. Mantener HTTPS con verificación del certificado. No hace falta instalar la biblioteca oficial ThingSpeak.
2. En `actualizar()`, enviar temperatura a `FIELD_TEMPERATURA` (1) y humedad a `FIELD_HUMEDAD` (2) cada `INTERVALO_THINGSPEAK_MS` (20 s). Comprobar `sensores.datosDhtValidos()` para no publicar una lectura fallida como actual.
3. En `enviarVoltajePotenciometro()`, publicar el voltaje en `FIELD_VOLTAJE_POT` (3). No modificar el significado de los campos.
4. Compartir el control del último envío entre publicación periódica y manual. Un canal gratuito requiere al menos 15 s entre escrituras; un `/thingup` inmediato puede ser rechazado por ese límite. Mantenerlo simple: si no puede enviarse ahora, devolver `false` y Telegram deberá informarlo. No prometer envío exitoso ni dejar una cola oculta.
5. Retornar `true` únicamente cuando el servicio confirme que guardó el dato. Para `/update` en formato texto, comprobar HTTP 200 y un ID de entrada positivo en el cuerpo; `0` indica fallo. Comprobar Wi-Fi y validez del dato, limitar el tiempo de espera y liberar la conexión con `http.end()`. No agregar `delay()` para espaciar escrituras.

### Comprobar para terminar

- [ ] Compila y las pruebas locales siguen pasando.
- [ ] El canal muestra temperatura/humedad en los campos correctos.
- [ ] El envío manual guarda el voltaje y devuelve éxito solo si fue aceptado.
- [ ] Dos envíos demasiado cercanos se manejan explícitamente, incluyendo manual/periódico.
- [ ] Sin red, con API Key incorrecta o DHT inválido, el módulo informa fallo y no bloquea indefinidamente el programa.
- [ ] Documentar el tiempo máximo observado de una operación de red y comprobar que respete el margen acordado para el apagado del actuador. `millis()` por sí solo no vuelve asíncrona una solicitud HTTP.

Referencias: [API de escritura de ThingSpeak](https://www.mathworks.com/help/thingspeak/writedata.html), [límites de actualización](https://www.mathworks.com/help/thingspeak/channel-control.html).

## Tarea 3 — Telegram, integración y entregable

**Responsable:** por asignar. **Dependencias:** Wi-Fi ya integrado y tarea 2 para probar `/thingup`.

**Archivos:** `lib/GestorTelegram/GestorTelegram.h` y `.cpp`. Usar las interfaces existentes de sensores, actuadores, display y `GestorHTTPClient`. El `.ino` y sus instrucciones de instalación se preparan como parte del cierre de esta tarea.

### Implementar

1. Consultar mensajes con el intervalo `INTERVALO_TELEGRAM_POLL_MS`. Conservar el certificado raíz configurado; comprobar hora válida y conexión antes de operar. `GestorWifi` ya solicita NTP al conectarse, pero la recepción de hora es asíncrona: consultar la hora sin un bucle de espera. Definir tiempos de espera acotados y limitar el trabajo por iteración para que las demás actualizaciones sigan ejecutándose.
2. Autorizar el `SECRET_CHAT_ID` configurado para controlar el sistema. No reemplazar automáticamente el destinatario de alertas por el último usuario que escribió al bot.
3. `/start`: bienvenida y teclado **inline**. Procesar también los callbacks de los botones y confirmarlos, no solo los mensajes escritos.
4. `/clima`: responder temperatura/humedad; si `datosDhtValidos()` es falso, informar el error en vez de presentar el dato anterior como actual.
5. `/releon <segundos>`: validar texto completo, entero positivo y rango 1–3600 antes de convertir/activar. Usar el retorno de `encenderReleTemporizado()`. Rechazar valores vacíos, negativos, fraccionarios, cero, texto sobrante o enormes. `/releoff` apaga inmediatamente; rechazar parámetros adicionales. Los botones deben generar órdenes válidas, por ejemplo encendido por 10 s.
6. `/thingup`: forzar la lectura del potenciómetro, pedir el envío y responder según el `bool` devuelto. Si se rechaza, explicar que no se envió y que puede deberse a conexión, frecuencia o servicio. No declarar éxito anticipadamente.
7. `/displayled`, `/displaypot`, `/displaydht`: llamar a las vistas correspondientes y confirmar la acción. Para LED consultar el estado real del actuador; para DHT pasar también su validez. Una variante desconocida de `/display...` debe llamar a `mostrarComandoNoIdentificado()`.
8. Alertas automáticas: consumir el evento de sensores y enviar al chat autorizado. Mantener una bandera local de aviso pendiente hasta que Telegram confirme el envío; reintentar espaciadamente si falla. El evento consumido no confirma entrega. Se acepta conservar un único aviso pendiente para evitar colas complejas; documentar esa limitación y no perderlo al desconectarse.
9. Preparar el sketch `.ino` con sus bibliotecas e instrucciones de Arduino IDE. Conservar una sola fuente de verdad para la lógica: si se exporta una copia, documentar cómo regenerarla y evitar corregir dos versiones por separado. Mantener `setup()` y `loop()` como llamadas a funciones.

### Comprobar para terminar

- [ ] Funcionan comandos escritos y todos los botones inline.
- [ ] Un chat no autorizado no acciona la salida ni cambia el destinatario de alarmas.
- [ ] `/releon 5` apaga tras 5 s mientras siguen atendidos sensores y bot; apagado manual y nueva orden coherentes.
- [ ] Los tiempos inválidos no cambian una salida ni un temporizador ya activo.
- [ ] Cada comando de display muestra el estado correcto; `/displayxyz` informa comando desconocido en OLED.
- [ ] Un cruce de umbral avisa sin petición del usuario; sostener el valor alto no inunda el chat. Probar el segundo episodio durante la espera y la recuperación de red con aviso pendiente.
- [ ] `/thingup` refleja el resultado verdadero del servicio.
- [ ] Prueba completa en Wokwi y en placa UTN, incluyendo errores de red y lectura.
- [ ] Proyecto Wokwi y `.ino` con bibliotecas preparados; compilación y ejecución comprobadas para la entrega.

## Comprobaciones comunes al cierre de la entrega

Estas verificaciones quedan asociadas a la integración de las tareas 2 y 3; no son un tercer módulo a implementar. No se realizaron con las pruebas nativas.

- [ ] Conexión y reconexión reales en Wokwi y en la placa, con y sin credenciales válidas; comprobar LED azul y llegada de hora NTP.
- [ ] Medir que el temporizador siga apagando el actuador mientras se reintenta y cuando operen los servicios. El código propio no espera la conexión, pero las llamadas al SDK real no se miden con los dobles de prueba.
- [ ] Confirmar DHT22, LED azul, voltaje del potenciómetro y controlador/dirección OLED. Seleccionar en main SSD1306 o SH1106 según el modelo físico. El diagrama Wokwi actual usa SSD1306; los ejemplos de Clase 03 con OLED y Práctica 04 usan SH1106.
- [ ] Si se elige usar el relé físico en lugar del LED verde, verificar GPIO 14, polaridad y alimentación antes de adaptar su control.
- [ ] Registrar pruebas hechas y pendientes; probar también el `.ino` y las bibliotecas entregadas.

## Forma de trabajar

Cada integrante puede tomar una tarea en su propia rama y proponer su aporte. Registrar qué se implementó y cómo se probó, y actualizar los casilleros al terminar. Los cambios de sensores/actuadores ya preparados son la base común; no reemplazarlos por un main con toda la lógica. La configuración física y los servicios externos siguen pendientes de validación hasta realizar las pruebas correspondientes.

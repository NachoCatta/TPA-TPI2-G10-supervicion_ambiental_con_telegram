# Apagado temporizado del actuador

El comando temporizado programa un `esp_timer` de una sola ejecución. Su callback corre en la tarea de temporizadores de ESP32 y pone el GPIO en LOW sin esperar a que termine una llamada de red en el loop. El callback deja pendiente un evento para que el loop pueda informar el apagado cuando vuelva a ejecutarse.

Una sección crítica breve protege el estado y el cambio del GPIO frente a una orden manual o una reprogramación concurrente. Un callback anterior que ya se hubiera despachado verifica el inicio y la duración actuales antes de apagar, de modo que no cancela una nueva orden antes de su vencimiento. Si no se puede crear o iniciar el temporizador, se rechaza el encendido temporizado; si falla el inicio, la salida se apaga.

El callback depende de la planificación de la tarea de temporizadores: no garantiza precisión estricta de tiempo real ni sustituye una protección física independiente. La prueba nativa invoca el callback simulado sin llamar a `actualizar()` y también simula un callback anterior tras reprogramar. Falta comprobar el comportamiento en la placa física.

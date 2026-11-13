# Apagado temporizado del actuador

El comando temporizado programa un `esp_timer` de una sola ejecución. Su callback corre en la tarea de temporizadores de ESP32 y ordena apagar el relé GPIO 14 y el LED verde GPIO 23 sin esperar a que termine una llamada de red en el loop. Son salidas separadas escritas consecutivamente, no simultáneamente con precisión eléctrica. El callback deja pendiente un evento para que el loop pueda informar el apagado cuando vuelva a ejecutarse.

Una sección crítica breve protege el estado y el cambio de las salidas frente a una orden manual o una reprogramación concurrente. Un callback anterior que ya se hubiera despachado verifica el inicio y la duración actuales antes de apagar, de modo que no cancela una nueva orden antes de su vencimiento. Si no se puede crear o iniciar el temporizador, se rechaza el encendido temporizado; si falla el inicio, se ordena apagar ambas salidas.

El relé se configura activo en HIGH mediante `NIVEL_RELE_ACTIVO`, una elección inicial pendiente de validación física; puede cambiarse a LOW si el módulo lo requiere. El LED azul GPIO 2 es independiente y señala Wi-Fi. La inicialización ordena apagar relé y LED verde, sin garantizar el estado previo a `setup()`. `estaReleEncendido()` refleja la orden, no una lectura de los contactos.

El callback depende de la planificación de la tarea de temporizadores: no garantiza precisión estricta de tiempo real ni sustituye una protección física independiente. La prueba nativa invoca el callback simulado sin llamar a `actualizar()` y también simula un callback anterior tras reprogramar. Falta comprobar el comportamiento en la placa física.

#include "GestorDisplay.h"

namespace {
constexpr uint16_t COLOR_BLANCO = 1;
constexpr uint16_t COLOR_NEGRO = 0;
}

GestorDisplay::GestorDisplay(TipoDisplay tipo)
    : _tipo(tipo),
      _ssd1306(OLED_ANCHO, OLED_ALTO, &Wire, -1),
      _sh1106(OLED_ANCHO, OLED_ALTO, &Wire, -1),
      _oled(tipo == TipoDisplay::SSD1306
                ? static_cast<Adafruit_GFX &>(_ssd1306)
                : static_cast<Adafruit_GFX &>(_sh1106)),
      _inicializado(false) {}

bool GestorDisplay::iniciar() {
  Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);

  // Solo se inicializa el controlador seleccionado en main.
  bool correcto = _tipo == TipoDisplay::SSD1306
                      ? _ssd1306.begin(SSD1306_SWITCHCAPVCC, OLED_DIRECCION_I2C)
                      : _sh1106.begin(OLED_DIRECCION_I2C, true);
  if (!correcto) {
    Serial.println(
        F("[Display] Error al inicializar el controlador OLED seleccionado."));
    _inicializado = false;
    return false;
  }

  _inicializado = true;
  _oled.fillScreen(COLOR_NEGRO);
  _oled.setTextColor(COLOR_BLANCO);
  mostrarBienvenida();
  return true;
}

void GestorDisplay::actualizarPantalla() {
  if (_tipo == TipoDisplay::SSD1306) {
    _ssd1306.display();
  } else {
    _sh1106.display();
  }
}

void GestorDisplay::dibujarEncabezado(const char *titulo) {
  _oled.fillScreen(COLOR_NEGRO);
  _oled.setTextSize(1);
  _oled.setTextColor(COLOR_BLANCO);
  _oled.setCursor(0, 0);
  _oled.println(titulo);
  _oled.drawLine(0, 10, OLED_ANCHO - 1, 10, COLOR_BLANCO);
}

void GestorDisplay::mostrarBienvenida() {
  if (!_inicializado) return;

  dibujarEncabezado("UTN FRC - TPI2 G10");
  _oled.setCursor(0, 18);
  _oled.println(F("Supervision Ambiental"));
  _oled.println(F("y Actuacion Remota"));
  _oled.setCursor(0, 46);
  _oled.println(F("Esperando comandos..."));
  actualizarPantalla();
}

void GestorDisplay::mostrarEstadoWifi(bool conectado,
                                      const String &ssid,
                                      const String &ip) {
  if (!_inicializado) return;

  dibujarEncabezado("CONEXION WI-FI");
  _oled.setCursor(0, 16);
  _oled.print(F("Red: "));
  _oled.println(ssid);

  _oled.setCursor(0, 32);
  if (conectado) {
    _oled.println(F("Estado: CONECTADO"));
    _oled.setCursor(0, 46);
    _oled.print(F("IP: "));
    _oled.println(ip);
  } else {
    _oled.println(F("Estado: CONECTANDO..."));
  }
  actualizarPantalla();
}

void GestorDisplay::mostrarEstadoLed(bool releEncendido,
                                     bool ledAzulEncendido,
                                     unsigned long segundosRestantes) {
  if (!_inicializado) return;

  dibujarEncabezado("ESTADO DE LED / RELE");
  _oled.setCursor(0, 18);
  _oled.setTextSize(1);

  // Mensaje principal requerido por el enunciado (/displayled)
  if (releEncendido) {
    _oled.println(F("Led encendido"));
  } else {
    _oled.println(F("Led apagado"));
  }

  _oled.setCursor(0, 34);
  if (releEncendido && segundosRestantes > 0) {
    _oled.printf("Timer: %lu s restantes\n", segundosRestantes);
  } else {
    _oled.println(F("Timer: inactivo"));
  }

  _oled.setCursor(0, 48);
  _oled.print(F("LED WiFi (Azul): "));
  _oled.println(ledAzulEncendido ? F("ON") : F("OFF"));
  actualizarPantalla();
}

void GestorDisplay::mostrarEstadoPotenciometro(float voltaje,
                                               uint16_t valorAdc,
                                               bool superaUmbral) {
  if (!_inicializado) return;

  dibujarEncabezado("ESTADO POTENCIOMETRO");
  _oled.setCursor(0, 16);
  _oled.printf("Voltaje: %.2f V\n", voltaje);

  _oled.setCursor(0, 28);
  _oled.printf("ADC (12b): %u / 4095\n", valorAdc);

  // Barra gráfica proporcional al valor ADC bruto.
  uint8_t anchoBarra =
      static_cast<uint8_t>(( static_cast<float>(valorAdc) / ADC_RESOLUCION) * 120.0f);
  _oled.drawRect(0, 40, 122, 8, COLOR_BLANCO);
  _oled.fillRect(1, 41, anchoBarra, 6, COLOR_BLANCO);

  _oled.setCursor(0, 52);
  if (superaUmbral) {
    _oled.println(F("Estado: ALERTA CRITICA"));
  } else {
    _oled.println(F("Estado: Normal"));
  }
  actualizarPantalla();
}

void GestorDisplay::mostrarEstadoDht(float temperatura,
                                     float humedad,
                                     bool valido) {
  if (!_inicializado) return;

  dibujarEncabezado("SENSOR CLIMA (DHT22)");
  if (!valido) {
    _oled.setCursor(0, 24);
    _oled.println(F("Error de lectura o"));
    _oled.println(F("inicializando DHT22"));
    actualizarPantalla();
    return;
  }

  _oled.setCursor(0, 18);
  _oled.printf("Temp: %.1f C\n", temperatura);

  _oled.setCursor(0, 34);
  _oled.printf("Hum:  %.1f %%\n", humedad);

  _oled.setCursor(0, 50);
  _oled.println(F("Estado: Lectura OK"));
  actualizarPantalla();
}

void GestorDisplay::mostrarComandoNoIdentificado(
    const String &comandoRecibido) {
  if (!_inicializado) return;

  dibujarEncabezado("ERROR DE COMANDO");
  _oled.setCursor(0, 18);
  _oled.println(F("Comando no"));
  _oled.println(F("identificado:"));

  _oled.setCursor(0, 42);
  _oled.println(comandoRecibido);
  actualizarPantalla();
}

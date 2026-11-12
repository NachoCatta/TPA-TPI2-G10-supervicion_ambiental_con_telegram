#ifndef ARDUINO_TS_PRUEBA_H
#define ARDUINO_TS_PRUEBA_H
#include "../../soporte/Arduino.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <cctype>
class String {
  std::string valor;
public:
  String() {}
  String(const char* s) : valor(s) {}
  String(float n, unsigned char precision) {
    std::ostringstream out; out << std::fixed << std::setprecision(precision) << n; valor = out.str();
  }
  String& operator+=(const char* s) { valor += s; return *this; }
  String& operator+=(const String& s) { valor += s.valor; return *this; }
  String& operator+=(unsigned char n) { valor += std::to_string(static_cast<unsigned int>(n)); return *this; }
  unsigned int length() const { return static_cast<unsigned int>(valor.length()); }
  char operator[](unsigned int i) const { return valor[i]; }
  const char* c_str() const { return valor.c_str(); }
  long toInt() const { return std::strtol(valor.c_str(), nullptr, 10); }
  void trim() {
    while (!valor.empty() && std::isspace(static_cast<unsigned char>(valor.front()))) valor.erase(0, 1);
    while (!valor.empty() && std::isspace(static_cast<unsigned char>(valor.back()))) valor.pop_back();
  }
};
#endif

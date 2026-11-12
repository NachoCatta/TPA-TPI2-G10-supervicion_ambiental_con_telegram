#pragma once
#include <vector>
#include <string>
#include <Arduino.h>
#include <WiFiClientSecure.h>
#define HTTP_CODE_OK 200
extern std::vector<std::string> urlsPrueba;
extern int codigoPrueba;
extern const char* payloadPrueba;
extern bool inicioHttpPrueba;
extern unsigned int iniciosHttpPrueba;
extern unsigned long duracionHttpPrueba;
class HTTPClient {
public:
  void setConnectTimeout(unsigned long) {}
  void setTimeout(unsigned long) {}
  bool begin(WiFiClientSecure&, const String& url) {
    ++iniciosHttpPrueba;
    urlActual = url.c_str();
    if (!inicioHttpPrueba) tiempoPrueba += duracionHttpPrueba;
    return inicioHttpPrueba;
  }
  int GET() {
    urlsPrueba.push_back(urlActual);
    tiempoPrueba += duracionHttpPrueba;
    return codigoPrueba;
  }
  String getString() { return payloadPrueba; }
  void end() {}
private:
  std::string urlActual;
};

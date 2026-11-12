#pragma once
class Stream { public: void setTimeout(unsigned long) {} };
class WiFiClientSecure : public Stream {
public:
  void setCACert(const char*) {}
  void setTimeout(unsigned long) {}
  void setHandshakeTimeout(unsigned long) {}
};

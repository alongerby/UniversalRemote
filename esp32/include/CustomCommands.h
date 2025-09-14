#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

bool initFS();
bool saveCustomButton(const String& remote,
                      const String& buttonName,
                      uint64_t code,
                      uint8_t bits,
                      const String& protocol);
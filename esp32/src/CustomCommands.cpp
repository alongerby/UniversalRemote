#include <Arduino.h>
#include <LittleFS.h>

bool initFS() {
  if (!LittleFS.begin(true)) { // true = format if mount fails - change in production
    Serial.println("LittleFS mount failed");
    return false;
  }
  LittleFS.mkdir("/custom");
  return true;
}

bool saveCustomButton(const String& remote,
                      const String& buttonName,
                      uint64_t code,
                      uint8_t bits,
                      const String& protocol){

    const String path = "/custom/" + remote + ".json";

}
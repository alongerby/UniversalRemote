#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

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

    DynamicJsonDocument doc(4096);

    if (LittleFS.exists(path))
    {
        File in = LittleFS.open(path, "r");
        if (in){
            DeserializationError e = deserializeJson(doc, in);
            in.close();

            if (e) doc.clear();
        }
    }
    doc["remote"] = remote;
    JsonObject buttons = doc["buttons"].to<JsonObject>();

    char hexBuf[19];
    snprintf(hexBuf, sizeof(hexBuf), "0x%016llX", (unsigned long long)code);

    JsonObject b = buttons[buttonName];
    b["code"] = code;
    b["bits"] = bits;
    b["protocol"] = protocol;

    File out = LittleFS.open(path, "w");
    if(!out) return false;

    serializeJsonPretty(doc, out);
    out.close();

    return true;   

}
#include <Arduino.h>
#include <ir_Gree.h>
#include <NimBLEDevice.h>
#include "ACCommands.h"
#include <ArduinoJson.h>


namespace ACCommands {
    void notifyMessage(const char* msg, NimBLECharacteristic *txChar) {
      if (!txChar) return;
        txChar->setValue((const uint8_t*)msg, strlen(msg)); // explicit length
        txChar->notify();
    }

    // Overload for Arduino String
    void notifyMessage(const String& s) {
      notifyMessage(s.c_str());
    }

    // Overload for std::string
    void notifyMessage(const std::string& s, NimBLECharacteristic *txChar) {
      if (!txChar) return;
      txChar->setValue((const uint8_t*)s.data(), s.size()); // handles embedded NULs too
      txChar->notify();
    }

    // POWER | MODE | FAN | TEMP C |
    void buildAcCommand(IRGreeAC& ac, JsonObjectConst obj){
      // POWER
      String cmd = obj["cmd"] | "";
      cmd.toUpperCase();
      if(cmd == "ON"){
        ac.on();
      }
      else if(cmd == "OFF"){
        ac.off();
      }
      // MODE
      const u_int8_t mode = obj["mode"] | kGreeAuto;
      ac.setMode(mode);
      // FAN
      const u_int8_t fan = obj["fan"] | kGreeFanAuto;
      ac.setFan(fan);
      // TEMP
      u_int8_t temp = obj["temp_c"] | 24;
      temp = std::max(temp, kGreeMinTempC);
      temp = std::min(temp, kGreeMaxTempC);
      ac.setTemp(temp);
    }

    void sendAcCmd(std::string data, IRGreeAC &ac){
      if(data == "OFF"){
        ac.off();
        ac.send();
        return;
      }
      JsonDocument doc;
      DeserializationError err = deserializeJson(doc, data);
      if (err) {
        Serial.print("JSON parse failed: ");
        Serial.println(err.c_str());
        return;
      }
      JsonObjectConst obj = doc.as<JsonObjectConst>();
      buildAcCommand(ac, obj);
      ac.send();
    }
    

}
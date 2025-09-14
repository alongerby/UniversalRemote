#include <Arduino.h>
#include <ir_Gree.h>
#include <NimBLEDevice.h>
#include "ACCommands.h"
#include <ArduinoJson.h>


namespace ACCommands {
    void setup(IRGreeAC &ac){
        ac.begin();
        ac.on();
        ac.setMode(kGreeCool);
        ac.setTemp(24);
        ac.setFan(kGreeFanAuto);
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

    void sendAcCmd(JsonObjectConst obj, IRGreeAC &ac){
      buildAcCommand(ac, obj);
      ac.send();
    }
    

}
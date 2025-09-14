#pragma once
#include <Arduino.h>
#include <string>
#include <ir_Gree.h>
#include <NimBLEDevice.h>
#include <ArduinoJson.h>

namespace ACCommands {

  void setup(IRGreeAC &ac);

  // --- IR helpers ---
  void sendState(IRGreeAC& ac, NimBLECharacteristic *txChar);
  void buildAcCommand(IRGreeAC& ac, JsonObjectConst obj);
  void sendAcCmd(JsonObjectConst data, IRGreeAC &ac);
}
#pragma once
#include <Arduino.h>
#include <string>
#include <ir_Gree.h>
#include <NimBLEDevice.h>
#include <ArduinoJson.h>

namespace ACCommands {

  void setup(IRGreeAC &ac);

  // --- Notify helpers ---
  void notifyMessage(const char* msg, NimBLECharacteristic* txChar);

  void notifyMessage(const String& s, NimBLECharacteristic* txChar);

  void notifyMessage(const std::string& s, NimBLECharacteristic* txChar);

  // --- IR helpers ---
  void sendState(IRGreeAC& ac, NimBLECharacteristic *txChar);
  void buildAcCommand(IRGreeAC& ac, JsonObjectConst obj);
  void sendAcCmd(JsonObjectConst data, IRGreeAC &ac);
}
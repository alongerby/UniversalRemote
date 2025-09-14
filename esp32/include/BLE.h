#pragma once
#include <NimBLEDevice.h>


void notifyMessage(const char* msg, NimBLECharacteristic* txChar);

void notifyMessage(const String& s, NimBLECharacteristic* txChar);

void notifyMessage(const std::string& s, NimBLECharacteristic* txChar);



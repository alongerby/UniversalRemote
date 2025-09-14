#include <NimBLEDevice.h>
#include <BLE.h>   

void notifyMessage(const char* msg, NimBLECharacteristic *txChar) {
    if (!txChar) return;
    txChar->setValue((const uint8_t*)msg, strlen(msg)); // explicit length
    txChar->notify();
}

// Overload for Arduino String
void notifyMessage(const String& s, NimBLECharacteristic* txChar) {
    notifyMessage(s.c_str(), txChar);
}

// Overload for std::string
void notifyMessage(const std::string& s, NimBLECharacteristic *txChar) {
    if (!txChar) return;
    txChar->setValue((const uint8_t*)s.data(), s.size()); // handles embedded NULs too
    txChar->notify();
}

   

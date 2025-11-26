#include <Arduino.h>
#include <NimBLEDevice.h>

#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Gree.h>
#include <ir_NEC.h>
#include <IRrecv.h>
#include <IRutils.h>

#include <UUIDs.h>
#include <ACCommands.h>
#include <TvCodes.h>
#include <TvDispatch.h>
#include <TvCommands.h>
#include <CustomCommands.h>
#include <BLE.h>
#include <TvPending.h>
#include <BleKeyboard.h>

using namespace ACCommands;
using namespace TvCommands;

// ----- IR pins -----
const uint16_t kIrTxPin = 4;   // your TX module DAT pin
const uint16_t kIrRxPin = 21;  // receiver OUT pin
volatile bool acPending = false;
volatile bool tvPending = false;
TvPending tv_pend{};
JsonObjectConst obj;
const int irPin = 4; // transistor base resistor connects here

// ----- IR objects ----- 
IRGreeAC ac(kIrTxPin); // TODO: Change to more a general idea for all ac's
IRsend tv(kIrTxPin);

// IRrecv irrecv(kIrRxPin);
IRrecv irrecv(kIrRxPin);
decode_results results;

// ----- BLE -----
NimBLECharacteristic* txChar = nullptr;
BleKeyboard ble("Universal Remote CC");

struct RxCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* ch) override {
    std::string data = ch->getValue();
    if (data.empty() || !txChar) return;

    StaticJsonDocument<256> doc;
    DeserializationError jsonErr = deserializeJson(doc, data.c_str());
    if (jsonErr) {
      notifyMessage(jsonErr.c_str(), txChar);
      return;
    }

    obj = doc.as<JsonObjectConst>();
    const char* type = obj["type"] | "";

    if (strcmp(type, "AC") == 0) {
      acPending = true;
      notifyMessage("Sent AC", txChar);

    } else if (strcmp(type, "TV") == 0) {
      String err;
      if (sendTvFromJson(tv_pend, obj, err)) {
        tvPending = true;
        notifyMessage("Sent TV", txChar);
      }else{
        notifyMessage(err.c_str(), txChar);
      }
    }
  }
};

void setup() {
    Serial.begin(115200);
    ble.begin();
}

void loop() {
    if (ble.isConnected()) {
        ble.sendConsumerRaw(0x43);
        delay(2000);
    }
}

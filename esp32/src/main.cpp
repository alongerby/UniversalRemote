#include <Arduino.h>
#include <NimBLEDevice.h>

#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Gree.h>
#include <IRrecv.h>
#include <IRutils.h>

#include <UUIDs.h>
#include <ACCommands.h>
#include <TvCodes.h>
#include <TvDispatch.h>
#include <TvCommands.h>

using namespace ACCommands;
using namespace TvCommands;

// ----- IR pins -----
const uint16_t kIrTxPin = 4;   // your TX module DAT pin
const uint16_t kIrRxPin = 21;  // receiver OUT pin

// ----- IR objects -----
IRGreeAC ac(kIrTxPin);
IRsend tv(kIrTxPin);

// IRrecv irrecv(kIrRxPin);
IRrecv irrecv(kIrRxPin);
decode_results results;

// ----- BLE -----
NimBLECharacteristic* txChar = nullptr;

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

    JsonObjectConst obj = doc.as<JsonObjectConst>();
    const char* type = obj["type"] | "";

    if (strcmp(type, "AC") == 0) {
      sendAcCmd(obj, ac);
      notifyMessage("Sent AC", txChar);

    } else if (strcmp(type, "TV") == 0) {
      String err;
      if (!sendTvFromJson(tv, obj, err)) {
        notifyMessage(err.c_str(), txChar);
      }else{
        notifyMessage("Sent TV", txChar);
        notifyMessage(data, txChar);
      }
    }
  }
};

void setup() {
  Serial.begin(115200);
  delay(2000);

  // ---- Transmittor setup ----
  setup(ac);
  tv.begin();

  // ---- Reciever setup ----
  irrecv.enableIRIn();
  Serial.println("IR reciever ready");

  // ---- BLE ----
  NimBLEDevice::init("ESP32-BLE-IR");
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);

  auto* server = NimBLEDevice::createServer();
  auto* svc = server->createService(UUIDs::Service);

  // TX (notify to phone)
  txChar = svc->createCharacteristic(UUIDs::TxChar, NIMBLE_PROPERTY::NOTIFY);

  // RX (writes from phone)
  auto* rxChar = svc->createCharacteristic(
      UUIDs::RxChar, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
  rxChar->setCallbacks(new RxCallbacks());

  svc->start();

  auto* adv = NimBLEDevice::getAdvertising();
  adv->addServiceUUID(UUIDs::Service);
  adv->setScanResponse(true);
  adv->start();

  Serial.println("BLE + IR RX/TX ready.");
}

void loop() {
  if(irrecv.decode(&results)){
    Serial.println(resultToHumanReadableBasic(&results));
    irrecv.resume();
  }
}

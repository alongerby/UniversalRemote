#include <Arduino.h>
#include <NimBLEDevice.h>

#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Gree.h>
#include <IRrecv.h>
#include <IRutils.h>

#include <UUIDs.h>
#include <ACCommands.h>

using namespace ACCommands;

// ----- IR pins -----
const uint16_t kIrTxPin = 4;   // your TX module DAT pin
const uint16_t kIrRxPin = 21;  // receiver OUT pin

// ----- IR objects -----
IRGreeAC ac(kIrTxPin);
IRrecv irrecv(kIrRxPin);
decode_results results;

// ----- BLE -----
NimBLECharacteristic* txChar = nullptr;

static void processCmd(const std::string &sraw) {
  // normalize to uppercase ASCII without CR/LF
  String s;
  for (char c : sraw) if (c >= 32 && c <= 126) s += c;
  s.trim(); s.toUpperCase();

  if (s == "ON") {
    ac.on();
    sendState(ac, txChar);
  } else if (s == "OFF") {
    ac.off();
    sendState(ac, txChar);
  } else {
    notifyMessage("Unknown. Use ON or OFF.", txChar);
  }
}

struct RxCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* ch) override {
    std::string data = ch->getValue();
    if (!data.empty() && txChar) {
      notifyMessage("Received", txChar);
      processCmd(data);
    }
  }
};

void setup() {
  Serial.begin(115200);
  delay(200);

  // ---- IR TX defaults ----
  ac.begin();
  ac.on();
  ac.setMode(kGreeCool);
  ac.setTemp(24);
  ac.setFan(kGreeFanAuto);

  // ---- IR RX start ----
  irrecv.enableIRIn();   // start the receiver

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
  // Poll the IR receiver and report concise info over BLE
  if (irrecv.decode(&results)) {
    String protoStr = typeToString(results.decode_type); // Arduino String
    char line[64];

    if (results.bits <= 32) {
      snprintf(line, sizeof(line), "RX %s 0x%08lX (%d)",
              protoStr.c_str(),
              (unsigned long)results.value,
              results.bits);
    } else {
      snprintf(line, sizeof(line), "RX %s (%d bits)",
              protoStr.c_str(),
              results.bits);
    }

    Serial.println(line);
    notifyMessage(line, txChar);   // now OK with the overloads above

    irrecv.resume();
  }
}

#include <Arduino.h>
#include <NimBLEDevice.h>

#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Gree.h>
#include <IRrecv.h>
#include <IRutils.h>

// ----- BLE UUIDs (Nordic UART-style) -----
static NimBLEUUID kSvcUUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
static NimBLEUUID kRxUUID ("6E400002-B5A3-F393-E0A9-E50E24DCCA9E"); // Write
static NimBLEUUID kTxUUID ("6E400003-B5A3-F393-E0A9-E50E24DCCA9E"); // Notify

// ----- IR pins -----
const uint16_t kIrTxPin = 4;   // your TX module DAT pin
const uint16_t kIrRxPin = 21;  // receiver OUT pin

// ----- IR objects -----
IRGreeAC ac(kIrTxPin);
IRrecv irrecv(kIrRxPin);
decode_results results;

// ----- BLE -----
NimBLECharacteristic* txChar = nullptr;

// Overload for const char*
static inline void notifyMessage(const char* msg) {
  if (!txChar) return;
  txChar->setValue((const uint8_t*)msg, strlen(msg)); // explicit length
  txChar->notify();
}

// Overload for Arduino String
static inline void notifyMessage(const String& s) {
  notifyMessage(s.c_str());
}

// Overload for std::string
static inline void notifyMessage(const std::string& s) {
  if (!txChar) return;
  txChar->setValue((const uint8_t*)s.data(), s.size()); // handles embedded NULs too
  txChar->notify();
}

static void sendState() {
  ac.send();
  notifyMessage("IR SENT");
}

static void processCmd(const std::string &sraw) {
  // normalize to uppercase ASCII without CR/LF
  String s;
  for (char c : sraw) if (c >= 32 && c <= 126) s += c;
  s.trim(); s.toUpperCase();

  if (s == "ON") {
    ac.on();
    sendState();
  } else if (s == "OFF") {
    ac.off();
    sendState();
  } else {
    notifyMessage("Unknown. Use ON or OFF.");
  }
}

struct RxCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* ch) override {
    std::string data = ch->getValue();
    if (!data.empty() && txChar) {
      notifyMessage("Received");
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
  auto* svc    = server->createService(kSvcUUID);

  // TX (notify to phone)
  txChar = svc->createCharacteristic(kTxUUID, NIMBLE_PROPERTY::NOTIFY);

  // RX (writes from phone)
  auto* rxChar = svc->createCharacteristic(
      kRxUUID, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
  rxChar->setCallbacks(new RxCallbacks());

  svc->start();

  auto* adv = NimBLEDevice::getAdvertising();
  adv->addServiceUUID(kSvcUUID);
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
    notifyMessage(line);   // now OK with the overloads above

    irrecv.resume();
  }
}

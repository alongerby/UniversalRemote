#include <Arduino.h>
#include <NimBLEDevice.h>

// #include <IRremoteESP8266.h>
// #include <IRsend.h>
// #include <ir_Gree.h>
// #include <ir_NEC.h>
// #include <IRrecv.h>
// #include <IRutils.h>

// #include <UUIDs.h>
// #include <ACCommands.h>
// #include <TvCodes.h>
// #include <TvDispatch.h>
// #include <TvCommands.h>
// #include <CustomCommands.h>
// #include <BLE.h>
// #include <TvPending.h>

// using namespace ACCommands;
// using namespace TvCommands;

// // ----- IR pins -----
// const uint16_t kIrTxPin = 4;   // your TX module DAT pin
// const uint16_t kIrRxPin = 21;  // receiver OUT pin
// volatile bool acPending = false;
// volatile bool tvPending = false;
// TvPending tv_pend{};
// JsonObjectConst obj;
// const int irPin = 4; // transistor base resistor connects here

// // ----- IR objects ----- 
// IRGreeAC ac(kIrTxPin); // TODO: Change to more a general idea for all ac's
// IRsend tv(kIrTxPin);

// // IRrecv irrecv(kIrRxPin);
// IRrecv irrecv(kIrRxPin);
// decode_results results;

// // ----- BLE -----
// NimBLECharacteristic* txChar = nullptr;

// struct RxCallbacks : public NimBLECharacteristicCallbacks {
//   void onWrite(NimBLECharacteristic* ch) override {
//     std::string data = ch->getValue();
//     if (data.empty() || !txChar) return;

//     StaticJsonDocument<256> doc;
//     DeserializationError jsonErr = deserializeJson(doc, data.c_str());
//     if (jsonErr) {
//       notifyMessage(jsonErr.c_str(), txChar);
//       return;
//     }

//     obj = doc.as<JsonObjectConst>();
//     const char* type = obj["type"] | "";

//     if (strcmp(type, "AC") == 0) {
//       acPending = true;
//       notifyMessage("Sent AC", txChar);

//     } else if (strcmp(type, "TV") == 0) {
//       String err;
//       if (sendTvFromJson(tv_pend, obj, err)) {
//         tvPending = true;
//         notifyMessage("Sent TV", txChar);
//       }else{
//         notifyMessage(err.c_str(), txChar);
//       }
//     }
//   }
// };

// void setup() {
//   Serial.begin(115200);
//   delay(2000);
//   // File system setup
//   if(!initFS()){
//     Serial.println("Fs failed");
//   }

//   // ---- Transmittor setup ----
//   setup(ac);
//   tv.begin();

//   // ---- Reciever setup ----
//   irrecv.enableIRIn();
//   Serial.println("IR reciever ready");

//   // ---- BLE ----
//   NimBLEDevice::init("ESP32-BLE-IR");
//   NimBLEDevice::setPower(ESP_PWR_LVL_P9);

//   auto* server = NimBLEDevice::createServer();
//   auto* svc = server->createService(UUIDs::Service);

//   // TX (notify to phone)
//   txChar = svc->createCharacteristic(UUIDs::TxChar, NIMBLE_PROPERTY::NOTIFY);

//   // RX (writes from phone)
//   auto* rxChar = svc->createCharacteristic(
//       UUIDs::RxChar, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
//   rxChar->setCallbacks(new RxCallbacks());

//   svc->start();

//   auto* adv = NimBLEDevice::getAdvertising();
//   adv->addServiceUUID(UUIDs::Service);
//   adv->setScanResponse(true); 
//   adv->start();

//   Serial.println("BLE + IR RX/TX ready.");
// }

// void loop() {
//   if(irrecv.decode(&results)){
//     notifyMessage(resultToHumanReadableBasic(&results), txChar);
//     delay(20);
//     irrecv.resume();
//   }
//   if (acPending) { // TODO: fix race condition - queue or mutex
//     acPending = false;
//     buildAcCommand(ac, obj);
//     ac.send();
//     delay(20);
//   }
//   if (tvPending) {
//     tvPending = false;
//     tv_pend.proto->send(tv, tv_pend.code, tv_pend.bits);
//     delay(20);
//   }
// }
#include <NimBLEDevice.h>

static NimBLEAddress target;
static bool locked=false, subbed=false;
static NimBLEClient* client=nullptr;

class ScanCB: public NimBLEAdvertisedDeviceCallbacks{
  void onResult(NimBLEAdvertisedDevice* d) override {
    if (d->isAdvertisingService(NimBLEUUID((uint16_t)0xFFC0)) && d->getRSSI()>-65) {
      target = d->getAddress(); locked=true;
      Serial.printf("Locked 0xFFC0: %s RSSI=%d\n", target.toString().c_str(), d->getRSSI());
      NimBLEDevice::getScan()->stop();
    }
  }
};

void onNotify(NimBLERemoteCharacteristic*, uint8_t* data, size_t len, bool){
  Serial.print("BTN "); for(size_t i=0;i<len;i++) Serial.printf("%02X", data[i]); Serial.println();
}
static NimBLERemoteCharacteristic* firstNotifyIn(NimBLERemoteService* s){
  if(!s) return nullptr;
  auto* chs = s->getCharacteristics(); if(!chs) return nullptr;
  for (auto* ch : *chs) if (ch && ch->canNotify()) return ch;
  return nullptr;
}

bool connectAndSubscribe(){
  client = NimBLEDevice::createClient();
  if(!client->connect(target)){ Serial.println("Connect fail"); return false; }
  Serial.println("Connected");

  auto* svcs = client->getServices(true);
  if(!svcs||svcs->empty()){ Serial.println("No services"); return false; }
  for(auto* s : *svcs){
    auto uuid = s->getUUID().toString();
    Serial.printf("SVC %s\n", uuid.c_str());
    NimBLERemoteCharacteristic* ch = firstNotifyIn(s);
    if(ch){
      if(ch->subscribe(true, onNotify)){
        Serial.printf("Subscribed to %s\n", ch->getUUID().toString().c_str());
        subbed=true; return true;
      } else Serial.println("Subscribe failed, trying next…");
    }
  }
  Serial.println("No notifiable chars found."); return false;
}

void setup(){
  Serial.begin(115200);
  NimBLEDevice::init("");
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);

  auto s=NimBLEDevice::getScan();
  s->setAdvertisedDeviceCallbacks(new ScanCB(), true);
  s->setActiveScan(true); s->setInterval(45); s->setWindow(30); s->setMaxResults(0);
  s->start(0, nullptr, true);
  Serial.println("Scanning for 0xFFC0… press/hold a YES button.");
}

void loop(){
  if(locked && !subbed){
    if(!connectAndSubscribe()){
      Serial.println("Retry…");
      locked=false; subbed=false;
      NimBLEDevice::getScan()->start(0, nullptr, true);
    }
  }
}

#include "BleKeyboard.h"

#if defined(USE_NIMBLE)
#include <NimBLEDevice.h>
#include <NimBLEServer.h>
#include <NimBLEUtils.h>
#include <NimBLEHIDDevice.h>
#else
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "BLE2902.h"
#include "BLEHIDDevice.h"
#endif // USE_NIMBLE
#include "HIDTypes.h"
#include <driver/adc.h>
#include "sdkconfig.h"
#include "Arduino.h"


#if defined(CONFIG_ARDUHAL_ESP_LOG)
  #include "esp32-hal-log.h"
  #define LOG_TAG ""
#else
  #include "esp_log.h"
  static const char* LOG_TAG = "BLEDevice";
#endif


// Report IDs:
#define KEYBOARD_ID 0x01
#define MEDIA_KEYS_ID 0x02

static const uint8_t _hidReportDescriptor[] = {
// ---------------------------------------------------------------
//                        Media Keys (RAW 16-bit Consumer Control)
// ---------------------------------------------------------------
USAGE_PAGE(1),      0x0C,         // USAGE_PAGE (Consumer)
USAGE(1),           0x01,         // USAGE (Consumer Control)
COLLECTION(1),      0x01,         // COLLECTION (Application)

REPORT_ID(1),       MEDIA_KEYS_ID, // Report ID (same as before)

USAGE_PAGE(1),      0x0C,         // USAGE_PAGE (Consumer)
LOGICAL_MINIMUM(1), 0x00,         // Logical Min = 0
LOGICAL_MAXIMUM(2), 0xFF, 0x03,   // Logical Max = 0x03FF (1023)

// Allow ANY consumer usage:
USAGE_MINIMUM(1),   0x00,
USAGE_MAXIMUM(2),   0xFF, 0x03,

REPORT_SIZE(1),     0x10,         // 16-bit field
REPORT_COUNT(1),    0x01,         // Only one field

// Input: Data, Array
HIDINPUT(1),        0x00,

END_COLLECTION(0),
};

BleKeyboard::BleKeyboard(std::string deviceName, std::string deviceManufacturer, uint8_t batteryLevel) 
    : hid(0)
    , deviceName(std::string(deviceName).substr(0, 15))
    , deviceManufacturer(std::string(deviceManufacturer).substr(0,15))
    , batteryLevel(batteryLevel) {}

void BleKeyboard::begin(void)
{
  BLEDevice::init(deviceName);
  BLEServer* pServer = BLEDevice::createServer();
  pServer->setCallbacks(this);

  hid = new BLEHIDDevice(pServer);
  inputMediaKeys = hid->inputReport(MEDIA_KEYS_ID);

  hid->manufacturer()->setValue(deviceManufacturer);

  hid->pnp(0x02, vid, pid, version);
  hid->hidInfo(0x00, 0x01);


#if defined(USE_NIMBLE)

  BLEDevice::setSecurityAuth(true, true, true);

#else

  BLESecurity* pSecurity = new BLESecurity();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM_BOND);

#endif // USE_NIMBLE

  hid->reportMap((uint8_t*)_hidReportDescriptor, sizeof(_hidReportDescriptor));
  hid->startServices();

  onStarted(pServer);

  advertising = pServer->getAdvertising();
  advertising->setAppearance(HID_TABLET);
  advertising->addServiceUUID(hid->hidService()->getUUID());
  advertising->setScanResponse(false);
  advertising->start();
  hid->setBatteryLevel(batteryLevel);

  ESP_LOGD(LOG_TAG, "Advertising started!");
}

void BleKeyboard::end(void)
{
}
void BleKeyboard::sendConsumerRaw(uint16_t usage) {
    if (!this->isConnected()) return;

    uint8_t report[2];
    report[0] = usage & 0xFF;        // LSB
    report[1] = usage >> 8;          // MSB

    inputMediaKeys->setValue(report, sizeof(report));
    inputMediaKeys->notify();

    releaseConsumerRaw();
}

void BleKeyboard::releaseConsumerRaw() {
    if (!this->isConnected()) return;

    uint8_t report[2] = {0x00, 0x00};
    inputMediaKeys->setValue(report, sizeof(report));
    inputMediaKeys->notify();
}

bool BleKeyboard::isConnected(void) {
  return this->connected;
}

void BleKeyboard::setBatteryLevel(uint8_t level) {
  this->batteryLevel = level;
  if (hid != 0)
    this->hid->setBatteryLevel(this->batteryLevel);
}

//must be called before begin in order to set the name
void BleKeyboard::setName(std::string deviceName) {
  this->deviceName = deviceName;
}

/**
 * @brief Sets the waiting time (in milliseconds) between multiple keystrokes in NimBLE mode.
 * 
 * @param ms Time in milliseconds
 */
void BleKeyboard::setDelay(uint32_t ms) {
  this->_delay_ms = ms;
}

void BleKeyboard::set_vendor_id(uint16_t vid) { 
	this->vid = vid; 
}

void BleKeyboard::set_product_id(uint16_t pid) { 
	this->pid = pid; 
}

void BleKeyboard::set_version(uint16_t version) { 
	this->version = version; 
}



void BleKeyboard::onConnect(BLEServer* pServer) {
  this->connected = true;

#if !defined(USE_NIMBLE)

  BLE2902* desc = (BLE2902*)this->inputMediaKeys->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
  desc->setNotifications(true);

#endif // !USE_NIMBLE

}

void BleKeyboard::onDisconnect(BLEServer* pServer) {
  this->connected = false;

#if !defined(USE_NIMBLE)

  BLE2902* desc = (BLE2902*)this->inputMediaKeys->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
  desc->setNotifications(false);

  advertising->start();

#endif // !USE_NIMBLE
}



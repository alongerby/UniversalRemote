#pragma once

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <ArduinoJson.h>
#include <ir_LG.h>
#include <TvCodes.h>
#include <TvDispatch.h>

namespace TvCommands{
    bool sendTvFromJson(IRsend& ir, JsonObjectConst obj, String& err);
}
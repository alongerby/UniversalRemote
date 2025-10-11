#pragma once

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <ArduinoJson.h>
#include <ir_LG.h>
#include <TvCodes.h>
#include <TvDispatch.h>
#include <TvPending.h>

namespace TvCommands{
    bool sendTvFromJson(TvPending &out, JsonObjectConst obj, String& err);
}
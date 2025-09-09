#include <Arduino.h>
#include <NimBLEDevice.h>
#include <ArduinoJson.h>
#include <ir_LG.h>
#include <TvCodes.h>
#include <TvDispatch.h>

namespace TvCommands{
    bool sendTvFromJson(IRsend& ir, JsonObjectConst obj, String& err) {
        String type = obj["type"]    | "";
        String brand = obj["brand"] | "";
        String cmd = obj["cmd"] | "";
        int8_t bits = 0; 

        type.toUpperCase();
        brand.toUpperCase();

        if (type != "TV") {
            err = "type!=TV";
            return false;
        }

        const TvProtocol* proto = lookupTvProtocol(brand);
        if (!proto) { 
            err = "unknown company";
            return false;
        }

        auto itCmd = proto->codes.commands.find(cmd.c_str());
        if (itCmd == proto->codes.commands.end()) {
            err = "Unknown command";
            return false;
        }

        if (bits == 0) bits = proto->default_bits;
        proto->send(ir, itCmd->second, bits);
        return true;
    }
}
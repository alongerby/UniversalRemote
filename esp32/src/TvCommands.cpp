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

        const TvBrand *b = findBrand(brand.c_str());
        if (!b){
            err = "brand meta missing";
            return false;
        }

        const TvCmd *c = findCmd(b, cmd.c_str());
        if (!c){
            err = "Unknown command";
            return false;
        }

        if (bits == 0) bits = b->bits;
        proto->send(ir, c->code, bits);
        return true;
    }
}
#include <Arduino.h>
#include <NimBLEDevice.h>
#include <ArduinoJson.h>
#include <ir_LG.h>
#include <TvCodes.h>
#include <TvDispatch.h>

namespace TvCommands{
    bool sendTvFromJson(IRsend& ir, JsonObjectConst obj, String& err) {
        String type = obj["type"]    | "";
        String brand= obj["company"] | "";
        const char* codeStr = obj["code"] | nullptr;
        uint16_t bits = obj["bits"] | 0;  

        type.toUpperCase();
        brand.toUpperCase();

        if (type != "TV") { err = "type!=TV"; return false; }
        if (!codeStr)     { err = "missing code"; return false; }

        uint64_t code = strtoull(codeStr, nullptr, 0); // base 0 handles 0x prefix

        const TvProtocol* proto = lookupTvProtocol(brand);
        if (!proto) { err = "unknown company"; return false; }

        if (bits == 0) bits = proto->default_bits;
        proto->send(ir, code, bits);
        return true;
    }
}
#pragma once
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <TvCodes.h>

using TvSendFn = void (*)(IRsend& ir, uint64_t code, uint16_t nbits);

// Brand wrappers 
inline void send_lg(IRsend& ir, uint64_t code, uint16_t nbits){
  ir.sendLG(code, nbits ? nbits : 32);
}

struct TvProtocol {
  const char* brand;   // "LG", "SAMSUNG", "SONY"
  TvSendFn    send;   // function pointer
};

static const TvProtocol kTvProtocols[] = {
  { "LG", &send_lg },
};

inline const TvProtocol* lookupTvProtocol(const String& brand) {
  for (auto& p : kTvProtocols) {
    if (brand.equalsIgnoreCase(p.brand)) return &p;
  }
  return nullptr;
}

#pragma once
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <TvCodes.h>

using TvSendFn = void (*)(IRsend& ir, uint64_t code, uint16_t nbits);

// Brand wrappers 
inline void send_lg(IRsend& ir, uint64_t code, uint16_t nbits)     { ir.sendNEC(code, nbits ? nbits : 32); }
struct TvProtocol {
  const char* brand;   // "LG", "SAMSUNG", "SONY"
  TvSendFn    send;   // function pointer
  TvCodes codes;
  uint16_t    default_bits;
};

static const TvProtocol kTvProtocols[] = {
  { "LG", &send_lg, LgTv, 32 },
};

inline const TvProtocol* lookupTvProtocol(const String& companyUpper) {
  for (auto& p : kTvProtocols) {
    if (companyUpper.equalsIgnoreCase(p.brand)) return &p;
  }
  return nullptr;
}

#pragma once
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

using TvSendFn = void (*)(IRsend& ir, uint64_t code, uint16_t nbits);

// Brand wrappers 
inline void send_lg(IRsend& ir, uint64_t code, uint16_t nbits)     { ir.sendLG(code,     nbits ? nbits : 32); }
// inline void send_samsung(IRsend& ir, uint64_t code, uint16_t nbits){ ir.sendSAMSUNG(code, nbits ? nbits : 32); }
// inline void send_sony(IRsend& ir, uint64_t code, uint16_t nbits)   { ir.sendSony(code,   nbits ? nbits : 12); }

struct TvProtocol {
  const char* name;   // "LG", "SAMSUNG", "SONY"
  TvSendFn    send;   // function pointer
  uint16_t    default_bits;
};

static const TvProtocol kTvProtocols[] = {
  { "LG",      &send_lg,      32 },
//   { "SAMSUNG", &send_samsung, 32 },
//   { "SONY",    &send_sony,    12 },
};

inline const TvProtocol* lookupTvProtocol(const String& companyUpper) {
  for (auto& p : kTvProtocols) {
    if (companyUpper.equalsIgnoreCase(p.name)) return &p;
  }
  return nullptr;
}

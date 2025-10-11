#pragma once
#include <stdint.h>

struct TvProtocol;

struct TvPending {
  const TvProtocol* proto = nullptr;
  uint64_t code = 0;
  uint8_t bits = 32;
};
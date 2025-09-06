#pragma once
#include <Arduino.h>

// LG TV IR codes (32-bit NEC protocol)
namespace LgTvCodes {
    constexpr uint8_t  LgBits        = 32;
    constexpr uint64_t POWER_TOGGLE  = 0x20DF10EF;
    constexpr uint64_t VOLUME_UP     = 0x20DF40BF;
    constexpr uint64_t VOLUME_DOWN   = 0x20DFC03F;
    constexpr uint64_t MUTE          = 0x20DF906F;
    constexpr uint64_t CHANNEL_UP    = 0x20DF00FF;
    constexpr uint64_t CHANNEL_DOWN  = 0x20DF807F;
    constexpr uint64_t INPUT_SOURCE  = 0x20DFD02F;
    constexpr uint64_t OK_ENTER      = 0x20DF22DD;
    constexpr uint64_t BACK          = 0x20DF14EB;
}
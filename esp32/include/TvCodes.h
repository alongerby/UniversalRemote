#pragma once
#include <Arduino.h>
#include <map>
#include <string>

struct TvCodes {
    const char* brand;
    uint8_t bits;
    std::map<std::string, uint64_t> commands;
};


// LG TV IR codes (32-bit NEC protocol)
const TvCodes LgTv {
    "LG",
    32,
    {
        { "POWER_TOGGLE", 0x20DF10EF },
        { "VOL_UP",       0x20DF40BF },
        { "VOL_DOWN",     0x20DFC03F },
        { "MUTE",         0x20DF906F },
        { "CH_UP",        0x20DF00FF },
        { "CH_DOWN",      0x20DF807F },
        { "INPUT",        0x20DFD02F },
        { "OK",           0x20DF22DD },
        { "BACK",         0x20DF14EB },
        { "HDMI_1",       0x20DF738C },
        { "HDMI_2",       0x20DF33CC },
        { "HDMI_3",       0x20DF9768 },
    }
};

const std::map<std::string, const TvCodes*> tvMap = {
    { "LG", &LgTv }
};
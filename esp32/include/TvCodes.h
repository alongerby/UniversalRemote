#pragma once
#include <Arduino.h>

struct TvCmd { const char *key; uint64_t code; };
struct TvBrand {const char *name; uint8_t bits; const TvCmd *cmds; size_t n; };

constexpr TvCmd LG_CMDS[] = {
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
};

// LG TV IR codes (32-bit NEC protocol)
constexpr TvBrand BRANDS[] = {
    {"LG", 32, LG_CMDS, sizeof(LG_CMDS) / sizeof(LG_CMDS[0])}
};

inline const TvBrand* findBrand(const char* name){
    for (auto &brand : BRANDS){
        if(strcasecmp(brand.name, name) == 0){
            return &brand;
        }
    }
    return nullptr;
}

inline const TvCmd* findCmd(const TvBrand *brand, const char *key){
    for(size_t i=0; i < brand->n; i++){
        if(strcasecmp(key, brand->cmds[i].key) == 0){
            return &brand->cmds[i];
        }
    }
    return nullptr;
}

// constexpr TvCodes LgTv {
//     "LG",
//     32,
//     {
//         { "POWER_TOGGLE", 0x20DF10EF },
//         { "VOL_UP",       0x20DF40BF },
//         { "VOL_DOWN",     0x20DFC03F },
//         { "MUTE",         0x20DF906F },
//         { "CH_UP",        0x20DF00FF },
//         { "CH_DOWN",      0x20DF807F },
//         { "INPUT",        0x20DFD02F },
//         { "OK",           0x20DF22DD },
//         { "BACK",         0x20DF14EB },
//         { "HDMI_1",       0x20DF738C },
//         { "HDMI_2",       0x20DF33CC },
//         { "HDMI_3",       0x20DF9768 },
//     }
// };

// const std::map<std::string, const TvCodes*> tvMap = {
//     { "LG", &LgTv }
// };
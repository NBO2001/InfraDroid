#ifndef MAPPING_CONTROL_LED_H
#define MAPPING_CONTROL_LED_H


const uint32_t BRIGHT_UP = 0xFF00EF00;
const uint32_t BRIGHT_DOWN = 0xFE01EF00;
const uint32_t OFF = 0xFD02EF00;
const uint32_t ON = 0xFC03EF00;

const uint32_t RED = 0xFB04EF00;
const uint32_t GREEN = 0xFA05EF00;
const uint32_t BLUE = 0xF906EF00;
const uint32_t WHITE = 0xF807EF00;

const uint32_t R1 = 0xF708EF00;
const uint32_t G1 = 0xF609EF00;
const uint32_t B1 = 0xF50AEF00;
const uint32_t FLASH = 0xF40BEF00;

const uint32_t R2 = 0xF30CEF00;
const uint32_t G2 = 0xF20DEF00;
const uint32_t B2 = 0xF10EEF00;
const uint32_t STROBE = 0xF00FEF00;

const uint32_t R3 = 0xEF10EF00;
const uint32_t G3 = 0xEE11EF00;
const uint32_t B3 = 0xED12EF00;
const uint32_t FADE = 0xEC13EF00;

const uint32_t R4 = 0xEB14EF00; //AMARELO
const uint32_t G4 = 0xEA15EF00;
const uint32_t B4 = 0xE916EF00;
const uint32_t SMOOTH = 0xE817EF00;



uint32_t mappingControlLed [6, 4] = {
    {BRIGHT_UP, BRIGHT_DOWN,  OFF,     ON},
    {RED,       GREEN,        BLUE,    WHITE},
    {R1,        G1,           B1,      FLASH},
    {R2,        G2,           B2,      STROBE},
    {R3,        G3,           B3,      FADE},
    {R4,        G4,           B4,      SMOOTH}
};




#endif

#pragma once
#include "arduino.h"

class EXP
{
    private:
        uint8_t orgasm;
        uint8_t squirt;
        uint8_t mouth;
        uint8_t vagina;
        uint8_t anal;
        uint8_t urethra;
        uint8_t expans_v;
        uint8_t expans_a;
        uint8_t sperm_m;
        uint8_t sperm_v;
        uint8_t sperm_a;
    public:
        EXP();
        ~EXP();
        void update(uint8_t item);
        void status(bool gender);
};

class CURRENT
{
    private:
        uint16_t furr;      //모근 깊이 45, 하루 2mm max 5000
        uint8_t  stamina;
        uint8_t  mental;
        uint8_t  stress;
        uint8_t  horny;
        uint8_t  fain;
        uint8_t  ecstasy;
        uint8_t  lubric;
        uint8_t  pee;
        uint8_t  poo;
    public:
        CURRENT();
        ~CURRENT();
        void update(uint8_t item, int8_t count);
        void status();
};

class MENS
{
    private:
        uint8_t blood;
        uint8_t cycle;
        uint8_t ovulation;
        uint8_t pregnant;
        uint8_t d_day;
    public:
        MENS();
        ~MENS();
        void update();
        void status();
};

class BREED
{
    private:
        uint8_t pregnancy;
        uint8_t birth;
        uint8_t miscarriage;
    public:
        BREED();
        ~BREED();
        void update();
        void status();
};
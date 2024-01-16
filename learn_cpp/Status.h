#pragma once
#include "arduino.h"

class HOLE
{
    private:
        uint16_t gape_u;
        uint16_t gape_v;
        uint16_t gape_a;
        uint16_t pressure_u;
        uint16_t pressure_v;
        uint16_t pressure_a;

    public:
        HOLE();
        ~HOLE();
        void meiosis(HOLE *mother, HOLE *father);
        void blend(HOLE *mother, HOLE *father);
        void status();
};

class STAT
{
    private:
        uint16_t intelligence;
        uint16_t strength;
        uint16_t dexterity;
        uint16_t charisma;
        uint16_t constitution;

    public:
        STAT();
        ~STAT();
        void meiosis(STAT *mother, STAT *father);
        void blend(STAT *mother, STAT *father);
        void status();
};
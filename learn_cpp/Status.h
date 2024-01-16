#pragma once
#include "arduino.h"

class STAT
{
    private:
        uint8_t intelligence;
        uint8_t strength;
        uint8_t dexterity;
        uint8_t charisma;
        uint8_t constitution;

    public:
        STAT();
        ~STAT();
        void meiosis(STAT *mother, STAT *father);
        void blend(STAT *mother, STAT *father);
        void status();
};

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
        void status(bool gender);
};

class SENSE
{
    private:
        uint8_t cervix;
        uint8_t skin;
        uint8_t clit_glans;
        uint8_t vagina_balls;
        uint8_t urethra;
        uint8_t anal;
        uint8_t nipple;

    public:
        SENSE();
        ~SENSE();
        void meiosis(SENSE *mother, SENSE *father);
        void blend(SENSE *mother, SENSE *father);
        void status(bool gender);
};

class NATURE
{
    private:
        uint8_t mbti;
        uint8_t at_e_i;
        uint8_t at_s_n;
        uint8_t fn_t_f;
        uint8_t fn_j_p;

    public:
        NATURE();
        ~NATURE();
        void meiosis(NATURE *mother, NATURE *father);
        void blend(NATURE *mother, NATURE *father);
        void status();
};

class EROS
{
    private:
        uint8_t lust;
        uint8_t sadism;
        uint8_t masohism;
        uint8_t exhibition;
        uint8_t service;

    public:
        EROS();
        ~EROS();
        void meiosis(EROS *mother, EROS *father);
        void blend(EROS *mother, EROS *father);
        void status();
};

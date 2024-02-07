#pragma once
#include <arduino.h>
#include "heredity.h"
#include "utility.h"

class MENS
{
    private:
        bool    gen_xy;
        bool    pregnant;
        uint8_t periode;
        uint8_t blood;
        uint8_t cycle;
        uint8_t ovulation;
        uint8_t d_day;
    public:
        MENS();
        ~MENS();
        void generate(bool gender);
        void daily();
        void status();
        uint8_t get();
        bool    get_pregnant();

        String  get_csv();
        void    set_csv(char *save_file);
};

class CURRENT
{
    private:
        uint16_t furr;      //모근 깊이 45mm, 하루 0.2mm max 5000
        uint8_t  lubric;
        uint8_t  pee;
        uint8_t  poo;
        uint8_t  stamina;
        uint8_t  mental;
        uint8_t  stress;
        uint8_t  horny;
        uint8_t  fain;
        uint8_t  ecstasy;
    public:
        CURRENT();
        ~CURRENT();
        void generate();
        void daily();
        void update(uint8_t item, int8_t count);
        void status();
        uint16_t get_furr();
        uint8_t  get(uint8_t item);

        String  get_csv();
        void    set_csv(char *save_file);
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
        void generate();
        void update();
        void status();
        uint8_t get(uint8_t item);

        String  get_csv();
        void    set_csv(char *save_file);
};
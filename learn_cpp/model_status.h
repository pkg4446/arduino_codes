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
        MENS(void);
        ~MENS(void);
        void generate(bool gender);
        bool daily(void);
        uint8_t get(void);
        bool    get_pregnant(void);

        String  get_csv(void);
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
        CURRENT(void);
        ~CURRENT(void);
        void generate(void);
        void daily(void);
        void update(uint8_t item, int8_t count);
        uint16_t get_furr(void);
        uint8_t  get(uint8_t item);

        String  get_csv(void);
        void    set_csv(char *save_file);
};

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
        EXP(void);
        ~EXP(void);
        void generate(void);
        void update(uint8_t item);
        void update_expansion(bool item);
        void update_shot(uint8_t item);
        uint8_t get(uint8_t item);
        uint8_t get_expansion(bool item);
        uint8_t get_shot(uint8_t item);

        String  get_csv(void);
        void    set_csv(char *save_file);
};

class BREED
{
    private:
        uint8_t pregnancy;
        uint8_t birth;
        uint8_t miscarriage;
    public:
        BREED(void);
        ~BREED(void);
        void generate(void);
        void update(void);
        uint8_t get(uint8_t item);

        String  get_csv(void);
        void    set_csv(char *save_file);
};
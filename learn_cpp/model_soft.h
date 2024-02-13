#pragma once
#include <arduino.h>
#include "heredity.h"
#include "utility.h"

class STAT
{
    private:
        uint8_t intelligence;
        uint8_t strength;
        uint8_t dexterity;
        uint8_t charisma;
        uint8_t constitution;
    public:
        STAT(void);
        ~STAT(void);
        void generate(void);
        void change(STAT *gene);
        void meiosis(STAT *mother, STAT *father);
        void blend(STAT *mother, STAT *father);
        uint8_t get_intelligence(void);
        uint8_t get_strength(void);
        uint8_t get_dexterity(void);
        uint8_t get_charisma(void);
        uint8_t get_constitution(void);

        String  get_csv(void);
        void    set_csv(char *save_file);
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
        HOLE(void);
        ~HOLE(void);
        void generate(void);
        void change(HOLE *gene);
        void meiosis(HOLE *mother, HOLE *father);
        void blend(HOLE *mother, HOLE *father);
        uint16_t get_gape(uint8_t item);
        uint16_t get_pressure(uint8_t item);

        String  get_csv(void);
        void    set_csv(char *save_file);
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
        SENSE(void);
        ~SENSE(void);
        void generate(void);
        void change(SENSE *gene);
        void meiosis(SENSE *mother, SENSE *father);
        void blend(SENSE *mother, SENSE *father);
        uint8_t get_cervix(void);
        uint8_t get_skin(void);
        uint8_t get_clit_glans(void);
        uint8_t get_vagina_balls(void);
        uint8_t get_urethra(void);
        uint8_t get_anal(void);
        uint8_t get_nipple(void);

        String  get_csv(void);
        void    set_csv(char *save_file);
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
        NATURE(void);
        ~NATURE(void);
        void generate(void);
        void change(NATURE *gene);
        void meiosis(NATURE *mother, NATURE *father);
        void blend(NATURE *mother, NATURE *father);
        void status(void);
        uint8_t get_MBTI(void);

        String  get_csv(void);
        void    set_csv(char *save_file);
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
        EROS(void);
        ~EROS(void);
        void generate(void);
        void change(EROS *gene);
        void meiosis(EROS *mother, EROS *father);
        void blend(EROS *mother, EROS *father);
        uint8_t get_lust(void);
        uint8_t get_sadism(void);
        uint8_t get_masohism(void);
        uint8_t get_exhibition(void);
        uint8_t get_service(void);

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
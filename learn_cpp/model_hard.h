#pragma once
#include "arduino.h"
#include "heredity.h"
#include "utility.h"
#include "database.h"

class INFO
{
    private:
        bool    gen_xy;
        String  family;
        String  name;
        uint8_t age;
    public:
        INFO();
        ~INFO();
        void   generate(bool gender, bool baby);
        void   set_family(String familyname);
        void   aging();
        void   status();
        bool    get_gender();
        String  get_family();
        String  get_name();
        uint8_t get_age();

        String  get_csv();
        void    set_csv(char *save_file);
};

class HEAD
{
    private:
        bool    gen_xy;
        uint8_t hair_color;
        uint8_t hair_curl;
        uint8_t eye_color;
        bool    eyelid;
        bool    dimple;
        bool    bald;
    public:
        HEAD();
        ~HEAD();
        void set_gender(bool gender);
        void generate();
        void change(HEAD *gene);
        void meiosis(HEAD *mother, HEAD *father);
        void blend(HEAD *mother, HEAD *father);
        void status();
        String  get_hair_color();
        uint8_t get_hair_curl();
        String  get_eye_color();
        bool    get_eyelid();
        bool    get_dimple();
        bool    get_bald();

        String  get_csv();
        void    set_csv(char *save_file);
};

class BODY {
    private:
        bool     gen_xy;
        uint8_t  blood_A;
        uint8_t  blood_B;
        uint8_t  body_color;
        uint16_t breast;    //w_ a:830  ,s:77.0 /woman only
        uint16_t height;    //w_ a:1612 ,s:58.8 /m_ a:1750 ,s:60.0
        uint16_t chest;     //w_ a:700  ,s:77.0 /m_ a:930  ,s:52.9 
        uint16_t waist;     //w_ a:600  ,s:65.0 /m_ a:770  ,s:49.6 
        uint16_t hip;       //w_ a:880  ,s:70.0 /m_ a:930  ,s:43.0
        uint16_t leg_ratio; //w_ a:458  ,s:18.0 /m_ a:453  ,s:20.0
    public:
        BODY();
        ~BODY();
        void set_gender(bool gender);
        void generate();
        void change(BODY *gene);
        void meiosis(BODY *mother, BODY *father);
        void blend(BODY *mother, BODY *father);
        void status();
        String   get_blood();
        uint16_t get_weight();
        uint16_t get_BMI(uint16_t weight);
        String   get_body_color();
        String   get_cup();
        uint16_t get_breast();
        uint16_t get_height();
        uint16_t get_chest();
        uint16_t get_waist();
        uint16_t get_hip();
        uint16_t get_leg_ratio();

        String  get_csv();
        void    set_csv(char *save_file);
};

class EROGENOUS {
    private:
        bool     gen_xy;
        uint8_t  hood_shape;
        uint8_t  hood_start;
        uint8_t  hood_texture;
        uint16_t hood_width;
        uint16_t hood_length;

        uint8_t  lip_i_shape;
        uint8_t  lip_i_texture;
        uint16_t lip_i_width;
        uint16_t lip_i_length;
        uint16_t lip_i_length_r;
        uint16_t lip_i_length_l;
        uint8_t  lip_o_shape;
        uint8_t  lip_o_texture;
        
        uint16_t clit_d;
        uint16_t clit_d_e;
        uint16_t clit_l;
        uint16_t clit_l_e;

        uint16_t glans_d;
        uint16_t glans_d_e;
        uint16_t glans_l;
        uint16_t glans_l_e;

        uint16_t body_d;
        uint16_t body_d_e;
        uint16_t body_l;
        uint16_t body_l_e;
        uint8_t  ball_r;
        uint8_t  ball_l;

        bool     prepuce;
        bool     invert;
        
        uint8_t  fork;
        uint8_t  perineum;
        uint8_t  wrinkle;

        uint16_t areola;
        uint16_t nipple_d;
        uint16_t nipple_d_e;
        uint16_t nipple_h;
        uint16_t nipple_h_e;

    public:
        EROGENOUS();
        ~EROGENOUS();
        void set_gender(bool gender);
        void generate();
        void change(EROGENOUS *gene);
        void meiosis(EROGENOUS *mother, EROGENOUS *father);
        void blend(EROGENOUS *mother, EROGENOUS *father);
        void status();
        uint16_t get_hood_width();
        uint16_t get_hood_length();
        String   get_hood();
        String   get_lips();
        uint16_t get_wing(uint8_t item);
        uint16_t get_clit(uint8_t item);
        uint16_t get_glans(uint8_t item);
        uint16_t get_body(uint8_t item);
        uint8_t  get_balls(bool item);
        bool     get_prepuce();
        bool     get_invert();
        String   get_perineum();
        uint8_t  get_wrinkle();
        uint16_t get_areola();
        uint16_t get_nipple(uint8_t item);
        
        String  get_csv();
        void    set_csv(char *save_file);
};

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
        void generate();
        void change(STAT *gene);
        void meiosis(STAT *mother, STAT *father);
        void blend(STAT *mother, STAT *father);
        void status();
        uint8_t get_intelligence();
        uint8_t get_strength();
        uint8_t get_dexterity();
        uint8_t get_charisma();
        uint8_t get_constitution();

        String  get_csv();
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
        HOLE();
        ~HOLE();
        void generate();
        void change(HOLE *gene);
        void meiosis(HOLE *mother, HOLE *father);
        void blend(HOLE *mother, HOLE *father);
        void status(bool gender);
        uint16_t get_gape(uint8_t item);
        uint16_t get_pressure(uint8_t item);

        String  get_csv();
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
        SENSE();
        ~SENSE();
        void generate();
        void change(SENSE *gene);
        void meiosis(SENSE *mother, SENSE *father);
        void blend(SENSE *mother, SENSE *father);
        void status(bool gender);
        uint8_t get_cervix();
        uint8_t get_skin();
        uint8_t get_clit_glans();
        uint8_t get_vagina_balls();
        uint8_t get_urethra();
        uint8_t get_anal();
        uint8_t get_nipple();

        String  get_csv();
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
        NATURE();
        ~NATURE();
        void generate();
        void change(NATURE *gene);
        void meiosis(NATURE *mother, NATURE *father);
        void blend(NATURE *mother, NATURE *father);
        void status();
        uint8_t get_MBTI();

        String  get_csv();
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
        EROS();
        ~EROS();
        void generate();
        void change(EROS *gene);
        void meiosis(EROS *mother, EROS *father);
        void blend(EROS *mother, EROS *father);
        void status();
        uint8_t get_lust();
        uint8_t get_sadism();
        uint8_t get_masohism();
        uint8_t get_exhibition();
        uint8_t get_service();

        String  get_csv();
        void    set_csv(char *save_file);
};


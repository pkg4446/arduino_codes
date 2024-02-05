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
#pragma once
#include "arduino.h"

class HEAD
{
    private:
        uint8_t hair_color;
        uint8_t hair_curl;
        uint8_t eye_color;
        bool    eyelid;
        bool    dimple;
        bool    bald;

    public:
        HEAD();
        ~HEAD();
        void change(HEAD *gene);
        void meiosis(HEAD *mother, HEAD *father);
        void blend(HEAD *mother, HEAD *father, bool gender);
        void status();
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
        BODY(bool gender);
        ~BODY();
        void change(BODY *gene);
        void meiosis(BODY *mother, BODY *father);
        void blend(BODY *mother, BODY *father);
        void status();
        uint16_t weight();
};

class EROGENOUS {
    private:
        bool     gen_xy;
        uint8_t  hood_c;
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
        EROGENOUS(bool gender);
        ~EROGENOUS();
        void change(EROGENOUS *gene);
        void meiosis(EROGENOUS *mother, EROGENOUS *father);
        void blend(EROGENOUS *mother, EROGENOUS *father);
        void status();
};
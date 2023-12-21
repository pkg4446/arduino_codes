#pragma once
#include "arduino.h"

class HEAD
{
    private:
        String  color_hair;
        String  color_eye;
        uint8_t hair;
        bool    eyelid;
        bool    dimple;
        bool    bald;

    public:
        HEAD();
        ~HEAD();
        void meiosis(HEAD *mother, HEAD *father);
        void blend(HEAD *mother, HEAD *father, bool gender);
        void status();
};

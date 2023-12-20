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
        void status();
        String  mutation_string(String mother, String father);
        uint8_t mutation_uint8_t(uint8_t mother, uint8_t father);
};

#include "interface.h"

uint8_t incounter_area(uint8_t map_gps_xy, uint8_t random_counter){
    uint8_t response = 0;
    if(map_gps_xy == e_field){
        if(random_counter == 0){
            Serial.println("닭!");
        }else if(random_counter < 2){
            Serial.println("과일!");
        }
    }else if(map_gps_xy == e_mountain){
        if(random_counter == 0){
            Serial.println("광석");
        }else if(random_counter < 7){
            Serial.println("돌!");
        }
    }else if(map_gps_xy == e_lake){
        if(random_counter < 2){
            Serial.println("물고기!");
        }else if(random_counter < 6){
            Serial.println("갈매기!");
        }
    }else if(map_gps_xy == e_forest){
        if(random_counter == 0 ){
            Serial.println("고라니!");
        }else if(random_counter < 2){
            Serial.println("과일!");
        }else if(random_counter < 7){
            Serial.println("나무!");
        }
    }else if(map_gps_xy == e_beach){
        if(random_counter < 3){
            Serial.println("코코넛 크랩!");
        }else if(random_counter < 6){
            Serial.println("코코넛!");
        }
    }
    return response;
}
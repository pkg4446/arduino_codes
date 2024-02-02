#include "funtions.h"
#include "utility.h"

void routines_day(bool gender, MENS *class_mens, CURRENT *class_current){
    class_mens->daily(gender);
    class_current->daily();
    #ifdef DEBUG
        perforation("daily");
        if(!gender){
            class_mens->status();
            Serial.println(class_mens->get());
        }
        class_current->status();
    #endif
}
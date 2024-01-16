#include "heredity.h"
#include "status.h"

#define RAND_NUM   100
#define RAND_FLAGE 2

/*********************************** HOLE CLASS FUNCTION ***********************************/
HOLE::HOLE() { //생성자
    Serial.println("Constructing Hole...");
    gape_u = gaussian_range(650,40);
    gape_v = gaussian_range(5550,400);
    gape_a = gaussian_range(6950,800);
    pressure_u = gaussian_range(2500,300);
    pressure_v = gaussian_range(6950,800);
    pressure_a = gaussian_range(9220,1300);
}
/*******************************************************************************************/
HOLE::~HOLE(){
    Serial.println("Destructing Hole...");
}
/*******************************************************************************************/
void HOLE::meiosis(HOLE *mother, HOLE *father){
    gape_u      = mutation_u16(mother->gape_u,      father->gape_u); 
    gape_v      = mutation_u16(mother->gape_v,      father->gape_v);
    gape_a      = mutation_u16(mother->gape_a,      father->gape_a);
    pressure_u  = mutation_u16(mother->pressure_u,  father->pressure_u);
    pressure_v  = mutation_u16(mother->pressure_v,  father->pressure_v);
    pressure_a  = mutation_u16(mother->pressure_a,  father->pressure_a);
}
/*******************************************************************************************/
void HOLE::blend(HOLE *mother, HOLE *father){
    gape_u      = heredity_u16(mother->gape_u,      father->gape_u);
    gape_v      = heredity_u16(mother->gape_v,      father->gape_v);
    gape_a      = heredity_u16(mother->gape_a,      father->gape_a);
    pressure_u  = heredity_u16(mother->pressure_u,  father->pressure_u);
    pressure_v  = heredity_u16(mother->pressure_v,  father->pressure_v);
    pressure_a  = heredity_u16(mother->pressure_a,  father->pressure_a);
}
/*******************************************************************************************/
void HOLE::status(){
    Serial.println("************************************");
    Serial.print("gape_u    : ");Serial.print(gape_u);
    Serial.print("gape_v    : ");Serial.print(gape_v);
    Serial.print("gape_a    : ");Serial.println(gape_a);
    Serial.print("pressure_u: ");Serial.println(pressure_u);
    Serial.print("pressure_v: ");Serial.println(pressure_v);
    Serial.print("pressure_a: ");Serial.println(pressure_a);
}
/*********************************** HOLE CLASS FUNCTION ***********************************/
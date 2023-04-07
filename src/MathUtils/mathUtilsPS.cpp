#include "mathUtilsPS.h"

//See note in .h file
uint16_t mod16PS(uint16_t num1, uint16_t num2){
    while( num1 >= num2) {
        num1 -= num2;
    }

    return num1;
};

//See note in .h file
uint16_t addMod16PS(uint16_t num1, uint16_t num2, uint16_t modNum){
    
    num1 += num2;
    while( num1 >= modNum) {
        num1 -= modNum;
    }

    return num1;
};

//See note in .h file
uint8_t clamp8PS(int16_t in, uint8_t min, uint8_t max){
    int16_t r = in < min ? min : in;
    return r > max ? max : r;
}
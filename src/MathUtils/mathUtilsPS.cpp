#include "mathUtilsPS.h"

uint16_t mod16PS(uint16_t num1, uint16_t num2){
    while( num1 >= num2) {
        num1 -= num2;
    }

    return num1;
};


uint16_t addMod16PS(uint16_t num1, uint16_t num2, uint16_t modNum){
    
    num1 += num2;
    while( num1 >= modNum) {
        num1 -= modNum;
    }

    return num1;
};
#ifndef mathUtilsPS_h
#define mathUtilsPS_h

#if ARDUINO >= 100
    #include "Arduino.h"
#else
    #include "WConstants.h"
    #include "WProgram.h"
    #include "pins_arduino.h"
#endif

//returns the maximum of the two inputs
#if !defined(maxPS)
    #define maxPS(a, b) ((a) > (b) ? (a) : (b))
#endif

//returns the minimum of the two inputs
#if !defined(minPS)
    #define minPS(a, b) ((a) < (b) ? (a) : (b))
#endif

//Digital unsigned subtraction macro. if result <0, then => 0. Otherwise, take on fixed value.
#if !defined(qsubd)
    #define qsubd(x, b) ((x > b) ? b : 0)
#endif

//Analog Unsigned subtraction macro. if result <0, then => 0
#if !defined(qsuba)
    #define qsuba(x, b) ((x > b) ? x - b : 0)
#endif

//Returns the length of an array
#if !defined(SIZE)
    #define SIZE(x) (sizeof((x)) / sizeof((x)[0]))
#endif

//based on fastLED mod8
//faster mod for 16 bit unsigned numbers
//!!ONLY works with unsigned numbers
uint16_t mod16PS(uint16_t num1, uint16_t num2);

//based on fastLED addmod8
//faster add mod for 16 bit unsigned numbers
//adds the two numbers together and returns the mod of the result from a third number
//!!ONLY works with unsigned numbers
uint16_t addMod16PS(uint16_t num1, uint16_t num2, uint16_t modNum);

//Clamps the input to be between the input min and max
//ie if the input is less than min you get min, if it's greater than max you get max
//The input is an int16_t to allow negative numbers, but the min and max must be 0 - 255
uint8_t clamp8PS(int16_t in, uint8_t min, uint8_t max);

#endif

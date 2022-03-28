#ifndef mathUtilsPS_h
#define mathUtilsPS_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WConstants.h"
#include "WProgram.h"
#include "pins_arduino.h"
#endif

#if !defined(max)
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#if !defined(min)
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#if !defined(SIZE)
#define SIZE(x) (sizeof((x)) / sizeof((x)[0]))
#endif

//based on fastLED mod8
//faster mod for 16 bit unsigned numbers
//!!ONLY works with unsigned numbers
uint16_t mod16PS(uint16_t num1, uint16_t num2);;

//based on fastLED addmod8
//faster add mod for 16 bit unsigned numbers
//adds the two numbers togther and returns the mod of the result from a third number
//!!ONLY works with unsigned numbers
uint16_t addMod16PS(uint16_t num1, uint16_t num2, uint16_t modNum);

#endif

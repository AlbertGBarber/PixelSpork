#ifndef ShiftingSeaUtilsPS_h
#define ShiftingSeaUtilsPS_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WConstants.h"
#include "WProgram.h"
#include "pins_arduino.h"
#endif

//fills in the pixelOffsets array for shiftingSea and shiftingRainbowSea functions
//assigns groups of leds a offset between 0 and totalCycleLength
//groups are formed between consecutive leds and vary randomly from 1 to grouping in size
namespace ShiftingSeaUtilsPS {

    void 
        genOffsetArray(uint16_t *offsets, uint16_t numPixels, uint8_t gradLength, uint16_t grouping, uint16_t totalCycleLength, uint8_t mode );

}
#endif
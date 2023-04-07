#ifndef shiftingSeaUtilsPS_h
#define shiftingSeaUtilsPS_h

#include "FastLED.h"

//fills in the pixelOffsets array for shiftingSea and shiftingRainbowSea functions
//assigns groups of leds a offset between 0 and totalCycleLength
//groups are formed between consecutive leds and vary randomly from 1 to grouping in size
namespace shiftingSeaUtilsPS {

    void 
        genOffsetArray(uint16_t *offsets, uint16_t numPixels, uint8_t gradLength, uint16_t grouping, uint16_t totalCycleLength, uint8_t mode );

}
#endif
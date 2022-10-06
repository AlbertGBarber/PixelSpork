#include "ShiftingSeaUtilsPS.h"

using namespace ShiftingSeaUtilsPS;
//fills in the passed in array with a set of offsets, consecutive array indexes can share the same offset based on the grouping value (is random)
//Modes:
    //0: The offsets are choosen from 0 to totalCycleLength
    //1: The offsets are choosen from 0 to gradLength
//Min value fro grouping is 1
void ShiftingSeaUtilsPS::genOffsetArray(uint16_t *offsets, uint16_t numPixels, uint8_t gradLength, uint16_t grouping, uint16_t totalCycleLength, uint8_t mode ){

    uint16_t groupSize;

    // grouping must be at least 1 (otherwise no pixels would be choosen)
    if (grouping <= 0) {
        grouping = 1;
    }

    for (uint16_t i = 0; i < numPixels; i++) {
        // for each value of i, pick a value somewhere between 1 and grouping
        // this is how many leds will share the same offset
        groupSize = random16(1, grouping);

        // pick an offset for the first led in the group
        // and assign it to the rest of the leds in the group
        // The offset range is based on the mode
        if (mode == 0) {
            // mode 0, groups of leds are assigned an offset between any value for totalCycleLength
            offsets[i] = random16(totalCycleLength);
        } else {
            // for mode 1, all the leds will start somewhere between the fade from the first to second colors
            offsets[i] = random8(gradLength);
        }
        //copy the offset to the grouped pixels
        for (uint16_t j = 0; j < groupSize; j++) {
            // if we try to write to an led off the end of the strip, skip the rest
            //(happens because the group size can be bigger than the remaining leds in the strip)
            if ((i + j) > (numPixels - 1)) {
                break;
            }
            offsets[i + j] = offsets[i];
        }
        // add the current groupSize to i, so we increment to the next group
        i += groupSize;
    }
}
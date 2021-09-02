#include "ShiftingRainbowSeaPS.h"

//Overview:
    //We start by initilizing the offset array using ShiftingSeaUtilsPS::genOffsetArray(); (in seperate file b/c it's shared with ShiftingSea)
    //Each index of the offset array holds the offset for its corrosponding pixel (max value of 255, since it's a rainbow)
    //Then, with each offset cycle, the pixel's color is caculated using the current cycle number and it's offset
    //So all pixels are following the same rainbow pattern, but their individual positions in the pattern are all different
    //creating the effect
    //If the randomShift is on, then with each cycle we do a random check to see if we should change the pixel's offset
    //if so, then we increment it by a random amount up to shiftStep.

//constructor for mode 0, we don't need GradLength, since we'll be picking the offsets from the whole rainbow
ShiftingRainbowSeaPS::ShiftingRainbowSeaPS(SegmentSet& SegmentSet, uint8_t Grouping, uint16_t Rate): 
    segmentSet(SegmentSet), grouping(Grouping) 
    {
        mode = 0;
        init(Rate);
    }

//constructor for mode 1
ShiftingRainbowSeaPS::ShiftingRainbowSeaPS(SegmentSet& SegmentSet, uint8_t GradLength, uint8_t Grouping, uint16_t Rate): 
    segmentSet(SegmentSet), gradLength(GradLength), grouping(Grouping) 
    {
        mode = 1;
        init(Rate);
    }

//initlizes core variables for effect
void ShiftingRainbowSeaPS::init(uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    resetOffsets();
}

//changes the gradLength, and also resets the offset array(if in mode 1),
//since changing the gradLength doesn't do anything without changing the offset array
void ShiftingRainbowSeaPS::setGradLength(uint8_t newGradLength) { 
    gradLength = newGradLength;
    if(mode == 1){
        resetOffsets();
    }
}

//changes the mode, also resets the offset array, since it depends on the mode
void ShiftingRainbowSeaPS::setMode(uint8_t newMode) {
    mode = newMode;
    resetOffsets();
}

//re-builds the offset array with new values
void ShiftingRainbowSeaPS::resetOffsets() {
    numPixels = segmentSet.numActiveSegLeds;
    delete[] offsets;
    offsets = new uint16_t[numPixels];
    //255 is the maxiumum length of the rainbow
    ShiftingSeaUtilsPS::genOffsetArray(offsets, numPixels, gradLength, grouping, 255, mode);
}

//updates the effect
//Runs through each pixel, calculates it's color based on the cycle number and the offset
//incrments the offset based on the randomShift values
//the writes it out
void ShiftingRainbowSeaPS::update() {
    currentTime = millis();
    // if we're using an external rate variable, get its value
    globalRateCheckPS();
    if ((currentTime - prevTime) >= rate) {
        prevTime = currentTime;

        for (uint16_t i = 0; i < numPixels; i++) {
            step = (cycleNum + offsets[i]) % 255; // where we are in the cycle of all the colors
            color = segDrawUtils::wheel(step, 0, sat, val); //wheel since it's a rainbow
            segDrawUtils::setPixelColor(segmentSet, i, color, 0);

            //randomly increment the offset
            if (randomShift) {
                if (random8(100) <= shiftThreshold) {
                    offsets[i] = (offsets[i] + random8(1, shiftStep) ) % 255;
                }
            }
        }
        //incrment the cycle, clamping it's max value to prevent any overflow
        cycleNum = (cycleNum + 1) % 255;
        showCheckPS();
    }
}

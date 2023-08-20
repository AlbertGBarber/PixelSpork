#include "ShiftingRainbowSeaSL.h"

//Overview:
    //We start by initializing the offset array using shiftingSeaUtilsPS::genOffsetArray(); (in separate file b/c it's shared with ShiftingSea)
    //Each index of the offset array holds the offset for its corresponding pixel (max value of 255, since it's a rainbow)
    //Then, with each offset cycle, the pixel's color is calculated using the current cycle number and it's offset
    //So all pixels are following the same rainbow pattern, but their individual positions in the pattern are all different
    //creating the effect
    //If the randomShift is on, then with each cycle we do a random check to see if we should change the pixel's offset
    //if so, then we increment it by a random amount up to shiftStep.

//Constructor for the effect. Note that by passing a gradLength of 0, you will set the effect to sMode 0
//In sMode 0 the gradLength isn't used, but to make sure that if you set it back to sMode 0,
//the gradLength will be set to random value between 20 - 40
ShiftingRainbowSeaSL::ShiftingRainbowSeaSL(SegmentSet &SegSet, uint8_t GradLength, uint8_t Grouping, uint16_t Rate): 
    grouping(Grouping), gradLength(GradLength)
    {
        if(GradLength == 0){
            sMode = 0;
            gradLength = random8(20, 40);
        } else {
            sMode = 1;
        }
        init(SegSet, Rate);
    }

ShiftingRainbowSeaSL::~ShiftingRainbowSeaSL(){
    free(offsets);
}

//initializes core variables for effect
void ShiftingRainbowSeaSL::init(SegmentSet &SegSet, uint16_t Rate){
    //bind the rate and segSet pointer vars since they are inherited from BaseEffectPS
    bindSegSetPtrPS();
    bindClassRatesPS();
    resetOffsets();
}

//changes the gradLength, and also resets the offset array(if in mode 1),
//since changing the gradLength doesn't do anything without changing the offset array
void ShiftingRainbowSeaSL::setGradLength(uint8_t newGradLength) { 
    gradLength = newGradLength;
    if(sMode == 1){
        resetOffsets();
    }
}

//changes the mode, also resets the offset array, since it depends on the mode
void ShiftingRainbowSeaSL::setMode(uint8_t newMode) {
    sMode = newMode;
    resetOffsets();
}

//changes the grouping, also resets the offset array
//since that's where the grouping is expressed
void ShiftingRainbowSeaSL::setGrouping(uint16_t newGrouping) {
    grouping = newGrouping;
    resetOffsets();
}

//re-builds the offset array with new values
void ShiftingRainbowSeaSL::resetOffsets() {
    numLines = segSet->numLines;

    //We only need to make a new offsets array if the current one isn't large enough
    //This helps prevent memory fragmentation by limiting the number of heap allocations
    //but this may use up more memory overall.
    if( alwaysResizeObjPS || (numLines > numLinesMax) ){
        numLinesMax = numLines;
        free(offsets);
        offsets = (uint16_t*) malloc(numLines * sizeof(uint16_t));
    }
    
    //255 is the maximum length of the rainbow
    uint8_t gradLengthTemp = gradLength;
    if(sMode == 0){
        gradLengthTemp = 255;
    }
    shiftingSeaUtilsPS::genOffsetArray(offsets, numLines, gradLengthTemp, grouping, 255, sMode);
}

//updates the effect
//Runs through each pixel, calculates it's color based on the cycle number and the offset
//increments the offset based on the randomShift values
//the writes it out
void ShiftingRainbowSeaSL::update() {
    currentTime = millis();

    if ((currentTime - prevTime) >= *rate) {
        prevTime = currentTime;

        for (uint16_t i = 0; i < numLines; i++) {
            step = addMod16PS( cycleNum, offsets[i], 256); // where we are in the cycle of all the colors
            color = colorUtilsPS::wheel(step, 0, sat, val); //wheel since it's a rainbow
            
            segDrawUtils::drawSegLine(*segSet, i, color, 0);

            //randomly increment the offset
            if (randomShift) {
                if (random8(100) <= shiftThreshold) {
                    offsets[i] = addMod16PS( offsets[i], random8(1, shiftStep), 256);
                }
            }
        }
        //increment the cycle, clamping it's max value to prevent any overflow
        cycleNum = addMod16PS(cycleNum, 1, 256); //(cycleNum + 1) % 255;
        showCheckPS();
    }
}

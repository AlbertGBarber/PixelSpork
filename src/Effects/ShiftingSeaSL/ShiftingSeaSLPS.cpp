#include "ShiftingSeaSLPS.h"

// Overview:
// We start by initializing the offset array using ShiftingSeaUtilsPS::genOffsetArray(); (in seperate file b/c it's shared with ShiftingRainbowSea)
// Each index of the offset array holds the offset for its corrosponding pixel (max value of pallet.length * gradLength) to cover all
// the color values the pixel can be
// Then, with each offset cycle, the pixel's color is caculated using the current cycle number and it's offset
// So all pixels are following the same pattern through the pallet, but their individual positions in the pattern are all different
// creating the effect
// If the randomShift is on, then with each cycle we do a random check to see if we should change the pixel's offset
// if so, then we increment it by a random amount up to shiftStep.

//Constructor for effect with pallet
ShiftingSeaSLPS::ShiftingSeaSLPS(SegmentSet& SegmentSet, palletPS* Pallet, uint8_t GradLength, uint8_t Smode, uint8_t Grouping, uint16_t Rate):
    segmentSet(SegmentSet), pallet(Pallet), gradLength(GradLength), sMode(Smode), grouping(Grouping) 
{
    init(Rate);
}

//Constructor for effect with randomly created pallet
ShiftingSeaSLPS::ShiftingSeaSLPS(SegmentSet& SegmentSet, uint8_t NumColors, uint8_t GradLength, uint8_t Smode, uint8_t Grouping, uint16_t Rate):
    segmentSet(SegmentSet), gradLength(GradLength), sMode(Smode), grouping(Grouping) 
{
    palletTemp = palletUtilsPS::makeRandomPallet(NumColors);
    pallet = &palletTemp;
    init(Rate);
}

ShiftingSeaSLPS::~ShiftingSeaSLPS(){
    delete[] offsets;
    delete[] palletTemp.palletArr;
}

//initializes core variables
void ShiftingSeaSLPS::init(uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    resetOffsets();
}

//changes the mode, also resets the offset array
//since that's where the mode is expressed
void ShiftingSeaSLPS::setMode(uint8_t newMode) {
    sMode = newMode;
    resetOffsets();
}

//changes the grouping, also resets the offset array
//since that's where the grouping is expressed
void ShiftingSeaSLPS::setGrouping(uint16_t newGrouping) {
    grouping = newGrouping;
    resetOffsets();
}

// re-builds the offset array with new values
void ShiftingSeaSLPS::resetOffsets() {
    numLines = segmentSet.maxSegLength;
    delete[] offsets;
    offsets = new uint16_t[numLines];
    totalCycleLength = pallet->length * gradLength;
    ShiftingSeaUtilsPS::genOffsetArray(offsets, numLines, gradLength, grouping, totalCycleLength, sMode);
}

// updates the effect
// Runs through each pixel, calculates it's color based on the cycle number and the offset
// and determines which colors from the pallet it's inbetween
// incrments the offset based on the randomShift values
// then writes it out
void ShiftingSeaSLPS::update() {
    currentTime = millis();

    if ((currentTime - prevTime) >= *rate) {
        prevTime = currentTime;
        
        //caculates the totalCycleLength, which represents the total number of possible offsets a pixel can have
        //we do this on the fly so you can change gradLength and the pallet freely
        totalCycleLength = pallet->length * gradLength;

        for (uint16_t i = 0; i < numLines; i++) {
            step = addMod16PS( cycleNum, offsets[i], totalCycleLength); // where we are in the cycle of all the colors
            gradStep = addMod16PS( cycleNum, offsets[i], gradLength); // what step we're on between the current and next color
            currentColorIndex = step / gradLength; // what color we've started from (integers always round down)
            currentColor = palletUtilsPS::getPalletColor(pallet, currentColorIndex);
            nextColor = palletUtilsPS::getPalletColor(pallet, currentColorIndex + 1); // the next color, wrapping to the start of the pallet as needed
            color = colorUtilsPS::getCrossFadeColor(currentColor, nextColor, gradStep, gradLength);

            segDrawUtils::drawSegLineSimple(segmentSet, i, color, 0);
            //segDrawUtils::setPixelColor(segmentSet, i, color, 0);

            // randomly increment the offset
            if (randomShift) {
                if (random8(100) <= shiftThreshold) {
                    offsets[i] = addMod16PS( offsets[i], random8(1, shiftStep), totalCycleLength ); //(offsets[i] + random8(1, shiftStep)) % totalCycleLength;
                }
            }
        }
        // incrment the cycle, clamping it's max value to prevent any overflow
        cycleNum = addMod16PS(cycleNum, 1, totalCycleLength);
        showCheckPS();
    }
}

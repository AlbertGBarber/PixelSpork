#include "ShiftingSeaPS.h"

// Overview:
// We start by initilizing the offset array using ShiftingSeaUtilsPS::genOffsetArray(); (in seperate file b/c it's shared with ShiftingRainbowSea)
// Each index of the offset array holds the offset for its corrosponding pixel (max value of pallet.length * gradLength) to cover all
// the color values the pixel can be
// Then, with each offset cycle, the pixel's color is caculated using the current cycle number and it's offset
// So all pixels are following the same pattern through the pallet, but their individual positions in the pattern are all different
// creating the effect
// If the randomShift is on, then with each cycle we do a random check to see if we should change the pixel's offset
// if so, then we increment it by a random amount up to shiftStep.

ShiftingSeaPS::ShiftingSeaPS(SegmentSet& SegmentSet, palletPS* Pallet, uint8_t GradLength, uint8_t Smode, uint8_t Grouping, uint16_t Rate):
    segmentSet(SegmentSet), pallet(Pallet), gradLength(GradLength), sMode(Smode), grouping(Grouping) 
{
    // bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    setTotalEffectLength();
    resetOffsets();
}

ShiftingSeaPS::~ShiftingSeaPS(){
    delete[] offsets;
}

// binds the pallet to a new one, can be done without reseting the offsets
// but we need to change the totalCycleLength
void ShiftingSeaPS::setPallet(palletPS* newPallet) {
    pallet = newPallet;
    setTotalEffectLength();
}

// sets the gradLength, can be done without reseting the offsets
// but we need to change the totalCycleLength
void ShiftingSeaPS::setGradLength(uint8_t newGradLength) {
    gradLength = newGradLength;
    setTotalEffectLength();
}

// caculates the totalCycleLength, which represents the total number of possible offsets a pixel can have
void ShiftingSeaPS::setTotalEffectLength() {
    // the number of steps in a full cycle (fading through all the colors)
    totalCycleLength = pallet->length * gradLength;
}

// changes the mode, also resets the offset array
// since that's where the mode is expressed
void ShiftingSeaPS::setMode(uint8_t newMode) {
    sMode = newMode;
    resetOffsets();
}

// re-builds the offset array with new values
void ShiftingSeaPS::resetOffsets() {
    numPixels = segmentSet.numActiveSegLeds;
    delete[] offsets;
    offsets = new uint16_t[numPixels];
    setTotalEffectLength();
    ShiftingSeaUtilsPS::genOffsetArray(offsets, numPixels, gradLength, grouping, totalCycleLength, sMode);
}

// updates the effect
// Runs through each pixel, calculates it's color based on the cycle number and the offset
// and determines which colors from the pallet it's inbetween
// incrments the offset based on the randomShift values
// then writes it out
void ShiftingSeaPS::update() {
    currentTime = millis();

    if ((currentTime - prevTime) >= *rate) {
        prevTime = currentTime;

        for (uint16_t i = 0; i < numPixels; i++) {
            step = (cycleNum + offsets[i]) % totalCycleLength; // where we are in the cycle of all the colors
            gradStep = (cycleNum + offsets[i]) % gradLength; // what step we're on between the current and next color
            currentColorIndex = step / gradLength; // what color we've started from (integers always round down)
            currentColor = palletUtilsPS::getPalletColor(pallet, currentColorIndex);
            nextColor = palletUtilsPS::getPalletColor(pallet, currentColorIndex + 1); // the next color, wrapping to the start of the pallet as needed
            color = segDrawUtils::getCrossFadeColor(currentColor, nextColor, gradStep, gradLength);
            segDrawUtils::setPixelColor(segmentSet, i, color, 0);

            // randomly increment the offset
            if (randomShift) {
                if (random8(100) <= shiftThreshold) {
                    offsets[i] = (offsets[i] + random8(1, shiftStep)) % totalCycleLength;
                }
            }
        }
        // incrment the cycle, clamping it's max value to prevent any overflow
        cycleNum = (cycleNum + 1) % totalCycleLength;
        showCheckPS();
    }
}

#include "ShiftingSeaSL.h"

// Overview:
// We start by initializing the offset array using ShiftingSeaUtilsPS::genOffsetArray(); (in seperate file b/c it's shared with ShiftingRainbowSea)
// Each index of the offset array holds the offset for its corrosponding pixel (max value of palette.length * gradLength) to cover all
// the color values the pixel can be
// Then, with each offset cycle, the pixel's color is caculated using the current cycle number and it's offset
// So all pixels are following the same pattern through the palette, but their individual positions in the pattern are all different
// creating the effect
// If the randomShift is on, then with each cycle we do a random check to see if we should change the pixel's offset
// if so, then we increment it by a random amount up to shiftStep.

//Constructor for effect with pattern and palette
ShiftingSeaSL::ShiftingSeaSL(SegmentSet& SegmentSet, patternPS *Pattern, palettePS* Palette, uint8_t GradLength, uint8_t Smode, uint8_t Grouping, uint16_t Rate):
    segmentSet(SegmentSet), pattern(Pattern), palette(Palette), gradLength(GradLength), sMode(Smode), grouping(Grouping) 
{
    init(Rate);
}

//Constructor for effect with palette
ShiftingSeaSL::ShiftingSeaSL(SegmentSet& SegmentSet, palettePS* Palette, uint8_t GradLength, uint8_t Smode, uint8_t Grouping, uint16_t Rate):
    segmentSet(SegmentSet), palette(Palette), gradLength(GradLength), sMode(Smode), grouping(Grouping) 
{
    setPaletteAsPattern();
    init(Rate);

}

//Constructor for effect with randomly created palette
ShiftingSeaSL::ShiftingSeaSL(SegmentSet& SegmentSet, uint8_t NumColors, uint8_t GradLength, uint8_t Smode, uint8_t Grouping, uint16_t Rate):
    segmentSet(SegmentSet), gradLength(GradLength), sMode(Smode), grouping(Grouping) 
{
    paletteTemp = paletteUtilsPS::makeRandomPalette(NumColors);
    palette = &paletteTemp;
    setPaletteAsPattern();
    init(Rate);
}

ShiftingSeaSL::~ShiftingSeaSL(){
    free(offsets);
    free(paletteTemp.paletteArr);
    free(patternTemp.patternArr);
}

//initializes core variables
void ShiftingSeaSL::init(uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    resetOffsets();
}

//sets the pattern to match the current palette
//ie for a palette length 5, the pattern would be 
//{0, 1, 2, 3, 4}
void ShiftingSeaSL::setPaletteAsPattern(){
    patternTemp = generalUtilsPS::setPaletteAsPattern(palette);
    pattern = &patternTemp;
    setTotalCycleLen();
}

//changes the mode, also resets the offset array
//since that's where the mode is expressed
void ShiftingSeaSL::setMode(uint8_t newMode) {
    sMode = newMode;
    resetOffsets();
}

//changes the grouping, also resets the offset array
//since that's where the grouping is expressed
void ShiftingSeaSL::setGrouping(uint16_t newGrouping) {
    grouping = newGrouping;
    resetOffsets();
}

// re-builds the offset array with new values
void ShiftingSeaSL::resetOffsets() {
    numLines = segmentSet.maxSegLength;
    free(offsets);
    offsets = (uint16_t*) malloc(numLines * sizeof(uint16_t));
    setTotalCycleLen();
    ShiftingSeaUtilsPS::genOffsetArray(offsets, numLines, gradLength, grouping, totalCycleLength, sMode);
}

//caculates the totalCycleLength, which represents the total number of possible offsets a pixel can hav
void ShiftingSeaSL::setTotalCycleLen(){
    patternLen = pattern->length;
    totalCycleLength = gradLength * (patternLen + (uint8_t)addBlank); //addBlank is a bool, so will be either 0 or 1
}

//Updates the effect
//Runs through each pixel, calculates it's color based on the cycle number and the offset
//and determines which colors from the palette it's inbetween
//incrments the offset based on the randomShift values
//then writes it out
//Note that if addBlank is true, then we want to add an extra blank color to the end of the cycle,
//but we don't want to modify the existing palette (since it may be used in other effects)
//So we have to artificially add the extra blank color by manipluating the total number of cycle steps
//and then setting the colors manually at the end of the cycle (rather than just getting them from the palette)
void ShiftingSeaSL::update() {
    currentTime = millis();

    if ((currentTime - prevTime) >= *rate) {
        prevTime = currentTime;
        
        //caculates the totalCycleLength, which represents the total number of possible offsets a pixel can have
        //we do this on the fly so you can change gradLength and the palette freely
        //If we're adding a blank color to the cycle, we add an extra gradLength to the totalCycleLength
        //to account for the extra blank color cycle steps
        setTotalCycleLen();

        for (uint16_t i = 0; i < numLines; i++) {
            step = addMod16PS( cycleNum, offsets[i], totalCycleLength); // where we are in the cycle of all the colors
            gradStep = addMod16PS( cycleNum, offsets[i], gradLength); // what step we're on between the current and next color
            curPatIndex = step / gradLength; // what pattern index we've started from (integers always round down)

            //Get the palette index from the pattern then the color from the palette
            curColorIndex = patternUtilsPS::getPatternVal(pattern, curPatIndex);
            currentColor = paletteUtilsPS::getPaletteColor(palette, curColorIndex);

            //Get the next pattern index, wrapping to the start of the pattern as needed, then the color from the palette
            nextColorIndex = patternUtilsPS::getPatternVal(pattern, curPatIndex + 1); 
            nextColor = paletteUtilsPS::getPaletteColor(palette, nextColorIndex);

            //if we're adding a blank color at the end of the cycle, we need to to catch the end
            //since the palette doesn't include the blank color
            //so we have the case where we're transitioning from the last palette color to the blank color
            //and the case after where we're transitioning from the blank color back to the start of the palette
            if(addBlank){
                if(curPatIndex + 1 == patternLen){
                    //going from the end of the palette to the blank color
                    nextColor = *blankColor;
                } else if(curPatIndex == patternLen){
                    //going from the blankColor to the start of the palette
                    currentColor = *blankColor;
                    nextColorIndex = patternUtilsPS::getPatternVal(pattern, 0); 
                    nextColor = paletteUtilsPS::getPaletteColor(palette, nextColorIndex);
                }
            }
            
            //get the cross faded color and write it out
            color = colorUtilsPS::getCrossFadeColor(currentColor, nextColor, gradStep, gradLength);
            segDrawUtils::drawSegLineSimple(segmentSet, i, color, 0);

            // randomly increment the offset (keeps the effect varied)
            if (randomShift) {
                if (random8(100) <= shiftThreshold) {
                    offsets[i] = addMod16PS( offsets[i], random8(1, shiftStep), totalCycleLength );
                }
            }
        }

        //increment the cycle, clamping it's max value to prevent any overflow
        cycleNum = addMod16PS(cycleNum, 1, totalCycleLength);
        
        showCheckPS();
    }
}

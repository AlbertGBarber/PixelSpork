#include "GradientCycleSL.h"

//constructor with pattern
GradientCycleSL::GradientCycleSL(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette,
                                 uint8_t GradLength, uint16_t Rate)
    : pattern(&Pattern), palette(&Palette), gradLength(GradLength)  //
{
    init(SegSet, Rate);
}

//constructor with palette as pattern
GradientCycleSL::GradientCycleSL(SegmentSetPS &SegSet, palettePS &Palette, uint8_t GradLength, uint16_t Rate)
    : palette(&Palette), gradLength(GradLength)  //
{
    setPaletteAsPattern();
    init(SegSet, Rate);
}

//constructor with random colors
GradientCycleSL::GradientCycleSL(SegmentSetPS &SegSet, uint8_t NumColors, uint8_t GradLength, uint16_t Rate)
    : gradLength(GradLength)  //
{
    paletteTemp = paletteUtilsPS::makeRandomPalette(NumColors);
    palette = &paletteTemp;
    setPaletteAsPattern();
    init(SegSet, Rate);
}

GradientCycleSL::~GradientCycleSL() {
    free(paletteTemp.paletteArr);
    free(patternTemp.patternArr);
}

//inits core variables for the effect
void GradientCycleSL::init(SegmentSetPS &SegSet, uint16_t Rate) {
    //bind the rate and segSet pointer vars since they are inherited from BaseEffectPS
    bindSegSetPtrPS();
    bindClassRatesPS();
    cycleNum = 0;
}

//sets the pattern to match the current palette
//ie for a palette length 5, the pattern would be
//{0, 1, 2, 3, 4}
void GradientCycleSL::setPaletteAsPattern() {
    generalUtilsPS::setPaletteAsPattern(patternTemp, *palette);
    pattern = &patternTemp;
}

//Updates the effect
//Each update we run across all the segment lines and calculate the gradient color based on the cycleNum
//The cycleNum varies from 0 to totalCycleLength, so each led will go through each color in all the gradients one time per cycle
//based on the cycleNum, we work out the color we started at, and which gradient step we're on
//Then we compute the blended color and output it
void GradientCycleSL::update() {
    currentTime = millis();

    if( (currentTime - prevTime) >= *rate ) {
        prevTime = currentTime;

        //we set the segment vars here since the pixel locations depend on them
        numLines = segSet->numLines;
        
        //calculates the totalCycleLength, which represents the total number of possible colors a pixel can have
        //ie the total length of all the color gradients combined
        //We re-calculate this each loop to allow you to freely change the patter and gradLength
        totalCycleLength = pattern->length * gradLength;

        //In the loop below, we only pick new colors once blendStep is 0
        //due to cycleNum, blendStep is not always 0 for the first pixel
        //so we need to pre-pick colors for it (otherwise the colors from the previous update cycle would be used)
        setNextColors(0);

        for( uint16_t i = 0; i < numLines; i++ ) {

            blendStep = addmod8(cycleNum, i, gradLength);  // what step we're on between the current and next color
            //If the blendStep is 0, then a gradient has finished, and we need to choose the next color
            if( blendStep == 0 ) {
                setNextColors(i);
            }

            colorOut = colorUtilsPS::getCrossFadeColor(currentColor, nextColor, blendStep, gradLength);
            //write the color out to all the leds in the segment line
            //(we used numLines - 1 - i in place of just i to make the default line motion positive)
            segDrawUtils::drawSegLine(*segSet, numLines - 1 - i, colorOut, 0);
        }

        cycleNum = addMod16PS(cycleNum, 1, totalCycleLength);

        showCheckPS();
    }
}

//sets the current and next colors for the gradient based on the line number, and how many cycles we've gone through
void GradientCycleSL::setNextColors(uint16_t lineNum) {
    step = addMod16PS(lineNum, cycleNum, totalCycleLength);  // where we are in the cycle of all the colors
    currentColorIndex = step / gradLength;                   // what color we've started from (integers always round down)
    //the color we're at based on the current index
    currentPattern = patternUtilsPS::getPatternVal(*pattern, currentColorIndex);
    currentColor = paletteUtilsPS::getPaletteColor(*palette, currentPattern);
    //the next color, wrapping to the start of the pattern as needed
    nextPattern = patternUtilsPS::getPatternVal(*pattern, currentColorIndex + 1);
    nextColor = paletteUtilsPS::getPaletteColor(*palette, nextPattern);
}
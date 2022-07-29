#include "GradientCyclePS.h"

//constructor with pattern
GradientCyclePS::GradientCyclePS(SegmentSet &SegmentSet, patternPS *Pattern, palettePS *Palette, uint8_t GradLength, uint16_t Rate):
    segmentSet(SegmentSet), pattern(Pattern), palette(Palette), gradLength(GradLength)
    {    
        init(Rate);
	}

//constuctor with palette as pattern
GradientCyclePS::GradientCyclePS(SegmentSet &SegmentSet, palettePS *Palette, uint8_t GradLength, uint16_t Rate):
    segmentSet(SegmentSet), palette(Palette), gradLength(GradLength)
    {    
        setPaletteAsPattern();
        init(Rate);
	}

//constructor with random colors
GradientCyclePS::GradientCyclePS(SegmentSet &SegmentSet, uint8_t NumColors, uint8_t GradLength, uint16_t Rate):
    segmentSet(SegmentSet), gradLength(GradLength)
    {    
        paletteTemp = paletteUtilsPS::makeRandomPalette(NumColors);
        palette = &paletteTemp;
        setPaletteAsPattern();
        init(Rate);
	}

GradientCyclePS::~GradientCyclePS(){
    delete[] paletteTemp.paletteArr;
    delete[] patternTemp.patternArr;
}

//inits core variables for the effect
void GradientCyclePS::init(uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    cycleNum = 0;
    setTotalEffectLength();
}

//sets the gradLength
//we need to change the totalCycleLength to match
void GradientCyclePS::setGradLength(uint8_t newGradLength){
    gradLength = newGradLength;
    setTotalEffectLength();
}

//sets a new pattern for the effect
//we need to change the totalCycleLength to match
void GradientCyclePS::setPattern(patternPS *newPattern){
    pattern = newPattern;
    setTotalEffectLength();
}

//binds the palette to a new one
//don't need to change the totalCycleLength since the palette is referenced
//only indirectly though the pattern
void GradientCyclePS::setPalette(palettePS* newPalette){
    palette = newPalette;
}

//sets the pattern to match the current palette
//ie for a palette length 5, the pattern would be 
//{0, 1, 2, 3, 4}
void GradientCyclePS::setPaletteAsPattern(){
    patternTemp = generalUtilsPS::setPaletteAsPattern(palette);
    pattern = &patternTemp;
    setTotalEffectLength();
}

//caculates the totalCycleLength, which represents the total number of possible colors a pixel can have
//ie the total length of all the color gradients combined
void GradientCyclePS::setTotalEffectLength(){
    // the number of steps in a full cycle (fading through all the colors)
    totalCycleLength = pattern->length * gradLength;
}

//Updates the effect
//Each update we run across all the LEDs and caculate the gradient color based on the cycleNum
//The cycleNum varies from 0 to totalCycleLength, so each led will go through each color in all the gradients one time per cycle
//based on the cycleNum, we work out the color we started at, and which gradient step we're on
//Then we compute the blended color and output it
void GradientCyclePS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        numPixels = segmentSet.numActiveSegLeds;

        //In the loop below, we only pick new colors once blendStep is 0
        //due to cycleNum, blendStep is not always 0 for the first pixel
        //so we need to pre-pick colors for it (otherwise the colors from the previous update cycle would be used)
        setnextColors(0);
       
        for (uint16_t i = 0; i < numPixels; i++) {

            blendStep = addmod8(cycleNum, i, gradLength);//(cycleNum + i) % gradLength; // what step we're on between the current and next color
            //If the blendStep is 0, then a gradient has finished, and we need to choose the next color
            if( blendStep == 0 ){
                setnextColors(i);
            }

            colorOut = colorUtilsPS::getCrossFadeColor(currentColor, nextColor, blendStep, gradLength);
            segDrawUtils::setPixelColor(segmentSet, i, colorOut, 0);
        }

        cycleNum = addMod16PS( cycleNum, 1, totalCycleLength ); //(cycleNum + 1) % totalCycleLength

        showCheckPS();
    }
}

//sets the current and next colors for the gradient based on the led number, and how many cycles we've gone through
void GradientCyclePS::setnextColors(uint16_t pixelNum){
    step = addMod16PS(pixelNum, cycleNum, totalCycleLength); //( pixelNum + cycleNum ) % totalCycleLength; // where we are in the cycle of all the colors
    currentColorIndex = step / gradLength; // what color we've started from (integers always round down)
    //the color we're at based on the current index
    currentPattern = patternUtilsPS::getPatternVal(pattern, currentColorIndex);
    currentColor = paletteUtilsPS::getPaletteColor(palette, currentPattern);
    //the next color, wrapping to the start of the pattern as needed
    nextPattern = patternUtilsPS::getPatternVal(pattern, currentColorIndex + 1);
    nextColor = paletteUtilsPS::getPaletteColor(palette, nextPattern);
}
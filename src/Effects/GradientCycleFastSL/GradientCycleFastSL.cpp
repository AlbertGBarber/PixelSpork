#include "GradientCycleFastSL.h"

//constructor with pattern
GradientCycleFastSL::GradientCycleFastSL(SegmentSet &SegSet, patternPS &Pattern, palettePS &Palette, uint8_t GradLength, uint16_t Rate):
    SegSet(SegSet), pattern(&Pattern), palette(&Palette), gradLength(GradLength)
    {    
        init(Rate);
	}

//constructor with palette as pattern
GradientCycleFastSL::GradientCycleFastSL(SegmentSet &SegSet, palettePS &Palette, uint8_t GradLength, uint16_t Rate):
    SegSet(SegSet), palette(&Palette), gradLength(GradLength)
    {    
        setPaletteAsPattern();
        init(Rate);
	}

//constructor with palette of randomly chosen colors
//(does not set randColors or randColor mode)
GradientCycleFastSL::GradientCycleFastSL(SegmentSet &SegSet, uint8_t NumColors, uint8_t GradLength, uint16_t Rate):
    SegSet(SegSet), gradLength(GradLength)
    {    
        paletteTemp = paletteUtilsPS::makeRandomPalette(NumColors);
        palette = &paletteTemp;
        setPaletteAsPattern();
        init(Rate);
	}

GradientCycleFastSL::~GradientCycleFastSL(){
    free(paletteTemp.paletteArr);
    free(patternTemp.patternArr);
}

//inits core variables for the effect
void GradientCycleFastSL::init(uint16_t Rate){
    //bind the rate and SegSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
}

//restarts the effect
void GradientCycleFastSL::reset(){
    initFillDone = false;
}

//sets the pattern to match the current palette
//ie for a palette length 5, the pattern would be 
//{0, 1, 2, 3, 4}
void GradientCycleFastSL::setPaletteAsPattern(){
    patternTemp = generalUtilsPS::setPaletteAsPattern(*palette);
    pattern = &patternTemp;
}

//We need to pre-fill the strip with the first set of gradients
//in order for the led colors to be copied properly in the main update cycle
void GradientCycleFastSL::initalFill(){
    cycleNum = 0;
    patternCount = 0;
    nextPattern = patternUtilsPS::getPatternVal(*pattern, 0);
    nextColor = paletteUtilsPS::getPaletteColor(*palette, nextPattern);

    //fetch some core vars
    numLines = SegSet.numLines;
    numLinesLim = numLines - 1;
    longestSeg = SegSet.segNumMaxNumLines;

    //We need to draw the initial gradients on the strip to pre-fill it for the main update cycle.
    //To do this we run across all the leds.
    //Every gradSteps number of leds, we rotate the gradient colors, transitioning from the current color to the next
    //We run across them backwards so that the end pattern runs positively along the strip
    //The loop direction must match that in the update() function, so that the inital output 
    //runs in the same direction as when the effect is updated
    for (int32_t i = numLinesLim; i >= 0 ; i--) {

        // if we've gone through gradLength cycles
        // a color transition is finished and we need to move to the next color
        if (cycleNum == 0) {
            pickNextColor();
        }

        // get the cross-fade between the current and next color, where the transition is gradSteps long
        colorOut = colorUtilsPS::getCrossFadeColor(currentColor, nextColor, cycleNum, gradLength);
        //Draw the colored line
        segDrawUtils::drawSegLine(SegSet, i, colorOut, 0);

        cycleNum = addmod8(cycleNum, 1, gradLength);//track what step we're on in the gradient
    }
    initFillDone = true;
}

//The update cycle
//Every gradLength steps we switch to the next color,
//To avoid re-calculating the same blend repeatedly, we only work out the blend for the first line in the segment set
//since this is the only new color entering the strip
//For all the other lines we simply copy the color of the line in front of it
//To copy the color we always copy from the pixel on the longest segment,
//Since all the pixels on the longest segment are on separate lines
//(unlike shorter segments, where a single pixel can be in multiple lines, so it's color may not be what we expect)
//This way we shift the gradients along the strip
//The gradient we're on is tracked by cycleNum and patternCount, which vary from 0 to gradLent,
//and 0 to the pattern length respectively
//(see notes for the restrictions this method causes)
void GradientCycleFastSL::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        //fetch some core vars
        numLines = SegSet.numLines;
        numLinesLim = numLines - 1;
        longestSeg = SegSet.segNumMaxNumLines;

        //We need to pre-fill the strip with a full cycle the first time the update is called
        //so that the colors are copied down the strip correctly on subsequent cycles
        if(!initFillDone){
            initalFill();
        }

        //Run backwards across all the segment lines and copy the color from the line before it
        //Running backwards makes the pattern move in the positive direction (forwards)
        //If we're at the first line, we pick new color
        for (int32_t i = numLinesLim; i >= 0; i--) {
            if (i == 0) {
                //we only need to pick new colors once a full gradient transition is done
                //our step in the gradient is marked by cycleNum, so once it reaches 0
                //a gradient has finished (this is esp important for random modes, since we don't know what the colors are)
                if(cycleNum == 0){
                    pickNextColor();
                }
                colorOut = colorUtilsPS::getCrossFadeColor(currentColor, nextColor, cycleNum, gradLength);
                segDrawUtils::drawSegLine(SegSet, i, colorOut, 0);
            } else {
                //Copy the pixel color from the previous line
                //To copy the color we always copy from the pixel on the longest segment,
                //Since all the pixels on the longest segment are on separate lines
                //(unlike shorter segments, where a single pixel can be in multiple lines, so it's color may not be what we expect)
                pixelNum = segDrawUtils::getPixelNumFromLineNum(SegSet, numLines, longestSeg, i - 1);
                colorOut = SegSet.leds[pixelNum];
                
                //write out the copied color to the whole line
                segDrawUtils::drawSegLine(SegSet, i, colorOut, 0);
            }
        }

        cycleNum = addMod16PS(cycleNum, 1, gradLength);//track what step we're on in the gradient
        showCheckPS();
    }
}

//For calling whenever a gradient has finished
//sets the currentColor to the color we've just transitioned to (ie the nextColor)
//and then chooses a new color for the the nextColor depending on the the options for random colors
void GradientCycleFastSL::pickNextColor(){
    //advance the current pattern and colors to the next pattern and color
    //since we've just transitioned to them
    currentPattern = nextPattern;
    currentColor = nextColor;
    //advance the pattern count,
    //even with random colors, I think you still need to do this for the initalFill?
    //idk, I'm tired today and it seemed to fix a bug where a gradient would start half-filled
    patternCount = addMod16PS(patternCount, 1, pattern->length);  
    if(randMode == 0){
        //if we're not choosing a random color, we need to pick the next color in the pattern
        nextPattern = patternUtilsPS::getPatternVal(*pattern, patternCount);
        //the next color, wrapping to the start of the pattern as needed
        nextColor = paletteUtilsPS::getPaletteColor(*palette, nextPattern );
    } else if(randMode == 1){
        //choose a completely random color
        nextColor = colorUtilsPS::randColor();
    } else {
        //choose a color randomly from the pattern (making sure it's not the same as the current color)
        nextPattern = patternUtilsPS::getShuffleVal(*pattern, currentPattern);
        nextColor = paletteUtilsPS::getPaletteColor( *palette, nextPattern );  
    }
}
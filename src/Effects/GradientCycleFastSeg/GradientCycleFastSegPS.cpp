#include "GradientCycleFastSegPS.h"

//constructor with pattern
GradientCycleFastSegPS::GradientCycleFastSegPS(SegmentSet &SegmentSet, patternPS *Pattern, palettePS *Palette, uint8_t GradLength, bool Direct, uint16_t Rate):
    segmentSet(SegmentSet), pattern(Pattern), palette(Palette), gradLength(GradLength), direct(Direct)
    {    
        init(Rate);
	}

//constuctor with palette as pattern
GradientCycleFastSegPS::GradientCycleFastSegPS(SegmentSet &SegmentSet, palettePS *Palette, uint8_t GradLength, bool Direct, uint16_t Rate):
    segmentSet(SegmentSet), palette(Palette), gradLength(GradLength), direct(Direct)
    {    
        setPaletteAsPattern();
        init(Rate);
	}

//constructor with palette of randomly choosen colors
//(does not set randColors or randColor mode)
GradientCycleFastSegPS::GradientCycleFastSegPS(SegmentSet &SegmentSet, uint8_t NumColors, uint8_t GradLength, bool Direct, uint16_t Rate):
    segmentSet(SegmentSet), gradLength(GradLength), direct(Direct)
    {    
        paletteTemp = paletteUtilsPS::makeRandomPalette(NumColors);
        palette = &paletteTemp;
        setPaletteAsPattern();
        init(Rate);
	}

GradientCycleFastSegPS::~GradientCycleFastSegPS(){
    delete[] paletteTemp.paletteArr;
    delete[] patternTemp.patternArr;
}

//inits core variables for the effect
void GradientCycleFastSegPS::init(uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    setDirection(direct);
}

//restarts the effect
void GradientCycleFastSegPS::reset(){
    initFillDone = false;
}

//sets the pattern to match the current palette
//ie for a palette length 5, the pattern would be 
//{0, 1, 2, 3, 4}
void GradientCycleFastSegPS::setPaletteAsPattern(){
    patternTemp = generalUtilsPS::setPaletteAsPattern(palette);
    pattern = &patternTemp;
}

//sets the direction of the effect to either move from the first to last segment or visa versa
//(true is last segment to first)
void GradientCycleFastSegPS::setDirection(bool newDirect){
    direct = newDirect;
    numSegs = segmentSet.numSegs;

    //loop direction and limit variables for the update cycle
    //For this loop, we're moving from the last to first
    //so the startLimit is 0 (first segment), the endLimit is numSegs (last segment)
    //and the step is 1
    //This is because the segment color is coppied from the next segment in line
    //The colored segment is the last segment, since that is the origin of new colors
    endLimit = numSegs;
    startLimit = 0;
    loopStep = 1;
    coloredSeg = numSegs - 1;

    //switch the loop variables if we want first segment to last
    //These are basically the opposite of the per last to first settings
    if (!direct) {
        endLimit = -1;
        startLimit = numSegs - 1;
        loopStep = -1;
        coloredSeg = 0;
    }
}

//We need to pre-fill the strip with the first set of gradients
//in order for the led colors to be copied properly in the main update cycle
void GradientCycleFastSegPS::initalFill(){
    cycleNum = 0;
    patternCount = 0;
    nextPattern = patternUtilsPS::getPatternVal(pattern, 0);
    nextColor = paletteUtilsPS::getPaletteColor(palette, nextPattern);

    //We need to draw the initial gradients on the strip to pre-fill it for the main update cycle.
    //To do this we run across all the segments.
    //Every gradsteps number of segments, we rotate the gradient colors, transitioning from the current color to the next
    //The loop limits and step are determined by the effect direction (setDirection())
    //So that the loop either runs from the first to last segment, or visa versa
    for (int16_t i = startLimit; i != endLimit ; i += loopStep) {

        // if we've gone through gradLength cycles
        // a color transition is finished and we need to move to the next color
        if (cycleNum == 0) {
            pickNextColor();
        }

        // get the crossfade between the current and next color, where the transition is gradsteps long
        colorOut = colorUtilsPS::getCrossFadeColor(currentColor, nextColor, cycleNum, gradLength);
        //Color the segment
        segDrawUtils::fillSegColor(segmentSet, i, colorOut, 0);

        cycleNum = addmod8(cycleNum, 1, gradLength);//track what step we're on in the gradient
    }
    initFillDone = true;
}

//The update cycle
//Every gradLength steps we switch to the next color,
//To avoid re-caculating the same blend repeatedly, we only work out the blend for the first (or last) segment 
//since this is the only new color entering the strip
//For all the other segments we simply copy the color of the segment in front (or behind) of it
//To copy the color we always copy from the first pixel of each segment
//The effect direction determines in which direction we copy from (see setDirection())
//The gradient we're on is tracked by cycleNum and patternCount, which vary from 0 to gradLent,
//and 0 to the pattern length respectively
//(see notes for the restrictions this method causes)
void GradientCycleFastSegPS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        //We need to pre-fill the strip with a full cycle the first time the update is called
        //so that the colors are copied down the strip correctly on subsequent cycles
        if(!initFillDone){
            initalFill();
        }

        //Loop through all the segments, either copying the color or the next segment, 
        //or setting the color if it's the "first" segment
        //To change the direction of motion we adjust the loop start and end points, and the loop step
        //So that we either move from the first to last segment or visa versa
        //(see setDirection())
        for (int16_t i = startLimit; i != endLimit; i += loopStep) {
            //If the segment is the origin segment, we need to pick a new color
            //otherwise we just copy from the next segment
            if (i == coloredSeg) {
                //we only need to pick new colors once a full gradient transition is done
                //our step in the gradient is marked by cycleNum, so once it reaches 0
                //a gradient has finished (this is esp important for random modes, since we don't know what the colors are)
                if(cycleNum == 0){
                    pickNextColor();
                }
                colorOut = colorUtilsPS::getCrossFadeColor(currentColor, nextColor, cycleNum, gradLength);
                //Color the segment
                segDrawUtils::fillSegColor(segmentSet, i, colorOut, 0);
            } else {
                //Copy the pixel color from the previous segment, based on the direction (loopStep)
                //To copy the color we always copy from the first pixel in the previous segment
                pixelNum = segDrawUtils::getSegmentPixel(segmentSet, i + loopStep, 0);
                colorOut = segmentSet.leds[pixelNum];
                
                //Color the segment
                segDrawUtils::fillSegColor(segmentSet, i, colorOut, 0);
            }
        }

        cycleNum = addMod16PS(cycleNum, 1, gradLength);//track what step we're on in the gradient
        showCheckPS();
    }
}

//For calling whenever a gradient has finished
//sets the currentColor to the color we've just transitioned to (ie the nextColor)
//and then chooses a new color for the the nextColor depending on the the options for random colors
void GradientCycleFastSegPS::pickNextColor(){
    //advance the current pattern and colors to the next pattern and color
    //since we've just transitioned to them
    currentPattern = nextPattern;
    currentColor = nextColor;
    //advance the pattern count,
    //even with random colors, I think you still need to do this for the initalFill?
    //idk, I'm tired today and it seemed to fix a bug where a gradient would start half-filled
    patternCount = (patternCount + 1) % pattern->length;
    if(randMode == 0){
        //if we're not choosing a random color, we need to pick the next color in the pattern
        nextPattern = patternUtilsPS::getPatternVal(pattern, patternCount);
        //the next color, wrapping to the start of the pattern as needed
        nextColor = paletteUtilsPS::getPaletteColor(palette, nextPattern );
    } else if(randMode == 1){
        //choose a completely random color
        nextColor = colorUtilsPS::randColor();
    } else {
        //choose a color randomly from the pattern (making sure it's not the same as the current color)
        nextPattern = patternUtilsPS::getShuffleIndex(pattern, currentPattern);
        nextColor = paletteUtilsPS::getPaletteColor( palette, nextPattern );  
    }
}
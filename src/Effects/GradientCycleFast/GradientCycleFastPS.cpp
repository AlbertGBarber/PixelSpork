#include "GradientCycleFastPS.h"

//constructor with pattern
GradientCycleFastPS::GradientCycleFastPS(SegmentSet &SegmentSet, patternPS *Pattern, palletPS *Pallet, uint8_t GradLength, uint16_t Rate):
    segmentSet(SegmentSet), pattern(Pattern), pallet(Pallet), gradLength(GradLength)
    {    
        init(Rate);
	}

//constuctor with pallet as pattern
GradientCycleFastPS::GradientCycleFastPS(SegmentSet &SegmentSet, palletPS *Pallet, uint8_t GradLength, uint16_t Rate):
    segmentSet(SegmentSet), pallet(Pallet), gradLength(GradLength)
    {    
        setPalletAsPattern();
        init(Rate);
	}

//constructor with pallet of randomly choosen colors
//(does not set randColors or randColor mode)
GradientCycleFastPS::GradientCycleFastPS(SegmentSet &SegmentSet, uint8_t NumColors, uint8_t GradLength, uint16_t Rate):
    segmentSet(SegmentSet), gradLength(GradLength)
    {    
        palletTemp = EffectUtilsPS::makeRandomPallet(NumColors);
        pallet = &palletTemp;
        setPalletAsPattern();
        init(Rate);
	}

GradientCycleFastPS::~GradientCycleFastPS(){
    delete[] palletTemp.palletArr;
    delete[] patternTemp.patternArr;
}

//inits core variables for the effect
void GradientCycleFastPS::init(uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
}

//restarts the effect
void GradientCycleFastPS::reset(){
    initFillDone = false;
}

//sets the pattern to match the current pallet
//ie for a pallet length 5, the pattern would be 
//{0, 1, 2, 3, 4}
void GradientCycleFastPS::setPalletAsPattern(){
    patternTemp = EffectUtilsPS::setPalletAsPattern(pallet);
    pattern = &patternTemp;
}

//sets a new pattern for the effect
void GradientCycleFastPS::setPattern(patternPS *newPattern){
    pattern = newPattern;
}

//binds the pallet to a new one
void GradientCycleFastPS::setPallet(palletPS* newPallet){
    pallet = newPallet;
}

//We need to pre-fill the strip with the first set of gradients
//in order for the led colors to be copied properly in the main update cycle
void GradientCycleFastPS::initalFill(){
    cycleNum = 0;
    patternCount = 0;
    nextPattern = patternUtilsPS::getPatternVal(pattern, 0);
    nextColor = palletUtilsPS::getPalletColor(pallet, nextPattern );

    //numPixels is the loop limit below, so we subtract 1
    numPixels = segmentSet.numActiveSegLeds - 1;

    //we need to draw the initial gradients on the strip
    //to pre-fill it for the main update cycle
    //to do this we run across all the leds
    //every gradsteps number of leds, we rotate the gradient colors,
    //transitioning from the current color to the next
    //we loop forwards to match the direction of the loop in update()
    //so that where this initial setup ends, 
    //the patternCount and cycleNum vars will be correct
    for (uint16_t i = 0; i <= numPixels; i++) {

        // if we've gone through gradLength cycles
        // a color transition is finished and we need to move to the next color
        if (cycleNum == 0) {
            pickNextColor();
        }

        // get the crossfade between the current and next color, where the transition is gradsteps long
        colorOut = segDrawUtils::getCrossFadeColor(currentColor, nextColor, cycleNum, gradLength);
        segDrawUtils::setPixelColor(segmentSet, i, colorOut, 0);

        cycleNum = (cycleNum + 1) % gradLength; //track what step we're on in the gradient
    }
    initFillDone = true;
}

//The update cycle
//Every gradLength steps we switch to the next color,
//To avoid re-caculating the same blend repeatedly, we only work out the blend for the first pixel in the strip
//since this is the only new color entering the strip
//For all the other pixels we simply copy the color of the pixel in front of it
//This way we shift the gradients along the strip
//The gradient we're on is tracked by cycleNum and patternCount, which vary from 0 to gradLent,
//and 0 to the pattern length respectively
//(see notes for the restrictions this method causes)
void GradientCycleFastPS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        //We need to pre-fill the strip with a full cycle the first time the update is called
        //so that the colors are copied down the strip correctly on subsequent cycles
        if(!initFillDone){
            initalFill();
        }

        //numPixels is the loop limit below, so we subtract 1
        numPixels = segmentSet.numActiveSegLeds - 1;

        //prep for the loop below.
        //The first pixel we need to copy into is the last pixel in the strip
        nextPixelNumber = segDrawUtils::getSegmentPixel(segmentSet, 0);

        //cycle along the strip, shifting the led colors forward by coping from the next led in line
        //Once we reach the last led we set it to the next transistion color
        //So we steadily copy the graident waves along the strip
        for (uint16_t i = 0; i <= numPixels; i++) {
            //The nextPixelNumber from the previous loop iteration is now
            //the pixelNumber for this iteration
            pixelNumber = nextPixelNumber;
            nextPixelNumber = segDrawUtils::getSegmentPixel(segmentSet, i + 1);
            if (i == numPixels) {
                //we only need to pick new colors once a full gradient transition is done
                //our step in the gradient is marked by cycleNum, so once it reaches 0
                //a gradient has finished (this is esp important for random modes, since we don't know what the colors are)
                if(cycleNum == 0){
                    pickNextColor();
                }
                colorOut = segDrawUtils::getCrossFadeColor(currentColor, nextColor, cycleNum, gradLength);
                segDrawUtils::setPixelColor(segmentSet, pixelNumber, colorOut, 0, 0, 0);
            } else {
                //copy the color of the next pixel in line into the current pixel
                segmentSet.leds[pixelNumber] = segmentSet.leds[nextPixelNumber];
            }
        }

        cycleNum = (cycleNum + 1) % gradLength; //track what step we're on in the gradient
        showCheckPS();
    }
}

//For calling whenever a gradient has finished
//sets the currentColor to the color we've just transitioned to (ie the nextColor)
//and then chooses a new color for the the nextColor depending on the the options for random colors
void GradientCycleFastPS::pickNextColor(){
    //advance the current pattern and colors to the next pattern and color
    //since we've just transitioned to them
    currentPattern = nextPattern;
    currentColor = nextColor;
    //advance the pattern count,
    //even with random colors, I think you still need to do this for the initalFill?
    //idk, I'm tired today and it seemed to fix a bug where a gradient would start half-filled
    patternCount = (patternCount + 1) % pattern->length;
    if(!randColors){
        //if we're not choosing a random color, we need to pick the next color in the pattern
        nextPattern = patternUtilsPS::getPatternVal(pattern, patternCount);
        //the next color, wrapping to the start of the pattern as needed
        nextColor = palletUtilsPS::getPalletColor(pallet, nextPattern );
    } else {
        //choose the next color based on the random mode
        if(randMode == 0){
            //choose a completely random color
            nextColor = segDrawUtils::randColor();
        } else {
            //choose a color randomly from the pattern (making sure it's not the same as the current color)
            nextPattern = EffectUtilsPS::shuffleIndex(pattern, currentPattern);
            nextColor = palletUtilsPS::getPalletColor( pallet, nextPattern );
        }
    }
}
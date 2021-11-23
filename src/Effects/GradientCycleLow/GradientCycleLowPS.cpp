#include "GradientCycleLowPS.h"

//constructor with pattern
GradientCycleLowPS::GradientCycleLowPS(SegmentSet &SegmentSet, patternPS *Pattern, palletPS *Pallet, uint8_t GradLength, uint16_t Rate):
    segmentSet(SegmentSet), pattern(Pattern), pallet(Pallet), gradLength(GradLength)
    {    
        init(Rate);
	}

//constuctor with pallet as pattern
GradientCycleLowPS::GradientCycleLowPS(SegmentSet &SegmentSet, palletPS *Pallet, uint8_t GradLength, uint16_t Rate):
    segmentSet(SegmentSet), pallet(Pallet), gradLength(GradLength)
    {    
        setPalletAsPattern();
        init(Rate);
	}

//constructor with random colors
GradientCycleLowPS::GradientCycleLowPS(SegmentSet &SegmentSet, uint8_t NumColors, uint8_t GradLength, uint16_t Rate):
    segmentSet(SegmentSet), gradLength(GradLength)
    {    
        palletTemp = EffectUtilsPS::makeRandomPallet(NumColors);
        pallet = &palletTemp;
        setPalletAsPattern();
        init(Rate);
	}

GradientCycleLowPS::~GradientCycleLowPS(){
    delete[] palletTemp.palletArr;
    delete[] patternTemp.patternArr;
}

//inits core variables for the effect
void GradientCycleLowPS::init(uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
}

void GradientCycleLowPS::reset(){
    initFillDone = false;
}

//sets the pattern to match the current pallet
//ie for a pallet length 5, the pattern would be 
//{0, 1, 2, 3, 4}
void GradientCycleLowPS::setPalletAsPattern(){
    patternTemp = EffectUtilsPS::setPalletAsPattern(pallet);
    pattern = &patternTemp;
}

//sets a new pattern for the effect
void GradientCycleLowPS::setPattern(patternPS *newPattern){
    pattern = newPattern;
}

//binds the pallet to a new one
void GradientCycleLowPS::setPallet(palletPS* newPallet){
    pallet = newPallet;
}

//We need to pre-fill the strip with the first set of gradients
//in order for the led colors to be copied properly in the main update cycle
void GradientCycleLowPS::initalFill(){
    cycleNum = 0;
    patternCount = 0;
    nextPattern = patternUtilsPS::getPatternVal(pattern, 0);

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
            patternCount = (patternCount + 1) % pattern->length;
            currentPattern = nextPattern;
            nextPattern = patternUtilsPS::getPatternVal(pattern, patternCount);
        }

        //the color we're at based on the current index
        currentColor = palletUtilsPS::getPalletColor(pallet, currentPattern);
        //the next color, wrapping to the start of the pattern as needed
        nextColor = palletUtilsPS::getPalletColor(pallet, nextPattern );
        // get the crossfade between the current and next color, where the transition is gradsteps long
        colorOut = segDrawUtils::getCrossFadeColor(currentColor, nextColor, cycleNum, gradLength);
        segDrawUtils::setPixelColor(segmentSet, i, colorOut, 0);

        cycleNum = (cycleNum + 1) % gradLength;
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
void GradientCycleLowPS::update(){
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

        //if we've gone through gradLengths cycles
        //a color gradient is finished and we need to move to the next color
        if (cycleNum == 0) {
            patternCount = (patternCount + 1) % pattern->length;
            currentPattern = nextPattern;
            nextPattern = patternUtilsPS::getPatternVal(pattern, patternCount);
        }

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
                //the color we're at based on the current index
                currentColor = palletUtilsPS::getPalletColor(pallet, currentPattern);
                //the next color, wrapping to the start of the pattern as needed
                nextColor = palletUtilsPS::getPalletColor(pallet, nextPattern );
                colorOut = segDrawUtils::getCrossFadeColor(currentColor, nextColor, cycleNum, gradLength);
                segDrawUtils::setPixelColor(segmentSet, pixelNumber, colorOut, 0, 0, 0);
            } else {
                //copy the color of the next pixel in line into the current pixel
                segmentSet.leds[pixelNumber] = segmentSet.leds[nextPixelNumber];
            }
        }

        cycleNum = (cycleNum + 1) % gradLength;
        showCheckPS();
    }
}
#include "GradientCyclePS.h"

//constructor with pattern
GradientCyclePS::GradientCyclePS(SegmentSet &SegmentSet, uint8_t *Pattern, uint8_t PatternLength, palletPS *Pallet, uint8_t GradLength, uint16_t Rate):
    segmentSet(SegmentSet), pattern(Pattern), patternLength(PatternLength), pallet(Pallet), gradLength(GradLength)
    {    
        init(Rate);
	}

//constuctor with pallet as pattern
GradientCyclePS::GradientCyclePS(SegmentSet &SegmentSet, palletPS *Pallet, uint8_t GradLength, uint16_t Rate):
    segmentSet(SegmentSet), pallet(Pallet), gradLength(GradLength)
    {    
        setPalletAsPattern(pallet);
        init(Rate);
	}

//constructor with random colors
GradientCyclePS::GradientCyclePS(SegmentSet &SegmentSet, uint8_t NumColors, uint8_t GradLength, uint16_t Rate):
    segmentSet(SegmentSet), gradLength(GradLength)
    {    
        palletTemp = EffectUtilsPS::makeRandomPallet(NumColors);
        pallet = &palletTemp;
        setPalletAsPattern(pallet);
        init(Rate);
	}

GradientCyclePS::~GradientCyclePS(){
    delete[] palletTemp.palletArr;
    //only delete the pattern if it's using a temp array
    if(tempPatternSet){
        delete[] pattern;
    }
}

//inits core variables for the effect
void GradientCyclePS::init(uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    cycleNum = 0;
    setTotalEffectLength();
}

//sets the pattern to match the passed in pallet
//ie pattern color 1 is pallet color 1, etc
void GradientCyclePS::setPalletAsPattern(palletPS *newPallet){
    tempPatternSet = true; //flag for destructor, since the pattern is being allocated with new
    pallet = newPallet;
    patternLength = pallet->length;
    pattern = new uint8_t[patternLength];
    for(uint8_t i = 0; i < patternLength; i++){
        pattern[i] = i;
    }
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
void GradientCyclePS::setPattern(uint8_t *newPattern, uint8_t newPatternLength){
    pattern = newPattern;
    patternLength = newPatternLength;
    tempPatternSet = false; //set flag to avoid deleting the pattern in the destuctor
    setTotalEffectLength();
}

//binds the pallet to a new one
//don't need to change the totalCycleLength since the pallet is referenced
//only indirectly though the pattern
void GradientCyclePS::setPallet(palletPS* newPallet){
    pallet = newPallet;
}

//caculates the totalCycleLength, which represents the total number of possible colors a pixel can have
//ie the total length of all the color gradients combined
void GradientCyclePS::setTotalEffectLength(){
    // the number of steps in a full cycle (fading through all the colors)
    totalCycleLength = patternLength * gradLength;
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
       
        for (uint16_t i = 0; i < numPixels; i++) {

            step = (cycleNum + i) % totalCycleLength; // where we are in the cycle of all the colors
            blendStep = (cycleNum + i) % gradLength; // what step we're on between the current and next color
            currentColorIndex = step / gradLength; // what color we've started from (integers always round down)
            
            //the color we're at based on the current index
            currentColor = palletUtilsPS::getPalletColor(pallet, pattern [ currentColorIndex ]);
            //the next color, wrapping to the start of the pattern as needed
            nextColor = palletUtilsPS::getPalletColor(pallet, pattern [ (currentColorIndex + 1) % patternLength ] );
            colorOut = segDrawUtils::getCrossFadeColor(currentColor, nextColor, blendStep, gradLength);
            segDrawUtils::setPixelColor(segmentSet, i, colorOut, 0);
        }

        cycleNum = (cycleNum + 1) % totalCycleLength;
        
        showCheckPS();
    }
}
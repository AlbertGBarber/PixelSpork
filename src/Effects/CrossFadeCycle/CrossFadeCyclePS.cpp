#include "CrossFadeCyclePS.h"

//consturctor for using a pattern and pallet
CrossFadeCyclePS::CrossFadeCyclePS(SegmentSet &SegmentSet, patternPS *Pattern, palletPS *Pallet, uint16_t NumFades, uint8_t Steps, uint16_t Rate):
    segmentSet(SegmentSet), pattern(Pattern), pallet(Pallet), numFades(NumFades), steps(Steps)
    {    
        fMode = 0;
        init(Rate);
	}

//constructor for using pallet as pattern
CrossFadeCyclePS::CrossFadeCyclePS(SegmentSet &SegmentSet, palletPS *Pallet, uint16_t NumFades, uint8_t Steps, uint16_t Rate):
    segmentSet(SegmentSet), pallet(Pallet), numFades(NumFades), steps(Steps)
    {    
        fMode = 0;
        setPalletAsPattern();
        init(Rate);
	}

//constructor for fully random colors (mode 2)
CrossFadeCyclePS::CrossFadeCyclePS(SegmentSet &SegmentSet, uint16_t NumFades, uint8_t Steps, uint16_t Rate):
    segmentSet(SegmentSet), numFades(NumFades), steps(Steps)
    {    
        fMode = 2; //set mode to 2 since we are doing a full random set of colors
        //setup a minimal backup pallet of random colors of length 2
        //this won't be used in the effect, but if you switched modes without 
        //setting up a pallet, you will crash
        //this prevents crashing
        palletTemp = palletUtilsPS::makeRandomPallet(2);
        pallet = &palletTemp;
        setPalletAsPattern();
        init(Rate);
	}

CrossFadeCyclePS::~CrossFadeCyclePS(){
    delete[] palletTemp.palletArr;
    delete[] patternTemp.patternArr;
}

//bind core class vars
void CrossFadeCyclePS::init(uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    reset();
}

//resets all loop and color variables to start the effect from scratch
void CrossFadeCyclePS::reset(){
    done = false;
    fadeCount = 0;
    currentStep = 0;
    //a pattern of length 1 is nonsensical
    //the only result is a single solid color
    //once that is drawn set done to true, as there is nothing else to draw
    if(pattern->length <= 1){
        segDrawUtils::fillSegSetColor(segmentSet, palletUtilsPS::getPalletColor( pallet, pattern->patternArr[0]), 0);
        done = true;
    }
    //passing 0 for numFades is a shorthand for turning infinite on
    if(numFades == 0){
        infinite = true;
    }
    
    currentIndex = patternUtilsPS::getPatternVal( pattern, 0 );
    //set the starting colors depending on the mode
    //for shuffle, we always start with the first color for simplicity
    switch (fMode) {
        case 0: 
            startColor = palletUtilsPS::getPalletColor( pallet, currentIndex );
            nextColor = palletUtilsPS::getPalletColor( pallet, patternUtilsPS::getPatternVal( pattern, 1 ) );
            break;
        case 1:
            startColor = palletUtilsPS::getPalletColor( pallet, currentIndex );
            nextColor = palletUtilsPS::getPalletColor( pallet, patternUtilsPS::getShuffleIndex(pattern, currentIndex) );
            break;
        default: //anything mode 2 or above
            startColor = colorUtilsPS::randColor();
            nextColor = colorUtilsPS::randColor();
            break;
    }
}

//binds the pallet to a new one
void CrossFadeCyclePS::setPallet(palletPS *newPallet){
    pallet = newPallet;
}

//sets a new pattern
void CrossFadeCyclePS::setPattern(patternPS *newPattern){
    pattern = newPattern;
}

//sets the pattern to match the current pallet
//ie for a pallet length 5, the pattern would be 
//{0, 1, 2, 3, 4}
void CrossFadeCyclePS::setPalletAsPattern(){
    patternTemp = generalUtilsPS::setPalletAsPattern(pallet);
    pattern = &patternTemp;
}

//updates the effect until we reach the fadeCount number of cycles
void CrossFadeCyclePS::update(){
    currentTime = millis();
    //if we've reached the fadeCount number of cycle
    //the effect is finished
    //other wise update the effect
    if(!infinite && (fadeCount == numFades ) ){
        done = true;
    } else if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;
        
        //caculate the next step of the current fade and display it
        newColor = colorUtilsPS::getCrossFadeColor(startColor, nextColor, currentStep, steps);
        segDrawUtils::fillSegSetColor(segmentSet, newColor, 0);
        currentStep++;
        showCheckPS();

        //if we've reached the end of the current fade
        //we need to choose the next color to fade to
        //and advance the cycle count since we've finished a cycle
        if(currentStep == steps){
            currentStep = 0;
            //since the fade is done, the new starting color is the previous next color
            startColor = nextColor;
            //set the next color depending on the mode
            switch (fMode) {
                case 0: 
                    //normal mode
                    //(fadeCount + 2) is used as the next index, because we start with the first pair of colors
                    //techincally, this means that cycle count starts at 1, but because fadeCount is also a measure of 
                    //how many actual cycles we've completed, it should start at zero, hence we add 2
                    currentIndex = patternUtilsPS::getPatternVal( pattern, (fadeCount + 2) );
                    nextColor = palletUtilsPS::getPalletColor( pallet, currentIndex);
                    break;
                case 1:
                    //shuffle mode
                    currentIndex = patternUtilsPS::getShuffleIndex(pattern, currentIndex);
                    nextColor = palletUtilsPS::getPalletColor( pallet, currentIndex);
                    break;
                default:
                    //random mode (for all cases above 1)
                    nextColor = colorUtilsPS::randColor();
                    break;
            }
            fadeCount++;
        }
    }
}

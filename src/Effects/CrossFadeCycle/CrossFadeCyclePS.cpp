#include "CrossFadeCyclePS.h"

//consturctor for using a pattern and pallet
CrossFadeCyclePS::CrossFadeCyclePS(SegmentSet &SegmentSet, uint8_t *Pattern, uint8_t PatternLength, palletPS *Pallet, uint16_t NumFades, uint8_t Steps, uint16_t Rate):
    segmentSet(SegmentSet), pattern(Pattern), patternLength(PatternLength), pallet(Pallet), numFades(NumFades), steps(Steps)
    {    
        fMode = 0;
        init(Rate);
	}

//constructor for using pallet as pattern
CrossFadeCyclePS::CrossFadeCyclePS(SegmentSet &SegmentSet, palletPS *Pallet, uint16_t NumFades, uint8_t Steps, uint16_t Rate):
    segmentSet(SegmentSet), pallet(Pallet), numFades(NumFades), steps(Steps)
    {    
        fMode = 0;
        setPalletAsPattern(pallet);
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
        patternLength = 2;
        CRGB *pallet_arr = new CRGB[patternLength];
        pallet_arr[0] = segDrawUtils::randColor();
        pallet_arr[1] = segDrawUtils::randColor();
        palletTemp = {pallet_arr, patternLength};
        setPalletAsPattern(&palletTemp);

        init(Rate);
	}

CrossFadeCyclePS::~CrossFadeCyclePS(){
    delete[] palletTemp.palletArr;
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
    if(patternLength <= 1){
        segDrawUtils::fillSegSetColor(segmentSet, palletUtilsPS::getPalletColor( pallet, pattern[0]), 0);
        done = true;
    }
    //passing 0 for numFades is a shorthand for turning infinite on
    if(numFades == 0){
        infinite = true;
    }
    
    currentIndex = 0;
    //set the starting colors depending on the mode
    //for shuffle, we always start with the first color for simplicity
    switch (fMode) {
        case 0: 
            startColor = palletUtilsPS::getPalletColor( pallet, pattern[0] );
            nextColor = palletUtilsPS::getPalletColor( pallet, pattern[1] );
            break;
        case 1:
            startColor = palletUtilsPS::getPalletColor( pallet, pattern[0] );
            nextColor = palletUtilsPS::getPalletColor( pallet, shuffleIndex() );
            break;
        default: //anything mode 2 or above
            startColor = segDrawUtils::randColor();
            nextColor = segDrawUtils::randColor();
            break;
    }
}

//retuns a random pallet index from the pattern
//the code checks to see if the random index matches the current index
//if it does we'll just advance the index by one and return that
//this stops the same color from being chosen again
uint8_t CrossFadeCyclePS::shuffleIndex(){
    uint8_t indexGuess = random8(patternLength);
    if( pattern[indexGuess] == pattern[currentIndex] ){
        currentIndex = (currentIndex + 1) % patternLength;
        return pattern[ currentIndex ];
    } else {
        currentIndex = indexGuess;
        return pattern[indexGuess];
    }
}

//binds the pallet to a new one
void CrossFadeCyclePS::setPallet(palletPS *newPallet){
    pallet = newPallet;
}

//sets the pattern to match the passed in pallet
//ie pattern color 1 is pallet color 1, etc
void CrossFadeCyclePS::setPalletAsPattern(palletPS *newPallet){
    pallet = newPallet;
    patternLength = pallet->length;
    pattern = new uint8_t[patternLength];
    for(int i = 0; i < patternLength; i++){
        pattern[i] = i;
    }
}

//sets a new pattern
void CrossFadeCyclePS::setPattern(uint8_t *newPattern, uint8_t newPatternLength){
    pattern = newPattern;
    patternLength = newPatternLength;
}

//updates the effect until we reach the fadeCount number of cycles
void CrossFadeCyclePS::update(){
    currentTime = millis();
    //if we're using an external rate variable, get its value
    globalRateCheckPS();
    //if we've reached the fadeCount number of cycle
    //the effect is finished
    //other wise update the effect
    if(!infinite && (fadeCount == numFades ) ){
        done = true;
    } else if( ( currentTime - prevTime ) >= rate ) {
        prevTime = currentTime;
        
        //caculate the next step of the current fade and display it
        CRGB newColor;
        newColor = segDrawUtils::getCrossFadeColor(startColor, nextColor, currentStep, steps);
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
                    currentIndex = pattern[ (fadeCount + 2) % patternLength ];
                    nextColor = palletUtilsPS::getPalletColor( pallet, currentIndex);
                    break;
                case 1:
                    //shuffle mode
                    nextColor = palletUtilsPS::getPalletColor( pallet, shuffleIndex());
                    break;
                default:
                    //random mode (for all cases above 1)
                    nextColor = segDrawUtils::randColor();
                    break;
            }
            fadeCount++;
        }
    }
}

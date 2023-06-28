#include "CrossFadeCyclePS.h"

//constructor for using a pattern and palette
CrossFadeCyclePS::CrossFadeCyclePS(SegmentSet &SegSet, patternPS &Pattern, palettePS &Palette, uint8_t Steps, uint16_t Rate):
    SegSet(SegSet), pattern(&Pattern), palette(&Palette), steps(Steps)
    {    
        init(Rate);
	}

//constructor for using palette as pattern
CrossFadeCyclePS::CrossFadeCyclePS(SegmentSet &SegSet, palettePS &Palette, uint8_t Steps, uint16_t Rate):
    SegSet(SegSet), palette(&Palette), steps(Steps)
    {    
        setPaletteAsPattern();
        init(Rate);
	}

//constructor for fully random colors (mode 1)
CrossFadeCyclePS::CrossFadeCyclePS(SegmentSet &SegSet, uint8_t Steps, uint16_t Rate):
    SegSet(SegSet), steps(Steps)
    {    
        randMode = 1; //set mode to 1 since we are doing a full random set of colors
        //setup a minimal backup palette of random colors of length 2
        //this won't be used in the effect, but if you switched modes without 
        //setting up a palette, you will crash
        //this prevents crashing
        paletteTemp = paletteUtilsPS::makeRandomPalette(2);
        palette = &paletteTemp;
        setPaletteAsPattern();
        init(Rate);
	}

CrossFadeCyclePS::~CrossFadeCyclePS(){
    free(paletteTemp.paletteArr);
    free(patternTemp.patternArr);
}

//bind core class vars
void CrossFadeCyclePS::init(uint16_t Rate){
    //bind the rate and SegSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    reset();
}

//resets all loop and color variables to start the effect from scratch
void CrossFadeCyclePS::reset(){
    fadeCount = 0;
    patternIndex = 0;
    currentStep = 0;
    palIndex = 0;
    paused = false;
    //a pattern of length 1 is nonsensical
    //the only result is a single solid color
    //once that is drawn set done to true, as there is nothing else to draw
    if(pattern->length <= 1){
        colorOut = paletteUtilsPS::getPaletteColor( *palette, pattern->patternArr[0]);
        segDrawUtils::fillSegSetColor(SegSet, colorOut, 0);
    }

    //Get the inital color and the first end color
    getNextColor();
    startColor = nextColor;
    getNextColor();
}

//sets the pattern to match the current palette
//ie for a palette length 5, the pattern would be 
//{0, 1, 2, 3, 4}
void CrossFadeCyclePS::setPaletteAsPattern(){
    patternTemp = generalUtilsPS::setPaletteAsPattern(*palette);
    pattern = &patternTemp;
}

//Gets the next fade color based on the pattern, palette, and randMode
//Also advances the patternIndex to track where we are in the pattern
//randModes are:
//  0: Colors will be chosen in order from the pattern (not random)
//  1: Colors will be chosen completely at random
//  2: Colors will be chosen randomly from the palette (not allowing repeats)
void CrossFadeCyclePS::getNextColor(){
    switch (randMode) {
        case 0: //Colors will be chosen in order from the pattern (not random)
        default:
            palIndex = patternUtilsPS::getPatternVal( *pattern, patternIndex );
            nextColor = paletteUtilsPS::getPaletteColor( *palette, palIndex );
            break;
        case 1: //Colors will be chosen completely at random
            nextColor = colorUtilsPS::randColor();
            break;
        case 2: //Colors will be chosen randomly from the palette (not allowing repeats)
            palIndex = patternUtilsPS::getShuffleVal( *pattern, palIndex );
            nextColor = paletteUtilsPS::getPaletteColor( *palette, palIndex );
            break;
    }

    //Advance the patternIndex for when we pick the next color
    patternIndex = addMod16PS(patternIndex, 1, pattern->length);  
}

//updates the effect
//For each update we fade towards a target color.
//Once the fade is finished (the fade step ==  the total number of steps) 
//we pick a new color (while recording the current color)
void CrossFadeCyclePS::update(){
    currentTime = millis();
    if( (currentTime - prevTime) >= *rate ) {

        //code for pausing the effect after a fade is finished
        //if we're in pause time, we simply return to break out of the function 
        if(paused){
            if( (currentTime - prevTime) > pauseTime ){
                paused = false;
            } else {
                return;
            }
        }

        prevTime = currentTime;
        
        //calculate the next step of the current fade and display it
        colorOut = colorUtilsPS::getCrossFadeColor(startColor, nextColor, currentStep, steps);
        segDrawUtils::fillSegSetColor(SegSet, colorOut, 0);
        currentStep++;

        //if we've reached the end of the current fade
        //we need to choose the next color to fade to
        //and advance the fade count since we've finished a fade
        //We also start the pause time
        if(currentStep == steps){
            currentStep = 0;
            //since the fade is done, the new starting color is the previous next color
            startColor = nextColor;
            //set the next color depending on the mode
            getNextColor();
            fadeCount++;
            paused = true;
        }

        showCheckPS();
    }
}

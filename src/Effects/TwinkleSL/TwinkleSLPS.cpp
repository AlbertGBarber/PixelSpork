#include "TwinkleSLPS.h"

//pallet based constructor
TwinkleSLPS::TwinkleSLPS(SegmentSet &SegmentSet, palletPS *Pallet, CRGB BgColor, uint16_t NumTwinkles, uint8_t FadeInSteps, uint8_t FadeOutSteps, uint16_t Rate):
    segmentSet(SegmentSet), pallet(Pallet), numTwinkles(NumTwinkles)
    {    
        init(FadeInSteps, FadeOutSteps, BgColor, Rate);
	}

//single color constructor
TwinkleSLPS::TwinkleSLPS(SegmentSet &SegmentSet, CRGB Color, CRGB BgColor, uint16_t NumTwinkles, uint8_t FadeInSteps, uint8_t FadeOutSteps, uint16_t Rate):
    segmentSet(SegmentSet), numTwinkles(numTwinkles)
    {    
        setSingleColor(Color);
        init(FadeInSteps, FadeOutSteps, BgColor, Rate);
	}

//random colors constructor
TwinkleSLPS::TwinkleSLPS(SegmentSet &SegmentSet, CRGB BgColor, uint16_t NumTwinkles, uint8_t FadeInSteps, uint8_t FadeOutSteps, uint16_t Rate):
    segmentSet(SegmentSet), numTwinkles(NumTwinkles)
    {    
        //we make a random pallet of one color so that 
        //if we switch to randMode 0 then we have a pallet to use
        setSingleColor(colorUtilsPS::randColor()); 
        //since we're choosing colors at random, set the randMode
        randMode = 1;
        init(FadeInSteps, FadeOutSteps, BgColor, Rate);
	}

TwinkleSLPS::~TwinkleSLPS(){
    delete[] palletTemp.palletArr;
    deleteTwinkleArrays();
}

//sets up all the core class vars, and initilizes the pixel and color arrays
void TwinkleSLPS::init(uint8_t FadeInSteps, uint8_t FadeOutSteps, CRGB BgColor, uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    bindBGColorPS();
    setSteps(FadeInSteps, FadeOutSteps);
    reset();
}

//creates an pallet of length 1 containing the passed in color
void TwinkleSLPS::setSingleColor(CRGB Color){
    delete[] palletTemp.palletArr;
    palletTemp = palletUtilsPS::makeSingleColorPallet(Color);
    pallet = &palletTemp;
}

//creates the 2D arrays for storing the random twinkle locations (relative to the segmentSet) and their colors
//There's two seperate arrays, which could be combined into an array of structs, but ehh
//The arrays are laid out like [numTwinkles][totalFadeSteps], where the column index is the fade step the twinkle is on
//the minium value of totalFadeSteps is 2, one step to fade in, and one step to fade out
//we don't initialize the arrays to anything, since we want a gradual build up to the twinkles fading
void TwinkleSLPS::initTwinkleArrays(){
    //the total length of each column is the number of steps needed to fade a twinkle in and out
    //We overlap the center step (where the twinkle is fully fade in) 
    //so that that step isn't doubly long
    totFadeSteps = fadeInSteps + fadeOutSteps;

    //delete the old arrays to prevent memory leak
    deleteTwinkleArrays();

    //create new 2D arrays
    //we need to make the rows first, and then fill them
    //with pointers to the column arrays
    ledArray = new uint16_t*[numTwinkles];

    colorIndexArr = new CRGB*[numTwinkles];

    for(uint16_t i = 0; i < numTwinkles; i++){
        ledArray[i] = new uint16_t[totFadeSteps];
        colorIndexArr[i] = new CRGB[totFadeSteps];
    }
    reset();
}

//deletes the twinkle index and color arrays
//used to prevent memory leaks
void TwinkleSLPS::deleteTwinkleArrays(){
    if(ledArray){
        uint16_t twinkleArrSize = SIZE(ledArray);
        for(uint16_t i = 0; i < twinkleArrSize; i++){
            delete[] ledArray[i];
            delete[] colorIndexArr[i];
        }
        delete[] ledArray;
        delete[] colorIndexArr;
    }
}

//resets the startup vars to their defaults
//and sets the BG to clear any mid-fade pixels
void TwinkleSLPS::reset(){
    startUpDone = false;
    totalSteps = 0;
    segDrawUtils::fillSegSetColor(segmentSet, *bgColor, bgColorMode);
}

//sets the number of fade in and out steps (min value of 1)
//it's easier to set the both together since we need to recreate the location and color arrays
//when every we set either of them
void TwinkleSLPS::setSteps(uint8_t newfadeInSteps, uint8_t newfadeOutSteps){
    fadeInSteps = newfadeInSteps;
    if(fadeInSteps < 1){
       fadeOutSteps = 1; 
    }

    fadeOutSteps = newfadeOutSteps;
    if(fadeOutSteps < 1){
       fadeOutSteps = 1; 
    }
    initTwinkleArrays();
}

//sets the number of random pixels
void TwinkleSLPS::setNumTwinkles(uint16_t newNumTwinkles){
    numTwinkles = newNumTwinkles;
    initTwinkleArrays();
}

//updates the effect
//How it works:
    //On each update cycle we spawn a new twinkle for each numTwinkles and put it at the start of the ledArray
    //while moving all the old twinkles one step forward in the ledArray
    //The twinkle's index in the ledArray indicates the fade step it's on, while the value is it's location

    //We run through all the pixels in the array and set their color based on the fade level (the array column index)
    //we run through the column's backwards, so that if there's overlapping pixels, we always put out the brightest level
    //If we're on the zeroth column, we choose a new color and pixel location
    //after each cycle we shift the array entries over by one (no wrapping)
    //so each led is picked, then shifted along the array, fading in and out as it goes

    //Note that the effect is set to draw along segment lines, so one twinkle is actually a twinkle along 
    //a whole segment line. So every time we draw the twinkle we draw it on a segment line, not just a single pixel.
    //This includes dealing with color modes for each line
    
    //For the first set of cycles, we don't have a full array of pixels in the ledArray
    //We're in "startup mode", so we limit how much of the pixel arrays we read from
    //incrementing the amount as we fill the array in after each cycle, until we've covered the whole array
void TwinkleSLPS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;
     
        numSegs = segmentSet.numSegs;
        numLines = segmentSet.maxSegLength;
        palletLength = pallet->length;

        //startup settings to limit how much of the array is written out
        //since the arrays start un-initilized
        if(!startUpDone){
            totalSteps++;
            if(totFadeSteps == totalSteps){
                startUpDone = true;
            }
        }

        //by default, we only touch twinkles that are fading
        //but for rainbow or gradient backgrounds that a cycling
        //you want to redraw the whole thing
        if(fillBG){
            segDrawUtils::fillSegSetColor(segmentSet, *bgColor, bgColorMode);
        }

        //We run through all the twinkles in the array and set their color based on the fade level (the array column index)
        //we run through the column's (j) backwards, so that if there's overlapping twinkles, we always put out the brightest level
        //If we're on the zeroth column, we choose a new color and twinkle location
        //after each cycle we shift the array entries over by one (no wrapping)
        //write out a test array if you are confused
        for (uint16_t i = 0; i < numTwinkles; i++) {
            for(int16_t j = totalSteps - 1; j >= 0; j--){
                if(j == 0){ //first index, set a new line location and color for a new twinkle
                    ledArray[i][0] = random16(numLines);
                    colorIndexArr[i][0] = pickColor();
                }
                lineNum = ledArray[i][j];
                //for each pixel in the segment line we output the cross-fade color based on the 
                //line color (colorIndexArr[i][j]) and the color mode
                for(uint8_t k = 0; k < numSegs; k++){
                    //get the physical pixel location based on the line and seg numbers
                    pixelNum = segDrawUtils::getPixelNumFromLineNum(segmentSet, numLines, k, lineNum);
                    //grab the background color, accounting for color modes
                    colorTarget = segDrawUtils::getPixelColor(segmentSet, pixelNum, *bgColor, bgColorMode, k, lineNum);
                    //get the twinkle color, accounting for color modes
                    twinkleColor = segDrawUtils::getPixelColor(segmentSet, pixelNum, colorIndexArr[i][j], colorMode, k, lineNum);

                    //we either fade in or out depending which index we're on (earlier fade in, later fade out)
                    //we don't want to double count the final step of the fade in and the initial step of the fade out
                    //since they're the same
                    if(j < fadeInSteps) {
                        //step is j + 1, since we skip the first step (fully Bg)
                        twinkleColor = colorUtilsPS::getCrossFadeColor(colorTarget, twinkleColor, j + 1, fadeInSteps);
                    } else {
                        //+1 since we skip the first step of the fade in
                        step = (j - fadeInSteps + 1);
                        //we need to clamp the step, because it overshoots if fadeout and fadein are 1
                        if(step > fadeOutSteps){ step = fadeOutSteps; }
                        twinkleColor = colorUtilsPS::getCrossFadeColor(twinkleColor, colorTarget, step, fadeOutSteps);
                    }
                    //set the pixel color
                    segmentSet.leds[pixelNum] = twinkleColor;
                }
            }
        }
        //shift the pixel arrays to prep for the next cycle
        incrementTwinkleArrays();
        showCheckPS();
    }
}

//set a color based on the size of the pallet and random mode
CRGB TwinkleSLPS::pickColor(){
    switch (randMode) {
        case 0: // we're picking from a set of colors 
            twinkleColor = palletUtilsPS::getPalletColor(pallet, random8(palletLength));
            break;
        default: //(mode 1) set colors at random
            twinkleColor = colorUtilsPS::randColor();
            break;
    }
    return twinkleColor;
}

//shift the columns of the pixel arrays to the left by one
//we don't need to worry about the values of the final index, 
//since pixels in that location will be fully faded out
void TwinkleSLPS::incrementTwinkleArrays(){
    for (uint16_t i = 0; i < numTwinkles; i++) {
        for(uint8_t j = totFadeSteps - 1; j > 0; j--){
            ledArray[i][j] = ledArray[i][j - 1];
            colorIndexArr[i][j] = colorIndexArr[i][j - 1];
        }
    }
}
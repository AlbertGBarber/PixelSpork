#include "RandomColorsPS.h"

//pallet based constructor
RandomColorsPS::RandomColorsPS(SegmentSet &SegmentSet, palletPS *Pallet, uint16_t NumPixels, CRGB BgColor, uint8_t FadeInSteps, uint8_t FadeOutSteps, uint16_t Rate):
    segmentSet(SegmentSet), pallet(Pallet), numPixels(NumPixels), bgColor(BgColor)
    {    
        init(FadeInSteps, FadeOutSteps, Rate);
	}

//single color constructor
RandomColorsPS::RandomColorsPS(SegmentSet &SegmentSet, CRGB Color, uint16_t NumPixels, CRGB BgColor, uint8_t FadeInSteps, uint8_t FadeOutSteps, uint16_t Rate):
    segmentSet(SegmentSet), numPixels(NumPixels), bgColor(BgColor)
    {    
        setSingleColor(Color);
        init(FadeInSteps, FadeOutSteps, Rate);
	}

//random colors constructor
RandomColorsPS::RandomColorsPS(SegmentSet &SegmentSet, uint16_t NumPixels, CRGB BgColor, uint8_t FadeInSteps, uint8_t FadeOutSteps, uint16_t Rate):
    segmentSet(SegmentSet), numPixels(NumPixels), bgColor(BgColor)
    {    
        setSingleColor(segDrawUtils::randColor());
        //although we set a single pallet, we want to choose the colors at random,
        //so we set the pallet length to 0 (see pickColor()  )
        pallet->length = 0;
        init(FadeInSteps, FadeOutSteps, Rate);
	}

//sets up all the core class vars, and initilizes the pixel and color arrays
void RandomColorsPS::init(uint8_t FadeInSteps, uint8_t FadeOutSteps, uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    setSteps(FadeInSteps, FadeOutSteps);
    reset();
}

//creates an pallet of length 1 containing the passed in color
void RandomColorsPS::setSingleColor(CRGB Color){
    CRGB *pallet_arr = new CRGB[1];
    pallet_arr[0] = Color;
    palletTemp = {pallet_arr, 1};
    pallet = &palletTemp;
}

//binds the pallet to a new one
void RandomColorsPS::setPallet(palletPS *newPallet){
    pallet = newPallet;
}

//creates the 2D arrays for storing the random pixel locations (relative to the segmentSet) and their colors
//There's two seperate arrays, which could be combined into an array of structs, but ehh
//The arrays are laid out like [numPixels][totalFadeSteps], where the column index is the fade step the pixel is on
//the minium value of totalFadeSteps is 2, one step to fade in, and one step to fade out
//we don't initialize the arrays to anything, since we want a gradual build up to the pixels fading
void RandomColorsPS::initPixelArrays(){
    //the total length of each column is the number of steps needed to fade a pixel in and out
    //We overlap the center step (where the pixel is fully fade in) 
    //so that that step isn't doubly long
    totFadeSteps = fadeInSteps + fadeOutSteps;

    //delete the old arrays to free up memory
    if(ledArray){
        for(int i = 0; i < numPixels; i++){
            delete[] ledArray[i];
            delete[] colorIndexArr[i];
        }
        delete[] ledArray;
        delete[] colorIndexArr;
    }

    //create new 2D arrays
    //we need to make the rows first, and then fill them
    //with pointers to the column arrays
    ledArray = new uint16_t*[numPixels];

    colorIndexArr = new CRGB*[numPixels];

    for(int i = 0; i < numPixels; i++){
        ledArray[i] = new uint16_t[ totFadeSteps ];
        colorIndexArr[i] = new CRGB[ totFadeSteps ];
    }
    reset();
}

//resets the startup vars to their defaults
//and sets the BG to clear any mid-fade pixels
void RandomColorsPS::reset(){
    startUpDone = false;
    totalSteps = 0;
    segDrawUtils::fillSegSetColor(segmentSet, bgColor, bgColorMode);
}

//sets the number of fade in and out steps (min value of 1)
//it's easier to set the both together since we need to recreate the location and color arrays
//when every we set either of them
void RandomColorsPS::setSteps(uint8_t newfadeInSteps, uint8_t newfadeOutSteps){
    fadeInSteps = newfadeInSteps;
    if(fadeInSteps < 1){
       fadeOutSteps = 1; 
    }

    fadeOutSteps = newfadeOutSteps;
    if(fadeOutSteps < 1){
       fadeOutSteps = 1; 
    }
    initPixelArrays();
}

//sets the number of random pixels
void RandomColorsPS::setNumPixels(uint16_t newNumPixels){
    numPixels = newNumPixels;
    initPixelArrays();
}

//updates the effect
//How it works:
    //If we're in startup mode, then we limit how much of the pixel arrays we read from
    //incrementing the amount as we fill the array in after each cycle, until we've covered the whole array

    //We run through all the pixels in the array and set their color based on the fade level (the array column index)
    //we run through the column's backwards, so that if there's overlapping pixels, we always put out the brightest level
    //If we're on the zeroth column, we choose a new color and pixel location
    //after each cycle we shift the array entries over by one (no wrapping)
    //so each led is picked, then shifted along the array, fading in and out as it goes
void RandomColorsPS::update(){
    currentTime = millis();
    //if we're using an external rate variable, get its value
    globalRateCheckPS();
    if( ( currentTime - prevTime ) >= rate ) {
        prevTime = currentTime;
        uint16_t numActiveLeds = segmentSet.numActiveSegLeds;

        //startup settings to limit how much of the array is written out
        //since the arrays start un-initilized
        if(!startUpDone){
            totalSteps++;
            if(totFadeSteps == totalSteps){
                startUpDone = true;
            }
        }

        //by default, we only touch pixels that are fading
        //but for rainbow or gradient backgrounds that a cycling
        //you want to redraw the whole thing
        if(fillBG){
            segDrawUtils::fillSegSetColor(segmentSet, bgColor, bgColorMode);
        }

        ////We run through all the pixels in the array and set their color based on the fade level (the array column index)
        //we run through the column's (j) backwards, so that if there's overlapping pixels, we always put out the brightest level
        //If we're on the zeroth column, we choose a new color and pixel location
        //after each cycle we shift the array entries over by one (no wrapping)
        //write out a test array if you are confused
        for (uint16_t i = 0; i < numPixels; i++) {
            for(int16_t j = totalSteps - 1; j >= 0; j--){
                if(j == 0){ //first index, set a new pixel location and color
                    ledArray[i][0] = random(numActiveLeds);
                    pickColor( ledArray[i][0] );
                    colorIndexArr[i][0] = color;
                }
                //grab the background color info
                segDrawUtils::getPixelColor(segmentSet, &pixelInfo, bgColor, bgColorMode, ledArray[i][j]);
                //we either fade in or out depending which index we're on (earlier fade in, later fade out)
                //we don't want to double count the final step of the fade in and the initial step of the fade out
                //since they're the same
                if(j < fadeInSteps) {
                    //step is j + 1, since we skip the first step (fully Bg)
                    color = segDrawUtils::getCrossFadeColor( pixelInfo.color, colorIndexArr[i][j], j + 1, fadeInSteps);
                } else {
                    //+1 since we skip the first step of the fade in
                    uint8_t step = (j - fadeInSteps + 1);
                    //we need to clamp the step, because it overshoots if fadeout and fadein are 1
                    if(step > fadeOutSteps){ step = fadeOutSteps; }
                    color = segDrawUtils::getCrossFadeColor(colorIndexArr[i][j], pixelInfo.color, step, fadeOutSteps);
                }
                //output the color, we can reuse the pixel info we got from the BG color, since it's the same pixel
                segDrawUtils::setPixelColor(segmentSet, pixelInfo.pixelLoc, color, 0, pixelInfo.segNum, pixelInfo.lineNum);
            }
        }
        //shift the pixel arrays to prep for the next cycle
        incrementPixelArrays();
        showCheckPS();
    }
}

//set a color based on the size of the pallet
void RandomColorsPS::pickColor(uint16_t pixelNum){
    uint8_t palletLength = pallet->length;
    switch (palletLength) {
        case 0: // 0 pallet length, no pallet, so set colors at random
            color = segDrawUtils::randColor();
            break;
        case 1: // pallet length one means all the pixels must be the same color
            color = palletUtilsPS::getPalletColor(pallet, 0);
            break;
        default: // we're picking from a set of colors
            color = palletUtilsPS::getPalletColor(pallet, random(palletLength));
            break;
    }
    //get the pixel color to account for any color modes
    segDrawUtils::getPixelColor(segmentSet, &pixelInfo, color, colorMode, pixelNum);
    color = pixelInfo.color;
}

//shift the columns of the pixel arrays to the left by one
//we don't need to worry about the values of the final index, 
//since pixels in that location will be fully faded out
void RandomColorsPS::incrementPixelArrays(){
    for (uint16_t i = 0; i < numPixels; i++) {
        for(uint8_t j = totFadeSteps - 1; j > 0; j--){
            ledArray[i][j] = ledArray[i][j - 1];
            colorIndexArr[i][j] = colorIndexArr[i][j - 1];
        }
    }
}
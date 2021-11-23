#include "GlimmerPS.h"

GlimmerPS::GlimmerPS(SegmentSet &SegmentSet, uint16_t NumPixels, CRGB GlimmerColor, CRGB BgColor, bool TwoPixelSets, uint8_t FadeSteps, uint16_t Rate):
    segmentSet(SegmentSet), numPixels(NumPixels), fadeSteps(FadeSteps), twoPixelSets(TwoPixelSets)
    {    
        init(GlimmerColor, BgColor, Rate);
	}


GlimmerPS::GlimmerPS(SegmentSet &SegmentSet, uint16_t NumPixels, CRGB GlimmerColor, CRGB BgColor, bool TwoPixelSets, uint8_t FadeSteps, uint8_t FadeMin, uint8_t FadeMax, uint16_t Rate):
    segmentSet(SegmentSet), numPixels(NumPixels), fadeSteps(FadeSteps), twoPixelSets(TwoPixelSets), fadeMin(FadeMin), fadeMax(FadeMax)
    {    
        //fade min can't be greater than fade max
        if(fadeMin > fadeMax){
            uint8_t temp = fadeMin;
            fadeMin = fadeMax;
            fadeMax = temp;
        }
        init(GlimmerColor, BgColor, Rate);
	}

GlimmerPS::~GlimmerPS(){
    delete[] fadePixelLocs;
    delete[] totFadeSteps;
}

void GlimmerPS::init(CRGB GlimmerColor, CRGB BgColor, uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    //bind background color pointer
    bindBGColorPS();
    //bind the glimmer color to a pointer
    //so it can be bound to an external variable like bgColor
    colorOrig = GlimmerColor;
    glimmerColor = &colorOrig;
    setupPixelArray();
}

//creates the pixel location and fade value arrays
//these store the locations for the fading pixels, and how much they will fade by
//if we're doing twoPixelSets, then there will be one set of pixels fading in
//while another is fading out
//these sets are tracked in the same array, so the array's length is 2 * numPixels
//pixels up to numPixels in the array are fading in,
//while pixels from numPixels to the end are fading out
void GlimmerPS::setupPixelArray(){
    arrayLength = numPixels;
    if(twoPixelSets){
        arrayLength = numPixels * 2;
    }
    delete[] fadePixelLocs;
    delete[] totFadeSteps;
    numActiveLeds = segmentSet.numActiveSegLeds;
    fadePixelLocs = new uint16_t[ arrayLength ];
    totFadeSteps = new uint8_t[ arrayLength ];
    //we need to set firstFade to since the arrays
    //are only filled up to numPixels because we don't 
    //want to start with set that is already faded when the effect begins
    //(note that firstFade is not needed if we're only doing a single pixel set)
    firstFade = true;
    fillPixelArray();
    segDrawUtils::fillSegSetColor(segmentSet, *bgColor, bgColorMode);
}

//fills in the pixel arrays with random locations and fade values
//only fills the arrays up to numPixels
//since if we're working with two pixel sets, the second numPixels length of the array
//is used for the fading out set, which should already be set, and we don't want to overwrite it
//We also set the step to zero, since we're starting a new set of pixels
void GlimmerPS::fillPixelArray(){
    step = 0;
    for(uint16_t i = 0; i < numPixels; i++){
        totFadeSteps[i] = random(fadeMin, fadeMax); //target fade amount between the bgColor and the glimmer color
        fadePixelLocs[i] = random(numActiveLeds);
    }
}

//Only relevant for when we're using two sets of pixels
//once a fade in is finished, we need to copy those array values into
//the second section of the array so they can be faded out.
//Note that we don't care about the pixels in the faded out section since they have finished fading
void GlimmerPS::advancePixelArray(){
    for(uint16_t i = 0; i < numPixels; i++){
        totFadeSteps[i + numPixels] = totFadeSteps[i];
        fadePixelLocs[i + numPixels] = fadePixelLocs[i];
    }
}

//sets the number of random pixels choosen per cycle
//Note that the pixel arrays will be reset, restarting the effect
void GlimmerPS::setNumPixels(uint16_t newNumPixels){
    numPixels = newNumPixels;
    setupPixelArray();
}

//changes the effect to use one or two sets for fading
//if the new value is different than the current, the pixel arrays
//will be reset, restarting the effect
void GlimmerPS::setTwoSets(bool newSetting){
    if(newSetting != twoPixelSets){
        twoPixelSets = newSetting;
        setupPixelArray();
    }
}

//How the effect works:
    //The goal of the effect is to fade a random set of leds in and out, between the glimmerColor and the bgColor
    //each led fades a random amount towards glimmerColor, but all leds fade together (their fades finish at the same time)
    //To keep track of the fading led locations and the fade amounts we use two arrays (uint16_t for the locations and uint8_t for the fade)
    //these are set by setupPixelArray(), and fillPixelArray()
    //the fade amounts are the number of steps out to fadeMax steps towards the glimmerColor
    //we work out the faded glimmer color on the fly to account for different colorModes

    //Each cycle, we fade the leds one step towards their target glimmer colors
    //once we reach the fadeSteps steps, then we reverse, and fade them back to the bgColor
    //the fade direction is controlled by the bool fadeIn (true for towards the glimmer color)
    //once the fade in and outs are finished, we choose a new set of pixels and fade amounts

    //There a two sub-modes for the effect: one using one set of leds and one using two sets
    //The sub-mode is indicated by the twoPixelSets bool (true for two sets of pixels)
        //For the single set, we only work with one set of leds at a time, fading them in and out completly before choosing a new set
        //For this mode, the array lengths are equal to the numPixels (num leds we're fading)

        //For two sets, then we fade one set of pixels out while another is fading in
        //so we're doing two sets of fades simultaneously 
        //For this mode, the array lengths are double the numPixels
        //The first length up to numPixels stores the location/fade data for fading in leds
        //While the second length stores data for the fading out leds
        //when we reach the fadeSteps steps, we copy the fading in data into the second length using advancePixelArray()
        //and generate a new set of leds in the first length

void GlimmerPS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;
        
        //by default, we only touch pixels that are fading
        //but for rainbow or gradient backgrounds that a cycling
        //you want to redraw the whole thing
        if(fillBG){
            segDrawUtils::fillSegSetColor(segmentSet, *bgColor, bgColorMode);
        }

        //increment the step, we mod by fadeSteps + 1 because we want to
        //hit fadeSteps, which will finish the fade fully
        step = (step + 1) % (fadeSteps + 1);

        //if we have two pixel sets then there's always going to be a set fading in
        //but if there's only one set, then that's no longer true
        if(twoPixelSets){
           fadeIn = true; 
        }

        //run over all the pixels in the arrays and output them
        for(uint16_t i = 0; i < arrayLength; i++){
            
            //only relevant for when we have two pixel sets
            //when we have two pixel sets, the pixels in the array 
            //after numPixels will be fading out, so we need to set
            //fadeIn to false, but we only need to do this once per loop
            if(fadeIn && i >= numPixels) {
                //if we're fading for the first time then the second half of the arrays
                //will be empty, so we skip them
                if(firstFade){
                    break;
                }
                fadeIn = false;
            } 
            //first we get the pixelInfo for the background color
            //this stores actual bgColor for the pixel in pixelInfo.color based on the bgColorMode
            //(see getPixelColor and pixelInfo.h for details)
            segDrawUtils::getPixelColor(segmentSet, &pixelInfo, *bgColor, bgColorMode, fadePixelLocs[i]);
            
            //we can reuse the other member so pixelInfo (segNum, pixelLoc, lineNum) from the above function
            //to get actual color of the output color based on color mode
            color = segDrawUtils::getPixelColor(segmentSet, pixelInfo.pixelLoc, *glimmerColor, colorMode, pixelInfo.segNum, pixelInfo.lineNum);

            //determine final transition color for the fade, note that we work this out dynamically
            //to account for any color modes or changes in fadeSteps
            targetColor = segDrawUtils::getCrossFadeColor(pixelInfo.color, color, totFadeSteps[i], fadeMax);

            //color is used as the start color for the fade, which is stored in pixelInfo (see above)
            color = pixelInfo.color;

            //if we're not fading in, then we're starting from the targetColor
            //so we need to switch it with color
            if(!fadeIn){
                CRGB colorTemp = targetColor;
                targetColor = color;
                color = colorTemp;
            } 
            
            //the final output color, blended towards the target color by the number of steps
            color = segDrawUtils::getCrossFadeColor(color, targetColor, step, fadeSteps);

            segDrawUtils::setPixelColor(segmentSet, pixelInfo.pixelLoc, color, 0, pixelInfo.segNum, pixelInfo.lineNum);
        }
        
        //if we've finished a fade we need to prep for the next fade
        if(step == fadeSteps){
            //if we have two pixel sets, then we need to advance the pixel array
            //to fade out the set of leds that just faded in, and then 
            //generate a new set of leds to fade in
            if(twoPixelSets){
                firstFade = false;
                advancePixelArray();
                fillPixelArray();
            } else {
                //if we don't have two pixel sets,
                //then the set we have will fade in and then out 
                //before we pick a new set to fade
                //this is done by flipping the fadeIn var
                if(!fadeIn){
                    fillPixelArray();
                }
                fadeIn = !fadeIn;
            }
        }
        showCheckPS();
    }
}
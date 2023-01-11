#include "GlimmerSL.h"
//Constructor using default fade in and out values
GlimmerSL::GlimmerSL(SegmentSet &SegmentSet, uint16_t NumGlims, CRGB GlimmerColor, CRGB BgColor, bool TwoPixelSets, uint8_t FadeSteps, uint16_t Rate):
    segmentSet(SegmentSet), numGlims(NumGlims), fadeSteps(FadeSteps), twoPixelSets(TwoPixelSets)
    {    
        init(GlimmerColor, BgColor, Rate);
	}

//Constuctor for setting maximum fade in and out values
GlimmerSL::GlimmerSL(SegmentSet &SegmentSet, uint16_t NumGlims, CRGB GlimmerColor, CRGB BgColor, bool TwoPixelSets, uint8_t FadeSteps, uint8_t FadeMin, uint8_t FadeMax, uint16_t Rate):
    segmentSet(SegmentSet), numGlims(NumGlims), fadeSteps(FadeSteps), twoPixelSets(TwoPixelSets), fadeMin(FadeMin), fadeMax(FadeMax)
    {    
        //fade min can't be greater than fade max
        if(fadeMin > fadeMax){
            uint8_t temp = fadeMin;
            fadeMin = fadeMax;
            fadeMax = temp;
        }
        init(GlimmerColor, BgColor, Rate);
	}

GlimmerSL::~GlimmerSL(){
    free(fadePixelLocs);
    free(totFadeSteps);
}

void GlimmerSL::init(CRGB GlimmerColor, CRGB BgColor, uint16_t Rate){
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

//Sets the number of glimmering particles to be the passed in value
//If the new number of glims is different from the current one: (If they're the same nothing happens)
    //creates the pixel location and fade value arrays
    //these store the locations for the fading pixels, and how much they will fade by
    //if we're doing twoPixelSets, then there will be one set of pixels fading in
    //while another is fading out
    //these sets are tracked in the same array, so the array's length is 2 * numGlims
    //pixels up to numGlims in the array are fading in,
    //while pixels from numGlims to the end are fading out
void GlimmerSL::setupPixelArray(){

    glimArrLen = numGlims;
    //if we have two sets of glimmers, the glimmer array needs to be double the length
    if(twoPixelSets){
        glimArrLen = numGlims * 2;
    }

    free(fadePixelLocs);
    fadePixelLocs = (uint16_t*) malloc(glimArrLen * sizeof(uint16_t));

    free(totFadeSteps);
    totFadeSteps = (uint8_t*) malloc(glimArrLen * sizeof(uint8_t));
    
    //we need to set firstFade to since the arrays
    //are only filled up to numGlims because we don't 
    //want to start with set that is already faded when the effect begins
    //(note that firstFade is not needed if we're only doing a single pixel set)
    firstFade = true;
    fillPixelArray();
    segDrawUtils::fillSegSetColor(segmentSet, *bgColor, bgColorMode);
}

//fills in the pixel arrays with random locations and fade values
//Note that the locations are choosen from the line or pixel numbers in the segment set, depending on lineMode
//Only fills the arrays up to numGlims since if we're working with two pixel sets, 
//the second numGlims length of the array is used for the fading out set, 
//which should already be set, and we don't want to overwrite it
//We also set the step to zero, since we're starting a new set of pixels
void GlimmerSL::fillPixelArray(){
    step = 0;
    //we set the segment vars here since the pixel locations depend on them
    //if we're in line mode, then we're drawing full lines,
    //otherwise we're drawing individual pixels
    if(lineMode){
        numLines = segmentSet.maxSegLength;
    } else {
        numLines = segmentSet.numLeds;
    }

    numSegs = segmentSet.numSegs;
    for(uint16_t i = 0; i < numGlims; i++){
        totFadeSteps[i] = random8(fadeMin, fadeMax); //target fade amount between the bgColor and the glimmer color
        fadePixelLocs[i] = random16(numLines);
    }
}

//Only relevant for when we're using two sets of pixels
//once a fade in is finished, we need to copy those array values into
//the second section of the array so they can be faded out.
//Note that we don't care about the pixels in the faded out section since they have finished fading
void GlimmerSL::advancePixelArray(){
    for(uint16_t i = 0; i < numGlims; i++){
        totFadeSteps[i + numGlims] = totFadeSteps[i];
        fadePixelLocs[i + numGlims] = fadePixelLocs[i];
    }
}

//sets the number of random pixels choosen per cycle
//If the number of pixels is different than the current numGlims,
//the glimmer arrays will be re-created, and the effect will be reset
//(otherwise, nothing will happen)
void GlimmerSL::setNumGlims(uint16_t newNumGlims){
    if(newNumGlims != numGlims){
        numGlims = newNumGlims;
        setupPixelArray();
    }
}

//changes the effect to use one or two sets for fading
//if the new value is different than the current, the pixel arrays
//will be reset, restarting the effect
void GlimmerSL::setTwoSets(bool newSetting){
    if(newSetting != twoPixelSets){
        twoPixelSets = newSetting;
        setupPixelArray();
    }
}

//How the effect works:
    //The goal of the effect is to fade a random set of leds in and out, between the glimmerColor and the bgColor
    //each led fades a random amount towards glimmerColor, but all leds fade together (their fades finish at the same time)
    //Note that we start at the background color, fade towards the glimmer color, then fade back down
    //To keep track of the fading led locations and the fade amounts we use two arrays (uint16_t for the locations and uint8_t for the fade)
    //these are set by fillPixelArray()
    //the fade amounts are the number of steps out to fadeMax steps towards the glimmerColor
    //we work out the faded glimmer color on the fly to account for different colorModes

    //Each cycle, we fade the leds one step towards their target glimmer colors
    //once we reach the fadeSteps steps, then we reverse, and fade them back to the bgColor
    //the fade direction is controlled by the bool fadeIn (true for towards the glimmer color)
    //once the fade in and outs are finished, we choose a new set of pixels and fade amounts

    //There a two sub-modes for the effect: one using one set of leds and one using two sets
    //The sub-mode is indicated by the twoPixelSets bool (true for two sets of pixels)
        //For the single set, we only work with one set of leds at a time, fading them in and out completely before choosing a new set
        //For this mode, the array lengths are equal to the numGlims (num leds we're fading)

        //For two sets, then we fade one set of pixels out while another is fading in
        //so we're doing two sets of fades simultaneously 
        //For this mode, the array lengths are double the numGlims
        //The first length up to numGlims stores the location/fade data for fading in leds
        //While the second length stores data for the fading out leds
        //when we reach the fadeSteps steps, we copy the fading in data into the second length using advancePixelArray()
        //and generate a new set of leds in the first length

void GlimmerSL::update(){
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
        step = addmod8(step, 1, fadeSteps + 1);

        //if we have two pixel sets then there's always going to be a set fading in
        //but if there's only one set, then that's no longer true
        if(twoPixelSets){
           fadeIn = true; 
        }

        //run over all the pixels in the glimmer array and output them
        for(uint16_t i = 0; i < glimArrLen; i++){
            
            //only relevant for when we have two pixel sets
            //when we have two pixel sets, the pixels in the array 
            //after numGlims will be fading out, so we need to set
            //fadeIn to false, but we only need to do this once per loop
            if(fadeIn && i >= numGlims) {
                //if we're fading for the first time then the second half of the arrays
                //will be empty, so we skip them
                if(firstFade){
                    break;
                }
                fadeIn = false;
            }

            //if we're in line mode, then we draw the glimmers along the segment lines, 
            //otherwise, each glimmer is a single pixel
            if(lineMode){
                //We draw the glimmers along segment lines, so for each glimmer location
                //we need to fill in all the segment pixesl on the line
                for(uint16_t j = 0; j < numSegs; j++){
                    //get the physical pixel location
                    pixelNum = segDrawUtils::getPixelNumFromLineNum(segmentSet, numLines, j, fadePixelLocs[i]);

                    //get the background color for the pixel, we check this every cycle to account for color modes
                    startColor = segDrawUtils::getPixelColor(segmentSet, pixelNum, *bgColor, bgColorMode, j, fadePixelLocs[i]);
                    
                    //get the final target glimmer color, accounting for color modes
                    targetColor = segDrawUtils::getPixelColor(segmentSet, pixelNum, *glimmerColor, colorMode, j, fadePixelLocs[i]);

                    //get the final faded color
                    fadeColor = getFadeColor(i);

                    segDrawUtils::setPixelColor(segmentSet, pixelNum, fadeColor, 0, j, fadePixelLocs[i]);
                }
            } else {
                //For single pixel's we need to get the info of where the pixel is in the segment set, then set its color
                //grab the background color, accounting for color modes
                //this also fills in the pixelInfo struct, telling us the pixel's segment number, line number, and physical address
                segDrawUtils::getPixelColor(segmentSet, &pixelInfo, *bgColor, bgColorMode, fadePixelLocs[i]);
                startColor = pixelInfo.color;

                //get the glimmer color, accounting for color modes
                targetColor = segDrawUtils::getPixelColor(segmentSet, pixelInfo.pixelLoc, *glimmerColor, colorMode, pixelInfo.segNum, pixelInfo.lineNum);
                
                //get the final faded color
                fadeColor = getFadeColor(i);

                segDrawUtils::setPixelColor(segmentSet, pixelInfo.pixelLoc, fadeColor, 0, 0, 0);
            }
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

//Returns the cross faded output color depending on if we're fading in or out
//NOTE that the color fading vars are set in the draw functions
CRGB GlimmerSL::getFadeColor(uint8_t glimNum){
    //determine final transition color for the fade
    //ie how far towards the glimmer color we're going
    fadeColor = colorUtilsPS::getCrossFadeColor(startColor, targetColor, totFadeSteps[glimNum], fadeMax);

    //if we're not fading in, then we're starting from the fadeColor
    //so we need to switch it with the startColor
    if(!fadeIn){
        targetColor = fadeColor;
        fadeColor = startColor;
        startColor = targetColor;
    } 

    //the final output color, blended towards the fade color by the number of steps
    return colorUtilsPS::getCrossFadeColor(startColor, fadeColor, step, fadeSteps);
}
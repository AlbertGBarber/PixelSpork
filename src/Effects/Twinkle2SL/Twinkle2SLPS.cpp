#include "Twinkle2SLPS.h"

//pallet based constructor
Twinkle2SLPS::Twinkle2SLPS(SegmentSet &SegmentSet, palletPS *Pallet, CRGB BgColor, uint16_t NumTwinkles, uint8_t SpawnChance,
                       uint8_t FadeInSteps, uint8_t FadeInRange, uint8_t FadeOutSteps, uint8_t FadeOutRange, uint16_t Rate):
    segmentSet(SegmentSet), pallet(Pallet), numTwinkles(NumTwinkles), spawnChance(SpawnChance), fadeInRange(FadeInRange), fadeOutRange(FadeOutRange)
    {    
        init(FadeInSteps, FadeOutSteps, BgColor, Rate);
	}

//single color constructor
Twinkle2SLPS::Twinkle2SLPS(SegmentSet &SegmentSet, CRGB Color, CRGB BgColor, uint16_t NumTwinkles, uint8_t SpawnChance,
                       uint8_t FadeInSteps, uint8_t FadeInRange, uint8_t FadeOutSteps, uint8_t FadeOutRange, uint16_t Rate):
    segmentSet(SegmentSet), numTwinkles(NumTwinkles), spawnChance(SpawnChance), fadeInRange(FadeInRange), fadeOutRange(FadeOutRange)
    {    
        setSingleColor(Color);
        init(FadeInSteps, FadeOutSteps, BgColor, Rate);
	}

//random colors constructor
Twinkle2SLPS::Twinkle2SLPS(SegmentSet &SegmentSet, CRGB BgColor, uint16_t NumTwinkles, uint8_t SpawnChance,
                       uint8_t FadeInSteps, uint8_t FadeInRange, uint8_t FadeOutSteps, uint8_t FadeOutRange, uint16_t Rate):
    segmentSet(SegmentSet), numTwinkles(NumTwinkles), spawnChance(SpawnChance), fadeInRange(FadeInRange), fadeOutRange(FadeOutRange)
    {    
        //we make a random pallet of one color so that 
        //if we switch to randMode 0 then we have a pallet to use
        setSingleColor(colorUtilsPS::randColor()); 
        //since we're choosing colors at random, set the randMode
        randMode = 1;
        init(FadeInSteps, FadeOutSteps, BgColor, Rate);
	}

Twinkle2SLPS::~Twinkle2SLPS(){
    delete[] palletTemp.palletArr;
    deleteTwinkleSet();
}

//sets up all the core class vars, and initilizes the twinkle set struct
void Twinkle2SLPS::init(uint8_t FadeInSteps, uint8_t FadeOutSteps, CRGB BgColor, uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    bindBGColorPS();
    setSteps(FadeInSteps, FadeOutSteps);
    initTwinkleArrays();
    reset();
}

//creates an pallet of length 1 containing the passed in color
void Twinkle2SLPS::setSingleColor(CRGB Color){
    delete[] palletTemp.palletArr;
    palletTemp = palletUtilsPS::makeSingleColorPallet(Color);
    pallet = &palletTemp;
}

//Creates the an array of twinkles and a twinkleSet struct for storing them
void Twinkle2SLPS::initTwinkleArrays(){

    //delete the old arrays to prevent memory leak
    deleteTwinkleSet();

    //make a new set of twinkles and a twinkleSet
    //and bind the twinkleSet to the twinkles
    twinkleArr = new twinkleStarPS*[numTwinkles];
    twinkleSetTemp = { twinkleArr, numTwinkles };
    twinkleSet = &twinkleSetTemp;
    for(uint8_t i = 0; i < numTwinkles; i++ ){
        twinkleStarPS* t = new twinkleStarPS();
        twinkleSetTemp.setTwinkle(t, i);
    }
    reset();
}

//Deletes twinkleSetTemp and any twinkles in it
//We need to do this because twinkleSetTemp is always created using new
//So we need to free up the memory if we ever create a new twinkleSet
void Twinkle2SLPS::deleteTwinkleSet(){
    //we need to delete all the twinkles in the set before deleting the twinkle array
    for(uint8_t i = 0; i < twinkleSetTemp.length; i++ ){
        delete[] twinkleSetTemp.twinkleArr[i];
    }
    delete[] twinkleSetTemp.twinkleArr;
}

//resets all the twinkles to be inactive
//and also fills the segment set with the background to clear any active twinkles
void Twinkle2SLPS::reset(){
    for(uint8_t i = 0; i < twinkleSet->length; i++ ){
        twinklePtr = twinkleSet->twinkleArr[i];
        twinklePtr->active = false;
        twinklePtr->stepNum = 0;
    }
    segDrawUtils::fillSegSetColor(segmentSet, *bgColor, bgColorMode);
}

//Sets the number of fade in and out steps (min value of 1)
//You can also set the steps without using this function, but make sure they are greater than 0!
void Twinkle2SLPS::setSteps(uint8_t newfadeInSteps, uint8_t newfadeOutSteps){
    fadeInSteps = newfadeInSteps;
    if(fadeInSteps < 1){
       fadeOutSteps = 1; 
    }

    fadeOutSteps = newfadeOutSteps;
    if(fadeOutSteps < 1){
       fadeOutSteps = 1; 
    }
}

//Sets the number of twinkles, and re-creates the twinkle array and twinkleSet
void Twinkle2SLPS::setNumTwinkles(uint16_t newNumTwinkles){
    numTwinkles = newNumTwinkles;
    initTwinkleArrays();
}

//updates the effect
//How it works:
    //Each cycle, for any active twinkles we draw them and then advance the twinkle step they're on
    //For any inactive twinkles we try to spawn them
    //If we're limiting spawing we use the spawnOk flag to stop new twinkles from spawning if one has already spawned this cycle
void Twinkle2SLPS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        numSegs = segmentSet.numSegs;
        numLines = segmentSet.maxSegLength;

        //we start spawnOk out as true
        //if we're limiting spawing, it will be set false as soon as a twinkle spawns for this cycle
        spawnOk = true;

        //by default, we only touch pixels that are fading
        //but for rainbow or gradient backgrounds that a cycling
        //you want to redraw the whole thing
        if(fillBG){
            segDrawUtils::fillSegSetColor(segmentSet, *bgColor, bgColorMode);
        }

        //Run over each twinkle, if it's active write out it's color based on it's step value
        //Otherwise try to spawn it
        for (uint16_t i = 0; i < numTwinkles; i++) {
            //get the twinkle from the set, and record some vars locally for ease of access
            twinklePtr = twinkleSet->twinkleArr[i];
            if(twinklePtr->active){
                //fetch the twinkle's vars
                twinkleLoc = twinklePtr->location;
                twinkleStep = twinklePtr->stepNum;
                twinkleColor = twinklePtr->color;
                twinkleFadeInSteps = twinklePtr->fadeInSteps;
                twinkleFadeOutSteps = twinklePtr->fadeOutSteps;

                //we either fade in or out depending which index we're on (earlier fade in, later fade out)
                //we don't want to double count the final step of the fade in and the initial step of the fade out
                //since they're the same. (we add 1 to the step for this reason)
                if(twinkleStep < twinkleFadeInSteps) {
                    fadeIn = true;
                } else {
                    fadeIn = false;
                    //+1 since we skip the first step of the fade in
                    twinkleStep = (twinkleStep - twinkleFadeInSteps + 1);
                    //Check if we've reached the final fade out step
                    //if so, we've finished fading, so we set the twinkle as inactive
                    if(twinkleStep >= twinkleFadeOutSteps){ 
                        twinkleStep = twinkleFadeOutSteps; 
                        twinklePtr->active = false;
                    }
                }
                //advance the twinkle's step
                twinklePtr->stepNum++;

                //draw twinkle along it's segment line
                //ie fill in all the pixels on the twinkle's segment line
                for(uint8_t j = 0; j < numSegs; j++){
                    //get the physical pixel location based on the line and seg numbers
                    pixelNum = segDrawUtils::getPixelNumFromLineNum(segmentSet, numLines, j, twinkleLoc);
                    //grab the background color, accounting for color modes
                    colorTarget = segDrawUtils::getPixelColor(segmentSet, pixelNum, *bgColor, bgColorMode, j, twinkleLoc);
                    //get the twinkle color, accounting for color modes
                    twinkleColor = segDrawUtils::getPixelColor(segmentSet, pixelNum, twinkleColor, colorMode, j, twinkleLoc);

                    //get the faded output color depending on if we're fading in or out
                    if(fadeIn){
                        //step is twinkleStep + 1, since we skip the first step (fully Bg)
                        colorOut = colorUtilsPS::getCrossFadeColor(colorTarget, twinkleColor, twinkleStep + 1, twinkleFadeInSteps);
                    } else {
                        //fading out
                        colorOut = colorUtilsPS::getCrossFadeColor(twinkleColor, colorTarget, twinkleStep, twinkleFadeOutSteps);
                    }
                    //set the pixel color
                    segmentSet.leds[pixelNum] = colorOut;
                }
            } else {  
                //try to spawn any inactive twinkles
                if( (random8(100) < spawnChance) && spawnOk){
                    spawnTwinkle(i);
                    if(limitSpawing){
                        spawnOk = false;
                    }
                }
            }
        }
        showCheckPS();
    }
}

//Set a twinkle to active and gives it a new location, fade in/out step values, and color
void Twinkle2SLPS::spawnTwinkle(uint8_t twinkleNum){
    twinklePtr = twinkleSet->twinkleArr[twinkleNum];
    //spawn a twinkle on a random segment line
    twinklePtr->location = random16(numLines);
    twinklePtr->active = true;

    twinklePtr->stepNum = 0;
    twinklePtr->fadeInSteps = fadeInSteps + random8(fadeInRange);
    twinklePtr->fadeOutSteps = fadeOutSteps + random8(fadeOutRange);

    //pick the color either from the pallet, or at random
    switch (randMode) {
        case 0: // we're picking from a set of colors 
            twinkleColor = palletUtilsPS::getPalletColor(pallet, random8(pallet->length));
            break;
        default: //(mode 1) set colors at random
            twinkleColor = colorUtilsPS::randColor();
            break;
    }
    twinklePtr->color = twinkleColor;
}
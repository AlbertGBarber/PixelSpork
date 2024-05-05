#include "Twinkle2SLSeg.h"

//palette based constructor
Twinkle2SLSeg::Twinkle2SLSeg(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, uint16_t NumTwinkles,
                             uint16_t SpawnChance, uint8_t FadeInSteps, uint8_t FadeInRange, uint8_t FadeOutSteps,
                             uint8_t FadeOutRange, uint8_t SegMode, uint16_t Rate)
    : palette(&Palette), numTwinkles(NumTwinkles), spawnChance(SpawnChance), fadeInRange(FadeInRange),
      fadeOutRange(FadeOutRange), segMode(SegMode)  //
{
    init(FadeInSteps, FadeOutSteps, BgColor, SegSet, Rate);
}

//single color constructor
Twinkle2SLSeg::Twinkle2SLSeg(SegmentSetPS &SegSet, CRGB Color, CRGB BgColor, uint16_t NumTwinkles, uint16_t SpawnChance,
                             uint8_t FadeInSteps, uint8_t FadeInRange, uint8_t FadeOutSteps, uint8_t FadeOutRange,
                             uint8_t SegMode, uint16_t Rate)
    : numTwinkles(NumTwinkles), spawnChance(SpawnChance), fadeInRange(FadeInRange), fadeOutRange(FadeOutRange),
      segMode(SegMode)  //
{
    setSingleColor(Color);
    init(FadeInSteps, FadeOutSteps, BgColor, SegSet, Rate);
}

Twinkle2SLSeg::~Twinkle2SLSeg() {
    free(paletteTemp.paletteArr);
    deleteTwinkleSet();
}

//sets up all the core class vars, and initializes the twinkle set struct
void Twinkle2SLSeg::init(uint8_t FadeInSteps, uint8_t FadeOutSteps, CRGB BgColor, SegmentSetPS &SegSet, uint16_t Rate) {
    //bind the rate and segSet pointer vars since they are inherited from BaseEffectPS
    bindSegSetPtrPS();
    bindClassRatesPS();
    //bind the bgColor pointer
    bindBGColorPS();

    //bind the twinkleSet pointer to our twinkle set
    //(there's no reason to use an external twinkle set)
    twinkleSet = &twinkleSetTemp;

    setSteps(FadeInSteps, FadeOutSteps);
    setNumTwinkles(numTwinkles);
    reset();
}

//creates an palette of length 1 containing the passed in color
void Twinkle2SLSeg::setSingleColor(CRGB Color) {
    free(paletteTemp.paletteArr);
    paletteTemp = paletteUtilsPS::makeSingleColorPalette(Color);
    palette = &paletteTemp;
}

//Creates the an array of twinkles and a twinkleSet struct for storing them
void Twinkle2SLSeg::initTwinkleArrays() {

    //delete the old arrays to prevent memory leak
    deleteTwinkleSet();

    //make a new set of twinkles and a twinkleSet
    //and bind the twinkleSet to the twinkles
    twinkleArr = (twinkleStarPS **)malloc(numTwinkles * sizeof(twinkleStarPS *));
    twinkleSetTemp = {twinkleArr, numTwinkles, numTwinkles};
    twinkleSet = &twinkleSetTemp;
    for( uint16_t i = 0; i < numTwinkles; i++ ) {
        twinkleStarPS *t = (twinkleStarPS *)malloc(sizeof(struct twinkleStarPS));
        twinkleSetTemp.setTwinkle(t, i);
    }
    reset();
}

//Deletes twinkleSetTemp and any twinkles in it
//We need to do this because twinkleSetTemp is always created using new
//So we need to free up the memory if we ever create a new twinkleSet
void Twinkle2SLSeg::deleteTwinkleSet() {
    twinkleSetTemp.deleteTwinkleSet();
}

//resets all the twinkles to be inactive
//and also fills the segment set with the background to clear any active twinkles
void Twinkle2SLSeg::reset() {
    twinkleSet->reset(); //sets all the twinkles to inactive and their blend step to 0.
    segDrawUtils::fillSegSetColor(*segSet, *bgColor, bgColorMode);
}

//Sets the number of fade in and out steps (min value of 1)
//You can also set the steps without using this function, but make sure they are greater than 0!
void Twinkle2SLSeg::setSteps(uint8_t newFadeInSteps, uint8_t newFadeOutSteps) {
    fadeInSteps = newFadeInSteps;
    if( fadeInSteps < 1 ) {
        fadeOutSteps = 1;
    }

    fadeOutSteps = newFadeOutSteps;
    if( fadeOutSteps < 1 ) {
        fadeOutSteps = 1;
    }
}

//Sets the number of twinkles, if we need more space for the twinkles, re-creates the twinkle array and twinkleSet
//If the new number of twinkles is different from the current, we reset the effect
void Twinkle2SLSeg::setNumTwinkles(uint16_t newNumTwinkles) {
    //We only need to make new twinkle array if the current one isn't large enough
    //This helps prevent memory fragmentation by limiting the number of heap allocations
    //but this may use up more memory overall.
    if( alwaysResizeObj_PS || (newNumTwinkles > twinkleSet->maxLength) ) {
        numTwinkles = newNumTwinkles;
        initTwinkleArrays();
    } else if( newNumTwinkles != numTwinkles ) {
        numTwinkles = newNumTwinkles;
        //If the new number of twinkles is different than the current number,
        //we have to reset() to clear any that may no longer be updated
        reset();
    }
}

//changes the segMode, if the new segMode is different than the current one, the effect is reset()
//this prevents us from trying to write to a twinkle who is located off the segment set due to the new segMode
void Twinkle2SLSeg::setSegMode(uint8_t newSegMode) {
    if( segMode != newSegMode ) {
        segMode = newSegMode;
        reset();
    }
}

/* updates the effect
How it works:
    Each cycle, for any active twinkles we draw them and then advance the twinkle step they're on
    For any inactive twinkles we try to spawn them
    If we're limiting spawning we use the spawnOk flag to stop new twinkles from spawning if one has already spawned this cycle */
void Twinkle2SLSeg::update() {
    currentTime = millis();

    if( (currentTime - prevTime) >= *rate ) {
        prevTime = currentTime;

        numSegs = segSet->numSegs;
        numLines = segSet->numLines;
        numLeds = segSet->numLeds;

        //we start spawnOk out as true
        //if we're limiting spawning, it will be set false as soon as a twinkle spawns for this cycle
        spawnOk = true;

        //by default, we only touch pixels that are fading
        //but for rainbow or gradient backgrounds that a cycling
        //you want to redraw the whole thing
        if( fillBg ) {
            segDrawUtils::fillSegSetColor(*segSet, *bgColor, bgColorMode);
        }

        //Run over each twinkle, if it's active write out it's color based on it's step value
        //Otherwise try to spawn it
        for( uint16_t i = 0; i < numTwinkles; i++ ) {
            //get the twinkle from the set, and record some vars locally for ease of access
            twinklePtr = twinkleSet->twinkleArr[i];
            if( twinklePtr->active ) {
                //fetch the twinkle's vars
                twinkleLoc = twinklePtr->location;
                twinkleStep = twinklePtr->stepNum;
                twinkleColor = twinklePtr->color;
                twinkleFadeInSteps = twinklePtr->fadeInSteps;
                twinkleFadeOutSteps = twinklePtr->fadeOutSteps;

                //we either fade in or out depending which index we're on (earlier fade in, later fade out)
                //we don't want to double count the final step of the fade in and the initial step of the fade out
                //since they're the same. (we add 1 to the step for this reason)
                if( twinkleStep < twinkleFadeInSteps ) {
                    fadeIn = true;
                } else {
                    fadeIn = false;
                    //+1 since we skip the first step of the fade in
                    twinkleStep = (twinkleStep - twinkleFadeInSteps + 1);
                    //Check if we've reached the final fade out step
                    //if so, we've finished fading, so we set the twinkle as inactive
                    if( twinkleStep >= twinkleFadeOutSteps ) {
                        twinkleStep = twinkleFadeOutSteps;
                        twinklePtr->active = false;
                    }
                }
                //advance the twinkle's step
                twinklePtr->stepNum++;
                //draw the twinkle either along a segment, or along a segment line
                switch( segMode ) {
                    case 0:
                    default:
                        drawLineTwinkle();
                        break;
                    case 1:
                        drawSegTwinkle();
                        break;
                    case 2:
                        drawPixelTwinkle();
                        break;
                }
            } else {
                //try to spawn any inactive twinkles
                if( (random16(spawnBasis) <= spawnChance) && spawnOk ) {
                    spawnTwinkle(i);
                    if( limitSpawning ) {
                        spawnOk = false;
                    }
                }
            }
        }
        showCheckPS();
    }
}

//Draws a twinkle along a segment line
//The tinkleLoc is the segment line number
//To draw the twinkle we get the cross fade between the background and the twinkle color
//for each pixel in the segment line
//and then output the color
void Twinkle2SLSeg::drawLineTwinkle() {
    //draw twinkle along it's segment line
    //ie fill in all the pixels on the twinkle's segment line
    for( uint16_t j = 0; j < numSegs; j++ ) {
        //get the physical pixel location based on the line and seg numbers
        pixelNum = segDrawUtils::getPixelNumFromLineNum(*segSet, j, twinkleLoc);
        //grab the background color, accounting for color modes
        colorTarget = segDrawUtils::getPixelColor(*segSet, pixelNum, *bgColor, bgColorMode, j, twinkleLoc);
        //get the twinkle color, accounting for color modes
        twinkleColor = segDrawUtils::getPixelColor(*segSet, pixelNum, twinkleColor, colorMode, j, twinkleLoc);

        //get the faded output color depending on if we're fading in or out
        colorOut = getFadeColor();
        //set the pixel color
        segDrawUtils::setPixelColor(*segSet, pixelNum, colorOut, 0, 0, 0);
    }
}

/* Draws a twinkle along a segment
The tinkleLoc is the segment number
To draw the twinkle we get the cross fade between the background and the twinkle color
To do this we just check the first get the color for the first pixel in the segment
This does prevent a couple of color modes from working properly, but is much faster than fading all the pixels in the segment
Once we have the cross-fade color, we output it to the whole segment
Does not support colorModes 1, 6, 2, 7, 3, 8 */
void Twinkle2SLSeg::drawSegTwinkle() {
    //Get the physical location of the first pixel in the segment
    //We'll use this pixel to get the blended twinkle color
    pixelNum = segDrawUtils::getSegmentPixel(*segSet, twinkleLoc, 0);
    //grab the background color, accounting for color modes
    colorTarget = segDrawUtils::getPixelColor(*segSet, pixelNum, *bgColor, bgColorMode, twinkleLoc, 0);
    //get the twinkle color, accounting for color modes
    twinkleColor = segDrawUtils::getPixelColor(*segSet, pixelNum, twinkleColor, colorMode, twinkleLoc, 0);
    //get the faded output color depending on if we're fading in or out
    colorOut = getFadeColor();
    //fill the segment with color
    segDrawUtils::fillSegColor(*segSet, twinkleLoc, colorOut, 0);
}

/* Draws a twinkle a a specific pixel
The tinkleLoc is the pixel location, local to the segment set (ie the 5th pixel in the segment set)
To draw the twinkle we get the cross fade between the background and the twinkle color
To do this we get the pixel's segment number, line number, and physical address
and then use this info to get the background and twinkle color for the pixel (taking into account any color modes)
we then cross fade between the two colors and output it */
void Twinkle2SLSeg::drawPixelTwinkle() {
    //grab the background color, accounting for color modes
    //this also fills in the pixelInfo struct, telling us the pixel's segment number, line number, and physical address
    segDrawUtils::getPixelColor(*segSet, twinkleLoc, &pixelInfo, *bgColor, bgColorMode);
    colorTarget = pixelInfo.color;
    //get the twinkle color, accounting for color modes
    twinkleColor = segDrawUtils::getPixelColor(*segSet, pixelInfo.pixelLoc, twinkleColor, colorMode, pixelInfo.segNum, pixelInfo.lineNum);
    //get the faded output color depending on if we're fading in or out
    colorOut = getFadeColor();

    segDrawUtils::setPixelColor(*segSet, pixelInfo.pixelLoc, colorOut, 0, 0, 0);
}

//Returns the cross faded output color depending on if we're fading in or out
//NOTE that the color fading vars are set in the draw functions
CRGB Twinkle2SLSeg::getFadeColor() {
    //get the faded output color depending on if we're fading in or out
    if( fadeIn ) {
        //step is twinkleStep + 1, since we skip the first step (fully Bg)
        return colorUtilsPS::getCrossFadeColor(colorTarget, twinkleColor, twinkleStep + 1, twinkleFadeInSteps);
    } else {
        //fading out
        return colorUtilsPS::getCrossFadeColor(twinkleColor, colorTarget, twinkleStep, twinkleFadeOutSteps);
    }
}

//Set a twinkle to active and gives it a new location, fade in/out step values, and color
void Twinkle2SLSeg::spawnTwinkle(uint8_t twinkleNum) {
    twinklePtr = twinkleSet->twinkleArr[twinkleNum];
    //spawn a twinkle on a random segment line or segment
    switch( segMode ) {
        case 0:
        default:
            twinklePtr->location = random16(numLines);
            break;
        case 1:
            twinklePtr->location = random16(numSegs);
            break;
        case 2:
            twinklePtr->location = random16(numLeds);
            break;
    }
    twinklePtr->active = true;

    twinklePtr->stepNum = 0;
    twinklePtr->fadeInSteps = fadeInSteps + random8(fadeInRange);
    twinklePtr->fadeOutSteps = fadeOutSteps + random8(fadeOutRange);

    //pick the color either from the palette, or at random
    switch( randMode ) {
        case 0:  //we're picking from a set of colors
        default: 
            twinkleColor = paletteUtilsPS::getPaletteColor(*palette, random8(palette->length));
            break;
        case 1:  //(mode 1) set colors at random
            twinkleColor = colorUtilsPS::randColor();
            break;
    }
    twinklePtr->color = twinkleColor;
}
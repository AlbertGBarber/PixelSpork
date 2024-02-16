#include "FairyLightsSLSeg.h"

//see update() for how the effect works

//palette based constructor
FairyLightsSLSeg::FairyLightsSLSeg(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, uint16_t NumTwinkles,
                                   uint8_t Tmode, uint8_t SegMode, uint16_t Rate)
    : palette(&Palette), numTwinkles(NumTwinkles), tMode(Tmode), segMode(SegMode)  //
{
    init(BgColor, SegSet, Rate);
}

//single color constructor
FairyLightsSLSeg::FairyLightsSLSeg(SegmentSetPS &SegSet, CRGB Color, CRGB BgColor, uint16_t NumTwinkles,
                                   uint8_t Tmode, uint8_t SegMode, uint16_t Rate)
    : numTwinkles(NumTwinkles), tMode(Tmode), segMode(SegMode)  //
{
    setSingleColor(Color);
    init(BgColor, SegSet, Rate);
}

//random colors constructor
FairyLightsSLSeg::FairyLightsSLSeg(SegmentSetPS &SegSet, uint16_t NumTwinkles, CRGB BgColor, uint8_t Tmode,
                                   uint8_t SegMode, uint16_t Rate)
    : numTwinkles(NumTwinkles), tMode(Tmode), segMode(SegMode)  //
{
    init(BgColor, SegSet, Rate);
    //we make a random palette of one color so that
    //if we switch to randMode 0 then we have a palette to use
    setSingleColor(colorUtilsPS::randColor());
    //since we're choosing colors at random, set the randMode
    randMode = 1;
}

//destructor
FairyLightsSLSeg::~FairyLightsSLSeg() {
    free(paletteTemp.paletteArr);
    free(colorSet);
    free(twinkleSet);
}

void FairyLightsSLSeg::init(CRGB BgColor, SegmentSetPS &SegSet, uint16_t Rate) {
    //bind the rate and segSet pointer vars since they are inherited from BaseEffectPS
    bindSegSetPtrPS();
    bindClassRatesPS();
    bindBGColorPS();
    setNumTwinkles(numTwinkles);
}

//Restarts the effect by clearing our any existing pixels and spawning a new set
void FairyLightsSLSeg::reset(void){
    cycleNum = 0;

    //clear out any old twinkles and spawn a new set
    segDrawUtils::fillSegSetColor(*segSet, *bgColor, bgColorMode);
    spawnTwinkles();

    //Since we've reset out pixels, we're effectively on the first update again.
    firstUpdate = true;
}

//changes the number of twinkles, re-spawns them, 
//and calls reset() to clear out any existing pixels.
//also resets the twinkleSet storage if needed
void FairyLightsSLSeg::setNumTwinkles(uint16_t newNumTwinkles) {
    numTwinkles = newNumTwinkles;

    //must have at least one twinkle
    if( numTwinkles < 1 ) {
        numTwinkles = 1;
    }

    //We only need to make a new twinkle set if the current one isn't large enough
    //This helps prevent memory fragmentation by limiting the number of heap allocations
    //but this may use up more memory overall.
    if( alwaysResizeObj_PS || (numTwinkles > maxNumTwinkles) ) {
        maxNumTwinkles = numTwinkles;

        free(twinkleSet);
        twinkleSet = (uint16_t *)malloc(numTwinkles * sizeof(uint16_t));

        free(colorSet);
        colorSet = (CRGB *)malloc(numTwinkles * sizeof(CRGB));
    }

    reset();
}

//creates an palette of length 1 containing the passed in color
//binds it to the temp palette to keep it in scope
void FairyLightsSLSeg::setSingleColor(CRGB Color) {
    free(paletteTemp.paletteArr);
    paletteTemp = paletteUtilsPS::makeSingleColorPalette(Color);
    palette = &paletteTemp;
}

//changes the segMode, will re-spawn the twinkles if the segMode is different 
//and reset() the effect to clear out any existing pixels.
void FairyLightsSLSeg::setSegMode(uint8_t newSegMode) {
    if( segMode != newSegMode ) {
        segMode = newSegMode;
        reset();
    }
}

/* Updates the effect
The twinkle locations (local to segment set) and colors are stored in the twinkleSet and colorSet arrays (the colorSet is only used if we're re-drawing)
Each time we update, we advance the cycleNum counter
This corresponds to the twinkle we're working with (ie it's the index of the twinkle and color arrays)
cycleNum wraps at numTwinkles, so we know when we've touched all the twinkles
Other than that, we call whatever mode function to draw the twinkles each cycle
see the individual functions for how they work
Note that twinkles are drawn along segment lines, so each twinkle will light up a whole segment line */
void FairyLightsSLSeg::update() {

    currentTime = millis();

    if( (currentTime - prevTime) >= *rate ) {
        prevTime = currentTime;
        paletteLength = palette->length;

        //the number of cycles to go through the whole array
        //we set this here to allow you to possibly change the numTwinkles on the fly
        cycleLimit = numTwinkles - 1;

        //by default, we only touch the twinkle at the current cycleNum
        //but for rainbow or gradient backgrounds that a cycling
        //you want to redraw the whole thing
        if( fillBg ) {
            reDrawAll = true;  //if fillBg is true, reDrawAll must be set true as well
            segDrawUtils::fillSegSetColor(*segSet, *bgColor, bgColorMode);
        }

        //if we want to prefill our twinkles, we need to do so, but 
        //only for the first update
        if(firstUpdate && prefill){
            preFillTwinkles();
        }
        firstUpdate = false;

        switch( tMode ) {
            case 0:
            default:
                modeZeroSet();
                break;
            case 1:
                modeOneSet();
                break;
            case 2:
                modeTwoSet();
                break;
        }
        cycleNum = addMod16PS(cycleNum, 1, numTwinkles);
        showCheckPS();
    }
}

//Pick a color based on the size of the palette and random modes
CRGB FairyLightsSLSeg::pickColor() {
    switch( randMode ) {
        case 0:  // we're picking from a set of colors
            color = paletteUtilsPS::getPaletteColor(*palette, random8(paletteLength));
            break;
        case 1:
        default:  //(mode 1) set colors at random
            color = colorUtilsPS::randColor();
            break;
    }
    return color;
}

//Creates a set of randomly placed twinkles based on the segMode
void FairyLightsSLSeg::spawnTwinkles() {
    //set the how many twinkle locations there are depending on the segMode
    //(segmentLines, whole segments, or individual pixels)
    switch( segMode ) {
        case 0:
        default:
            twinkleRange = segSet->numLines;
            break;
        case 1:
            twinkleRange = segSet->numSegs;
            break;
        case 2:
            twinkleRange = segSet->numLeds;
            break;
    }

    //pick locations for the twinkles (local to the segment set)
    for( uint16_t i = 0; i < numTwinkles; i++ ) {
        twinkleSet[i] = random16(twinkleRange);
    }
}

//Pre-draws a full set of twinkles on the first update
//as if a full cycle had been completed and we're ready to turn off the pixels.
//Won't work for tMode 0 since all the pixels will be immediately turned off.
//(Which is fine, since that's what that mode does)
void FairyLightsSLSeg::preFillTwinkles() {
    for( uint16_t i = 0; i < numTwinkles; i++ ) {
        //pick a twinkle color, this also gives us the twinkle location info
        color = pickColor();
        colorSet[i] = color;
        drawTwinkle(i, color, colorMode);
    } 

    cycleNum = 0;
    turnOff = true;
}

//Draws the twinkle, either along the whole segment, segment line, or an individual pixel depending on segMode
//Inputs are the index of the twinkle array of the twinkle, the twinkle color, and the color mode
void FairyLightsSLSeg::drawTwinkle(uint16_t twinkleNum, CRGB &tColor, uint8_t cMode) {
    //if in segMode we draw the twinkles along segments, otherwise we draw the along segment lines
    switch( segMode ) {
        case 0:
        default:
            //fill the segment line at the twinkle location with color
            segDrawUtils::drawSegLine(*segSet, twinkleSet[twinkleNum], tColor, cMode);
            break;
        case 1:
            //fill twinkleSet[i]'th segment with color
            segDrawUtils::fillSegColor(*segSet, twinkleSet[twinkleNum], tColor, cMode);
            break;
        case 2:
            //set the single pixel at twinkleSet[twinkleNum] to the tColor
            segDrawUtils::setPixelColor(*segSet, twinkleSet[twinkleNum], tColor, cMode);
            break;
    }
}

/* Mode 0: turns one twinkle on after another and then resets all at once
Overview:
    The function basically has two separate modes, one where we're turning the twinkles on, and one where they're turing off
    While turning on, we switch one twinkle on each cycle, using the twinkleSet and choosing a color each time, storing it in the colorSet
    If we are re-drawing the background each time, we have to re-draw all the twinkle's we've turned on so far
    We do this by looping from the first twinkle to the one at the most current cycle
    and setting the color based on the colorSet
    If we've finished turning all the twinkle's on, then we need to turn them all off on the next cycle
    (At this point we decrement the cycleNum by 1, so that we don't skip the first twinkle when we next turn on)
    Once the twinkle's are off, we generate a new set of twinkle locations, and start over */
void FairyLightsSLSeg::modeZeroSet() {

    if( !turnOff ) {
        //to save space, we always loop to set the twinkles
        //but if we're not re-drawing all, then we only need to draw the twinkle at the current cycleNum
        loopStart = cycleNum;
        if( reDrawAll ) {
            loopStart = 0;
        }

        for( uint16_t i = loopStart; i <= cycleNum; i++ ) {
            //pick a twinkle color
            color = pickColor();
            //if we're re-drawing, then we need to get the color from the colorSet
            //(unless we're setting the twinkle at the current cycleNum, then we need choose a new color)
            if( reDrawAll && (i != cycleNum) ) {
                color = colorSet[i];
            } else {
                colorSet[i] = color;
            }
            //draw the twinkle
            drawTwinkle(i, color, colorMode);
        }

        //if we've reached the cycle limit, we need to turn off all the twinkles on the next cycle
        //we don't want the turn off step to count as a cycle (since turning on the first twinkle would be skipped)
        //so we decrement the cycleNum
        if( cycleNum == cycleLimit ) {
            turnOff = true;
            cycleNum--;
        }

    } else {
        //turn off all the twinkles, and make a new twinkle group to turn on
        for( uint16_t i = 0; i <= cycleLimit; i++ ) {
            drawTwinkle(i, *bgColor, bgColorMode);
        }
        spawnTwinkles();
        turnOff = false;
    }
}

/* Mode 1: Turns on each twinkle one at a time, then off one at a time
Overview:
    The function basically has two separate modes, one where we're turning the twinkles on, and one where they're turing off
    While turning on, we switch one twinkle on each cycle, using the twinkleSet and choosing a color each time, storing it in the colorSet
    If we are re-drawing the background each time, we have to re-draw all the twinkle's we've turned on so far
    We do this by looping from the first twinkle to the one at the most current cycle
    and setting the color based on the colorSet
    If we've finished turning all the twinkle's on, then we need to start turning them off
    This is basically the reverse of turning them on
    If we need to re-draw them, we loop from the current twinkle to the last */
void FairyLightsSLSeg::modeOneSet() {
    //sort out the re-drawing
    //If we're turning the twinkles on, we need to loop through all the twinkles up to the cycleNum twinkle
    //If we're turning them off, we need to loop through all the twinkles starting at the cycleNum twinkle
    //(since all the one's before will already be off)
    if( reDrawAll ) {
        loopStart = 0;
        loopEnd = cycleNum;
        if( turnOff ) {
            loopStart = cycleNum;
            loopEnd = cycleLimit;
        }

        for( uint16_t i = loopStart; i <= loopEnd; i++ ) {
            color = colorSet[i];  //since we're re-drawing, we get the color from the colorSet
            //draw the twinkle
            drawTwinkle(i, color, colorMode);
        }
    }

    if( !turnOff ) {
        //we're turning the twinkles on, so we pick and record a color
        //then set the twinkle
        color = pickColor();
        colorSet[cycleNum] = color;
        //fill the segment line at the twinkle location with color
        //draw the twinkle
        drawTwinkle(cycleNum, color, colorMode);
        //once we've finished drawing all the twinkles, we need to start turning them off
        if( cycleNum == cycleLimit ) {
            turnOff = true;
        }
    } else {
        //turn the twinkles off one at a time
        drawTwinkle(cycleNum, *bgColor, bgColorMode);
        //once we've finished turning all the twinkles off, we need make a new twinkleSet and start again
        if( cycleNum == cycleLimit ) {
            spawnTwinkles();
            turnOff = false;
        }
    }
}

/* Mode 2: Each cycle, a new twinkle is turned on while an old is turned off (first on first off)
Overview:
    Each cycle we first get the twinkle info for the current twinkle (the cycleNum)
    Then we turn it off, and pick a new random location for the twinkle
    We pick a color and draw it
    So each cycle we exchange the current twinkle with a new one
    Since the cycle's are in order, we always turn the twinkles on and off in order
    If we're re-drawing, then we loop through all the twinkles each cycle to set them */
void FairyLightsSLSeg::modeTwoSet() {

    //turn the current twinkle off
    drawTwinkle(cycleNum, *bgColor, bgColorMode);

    //pick a new twinkle to turn on
    twinkleSet[cycleNum] = random16(twinkleRange);  // set a new twinkle to turn on and put it in the array

    //to save space, we always use a loop to draw the twinkles
    //we we're not redrawing, we set the limits to only do one cycle at the current cycleNum
    loopStart = cycleNum;
    loopEnd = cycleNum;
    if( reDrawAll ) {
        loopStart = 0;
        loopEnd = cycleLimit;
    }

    for( uint16_t i = loopStart; i <= loopEnd; i++ ) {
        //pick a twinkle color, this also gives us the twinkle location info
        color = pickColor();
        //if we're re-drawing, then we need to get the color from the colorSet
        //(unless we're setting the twinkle at the current cycleNum, then we need choose a new color)
        if( reDrawAll && (i != cycleNum) ) {
            color = colorSet[i];
        } else {
            colorSet[i] = color;
        }
        //draw the twinkle
        drawTwinkle(i, color, colorMode);
    }
}

#include "RollingWavesSL2.h"

//constructor with pattern
RollingWavesSL2::RollingWavesSL2(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, CRGB BgColor,
                                 uint8_t GradLength, uint8_t TrailMode, uint8_t Spacing, uint16_t Rate)
    : pattern(&Pattern), palette(&Palette), gradLength(GradLength), spacing(Spacing), trailMode(TrailMode)  //
{
    init(BgColor, SegSet, Rate);
}

//constructor with palette as pattern
RollingWavesSL2::RollingWavesSL2(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, uint8_t GradLength,
                                 uint8_t TrailMode, uint8_t Spacing, uint16_t Rate)
    : palette(&Palette), gradLength(GradLength), spacing(Spacing), trailMode(TrailMode)  //
{
    setPaletteAsPattern();
    init(BgColor, SegSet, Rate);
}

//constructor with random colors
RollingWavesSL2::RollingWavesSL2(SegmentSetPS &SegSet, uint8_t NumColors, CRGB BgColor, uint8_t GradLength,
                                 uint8_t TrailMode, uint8_t Spacing, uint16_t Rate)
    : gradLength(GradLength), spacing(Spacing), trailMode(TrailMode)  //
{
    paletteTemp = paletteUtilsPS::makeRandomPalette(NumColors);
    palette = &paletteTemp;
    setPaletteAsPattern();
    init(BgColor, SegSet, Rate);
}

RollingWavesSL2::~RollingWavesSL2() {
    free(paletteTemp.paletteArr);
    free(patternTemp.patternArr);
    free(nextLine);
}

//inits core variables for the effect
void RollingWavesSL2::init(CRGB BgColor, SegmentSetPS &SegSet, uint16_t Rate) {
    //bind the rate and segSet pointer vars since they are inherited from BaseEffectPS
    bindSegSetPtrPS();
    bindClassRatesPS();

    //bind bgColor pointer
    bindBGColorPS();

    cycleNum = 0;
    setTrailMode(trailMode);
    setTotalEffectLength();
    buildLineArr();
}

//sets the gradLength
//we need to change the totalCycleLength to match
void RollingWavesSL2::setGradLength(uint8_t newGradLength) {
    gradLength = newGradLength;
    setTrailMode(trailMode);
}

//sets the pattern to match the current palette
//ie for a palette length 5, the pattern would be
//{0, 1, 2, 3, 4}
void RollingWavesSL2::setPaletteAsPattern() {
    generalUtilsPS::setPaletteAsPattern(patternTemp, *palette);
    pattern = &patternTemp;
}

//creates an array for storing the physical led locations of the
//next line to be drawn
//!!! You should call this if you ever change the segment set
void RollingWavesSL2::buildLineArr() {
    numSegs = segSet->numSegs;

    //We only need to make a new nextLine array if the current one isn't large enough
    //This helps prevent memory fragmentation by limiting the number of heap allocations
    //but this may use up more memory overall.
    if( alwaysResizeObj_PS || (numSegs > numSegsMax) ) {
        numSegsMax = numSegs;
        free(nextLine);
        nextLine = (uint16_t *)malloc(numSegs * sizeof(uint16_t));
    }

    //fetch some core vars
    numLines = segSet->numLines;
    numLinesLim = numLines - 1;
}

/* sets various limits for drawing different types of trails
Note that when the trails are drawn we draw the leading trail first, then the center pixel
and then the ending trail
So we adjust the limits to set the lengths of each trail depending on the mode
Length vars:
    firstHalfGrad => the length of the leading trail (fades in from 0) 
                    anything after this will be the ending trail
    halfGrad => The length of half a wave (or full wave if not mode 1)
    blendStepAdjust => offsets the blend step if needed
    midPoint => The location of the pixel with the highest brightness in the wave
               ie the wave "head"
The Modes:
 0: Only ending trail will be drawn
 1: Both ending and leading trails will be drawn (at half gradLength)
 2: Only the leading trail will be drawn */
void RollingWavesSL2::setTrailMode(uint8_t newTrailMode) {
    trailMode = newTrailMode;
    switch( trailMode ) {
        case 0:  //end trail only
            //Since we only draw the end trail firstHalfGrad = 0
            //The first pixel is the "head", so the midPoint is 0
            halfGrad = gradLength;
            firstHalfGrad = 0;
            blendStepAdjust = 0;
            midPoint = 0;
            break;
        case 2:  //leading trail only
            //Since we only draw the leading trail firstHalfGrad = gradLength
            //The last pixel is the "head", so the midPoint is gradLength - 1
            halfGrad = gradLength;
            firstHalfGrad = gradLength;
            blendStepAdjust = 0;
            midPoint = gradLength - 1;
            break;
        default:  //case 1, both trails
            //Since we're drawing both trails, halfGrad = (gradLength)/2 <- rounds down
            //and firstHalfGrad = halfGrad, so both trails are equal length
            //blendStepAdjust = 0 or 1 to adjust the center for odd length waves
            //The "head" is the wave center which is at halfGrad
            halfGrad = (gradLength) / 2;
            firstHalfGrad = halfGrad;
            blendStepAdjust = mod16PS(gradLength, 2);
            //if( (gradLength % 2) != 0){
            //blendStepAdjust = 1;
            //}
            midPoint = halfGrad;
            break;
    }
}

//We need to pre-fill the strip with the first set of waves
//in order for the led colors to be copied properly in the main update cycle
void RollingWavesSL2::initalFill() {
    cycleNum = 0;

    //Calculate the total cycle length based on the current gradLength and spacing
    setTotalEffectLength();

    //we need to draw the initial waves on the strip
    //to pre-fill it for the main update cycle
    //to do this we run across all the leds
    //every gradSteps number of leds, we increment the wave colors,
    //we loop forwards to match the direction of the loop in update()
    //so that where this initial setup ends,
    //the cycleNum var will be correct
    //We run across them backwards so that the end pattern runs positively along the strip
    for( int32_t i = numLinesLim; i >= 0; i-- ) {

        // if we've gone through gradLength cycles
        // a wave is finished and we need to move to the next color
        if( cycleNum == 0 ) {
            setNextColors(i);
        }
        colorOut = getWaveColor(cycleNum);

        //Draw the colored line
        segDrawUtils::drawSegLine(*segSet, i, colorOut, 0);

        cycleNum = addmod8(cycleNum, 1, blendLimit);  //track what step we're on in the wave
    }
    initFillDone = true;
}

/* Updates the effect
For all lines up to the final on we copy the color of the line behind it
To save time, we store the current line pixels locations in the nextLine array, and then use them in the next loop step
This saves us from having to work out the pixels for both lines at each step
But also causes an "error" with segment sets with different length segments
Because one pixel can be in multiple lines, so the colors of those pixels can be copied multiple times
This creates a neat effect, but is technically incorrect
For the final led we calculate the next wave blendStep based on the cycleNum and the pattern
The blendStep after cycle, which tracks what step of the wave we're on
The blendStep cycles between 0 and the blendLimit (gradLength + spacing)
If the blendStep is 0, then a wave and it's spacing have finished and we choose the next color for the next wave
To draw the waves we use the variables set by setTrailMode()
We first draw the leading trail up to firstHalfGrad
Then we draw the ending trail up to gradLength
After the gradLength we draw any spacing pixels
At some point during the wave we will hit the midPoint, this is the "head" of the wave
and is drawn at full brightness (so that the wave dimming can be non-linear, but the "head" is always full color)
Once all the leds have been filled a cycle is complete and cycleNum is incremented */
void RollingWavesSL2::update() {
    currentTime = millis();

    if( (currentTime - prevTime) >= *rate ) {
        prevTime = currentTime;

        //re-calculate the total cycle length based on the current gradLength and spacing
        //This allows you to change their values on the fly
        setTotalEffectLength();

        //We need to pre-fill the strip with a full cycle the first time the update is called
        //so that the colors are copied down the strip correctly on subsequent cycles
        if( !initFillDone ) {
            initalFill();
        }

        //prep for the loop below.
        //We need to get the pixel locations of the final segment line
        //(Which is the one the loop starts on)
        for( uint16_t j = 0; j < numSegs; j++ ) {
            nextLine[j] = segDrawUtils::getPixelNumFromLineNum(*segSet, j, numLinesLim);
        }

        //Run backwards across all the segment lines and copy the color from the line before it
        //Running backwards makes the pattern move in the positive direction (forwards)
        //If we're at the first line, we pick new color
        for( int32_t i = numLinesLim; i >= 0; i-- ) {

            //if we're at the first line, pick a color, otherwise just copy the colors from the line before it
            if( i == 0 ) {
                blendStep = addmod8(cycleNum, i, blendLimit);  // what step we're on between the current and next color
                //If the blendStep is 0, then a wave has finished, and we need to choose the next color
                if( blendStep == 0 ) {
                    //the color we're at based on the current index
                    setNextColors(i);
                }

                //get the next wave step color and set it
                colorOut = getWaveColor(blendStep);

                //Draw the line
                //Note that we only need to the pixels from the nextLine array
                //because they were looked up in the previous loop step
                for( uint16_t j = 0; j < numSegs; j++ ) {
                    //The nextPixelNumber from the previous loop iteration is now
                    //the pixelNumber for this iteration
                    pixelNum = nextLine[j];
                    segDrawUtils::setPixelColor(*segSet, pixelNum, colorOut, 0, j, i);
                }
            } else {
                //copy the pixel colors from the previous line to the current one
                //Note that this is where the copping "error" occurs where colors mis-match
                //between multiple segment lines of different lengths
                for( uint16_t j = 0; j < numSegs; j++ ) {
                    //The pixel locations from the previous loop are now the current ones
                    //and we now need to fill in the next line locations
                    pixelNum = nextLine[j];
                    nextLine[j] = segDrawUtils::getPixelNumFromLineNum(*segSet, j, i - 1);
                    //copy the color of the next pixel in line into the current pixel
                    segSet->leds[pixelNum] = segSet->leds[nextLine[j]];
                }
            }
        }
        cycleNum = addMod16PS(cycleNum, 1, totalCycleLength);
        showCheckPS();
    }
}

//calculates the totalCycleLength, which represents the total number of possible colors a pixel can have
//ie the total length of all the waves (of each color in the pattern) combined
//includes the spacing after each wave
//The blend limit is the length of a wave plus its spacing
void RollingWavesSL2::setTotalEffectLength() {
    // the number of steps in a full cycle (fading through all the colors)
    blendLimit = gradLength + spacing;
    totalCycleLength = pattern->length * blendLimit;
}

/* Returns a wave color blended towards 0 based on the trailMode and the step
For each wave we first draw the leading trail up to firstHalfGrad
Then we draw the ending trail up to gradLength
After the gradLength we draw any spacing pixels
At some point during the wave we will hit the midPoint, this is the "head" of the wave
and is drawn at full brightness (so that the wave dimming can be non-linear, but the "head" is always full color)
Once all the leds have been filled a cycle is complete and cycleNum is incremented */
CRGB RollingWavesSL2::getWaveColor(uint8_t step) {

    //draw the various parts of the wave
    //the limit vars are set by setTrailMode
    //We first draw the leading trail up to firstHalfGrad
    //Then we draw the ending trail up to gradLength
    //After the gradLength we draw any spacing pixels
    if( step < firstHalfGrad ) {
        //For full dimming we want stepTemp to be halfGrad
        //since we're doing the leading trail, we start at halfGrad
        stepTemp = halfGrad - step;
        setBg = false;
    } else if( blendStep < gradLength ) {
        //For the ending trail we do the same as the first trail, but
        //we need to start at 0 (no dimming), and we need to adjust for the inital blendStep value
        stepTemp = halfGrad - (gradLength - step) + blendStepAdjust;
        setBg = false;
    } else {
        setBg = true;
    }

    //return the wave color depending on if the current led is a spacing pixel
    if( setBg ) {
        colorOut = *bgColor;
    } else {
        if( step == midPoint ) {
            //if the blendStep is at the "head" led, we don't dim it
            colorOut = currentColor;
        } else {
            //Get the dimmed wave color, note that we use the same function as particles use to get the color
            //This produces a more non-linear fade (depending on the dimPow), for a better overall look
            //See notes in particleUtils.h or particles.h for more
            colorOut = particleUtilsPS::getTrailColor(currentColor, *bgColor, stepTemp, halfGrad, dimPow);
        }
    }
    return colorOut;
}

//For calling whenever a wave has finished
//Chooses a new color for the next wave depending on the the options for random colors
void RollingWavesSL2::setNextColors(uint16_t segPixelNum) {
    if( randMode == 0 ) {
        currentColorIndex = addMod16PS(segPixelNum, cycleNum, totalCycleLength) / blendLimit;  // what color we've started from (integers always round down)
        //the color we're at based on the current index
        currentPattern = patternUtilsPS::getPatternVal(*pattern, currentColorIndex);
        currentColor = paletteUtilsPS::getPaletteColor(*palette, currentPattern);
    } else if( randMode == 1 ) {
        //choose a completely random color
        currentColor = colorUtilsPS::randColor();
    } else {
        //choose a color randomly from the pattern (making sure it's not the same as the current color)
        currentPattern = patternUtilsPS::getShuffleVal(*pattern, currentPattern);
        currentColor = paletteUtilsPS::getPaletteColor(*palette, currentPattern);
    }
}

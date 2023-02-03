#include "SegWavesFast.h"

//constructor for using the passed in pattern and palette for the wave
SegWavesFast::SegWavesFast(SegmentSet &SegmentSet, patternPS *Pattern, palettePS *Palette, CRGB BgColor, bool Direct, uint16_t Rate):
    segmentSet(SegmentSet), pattern(Pattern), palette(Palette), direct(Direct)
    {    
        init(BgColor, Rate);
	}

//constructor for building the wave pattern from the passed in pattern and the palette, using the passed in colorLength and spacing
SegWavesFast::SegWavesFast(SegmentSet &SegmentSet, patternPS *Pattern, palettePS *Palette, uint8_t WaveThickness, uint8_t Spacing, CRGB BgColor, bool Direct, uint16_t Rate):
    segmentSet(SegmentSet), palette(Palette), direct(Direct)
    {    
        //short cut for creating a single segment wave
        if(WaveThickness == 0){
            WaveThickness = 1;
            Spacing = segmentSet.numSegs;
        }
        setPatternAsPattern(Pattern, WaveThickness, Spacing);
        init(BgColor, Rate);
	}
    
//constructor for building a wave using all the colors in the passed in palette, using the colorLength and spacing for each color
SegWavesFast::SegWavesFast(SegmentSet &SegmentSet, palettePS *Palette, uint8_t WaveThickness, uint8_t Spacing, CRGB BgColor, bool Direct, uint16_t Rate):
    segmentSet(SegmentSet), palette(Palette), direct(Direct)
    {    
        //short cut for creating a single segment wave
        if(WaveThickness == 0){
            WaveThickness = 1;
            Spacing = segmentSet.numSegs;
        }
        setPaletteAsPattern(WaveThickness, Spacing);
        init(BgColor, Rate);
	}

//constructor for doing a single colored wave, using colorLength and spacing
SegWavesFast::SegWavesFast(SegmentSet &SegmentSet, CRGB Color, uint8_t WaveThickness, uint8_t Spacing, CRGB BgColor, bool Direct, uint16_t Rate):
    segmentSet(SegmentSet), direct(Direct)
    {    
        paletteTemp = paletteUtilsPS::makeSingleColorPalette(Color);
        palette = &paletteTemp;
        //short cut for creating a single segment wave
        if(WaveThickness == 0){
            WaveThickness = 1;
            Spacing = segmentSet.numSegs;
        }
        setPaletteAsPattern(WaveThickness, Spacing);
        init(BgColor, Rate);
	}

SegWavesFast::~SegWavesFast(){
    free(paletteTemp.paletteArr);
    free(patternTemp.patternArr);
}

//intilization of core variables and pointers
void SegWavesFast::init(CRGB BgColor, uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    //bind background color pointer
    bindBGColorPS();
    reset();
}

//resets the wave to it's original starting point
void SegWavesFast::reset(){
    cycleNum = 0;
    initFillDone = false;
}

//creates a pattern so that there's only a single segment wave on the segment set at one time
//ie a wave of thickness 1, and a spacing equal to the number of segments, so there's only one 
//wave on the segment at once
void SegWavesFast::makeSingleWave(){
    setPaletteAsPattern(1, segmentSet.numSegs);
}

//takes the passed in pattern and creates a pattern for the wave
//using the passed in color length and spacing
//then sets this pattern to be the wave pattern
//ex : inputPattern is {1, 2, 4} with color length 2, and 1 spacing
//the wave pattern would be: {1, 1, 255, 2, 2, 255, 4, 4, 255}
void SegWavesFast::setPatternAsPattern(patternPS *inputPattern, uint8_t colorLength, uint8_t spacing){
    uint8_t patternIndex;
    uint8_t repeatLength = (colorLength + spacing);
    uint16_t patternLength = inputPattern->length;
    uint16_t totalPatternLength = patternLength * repeatLength;
    free(patternTemp.patternArr);
    uint8_t *pattern_arr = (uint8_t*) malloc(totalPatternLength * sizeof(uint8_t));

    for(uint16_t i = 0; i < patternLength; i++){
        patternIndex = patternUtilsPS::getPatternVal(inputPattern, i);
        for(uint8_t j = 0; j < repeatLength; j++){
            if(j < colorLength){
                pattern_arr[i * repeatLength + j] = patternIndex;
            } else {
                pattern_arr[i * repeatLength + j] = 255;
            }
        }
    }

    patternTemp = {pattern_arr, totalPatternLength};
    pattern = &patternTemp;
}

//sets the current palette to be the wave pattern (using all colors in the palette)
//using the passed in colorLength and spacing
//ex: for palette of lenth 3, and a colorLength of 2, and spacing of 1
//the final wave pattern would be : {0, 0, 255, 1, 1, 255, 2, 2, 255}
void SegWavesFast::setPaletteAsPattern(uint8_t colorLength, uint8_t spacing){
    uint8_t repeatLength = (colorLength + spacing);
    uint8_t palettelength = palette->length;
    uint16_t totalPatternLength = palettelength * repeatLength;
    free(patternTemp.patternArr);
    uint8_t *pattern_arr = (uint8_t*) malloc(totalPatternLength * sizeof(uint8_t));

    for(uint16_t i = 0; i < palettelength; i++){
        for(uint8_t j = 0; j < repeatLength; j++){
            if(j < colorLength){
                pattern_arr[i * repeatLength + j] = i;
            } else {
                pattern_arr[i * repeatLength + j] = 255;
            }
        }
    }

    patternTemp = {pattern_arr, totalPatternLength};
    pattern = &patternTemp;
}


//sets the direction of the effect to either move from the first to last segment or visa versa
//(true is last segment to first)
void SegWavesFast::getDirection(){
    numSegs = segmentSet.numSegs;

    //loop direction and limit variables for the update cycle
    //For this loop, we're moving from the last to first
    //so the startLimit is 0 (first segment), the endLimit is numSegs (last segment)
    //and the step is 1
    //This is because the segment color is copied from the next segment in line
    //The colored segment is the last segment, since that is the origin of new colors
    endLimit = numSegs;
    startLimit = 0;
    loopStep = 1;
    coloredSeg = numSegs - 1;

    //switch the loop variables if we want first segment to last
    //These are basically the opposite of the per last to first settings
    if (!direct) {
        endLimit = -1;
        startLimit = numSegs - 1;
        loopStep = -1;
        coloredSeg = 0;
    }
}

//Since the main update function only picks a new color for the first segment and then shifts the rest forward
//On the first cycle we need to fill in the segments with the waves
//To do this we basically do one full update cycle, drawing the wave pattern onto the whole strip
//note that a spacing pixel is indicated by a pattern value of 255, these pixels will be filled in with the bgColor
void SegWavesFast::initalFill(){
    cycleNum = 0;
    uint16_t patternLength = pattern->length;
    prevPattern = 255; //base value for previous pattern value, it's set to the spacing value b.c we don't expect a pattern to start with spacing
    
    getDirection();
    //We need to draw the initial waves on the segments to pre-fill them for the main update cycle.
    //To do this we run across all the segments, picking a color for each segment,
    //while counting how many update cycle's we've covered
    //The loop limits and step are determined by the effect direction (setDirection())
    //So that the loop either runs from the first to last segment, or visa versa
    for (int32_t i = startLimit; i != endLimit; i += loopStep) {

        nextPattern = patternUtilsPS::getPatternVal(pattern, cycleNum);
        nextColor = pickStreamerColor(nextPattern);

        segDrawUtils::fillSegColor(segmentSet, i, nextColor, 0);
        //every time we fill a segment, we're basically doing one whole update()
        //so we need to increment the cycleNum, so that once the preFill is done, the 
        //next update() call will sync properly
        cycleNum = addMod16PS( cycleNum, 1, patternLength );
    }
    initFillDone = true;
}
    
//Each update cycle, we run along the strip, coping the color of the next segment into the current segment
//effectively shifting all the waves down the strip
//We only choose a new color for the final pixel
//To copy the color we always copy from the first pixel of each segment
//The effect direction determines in which direction we copy from (see setDirection())
//This is much faster than caculating the next pattern value for each pixel, but we cannot do fades, use color modes, or palette blend
//note that a spacing pixel is indicated by a pattern value of 255, these pixels will be filled in with the bgColor
void SegWavesFast::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        //We need to pre-fill the strip with a full cycle the first time the update is called
        //so that the colors are copied down the strip correctly on subsequent cycles
        if(!initFillDone){
            initalFill();
        }

        getDirection();

        //Loop through all the segments, either copying the color of the next segment, 
        //or setting the color if it's the "first" segment
        //To change the direction of motion we adjust the loop start and end points, and the loop step
        //So that we either move from the first to last segment or visa versa
        for (int32_t i = startLimit; i != endLimit; i += loopStep) {
            //If we're at the final segment, we need to insert a new color
            //otherwise, we just copy the color from the next segment into the current one
            if (i == coloredSeg) {
                nextPattern = patternUtilsPS::getPatternVal(pattern, cycleNum);
                nextColor = pickStreamerColor(nextPattern);
            } else {
                //Copy the pixel color from the previous segment, based on the direction (loopStep)
                //To copy the color we always copy from the first pixel in the previous segment
                pixelNum = segDrawUtils::getSegmentPixel(segmentSet, i + loopStep, 0);
                nextColor = segmentSet.leds[pixelNum];
            }
            //Color the segment
            segDrawUtils::fillSegColor(segmentSet, i, nextColor, 0);
        }
        cycleNum = addMod16PS( cycleNum, 1, pattern->length );//one update = one cycle

        showCheckPS();
    }
}

//returns the color of the next wave based on the passed in pattern index
//if the index is 255, this notes a spacing pixel, so the bgColor is returned
//otherwise the color is choosen either from the palette, or randomly
//according to the randMode:
//  0: Colors will be choosen in order from the pattern (not random)
//  1: Colors will be choosen completely at random
//  2: Colors will be choosen at random from the !!palette!!, but the same color won't be repeated in a row
//  3: Colors will be choosen randomly from the pattern
//  4: Colors will be choosen randomly from the !!palette!! 
//     (option included b/c the pattern may have a lot of spaces, so choosing from it may be very biased)
CRGB SegWavesFast::pickStreamerColor(uint8_t patternIndex){
    if(patternIndex == 255){
        nextColor = *bgColor;
    } else if(randMode == 0){
        //the color we're at based on the current index
        nextColor = paletteUtilsPS::getPaletteColor(palette, patternIndex);
    } else if(patternIndex != prevPattern){
        //if we're doing random colors, we still want to stick to the streamer lengths in the pattern
        //but replace the color with a random one 
        //So we only pick a new random color each time the nextPattern is different from the previous one
        //since that indicates a new wave length
        //(we don't need to worry about the 255 spacing values, since they're caught further up in the if)
        if(randMode == 1){
            //choose a completely random color
            nextColor = colorUtilsPS::randColor();
        } else if(randMode == 2) {
            //choose a color randomly from the palette (making sure it's not the same as the current color)
            //(Can't shuffle the pattern directly, because it contains repeats of the same index)
            nextColor = paletteUtilsPS::getShuffleIndex(palette, randColor);
            randColor = nextColor; //record the random color so we don't pick it again
        } else if(randMode == 3) {
            //choose a color randomly from the pattern (can repeat)
            //we use nextPatternRand because we don't want to interfere with nextPattern
            //since it keeps track of the spaces
            nextPatternRand = patternUtilsPS::getPatternVal( pattern, random16(pattern->length) );
            nextColor = paletteUtilsPS::getPaletteColor(palette, nextPatternRand);
        } else {
            //choose a color randomly from the palette (can repeat)
            nextColor = paletteUtilsPS::getPaletteColor( palette, random8(palette->length) );
        }
    }
    prevPattern = patternIndex; //save the current pattern value (only needed for the random color case)
    return nextColor;
}
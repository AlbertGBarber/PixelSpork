#include "StreamerFastSL.h"

//constructor for using the passed in pattern and palette for the streamer
StreamerFastSL::StreamerFastSL(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, CRGB BgColor,
                               uint16_t Rate)
    : pattern(&Pattern), palette(&Palette)  //
{
    init(BgColor, SegSet, Rate);
}

//constructor for building the streamer pattern from the passed in pattern and the palette, using the passed in colorLength and spacing
StreamerFastSL::StreamerFastSL(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, uint8_t ColorLength,
                               uint8_t Spacing, CRGB BgColor, uint16_t Rate)
    : palette(&Palette)  //
{
    setPatternAsPattern(Pattern, ColorLength, Spacing);
    init(BgColor, SegSet, Rate);
}

//constructor for building a streamer using all the colors in the passed in palette, using the colorLength and spacing for each color
StreamerFastSL::StreamerFastSL(SegmentSetPS &SegSet, palettePS &Palette, uint8_t ColorLength, uint8_t Spacing,
                               CRGB BgColor, uint16_t Rate)
    : palette(&Palette)  //
{
    setPaletteAsPattern(ColorLength, Spacing);
    init(BgColor, SegSet, Rate);
}

//constructor for doing a single colored streamer, using colorLength and spacing
StreamerFastSL::StreamerFastSL(SegmentSetPS &SegSet, CRGB Color, uint8_t ColorLength, uint8_t Spacing, CRGB BgColor,
                               uint16_t Rate)  //
{
    paletteTemp = paletteUtilsPS::makeSingleColorPalette(Color);
    palette = &paletteTemp;
    setPaletteAsPattern(ColorLength, Spacing);
    init(BgColor, SegSet, Rate);
}

StreamerFastSL::~StreamerFastSL() {
    free(paletteTemp.paletteArr);
    free(patternTemp.patternArr);
}

//takes the passed in pattern and creates a pattern for the streamer
//using the passed in color length and spacing
//then sets this pattern to be the streamer pattern
//ex : inputPattern is {1, 2, 4} with color length 2, and 1 spacing
//the streamer pattern would be: {1, 1, 255, 2, 2, 255, 4, 4, 255}
void StreamerFastSL::setPatternAsPattern(patternPS &inputPattern, uint8_t colorLength, uint8_t spacing) {
    generalUtilsPS::setPatternAsPattern(patternTemp, inputPattern, colorLength, spacing);
    pattern = &patternTemp;
}

//sets the current palette to be the streamer pattern (using all colors in the palette)
//using the passed in colorLength and spacing
//ex: for palette of length 3, and a colorLength of 2, and spacing of 1
//the final streamer pattern would be : {0, 0, 255, 1, 1, 255, 2, 2, 255}
void StreamerFastSL::setPaletteAsPattern(uint8_t colorLength, uint8_t spacing) {
    generalUtilsPS::setPaletteAsPattern(patternTemp, *palette, colorLength, spacing);
    pattern = &patternTemp;
}

//resets the streamer to it's original starting point
void StreamerFastSL::reset() {
    cycleNum = 0;
    initFillDone = false;
}

//initialization of core variables and pointers
void StreamerFastSL::init(CRGB BgColor, SegmentSetPS &SegSet, uint16_t Rate) {
    //bind the rate and segSet pointer vars since they are inherited from BaseEffectPS
    bindSegSetPtrPS();
    bindClassRatesPS();
    //bind background color pointer
    bindBGColorPS();
    reset();
}

/* returns the color of the next streamer based on the passed in pattern index
if the index is 255, this notes a spacing pixel, so the bgColor is returned
otherwise the color is chosen either from the palette, or randomly
according to the randMode:
    0: Colors will be chosen in order from the pattern (not random)
    1: Colors will be chosen completely at random
    2: Colors will be chosen at random from the !!palette!!, but the same color won't be repeated in a row
    3: Colors will be chosen randomly from the pattern
    4: Colors will be chosen randomly from the !!palette!! 
        (option included b/c the pattern may have a lot of spaces, so choosing from it may be very biased) */
CRGB StreamerFastSL::pickStreamerColor(uint8_t nextPattern) {
    if( nextPattern == 255 ) {
        nextColor = *bgColor;
    } else if( randMode == 0 ) {
        //the color we're at based on the current index
        nextColor = paletteUtilsPS::getPaletteColor(*palette, nextPattern);
    } else if( prevPattern != nextPattern ) {
        //if we're doing random colors, we still want to stick to the streamer lengths in the pattern
        //but replace the color with a random one
        //So we only pick a new random color each time the nextPattern is different from the previous one
        //since that indicates a new wave length
        //(we don't need to worry about the 255 spacing values, since they're caught further up in the if)
        if( randMode == 1 ) {
            //choose a completely random color
            nextColor = colorUtilsPS::randColor();
        } else if( randMode == 2 ) {
            //choose a color randomly from the palette (making sure it's not the same as the current color)
            //(Can't shuffle the pattern directly, because it contains repeats of the same index)
            nextColor = paletteUtilsPS::getShuffleColor(*palette, randColor);
            randColor = nextColor;  //record the random color so we don't pick it again
        } else if( randMode == 3 ) {
            //choose a color randomly from the pattern (can repeat)
            //we use nextPatternRand because we don't want to interfere with nextPattern
            //since it keeps track of the spaces
            nextPatternRand = patternUtilsPS::getPatternVal(*pattern, random16(pattern->length));
            nextColor = paletteUtilsPS::getPaletteColor(*palette, nextPatternRand);
        } else {
            //choose a color randomly from the palette (can repeat)
            nextColor = paletteUtilsPS::getPaletteColor(*palette, random8(palette->length));
        }
    }
    prevPattern = nextPattern;  //save the current pattern value (only needed for the random color cases)
    return nextColor;
}

//since the main update function only picks a new color for the first led and then shifts the rest forward
//On the first cycle we need to fill in the strip with the streamers
//To do this we basically do one full update cycle, drawing the streamer pattern onto the whole strip
//note that a spacing pixel is indicated by a pattern value of 255, these pixels will be filled in with the bgColor
void StreamerFastSL::initalFill() {

    cycleNum = 0;

    //fetch some core vars
    numLines = segSet->numLines;
    numLinesLim = numLines - 1;

    uint16_t patternLength = pattern->length;
    prevPattern = 255;  //base value for previous pattern value (we don't expect the first value of the pattern to be spacing)

    for( int32_t i = numLinesLim; i >= 0; i-- ) {
        nextPattern = patternUtilsPS::getPatternVal(*pattern, cycleNum);
        nextColor = pickStreamerColor(nextPattern);

        //write out the copied color to the whole line
        segDrawUtils::drawSegLine(*segSet, i, nextColor, 0);
        //every time we draw a pixel, we're basically doing one whole update()
        //so we need to increment the cycleNum, so that once the preFill is done, the
        //next update() call will sync properly
        cycleNum = addMod16PS(cycleNum, 1, patternLength);
    }

    initFillDone = true;
}

//Each update cycle, we run along the strip, coping the color of the next pixel into the current pixel
//effectively shifting all the streamers down the strip
//We only choose a new color for the final pixel
//This is much faster than calculating the next pattern value for each pixel, but we cannot do fades, use color modes, or palette blend
//note that a spacing pixel is indicated by a pattern value of 255, these pixels will be filled in with the bgColor
void StreamerFastSL::update() {
    currentTime = millis();

    if( (currentTime - prevTime) >= *rate ) {
        prevTime = currentTime;

        //We need to pre-fill the strip with a full cycle the first time the update is called
        //so that the colors are copied down the strip correctly on subsequent cycles
        if( !initFillDone ) {
            initalFill();
        }

        //fetch some core vars
        numLines = segSet->numLines;
        numLinesLim = numLines - 1;
        longestSeg = segSet->segNumMaxNumLines;

        for( int32_t i = numLinesLim; i >= 0; i-- ) {

            //if we're at the final pixel, we need to insert a new color
            //otherwise, we just copy the color from the next pixel location into the current one
            if( i == 0 ) {
                nextPattern = patternUtilsPS::getPatternVal(*pattern, cycleNum);
                nextColor = pickStreamerColor(nextPattern);
            } else {
                //Copy the pixel color from the previous line
                //To copy the color we always copy from the pixel on the longest segment,
                //Since all the pixels on the longest segment are on separate lines
                //(unlike shorter segments, where a single pixel can be in multiple lines, so it's color may not be what we expect)
                pixelNum = segDrawUtils::getPixelNumFromLineNum(*segSet, numLines, longestSeg, i - 1);
                nextColor = segSet->leds[pixelNum];
            }
            //write out the copied color to the whole line
            segDrawUtils::drawSegLine(*segSet, i, nextColor, 0);
        }
        cycleNum = addMod16PS(cycleNum, 1, pattern->length);  //one update = one cycle

        showCheckPS();
    }
}

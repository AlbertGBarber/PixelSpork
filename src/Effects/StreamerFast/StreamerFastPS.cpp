#include "StreamerFastPS.h"

//constructor for using the passed in pattern and palette for the streamer
StreamerFastPS::StreamerFastPS(SegmentSet &SegmentSet, patternPS *Pattern, palettePS *Palette, CRGB BgColor, uint16_t Rate):
    segmentSet(SegmentSet), pattern(Pattern), palette(Palette)
    {    
        init(BgColor, Rate);
	}

//constructor for building the streamer pattern from the passed in pattern and the palette, using the passed in colorLength and spacing
StreamerFastPS::StreamerFastPS(SegmentSet &SegmentSet, patternPS *Pattern, palettePS *Palette, uint8_t ColorLength, uint8_t Spacing, CRGB BgColor, uint16_t Rate):
    segmentSet(SegmentSet), palette(Palette)
    {    
        setPatternAsPattern(Pattern, ColorLength, Spacing);
        init(BgColor, Rate);
	}
    
//constructor for building a streamer using all the colors in the passed in palette, using the colorLength and spacing for each color
StreamerFastPS::StreamerFastPS(SegmentSet &SegmentSet, palettePS *Palette, uint8_t ColorLength, uint8_t Spacing, CRGB BgColor, uint16_t Rate):
    segmentSet(SegmentSet), palette(Palette)
    {    
        setPaletteAsPattern(ColorLength, Spacing);
        init(BgColor, Rate);
	}

//constructor for doing a single colored streamer, using colorLength and spacing
StreamerFastPS::StreamerFastPS(SegmentSet &SegmentSet, CRGB Color, uint8_t ColorLength, uint8_t Spacing, CRGB BgColor, uint16_t Rate):
    segmentSet(SegmentSet)
    {    
        paletteTemp = paletteUtilsPS::makeSingleColorPalette(Color);
        palette = &paletteTemp;
        setPaletteAsPattern(ColorLength, Spacing);
        init(BgColor, Rate);
	}

StreamerFastPS::~StreamerFastPS(){
    delete[] paletteTemp.paletteArr;
    delete[] patternTemp.patternArr;
}

//takes the passed in pattern and creates a pattern for the streamer
//using the passed in color length and spacing
//then sets this pattern to be the streamer pattern
//ex : inputPattern is {1, 2, 4} with color length 2, and 1 spacing
//the streamer pattern would be: {1, 1, 255, 2, 2, 255, 4, 4, 255}
void StreamerFastPS::setPatternAsPattern(patternPS *inputPattern, uint8_t colorLength, uint8_t spacing){
    uint8_t patternIndex;
    uint8_t repeatLength = (colorLength + spacing);
    uint16_t patternLength = inputPattern->length;
    uint16_t totalPatternLength = patternLength * repeatLength;
    delete[] patternTemp.patternArr;
    uint8_t *pattern_arr = new uint8_t[totalPatternLength];

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

//sets the current palette to be the streamer pattern (using all colors in the palette)
//using the passed in colorLength and spacing
//ex: for palette of lenth 3, and a colorLength of 2, and spacing of 1
//the final streamer pattern would be : {0, 0, 255, 1, 1, 255, 2, 2, 255}
void StreamerFastPS::setPaletteAsPattern(uint8_t colorLength, uint8_t spacing){
    uint8_t repeatLength = (colorLength + spacing);
    uint8_t palettelength = palette->length;
    uint16_t totalPatternLength = palettelength * repeatLength;
    delete[] patternTemp.patternArr;
    uint8_t *pattern_arr = new uint8_t[totalPatternLength];

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

//resets the streamer to it's original starting point
void StreamerFastPS::reset(){
    cycleCount = 0;
    preFillDone = false;
}

//intilization of core variables and pointers
void StreamerFastPS::init(CRGB BgColor, uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    //bind background color pointer
    bindBGColorPS();
    reset();
}

//returns the color of the next streamer based on the passed in pattern index
//if the index is 255, this notes a spacing pixel, so the bgColor is returned
//otherwise the color is choosen either from the palette, or randomly
//according to the randMode
CRGB StreamerFastPS::pickStreamerColor(uint8_t patternIndex){
    if(patternIndex == 255){
        nextColor = *bgColor;
    } else if(randMode == 0){
        //the color we're at based on the current index
        nextColor = paletteUtilsPS::getPaletteColor(palette, patternIndex);
    } else if(patternIndex != prevPattern){
        //if we're doing random colors, we still want to stick to the streamer lengths in the pattern
        //but replace the color with a random one 
        //So we only pick a new random color each time the nextPattern is different from the previous one
        //since that indicates a new streamer length
        //(we don't need to worry about the 255 spacing values, since they're caught further up in the if)
        if(randMode == 1){
            //choose a completely random color
            nextColor = colorUtilsPS::randColor();
        } else {
            //choose a color randomly from the palette
            nextColor = paletteUtilsPS::getPaletteColor( palette, random8(palette->length) );
        }
    }
    prevPattern = patternIndex; //save the current pattern value (only needed for the random color case)
    return nextColor;
}

//since the main update function only picks a new color for the first led and then shifts the rest forward
//On the first cycle we need to fill in the strip with the streamers
//To do this we basically do one full update cycle, drawing the streamer pattern onto the whole strip
//note that a spacing pixel is indicated by a pattern value of 255, these pixels will be filled in with the bgColor
void StreamerFastPS::preFill(){
    numPixels = segmentSet.numLeds;
    uint16_t patternLength = pattern->length;
    prevPattern = 255; //base value for previous pattern value, it's set to the spacing value b.c we don't expect a pattern to start with spacing

    for(uint16_t i = 0; i < numPixels; i++){

        pixelNumber = segDrawUtils::getSegmentPixel(segmentSet, i);
        nextPattern = patternUtilsPS::getPatternVal(pattern, i);

        nextColor = pickStreamerColor(nextPattern);

        segDrawUtils::setPixelColor(segmentSet, pixelNumber, nextColor, 0, 0, 0);
        //every time we draw a pixel, we're basically doing one whole update()
        //so we need to increment the cycleCount, so that once the preFill is done, the 
        //next update() call will sync properly
        cycleCount = addMod16PS( cycleCount, 1, patternLength );// (cycleCount + 1) % patternLength;
    }
    preFillDone = true;
}
    
//Each update cycle, we run along the strip, coping the color of the next pixel into the current pixel
//effectively shifting all the streamers down the strip
//We only choose a new color for the final pixel
//This is much faster than caculating the next pattern value for each pixel, but we cannot do fades, use color modes, or palette blend
//note that a spacing pixel is indicated by a pattern value of 255, these pixels will be filled in with the bgColor
void StreamerFastPS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        //We need to pre-fill the strip with a full cycle the first time the update is called
        //so that the colors are copied down the strip correctly on subsequent cycles
        if(!preFillDone){
            preFill();
        }

        //numPixels is the loop limit below, so we subtract 1
        numPixels = segmentSet.numLeds - 1;

        //prep for the loop below.
        //We need a initial value for the nextPixelNumber
        //Which is the location of the first pixel in the segment set
        nextPixelNumber = segDrawUtils::getSegmentPixel(segmentSet, 0);

        for (uint16_t i = 0; i <= numPixels; i++) {
            //The next pixel location from the previous loop iteration is now
            //the pixel location for this iteration
            pixelNumber = nextPixelNumber;
            nextPixelNumber = segDrawUtils::getSegmentPixel(segmentSet, i + 1);

            //if we're at the final pixel, we need to insert a new color
            //otherwise, we just copy the color from the next pixel location into the current one
            if (i == numPixels) {
                nextPattern = patternUtilsPS::getPatternVal(pattern, cycleCount);
                nextColor = pickStreamerColor(nextPattern);
                segDrawUtils::setPixelColor(segmentSet, pixelNumber, nextColor, 0, 0, 0);
            } else {
                //copy the color of the next pixel in line into the current pixel
                segmentSet.leds[pixelNumber] = segmentSet.leds[nextPixelNumber];
            }

        }
        cycleCount = addMod16PS( cycleCount, 1, pattern->length );//(cycleCount + 1) % pattern->length; //one update = one cycle

        showCheckPS();
    }
}

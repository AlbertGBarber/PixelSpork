#include "ShimmerSL.h"

//Constructor using a random shimmer color
ShimmerSL::ShimmerSL(SegmentSet &SegmentSet, uint8_t ShimmerMin, uint8_t ShimmerMax, uint16_t Rate):
    segmentSet(SegmentSet), shimmerMin(ShimmerMin), shimmerMax(ShimmerMax)
    {    
        //we make a random palette of one color so that 
        //if we switch to randMode 0 then we have a palette to use
        setSingleColor(colorUtilsPS::randColor()); 
        //since we're choosing colors at random, set the randMode
        randMode = 1;
        init(Rate);
	}

//Constructor using a set shimmer color
ShimmerSL::ShimmerSL(SegmentSet &SegmentSet, CRGB ShimmerColor, uint8_t ShimmerMin, uint8_t ShimmerMax, uint16_t Rate):
    segmentSet(SegmentSet), shimmerMin(ShimmerMin), shimmerMax(ShimmerMax)
    {    
        setSingleColor(ShimmerColor);
        init(Rate);
	}

//Constuctor for colors randomly choosen from palette
ShimmerSL::ShimmerSL(SegmentSet &SegmentSet, palettePS &Palette, uint8_t ShimmerMin, uint8_t ShimmerMax, uint16_t Rate):
    segmentSet(SegmentSet), palette(&Palette), shimmerMin(ShimmerMin), shimmerMax(ShimmerMax)
    {    
       init(Rate);
	}

ShimmerSL::~ShimmerSL(){
    free(paletteTemp.paletteArr);
}

void ShimmerSL::init(uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
}

//creates an palette of length 1 containing the passed in color
void ShimmerSL::setSingleColor(CRGB Color){
    free(paletteTemp.paletteArr);
    paletteTemp = paletteUtilsPS::makeSingleColorPalette(Color);
    palette = &paletteTemp;
}

//set a color based on the size of the palette
CRGB ShimmerSL::pickColor(){
    switch (randMode) {
        case 0: // we're picking from a set of colors 
        default:
            color = paletteUtilsPS::getPaletteColor(*palette, random8(paletteLength));
            break;
        case 1:
            color = colorUtilsPS::randColor();
            break;
    }
    return color;
}

//updates the effect
//effect is quite simple. 
//Each cycle we go through all the leds, pick a color for the leds, fade that color by a random amount and output it
void ShimmerSL::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        numLines = segmentSet.numLines;
        numSegs = segmentSet.numSegs;
        paletteLength = palette->length;

        for (uint16_t i = 0; i < numLines; i++) {
            shimmerVal = 255 - random8(shimmerMin, shimmerMax);
            color = pickColor();

            for(uint16_t j = 0; j < numSegs; j++){
                pixelNum = segDrawUtils::getPixelNumFromLineNum(segmentSet, numLines, j, i);

                //If we're not in line mode, then each individual pixel is to have it's own shimmer brightness
                if(!lineMode){
                    shimmerVal = 255 - random8(shimmerMin, shimmerMax);
                }

                colorOut = segDrawUtils::getPixelColor(segmentSet, pixelNum, color, colorMode, j, i);
                nscale8x3(colorOut.r, colorOut.g, colorOut.b, shimmerVal);
                segDrawUtils::setPixelColor(segmentSet, pixelNum, colorOut, 0, 0, 0);
            }
        }

        showCheckPS();
    }
}
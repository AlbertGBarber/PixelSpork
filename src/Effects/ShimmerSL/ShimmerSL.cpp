#include "ShimmerSL.h"

//Constructor using a random shimmer color
ShimmerSL::ShimmerSL(SegmentSetPS &SegSet, uint8_t ShimmerMin, uint8_t ShimmerMax, uint16_t Rate)
    : shimmerMin(ShimmerMin), shimmerMax(ShimmerMax)  //
{
    //we make a random palette of one color so that
    //if we switch to randMode 0 then we have a palette to use
    setSingleColor(colorUtilsPS::randColor());
    //since we're choosing colors at random, set the randMode
    randMode = 1;
    init(SegSet, Rate);
}

//Constructor using a set shimmer color
ShimmerSL::ShimmerSL(SegmentSetPS &SegSet, CRGB Color, uint8_t ShimmerMin, uint8_t ShimmerMax, uint16_t Rate)
    : shimmerMin(ShimmerMin), shimmerMax(ShimmerMax)  //
{
    setSingleColor(Color);
    init(SegSet, Rate);
}

//Constructor for colors randomly chosen from palette
ShimmerSL::ShimmerSL(SegmentSetPS &SegSet, palettePS &Palette, uint8_t ShimmerMin, uint8_t ShimmerMax, uint16_t Rate)
    : palette(&Palette), shimmerMin(ShimmerMin), shimmerMax(ShimmerMax)  //
{
    init(SegSet, Rate);
}

ShimmerSL::~ShimmerSL() {
    free(paletteTemp.paletteArr);
}

void ShimmerSL::init(SegmentSetPS &SegSet, uint16_t Rate) {
    //bind the rate and segSet pointer vars since they are inherited from BaseEffectPS
    bindSegSetPtrPS();
    bindClassRatesPS();
}

//creates an palette of length 1 containing the passed in color
void ShimmerSL::setSingleColor(CRGB color) {
    free(paletteTemp.paletteArr);
    paletteTemp = paletteUtilsPS::makeSingleColorPalette(color);
    palette = &paletteTemp;
}

//set a color based on the size of the palette
CRGB ShimmerSL::pickColor() {
    switch( randMode ) {
        case 0:  // we're picking from a set of colors
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
void ShimmerSL::update() {
    currentTime = millis();

    if( (currentTime - prevTime) >= *rate ) {
        prevTime = currentTime;

        numLines = segSet->numLines;
        numSegs = segSet->numSegs;
        paletteLength = palette->length;

        for( uint16_t i = 0; i < numLines; i++ ) {
            shimmerVal = 255 - random8(shimmerMin, shimmerMax);
            color = pickColor();

            for( uint16_t j = 0; j < numSegs; j++ ) {
                pixelNum = segDrawUtils::getPixelNumFromLineNum(*segSet, j, i);

                //If we're not in line mode, then each individual pixel is to have it's own shimmer brightness
                if( !lineMode ) {
                    shimmerVal = 255 - random8(shimmerMin, shimmerMax);
                }

                colorOut = segDrawUtils::getPixelColor(*segSet, pixelNum, color, colorMode, j, i);
                nscale8x3(colorOut.r, colorOut.g, colorOut.b, shimmerVal);
                segDrawUtils::setPixelColor(*segSet, pixelNum, colorOut, 0, 0, 0);
            }
        }

        showCheckPS();
    }
}
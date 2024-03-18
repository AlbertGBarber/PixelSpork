#include "TwinkleFastSL.h"

//palette color constructor
TwinkleFastSL::TwinkleFastSL(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, uint16_t NumTwinkles,
                             bool Sparkle, uint8_t FadeOutRate, uint16_t Rate)
    : palette(&Palette), numTwinkles(NumTwinkles), sparkle(Sparkle), fadeOutRate(FadeOutRate)  //
{
    init(BgColor, SegSet, Rate);
}

//single color constructor
TwinkleFastSL::TwinkleFastSL(SegmentSetPS &SegSet, CRGB Color, CRGB BgColor, uint16_t NumTwinkles, bool Sparkle,
                             uint8_t FadeOutRate, uint16_t Rate)
    : numTwinkles(NumTwinkles), sparkle(Sparkle), fadeOutRate(FadeOutRate)  //
{
    setSingleColor(Color);
    init(BgColor, SegSet, Rate);
}

TwinkleFastSL::~TwinkleFastSL() {
    free(paletteTemp.paletteArr);
}

//sets up all the core class vars, and initializes the pixel and color arrays
void TwinkleFastSL::init(CRGB BgColor, SegmentSetPS &SegSet, uint16_t Rate) {
    //bind the rate and segSet pointer vars since they are inherited from BaseEffectPS
    bindSegSetPtrPS();
    bindClassRatesPS();
    //bind the bgColor pointer
    bindBGColorPS();
    paletteLength = palette->length;
}

//creates an palette of length 1 containing the passed in color
void TwinkleFastSL::setSingleColor(CRGB Color) {
    free(paletteTemp.paletteArr);
    paletteTemp = paletteUtilsPS::makeSingleColorPalette(Color);
    palette = &paletteTemp;
}

//Resets the effect by filling in the background
void TwinkleFastSL::reset(){
    segDrawUtils::fillSegSetColor(*segSet, *bgColor, bgColorMode);
}

void TwinkleFastSL::update() {
    currentTime = millis();

    if( (currentTime - prevTime) >= *rate ) {
        prevTime = currentTime;

        numLines = segSet->numLines;
        paletteLength = palette->length;

        //controls the background setting, when sparkleOn is true, the strip will dim with each cycle
        //instead of setting a background
        if( sparkle ) {
            segDrawUtils::fadeSegSetToBlackBy(*segSet, fadeOutRate);
        } else if( fillBg ) {
            segDrawUtils::fillSegSetColor(*segSet, *bgColor, bgColorMode);
        }

        //sets a random set of lines to a random or indicated color(s)
        for( uint16_t i = 0; i < numTwinkles; i++ ) {
            randLine = random16(numLines);
            switch( randMode ) {
                case 0:  // we're picking from a set of colors
                    color = paletteUtilsPS::getPaletteColor(*palette, random8(paletteLength));
                    break;
                default:  //(mode 1) set colors at random
                    color = colorUtilsPS::randColor();
                    break;
            }
            //fill the segment line at the twinkle location with color
            segDrawUtils::drawSegLine(*segSet, randLine, color, colorMode);
        }
        showCheckPS();
    }
}
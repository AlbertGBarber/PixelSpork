#include "TwinkleLowRamPS.h"

TwinkleLowRamPS::TwinkleLowRamPS(SegmentSet &SegmentSet, palletPS *Pallet, uint16_t NumPixels, CRGB BgColor, bool Sparkle, uint8_t FadeOutRate, uint16_t Rate):
    segmentSet(SegmentSet), pallet(Pallet), numPixels(NumPixels), sparkle(Sparkle), fadeOutRate(FadeOutRate)
    {    
        init(BgColor, Rate);
    }

//single color constructor
TwinkleLowRamPS::TwinkleLowRamPS(SegmentSet &SegmentSet, CRGB Color, uint16_t NumPixels, CRGB BgColor, bool Sparkle, uint8_t FadeOutRate, uint16_t Rate):
    segmentSet(SegmentSet), numPixels(NumPixels), sparkle(Sparkle), fadeOutRate(FadeOutRate)
    {    
        setSingleColor(Color);
        init(BgColor, Rate);
	}

//random colors constructor
TwinkleLowRamPS::TwinkleLowRamPS(SegmentSet &SegmentSet, uint16_t NumPixels, CRGB BgColor, bool Sparkle, uint8_t FadeOutRate, uint16_t Rate):
    segmentSet(SegmentSet), numPixels(NumPixels), sparkle(Sparkle), fadeOutRate(FadeOutRate)
    {    
        //we make a random pallet of one color so that 
        //if we switch to randMode 0 then we have a pallet to use
        setSingleColor(colorUtilsPS::randColor()); 
        //since we're choosing colors at random, set the randMode
        randMode = 1;
        init(BgColor, Rate);
	}

TwinkleLowRamPS::~TwinkleLowRamPS(){
    delete[] palletTemp.palletArr;
}

//sets up all the core class vars, and initilizes the pixel and color arrays
void TwinkleLowRamPS::init(CRGB BgColor, uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    bindBGColorPS();
    palletLength = pallet->length;
}

//binds the pallet to a new one
void TwinkleLowRamPS::setPallet(palletPS *newPallet){
    pallet = newPallet;
}

//creates an pallet of length 1 containing the passed in color
void TwinkleLowRamPS::setSingleColor(CRGB Color){
    delete[] palletTemp.palletArr;
    palletTemp = palletUtilsPS::makeSingleColorPallet(Color);
    pallet = &palletTemp;
}

void TwinkleLowRamPS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;
        numActiveLeds = segmentSet.numActiveSegLeds;
        palletLength = pallet->length;

        //controls the background setting, when sparkleOn is true, the strip will dim with each cycle
        //instead of setting a background
        if (sparkle) {
            segDrawUtils::fadeSegSetToBlackBy(segmentSet, fadeOutRate);
        } else if(fillBG) {
            segDrawUtils::fillSegSetColor(segmentSet, *bgColor, bgColorMode);
        }

        // sets a random set of pixels to a random or indicated color(s)
        for (uint16_t i = 0; i < numPixels; i++) {
            randPixel = random16(numActiveLeds);
            switch (randMode) {
                case 0: // we're picking from a set of colors 
                    color = palletUtilsPS::getPalletColor(pallet, random8(palletLength));
                    break;
                default: //(mode 1) set colors at random
                    color = colorUtilsPS::randColor();
                    break;
            }
            //get the pixel color to account for any color modes
            segDrawUtils::setPixelColor(segmentSet, randPixel, color, colorMode);
        }
        showCheckPS();
    }
}
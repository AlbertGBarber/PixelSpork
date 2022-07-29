#include "ShimmerSLPS.h"

//Constructor using a random shimmer color
ShimmerSLPS::ShimmerSLPS(SegmentSet &SegmentSet, uint8_t ShimmerMin, uint8_t ShimmerMax, uint16_t Rate):
    segmentSet(SegmentSet), shimmerMin(ShimmerMin), shimmerMax(ShimmerMax)
    {    
        //we make a random pallet of one color so that 
        //if we switch to randMode 0 then we have a pallet to use
        setSingleColor(colorUtilsPS::randColor()); 
        //since we're choosing colors at random, set the randMode
        randMode = 1;
        init(Rate);
	}

//Constructor using a set shimmer color
ShimmerSLPS::ShimmerSLPS(SegmentSet &SegmentSet, CRGB ShimmerColor, uint8_t ShimmerMin, uint8_t ShimmerMax, uint16_t Rate):
    segmentSet(SegmentSet), shimmerMin(ShimmerMin), shimmerMax(ShimmerMax)
    {    
        setSingleColor(ShimmerColor);
        init(Rate);
	}

//Constuctor for colors randomly choosen from pallet
ShimmerSLPS::ShimmerSLPS(SegmentSet &SegmentSet, palletPS *Pallet, uint8_t ShimmerMin, uint8_t ShimmerMax, uint16_t Rate):
    segmentSet(SegmentSet), pallet(Pallet), shimmerMin(ShimmerMin), shimmerMax(ShimmerMax)
    {    
       init(Rate);
	}

ShimmerSLPS::~ShimmerSLPS(){
    delete[] palletTemp.palletArr;
}

void ShimmerSLPS::init(uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
}

//creates an pallet of length 1 containing the passed in color
void ShimmerSLPS::setSingleColor(CRGB Color){
    delete[] palletTemp.palletArr;
    palletTemp = palletUtilsPS::makeSingleColorPallet(Color);
    pallet = &palletTemp;
}

//set a color based on the size of the pallet
CRGB ShimmerSLPS::pickColor(){
    switch (randMode) {
        case 0: // we're picking from a set of colors 
        default:
            color = palletUtilsPS::getPalletColor(pallet, random8(palletLength));
            break;
        case 1:
            color = colorUtilsPS::randColor();
            break;
    }
    return color;
}

//updates the effect
//effect is quite simple. 
//Each cycle we go through all the leds, pick a color for the leds, fade that color by a random amount
//and output it
void ShimmerSLPS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        numLines = segmentSet.maxSegLength;
        numSegs = segmentSet.numSegs;
        palletLength = pallet->length;

        for (uint16_t i = 0; i < numLines; i++) {
            shimmerVal = 255 - random8(shimmerMin, shimmerMax);
            color = pickColor();

            for(uint8_t j = 0; j < numSegs; j++){
                pixelNum = segDrawUtils::getPixelNumFromLineNum(segmentSet, numLines, j, i);

                colorOut = segDrawUtils::getPixelColor(segmentSet, pixelNum, color, colorMode, j, i);
                nscale8x3(colorOut.r, colorOut.g, colorOut.b, shimmerVal);
                segmentSet.leds[pixelNum] = colorOut;
            }
        }

        showCheckPS();
    }
}
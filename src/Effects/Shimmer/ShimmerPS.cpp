#include "ShimmerPS.h"

ShimmerPS::ShimmerPS(SegmentSet &SegmentSet, uint16_t Rate):
    segmentSet(SegmentSet)
    {    
        //we make a random pallet of one color so that 
        //if we switch to randMode 0 then we have a pallet to use
        setSingleColor(colorUtilsPS::randColor()); 
        //since we're choosing colors at random, set the randMode
        randMode = 1;
        init(Rate);
	}

ShimmerPS::ShimmerPS(SegmentSet &SegmentSet, CRGB ShimmerColor, uint16_t Rate):
    segmentSet(SegmentSet)
    {    
        setSingleColor(ShimmerColor);
        init(Rate);
	}

ShimmerPS::ShimmerPS(SegmentSet &SegmentSet, palletPS *Pallet, uint16_t Rate):
    segmentSet(SegmentSet), pallet(Pallet)
    {    
       init(Rate);
	}

ShimmerPS::~ShimmerPS(){
    delete[] palletTemp.palletArr;
}

void ShimmerPS::init(uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
}

//creates an pallet of length 1 containing the passed in color
void ShimmerPS::setSingleColor(CRGB Color){
    delete[] palletTemp.palletArr;
    palletTemp = palletUtilsPS::makeSingleColorpallet(Color);
    pallet = &palletTemp;
}

//binds the pallet to a new one
void ShimmerPS::setPallet(palletPS *newPallet){
    pallet = newPallet;
}

//set a color based on the size of the pallet
void ShimmerPS::pickColor(uint16_t pixelNum){
    switch (randMode) {
        case 0: // we're picking from a set of colors 
            color = palletUtilsPS::getPalletColor(pallet, random8(palletLength));
            break;
        default: //(mode 1) set colors at random
            color = colorUtilsPS::randColor();
            break;
    }
    //get the pixel color to account for any color modes
    //this also fills in pixelInfo for the pixel location
    segDrawUtils::getPixelColor(segmentSet, &pixelInfo, color, colorMode, pixelNum);
    color = pixelInfo.color;
}

//updates the effect
//effect is quite simple. 
//Each cycle we go through all the leds, pick a color for the leds, fade that color by a random amount
//and output it
void ShimmerPS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        numActiveLeds = segmentSet.numActiveSegLeds;
        palletLength = pallet->length;
        for (uint16_t i = 0; i < numActiveLeds; i++) {
            pickColor(i);
            shimmerVal = 255 - random8(shimmerMin, shimmerMax);
            //scale the rgb components toward 0 by the shimmer val
            //(this is basically the same as uing getCrossFadeColor, but faster)
            nscale8x3(color.r, color.g, color.b, shimmerVal);

            //output the color, we can use the pixel info we got from calling pickColor()
            segDrawUtils::setPixelColor(segmentSet, pixelInfo.pixelLoc, color, 0, pixelInfo.segNum, pixelInfo.lineNum);
        }

        showCheckPS();
    }
}
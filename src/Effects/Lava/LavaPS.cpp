#include "LavaPS.h"

//constructor for base lava effect with default values
LavaPS::LavaPS(SegmentSet &SegmentSet, uint16_t Rate):
    segmentSet(SegmentSet)
    {    
        palletTemp = {lavalPallet_arr, SIZE(lavalPallet_arr)};
        pallet = &palletTemp;
        blendSteps = 150;
        blendScale = 80;
        init(Rate);
	}

//constructor for lava with adjustable values
LavaPS::LavaPS(SegmentSet &SegmentSet, uint16_t BlendSteps, uint16_t BlendScale, uint16_t Rate):
    segmentSet(SegmentSet), blendSteps(BlendSteps), blendScale(BlendScale)
    {    
        palletTemp = {lavalPallet_arr, SIZE(lavalPallet_arr)};
        pallet = &palletTemp;
        init(Rate);
	}

//constructor for a custom pallet
LavaPS::LavaPS(SegmentSet &SegmentSet, palletPS *Pallet, uint16_t BlendSteps, uint16_t BlendScale, uint16_t Rate):
    segmentSet(SegmentSet), pallet(Pallet), blendSteps(BlendSteps), blendScale(BlendScale)
    {    
        palletTemp = {lavalPallet_arr, SIZE(lavalPallet_arr)};
        init(Rate);
	}

//constructor for a custom pallet
LavaPS::LavaPS(SegmentSet &SegmentSet, uint8_t numColors, uint16_t BlendSteps, uint16_t BlendScale, uint16_t Rate):
    segmentSet(SegmentSet), blendSteps(BlendSteps), blendScale(BlendScale)
    {    
        palletTemp = palletUtilsPS::makeRandomPallet(numColors);
        pallet = &palletTemp;
        //since we're created a random pallet using new, we'll need to flag it for
        //deletion at in the destructor
        randPalletCreated = true;
        init(Rate);
	}

LavaPS::~LavaPS(){
    //Only delete the temp pallet color array if it was created randomly
    //otherwise we'd be deleting the lava colors array
    if(randPalletCreated){
        delete[] palletTemp.palletArr;
    }
}

void LavaPS::init(uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();   
}

void LavaPS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;
        
        numSegs = segmentSet.numSegs;
        totBlendLength = blendSteps * pallet->length;
        //run over each of the leds in the segment set and set a noise/color value
        for (uint8_t i = 0; i < numSegs; i++) {
            totSegLen = segmentSet.getTotalSegLength(i);
            for(uint16_t j = 0; j < totSegLen; j++){
                
                //get the current pixel's location in the segment set
                pixelNum = segDrawUtils::getSegmentPixel(segmentSet, i, j);

                //do some noise magic to get a brightness val and color index
                brightness = inoise8(pixelNum * brightnessScale, currentTime/5);
                index = inoise8(pixelNum * blendScale, currentTime/10);

                //scale color index to be somewhere between 0 and totBlendLength to put it somewhere in the blended pallet
                index = scale16by8( totBlendLength, index ); //colorIndex * totBlendLength /255;   

                //get the blended color from the pallet and set it's brightness
                colorOut = palletUtilsPS::getPalletGradColor(pallet, index, 0, totBlendLength, blendSteps);
                nscale8x3(colorOut.r, colorOut.g, colorOut.b, brightness);
                segmentSet.leds[pixelNum] = colorOut;
            }
        }
        showCheckPS();
    }
}
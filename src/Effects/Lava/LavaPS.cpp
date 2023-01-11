#include "LavaPS.h"

//constructor for base lava effect with default values
LavaPS::LavaPS(SegmentSet &SegmentSet, uint16_t Rate):
    segmentSet(SegmentSet)
    {    
        paletteTemp = {lavalPalette_arr, SIZE(lavalPalette_arr)};
        palette = &paletteTemp;
        blendSteps = 150;
        blendScale = 80;
        init(Rate);
	}

//constructor for lava with adjustable values
LavaPS::LavaPS(SegmentSet &SegmentSet, uint16_t BlendSteps, uint16_t BlendScale, uint16_t Rate):
    segmentSet(SegmentSet), blendSteps(BlendSteps), blendScale(BlendScale)
    {    
        paletteTemp = {lavalPalette_arr, SIZE(lavalPalette_arr)};
        palette = &paletteTemp;
        init(Rate);
	}

//constructor for a custom palette
LavaPS::LavaPS(SegmentSet &SegmentSet, palettePS *Palette, uint16_t BlendSteps, uint16_t BlendScale, uint16_t Rate):
    segmentSet(SegmentSet), palette(Palette), blendSteps(BlendSteps), blendScale(BlendScale)
    {    
        paletteTemp = {lavalPalette_arr, SIZE(lavalPalette_arr)};
        init(Rate);
	}

//constructor for a custom palette
LavaPS::LavaPS(SegmentSet &SegmentSet, uint8_t numColors, uint16_t BlendSteps, uint16_t BlendScale, uint16_t Rate):
    segmentSet(SegmentSet), blendSteps(BlendSteps), blendScale(BlendScale)
    {    
        paletteTemp = paletteUtilsPS::makeRandomPalette(numColors);
        palette = &paletteTemp;
        //since we're created a random palette using new, we'll need to flag it for
        //deletion at in the destructor
        randPaletteCreated = true;
        init(Rate);
	}

LavaPS::~LavaPS(){
    //Only delete the temp palette color array if it was created randomly
    //otherwise we'd be deleting the lava colors array
    if(randPaletteCreated){
        free(paletteTemp.paletteArr);
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
        pixelCount = 0;

        numSegs = segmentSet.numSegs;
        totBlendLength = blendSteps * palette->length;
        //run over each of the leds in the segment set and set a noise/color value
        for (uint16_t i = 0; i < numSegs; i++) {
            totSegLen = segmentSet.getTotalSegLength(i);
            for(uint16_t j = 0; j < totSegLen; j++){
                
                //get the current pixel's location in the segment set
                pixelNum = segDrawUtils::getSegmentPixel(segmentSet, i, j);

                //do some noise magic to get a brightness val and color index
                brightness = inoise8(pixelCount * brightnessScale, currentTime/5);
                index = inoise8(pixelCount * blendScale, currentTime/10);

                //scale color index to be somewhere between 0 and totBlendLength to put it somewhere in the blended palette
                index = scale16by8( totBlendLength, index ); //colorIndex * totBlendLength /255;   

                //get the blended color from the palette and set it's brightness
                colorOut = paletteUtilsPS::getPaletteGradColor(palette, index, 0, totBlendLength, blendSteps);
                nscale8x3(colorOut.r, colorOut.g, colorOut.b, brightness);
                segDrawUtils::setPixelColor(segmentSet, pixelNum, colorOut, 0, 0, 0);

                pixelCount++;
            }
        }
        showCheckPS();
    }
}
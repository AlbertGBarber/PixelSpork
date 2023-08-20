#include "LavaPS.h"

//constructor for base lava effect with default values
LavaPS::LavaPS(SegmentSet &SegSet, uint16_t Rate)
    {    
        palette = &lavaPal; //set the effect palette to the lava palette from paletteList.h
        blendSteps = 150;
        blendScale = 80;
        //Fill in the temp palette in case the use switches to it
        paletteTemp = paletteUtilsPS::makeRandomPalette(3);
        init(SegSet, Rate);
	}

//constructor for lava with adjustable values
LavaPS::LavaPS(SegmentSet &SegSet, uint16_t BlendSteps, uint16_t BlendScale, uint16_t Rate):
    blendSteps(BlendSteps), blendScale(BlendScale)
    {    
        palette = &lavaPal; //set the effect palette to the lava palette from paletteList.h
        //Fill in the temp palette in case the use switches to it
        paletteTemp = paletteUtilsPS::makeRandomPalette(3);
        init(SegSet, Rate);
	}

//constructor for a custom palette
LavaPS::LavaPS(SegmentSet &SegSet, palettePS &Palette, uint16_t BlendSteps, uint16_t BlendScale, uint16_t Rate):
    palette(&Palette), blendSteps(BlendSteps), blendScale(BlendScale)
    {    
        //Fill in the temp palette in case the use switches to it
        paletteTemp = paletteUtilsPS::makeRandomPalette(3);
        init(SegSet, Rate);
	}

//constructor for a random palette
LavaPS::LavaPS(SegmentSet &SegSet, uint8_t numColors, uint16_t BlendSteps, uint16_t BlendScale, uint16_t Rate):
    blendSteps(BlendSteps), blendScale(BlendScale)
    {    
        paletteTemp = paletteUtilsPS::makeRandomPalette(numColors);
        palette = &paletteTemp;
        init(SegSet, Rate);
	}

LavaPS::~LavaPS(){
    free(paletteTemp.paletteArr);
}

void LavaPS::init(SegmentSet &SegSet, uint16_t Rate){
    //bind the rate and segSet pointer vars since they are inherited from BaseEffectPS
    bindSegSetPtrPS();
    bindClassRatesPS();   
}

void LavaPS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        //if we're in rainbow mode and it's time to change the hue offset, do so
        if(rainbowMode && (currentTime - prevHueTime) >= *hueRate){
            hueOffset++;
            prevHueTime = currentTime;
        }

        pixelCount = 0;
        numSegs = segSet->numSegs;

        //get the total blend length
        if(rainbowMode){
            //in rainbow mode, the blend length is one full cycle of the rainbow
            totBlendLength = 255;
        } else {
            //in palette mode, the blend length is all the possible palette blend colors a pixel may have
            totBlendLength = blendSteps * palette->length;
        }

        //run over each of the leds in the segment set and set a noise/color value
        for (uint16_t i = 0; i < numSegs; i++) {
            totSegLen = segSet->getTotalSegLength(i);
            for(uint16_t j = 0; j < totSegLen; j++){
                
                //get the current pixel's location in the segment set
                pixelNum = segDrawUtils::getSegmentPixel(*segSet, i, j);

                //do some noise magic to get a brightness val and color index
                brightness = inoise8(pixelCount * brightnessScale, currentTime/5);
                index = inoise8(pixelCount * blendScale, currentTime/10);

                //scale color index to be somewhere between 0 and totBlendLength to put it somewhere in the blended palette
                index = scale16by8( totBlendLength, index ); //colorIndex * totBlendLength /255;   

                if(rainbowMode){
                    //get the rainbow color at the noise value
                    colorOut = colorUtilsPS::wheel(index, hueOffset);
                } else {
                    //get the blended color from the palette
                    colorOut = paletteUtilsPS::getPaletteGradColor(*palette, index, 0, totBlendLength, blendSteps);
                }

                //set the output color's brightness
                nscale8x3(colorOut.r, colorOut.g, colorOut.b, brightness);
                segDrawUtils::setPixelColor(*segSet, pixelNum, colorOut, 0, 0, 0);

                pixelCount++;
            }
        }
        showCheckPS();
    }
}
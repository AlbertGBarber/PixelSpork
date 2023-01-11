#include "Noise16PS.h"

//constructor with palette
Noise16PS::Noise16PS(SegmentSet &SegmentSet, palettePS *Palette, uint16_t BlendSteps, uint16_t BlendScale,
                    uint8_t X_mode, uint8_t Y_mode, uint8_t Z_mode, uint16_t X_val, uint16_t Y_val, uint16_t Z_val, 
                    uint16_t Rate):
    segmentSet(SegmentSet), palette(Palette), blendSteps(BlendSteps), blendScale(BlendScale), 
    x_mode(X_mode), y_mode(Y_mode), z_mode(Z_mode), x_val(X_val), y_val(Y_val), z_val(Z_val)
    {    
        init(Rate);
	}

//constructor with randomly generated palette
Noise16PS::Noise16PS(SegmentSet &SegmentSet, uint8_t numColors, uint16_t BlendSteps, uint16_t BlendScale,
                    uint8_t X_mode, uint8_t Y_mode, uint8_t Z_mode, uint16_t X_val, uint16_t Y_val, uint16_t Z_val, 
                    uint16_t Rate):
    segmentSet(SegmentSet), blendSteps(BlendSteps), blendScale(BlendScale), 
    x_mode(X_mode), y_mode(Y_mode), z_mode(Z_mode), x_val(X_val), y_val(Y_val), z_val(Z_val)
    { 
        paletteTemp = paletteUtilsPS::makeRandomPalette(numColors);
        palette = &paletteTemp;   
        init(Rate);
	}

Noise16PS::~Noise16PS(){
    free(paletteTemp.paletteArr);
}

void Noise16PS::init(uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
}

//returns a noise scaling value based on the input mode
//We want the scaling value to shift over time. 
//This function will shift the value using one of three methods
//0: Keep the scaling value static
//1: scale the value based on the current time
//2: scale the value by an sin wave at a passed in bpm
//3: scale the value by multiplying the current time (only use for z part of noise)
uint32_t Noise16PS::getShiftVal( uint8_t shiftMode, uint16_t constVal ){
    switch (shiftMode){
        default:
        case 0:
            return constVal;
            break;
        case 1:
            return currentTime / constVal;
            break;
        case 2:
            return beatsin8( constVal );
            break;
        case 3:
            return currentTime * constVal;
            break;
    }
}

//updates the effect by recaculating the noise for each pixel
void Noise16PS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;
        pixelCount = 0;

        //get noise inputs for the current cycle
        shift_x = getShiftVal( x_mode, x_val );
        shift_y = getShiftVal( y_mode, y_val );
        real_z = getShiftVal( z_mode, z_val );
    
        numSegs = segmentSet.numSegs;
        totBlendLength = blendSteps * palette->length;
        //run over each of the leds in the segment set and set a noise/color value
        for (uint16_t i = 0; i < numSegs; i++) {
            totSegLen = segmentSet.getTotalSegLength(i);
            for(uint16_t j = 0; j < totSegLen; j++){
                //get the current pixel's location in the segment set
                pixelNum = segDrawUtils::getSegmentPixel(segmentSet, i, j);
                
                //scale x and y noise inputs for the current pixel
                real_x = (pixelCount + shift_x) * blendScale;
                real_y = (pixelCount + shift_y) * blendScale;
                
                //get the noise data and scale it down
                noise = inoise16(real_x, real_y, real_z) >> 8;

                //map LED gradient color index based on noise data
                index = scale16by8( totBlendLength, sin8( noise * 3 ));
                bri = noise; //inoise8(real_x, currentTime/5); //
                
                //get the blended color from the palette and set it's brightness
                colorOut = paletteUtilsPS::getPaletteGradColor(palette, index, 0, totBlendLength, blendSteps);
                nscale8x3( colorOut.r, colorOut.g, colorOut.b, bri);
                segDrawUtils::setPixelColor(segmentSet, pixelNum, colorOut, 0, 0, 0); 

                pixelCount++;
            }
        }
        showCheckPS();
    }
}

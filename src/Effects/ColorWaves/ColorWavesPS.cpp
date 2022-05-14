#include "ColorWavesPS.h"

ColorWavesPS::ColorWavesPS(SegmentSet &SegmentSet, palletPS *Pallet, uint16_t Rate):
    segmentSet(SegmentSet), pallet(Pallet)
    {    
        init(Rate);
	}

ColorWavesPS::ColorWavesPS(SegmentSet &SegmentSet, uint8_t numColors, uint16_t Rate):
    segmentSet(SegmentSet)
    {    
        init(Rate);
        palletTemp = palletUtilsPS::makeRandomPallet(numColors);
        pallet = &palletTemp;
	}

void ColorWavesPS::init(uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
}

ColorWavesPS::~ColorWavesPS(){
    delete[] palletTemp.palletArr;
}

//updates the effect
//idk how all the timing and waves work
//the only adjustments I've made from Mark Kriegsman's original code
//are to add parts specific to my segments and pallets 
//(getting the pixel numbers and blended pallet colors)
void ColorWavesPS::update(){
    currentTime = millis();
    deltams = currentTime - prevTime;
    if( ( deltams ) >= *rate ) {
        prevTime = currentTime;
            
        sat8 = beatsin88(87, 220, 250);
        brightdepth = beatsin88(341, 96, 224);
        brightnessthetainc16 = beatsin88(203, (25 * 256), (40 * 256));
        msmultiplier = beatsin88(147, 23, 60);

        hue16 = sHue16;//gHue * 256;
        hueinc16 = beatsin88(113, 300, 1500);
            
        sPseudotime += deltams * msmultiplier;
        sHue16 += deltams * beatsin88(400, 5,9);
        brightnesstheta16 = sPseudotime;
        
        //do the subtraction here so we don't need to do it each loop
        //@getSegmentPixel
        //The loop limit is adjusted up by one
        numActiveLeds = segmentSet.numActiveSegLeds - 1;

        for(uint16_t i = 0 ; i <= numActiveLeds; i++) {
            hue16 += hueinc16;
            hue8 = hue16 / 256;
            h16_128 = hue16 >> 7;
            if(h16_128 & 0x100) {
                hue8 = 255 - (h16_128 >> 1);
            } else {
                hue8 = h16_128 >> 1;
            }

            brightnesstheta16  += brightnessthetainc16;
            b16 = sin16(brightnesstheta16) + 32768;

            bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
            bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
            bri8 += (255 - brightdepth);
                
            index = hue8;
            //index = triwave8( index);
            index = scale8(index, numSteps);

            //returns the blended color from the pallet mapped into numSteps
            newColor = palletUtilsPS::getPalletGradColor(pallet, index, 0, numSteps);

            nscale8x3(newColor.r, newColor.g, newColor.b, bri8);

            pixelnumber = segDrawUtils::getSegmentPixel(segmentSet, numActiveLeds - i);
                
            nblend(segmentSet.leds[pixelnumber], newColor, 128);
        }
                    
        showCheckPS();
    }
    
}
#ifndef ColorWavesPS_h
#define ColorWavesPS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

//A direct port of ColorWavesWithPalettes by Mark Kriegsman (https://gist.github.com/kriegsman)
//This effect draws colors from pallet with an ever-changing,
//widely-varying set of parameters.

//Only input is the segmentSet and a pallet, the update rate is hard coded into the pattern, so you can't adjust it
//Likewise, do not try to run this on a segmentSet with other effects at the same time,
//It will most likely not work b/c it pulls colors directly from the leds

//The core of the code is directly from the original ColorWavesWithPalettes code,
//I just adjusted a couple of lines to make it compatible with my segments and pallets
//If you have any questions about how it works, direct them to Mark Kriegsman

//There is also a shorthand call to do the effect with a pallet of randomly choosen colors

//Example call:
//   ColorWavesPS(mainSegments, pallet) -- That's it
//   ColorWavesPS(mainSegments, numColors) -- does the effect with a pallet or random colors (numColors = pallet length)

//Functions:
//   call update() to update the effect
class ColorWavesPS : public EffectBasePS {
    public:
        ColorWavesPS(SegmentSet &SegmentSet, palletPS *Pallet);  

        ColorWavesPS(SegmentSet &SegmentSet, uint8_t numColors);

        ~ColorWavesPS();

        SegmentSet 
            &segmentSet; 
        
        uint16_t 
            numSteps = 240; //240 //Adjusting this doesn't seem to do anything
        
        palletPS
            palletTemp,
            *pallet;

        void 
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        uint16_t
            deltams,
            sPseudotime = 0,
            sHue16 = 0,
            hue16,
            hueinc16,
            brightnesstheta16,
            brightnessthetainc16,
            numActiveLeds,
            b16,
            bri16,
            h16_128,
            pixelnumber;
        
        uint8_t
            sat8,
            bri8,
            hue8,
            brightdepth,
            index,
            msmultiplier;
    
        CRGB
            newColor;
        
        void
            init();
};

#endif
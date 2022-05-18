#ifndef PrideWPalPS_h
#define PrideWPalPS_h

//TODO: -- Add effect vars from PrideWPalSegLine and randomizers?
//         Not super needed imo, the base effect looks good already

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

/*
A direct port of Pride2015 by Mark Kriegsman (https://gist.github.com/kriegsman)
This effect draws rainbows or pallets with an ever-changing,
widely-varying set of parameters.

The effect runs two sets of waves, one for brightness and one for color
The way these waves interact creates the constantly blending patterns
I've added the option to set the brightness waves to either rotate with or contra to the colors

Colors can either be taken from a pallet or from the rainbow

Example call:
    PrideWPalPS(mainSegments, true, 70);
    Will do a rainbow effect, with the brightness waves rotating with the colors
    The effect updates at 70ms

    PrideWPalPS(mainSegments, 4, false, 50);
    Will use a pallet of 4 random colors for the effect
    The brightness waves rotate counter to the colors
    The effect updates at 50ms

    PrideWPalPS(mainSegments, &pallet1, true, 80);
    Will use a pallet1 for the color waves
    The brightness waves rotate with to the colors
    The effect updates at 80ms

Constructor Inputs:
    pallet (optional, see constructors) -- A custom pallet passed to the effect
    numColors (optional, see constructors) -- How many colors will be in the randomly created pallet
    briDirect -- Sets if the brightness waves move with or against the colors, true is with
    Rate -- The update rate (ms) note that this is synced with all the particles.

Funtions:
    update() -- Updates the effect

Other Settings:
    prideMode (default false, set automatically by constructor) -- If true, a rainbow will be used for colors instead of a pallet
    gradLength (default 20) -- How many gradient steps between pallet colors (not used for rainbow)
                               You shouldn't need to adjust this
*/
class PrideWPalPS : public EffectBasePS {
    public:
        //constructor for rainbow mode
        PrideWPalPS(SegmentSet &SegmentSet, bool BriDirect, uint16_t Rate);  

        //constructor for pallet input
        PrideWPalPS(SegmentSet &SegmentSet, palletPS *Pallet, bool BriDirect, uint16_t Rate);  

        //constructor for making a random pallet
        PrideWPalPS(SegmentSet &SegmentSet, uint8_t numColors, bool BriDirect, uint16_t Rate);

        ~PrideWPalPS();

        SegmentSet 
            &segmentSet; 
        
        uint16_t 
            gradLength = 20;
        
        bool 
            briDirect,   
            prideMode = false;
        
        palletPS
            palletTemp,
            *pallet;

        void 
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;

        int8_t
            briDirectMult;
        
        uint8_t
            blendRatio,
            sat8,
            bri8,
            hue8,
            brightdepth,
            msmultiplier,
            index;
        
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
            pixelnumber,
            numSteps;
        
        CRGB 
            newColor;

        void
            init(uint16_t Rate);
};

#endif
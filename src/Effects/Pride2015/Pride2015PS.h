#ifndef Pride2015PS_h
#define Pride2015PS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

//A direct port of pride2015 by Mark Kriegsman (https://gist.github.com/kriegsman)
//This effect draws rainbows with an ever-changing,
//widely-varying set of parameters.

//Only input is the segmentSet, the update rate is hard coded into the pattern, so you can't adjust it
//Likewise, do not try to run this on a segmentSet with other effects at the same time,
//It will most likely not work b/c it pulls colors directly from the leds

//The core of the code is directly from the original pride2015 code,
//I just adjusted a couple of lines to make it compatible with my segments
//If you have any questions about how it works, direct them to Mark Kriegsman

//Example call:
//   Pride2015PS(mainSegments, Rate) -- That's it
//   The Rate should be 60 or greater depending on your strip length
//   call update() to update the effect
class Pride2015PS : public EffectBasePS {
    public:
        Pride2015PS(SegmentSet &SegmentSet, uint16_t Rate);  

        SegmentSet 
            &segmentSet; 

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
            pixelnumber;
        
        uint8_t
            sat8,
            bri8,
            hue8,
            brightdepth,
            msmultiplier;
        
        CRGB 
            newColor;

};

#endif
#ifndef ColorModeFillPS_h
#define ColorModeFillPS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

/* 
Not a full effect, but could be used as one
Fills in the Segment Set using the specified colorMode (see segDrawUtils::getPixelColor()) at the specified rate
Useful for drawing simple rainbow shifting effects on the segment set using the color modes 
or filling in a background before drawing an effect 

Example call: 
    ColorModeFillPS(mainSegments, 7, 70);
    Will fill the segment using color mode 7 every 70ms

Inputs:
    colorMode -- the colorMode that will be used in segDrawUtils::getPixelColor()
    rate -- The update rate

Functions:
    update() -- updates the effect

Other Settings:
    color (default to 0) -- if you want just a static color, you could use this along with colorMode = 0, but 
                            effects should already give you this option by themselves 
*/
class ColorModeFillPS : public EffectBasePS {
    public:
        ColorModeFillPS(SegmentSet &SegSet, uint8_t ColorMode, uint16_t Rate);  

        SegmentSet 
            &SegSet;
        
        uint8_t 
            colorMode;
        
        CRGB 
            color = 0;
        
        void 
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
};

#endif
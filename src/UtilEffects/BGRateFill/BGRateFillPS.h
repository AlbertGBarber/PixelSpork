#ifndef BGRateFillPS_h
#define BGRateFillPS_h

#include "SegmentFiles.h"
#include "paletteFiles.h"

#include "Effects/EffectBasePS.h"

/* 
Not a full effect, but could be used as one
Fills in the segmentSet using the specified colorMode (see segDrawUtils::getPixelColor())
at the specified rate
Useful for filling in a background before drawing an effect
or can be combined with a RainbowOffsetCycle to do simple rainbow shifting effects

Example call: 
    BGRateFillPS(mainSegments, 7, 70);
    Will fill the segment using color mode 7 every 70ms

Inputs:
    ColorMode -- the colorMode that will be used in segDrawUtils::getPixelColor()
    Rate -- The update rate

Functions:
    update() -- updates the effect

Other Settings:
    color (default to 0) -- if you want just a static color, you could use this along with colorMode = 0, but 
                           effects should already give you this option by themselves 
*/
class BGRateFillPS : public EffectBasePS {
    public:
        BGRateFillPS(SegmentSet &SegmentSet, uint8_t ColorMode, uint16_t Rate);  

        SegmentSet 
            &segmentSet;
        
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
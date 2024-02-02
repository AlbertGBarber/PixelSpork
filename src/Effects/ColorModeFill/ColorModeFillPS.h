#ifndef ColorModeFillPS_h
#define ColorModeFillPS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

/* 
Not a full effect, but could be used as one
Fills in the segment Set using the specified colorMode (see segDrawUtils::getPixelColor()) at the specified rate
Useful for drawing simple rainbow shifting effects on the segment set using the color modes 
or filling in a background before drawing an effect.

For a moving rainbow try out these segment settings with `color mode = 1` for the effect:
    //(place these in the Arduino Setup() function)
    mainSegments.runOffset = true; //Tells Pixel Spork to move the segment set's rainbow 
    mainSegments.gradLenVal = 30; //Sets the length of the rainbow for color mode 3
    mainSegments.offsetRateOrig = 80; //Sets the speed of the rainbow movement
    mainSegments.offsetDirect = true; //Sets the direction of the rainbow's motion 
    //                                  (start of the segment set to the end)

Example call: 
    mainSegments.runOffset = true; //Set the Segment Set to shift its color mode offset over time
    ColorModeFillPS colorModeFill(mainSegments, 7, 70);
    Will fill the segment using color mode 7 every 70ms

Constructor Inputs:
    colorMode -- the colorMode that will be used in segDrawUtils::getPixelColor()
    rate -- The update rate

Other Settings:
    color (default to 0) -- if you want just a static color, you could use this along with colorMode = 0, but 
                            effects should already give you this option by themselves 

Functions:
    update() -- updates the effect

*/
class ColorModeFillPS : public EffectBasePS {
    public:
        ColorModeFillPS(SegmentSetPS &SegSet, uint8_t ColorMode, uint16_t Rate);

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
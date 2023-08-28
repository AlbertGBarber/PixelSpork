#ifndef RainbowCycleSLSeg_h
#define RainbowCycleSLSeg_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

/*
Draws a repeated rainbow of a set length repeatedly along segment lines
shifts the rainbows in along the SegmentSetPS liens in the passed in direction at the passed in rate
suggested min length is 5 (ie red, yellow, green, blue, indigo/violet)

This is the 2D version of rainbowCycle which draws on segment lines or whole segments
(although if you pass in a 1D segment the results will be the same)
For most effects I would combine the two effects for one that just works on segment lines
But since this is a very basic effect I wanted to keep it more user friendly, so I kept the 1D version of the effect

For a better version of this effect you should use the segment rainbow effects that are built into
the library's colorModes. These produce a smoother rainbow.
To do this you make a BGRateFill effect with color mode 3 or 4
Then change the segment's gradLineVal or gradSegVal to set the rainbow length, set the offsetRate to set the speed
set runOffset to true, and change the offsetDirect if needed. 
ie:   mainSegments.runOffset = true;
      mainSegments.gradLineVal = 30;
      mainSegments.offsetRateOrig = 80;
      mainSegments.offsetDirect = true;
This would also let you change to different color modes on the fly
But this is maybe too complicated for learners.

Example call: 
    RainbowCycleSLSeg rainbowCycleSLSeg(mainSegments, 30, true, false, 80); 
    Will draw rainbows of length 30, moving towards the end of the segment lines, at 80ms
    The rainbows will be drawn along segment lines

    RainbowCycleSLSeg rainbowCycleSLSeg(mainSegments, 50, false, true, 80); 
    Will draw rainbows of length 50, moving from the last to first segment, at 80ms
    The rainbow colors will be drawn on each segment

    RainbowCycleSLSeg rainbowCycleSLSeg(mainSegments, false, true, 80); 
    Will draw rainbows of length 255 (the length is set to 255 by default b/c it is omitted from the constructor), 
    moving from the last to first segment, at 80ms
    The rainbow colors will be drawn on each segment

Constructor Inputs:
    length (optional) -- The length of each rainbow, if omitted, 255 will be used
    direct -- The direction the rainbows will move in (true is forward)
    segMode -- Sets if the rainbows will be drawn along segment lines or whole segments
               (true will do whole segments)
    Rate -- update rate (ms)

Functions:
    setLength(newLength) -- sets a new rainbow length
    update() -- updates the effect 

Other Settings:
    satur (default 255) -- rainbow saturation value
    value (default 255) -- rainbow value value

Reference Vars:
    length -- The length of the rainbow. Set using setLength();
 
*/
class RainbowCycleSLSeg : public EffectBasePS {
    public:
        //Constructor for a custom length rainbow cycle
        RainbowCycleSLSeg(SegmentSetPS &SegSet, uint16_t Length, bool Direct, bool SegMode, uint16_t Rate);

        //Constructor for a full length rainbow (255)
        RainbowCycleSLSeg(SegmentSetPS &SegSet, bool Direct, bool SegMode, uint16_t Rate);

        uint8_t
            satur = 255,
            value = 255;

        uint16_t
            length;  //reference only, call setLength() to change the rainbow length

        bool
            segMode,
            direct;

        void
            setLength(uint16_t newLength),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0;

        int8_t
            stepDirect;

        uint16_t
            cycleNum,
            numLines,
            numSegs,
            maxCycleLength,
            stepVal;

        CRGB
            getRainbowColor(uint16_t index),
            color;

        void
            init(SegmentSetPS &SegSet, uint16_t Rate);
};

#endif
#ifndef ShiftingRainbowSeaSL_h
#define ShiftingRainbowSeaSL_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"
#include "Effects/ShiftingSeaSL/Utils/shiftingSeaUtilsPS.h"

/*
A rainbow version of ShiftingSeaSL. Works in much the same way.
Cycles each line of a segment set through a rainbow. Each line is given an random 
offset to place it somewhere mid-rainbow, so the result is a shifting sea of rainbow colors.

There are two shift modes: 
    0 -- where the offsets are chosen randomly from any point of the rainbow
    1 -- where the offsets are chosen randomly up to a set value (gradLength, max 255)

In shift mode 1, the lines will all generally shift from color to color, but some will be ahead of others
creating a varied look, whereas in shift mode 0 it is more random.

You can specify a grouping for the pixels, this will set the offsets of consecutive lines to be the same
the number of lines grouped together is chosen randomly (up to the grouping amount)
This makes the effect more uniform, and may look better with longer segment sets.

By default, once the offsets are set, they do not change. This can make the effect look a bit repetitive
to counter this, you can turn on random shifting, which will increment the offset of a pixel
by up to shiftStep (default 1) if a random threshold is met. 
You can modify the likelihood of shifting by changing "shiftThreshold" and "shiftBasis".
(see Other Settings below)

Note that by passing a gradLength of 0 to the constructor, you will set the effect to shift mode 0
In shift mode 0 the gradLength isn't used, but that if you set it back to shift mode 0,
the gradLength will be set to random value between 20 - 40 to prevent crashing.

The effect is adapted to work on segment lines for 2D use, but you can keep it 1D by
passing in a SegmentSetPS with only one segment containing the whole strip.

Also note that the class needs a uint16_t array the length of the number of pixels in the segment in order to work
So if you are short on ram, you might not be able to run this!

Example calls: 
    ShiftingRainbowSeaSL shiftingRainSea(mainSegments, 0, 3, 30);
    Will shift through the rainbow, using shift mode 0 (offsets can be anything between 0 and 255), 
    Note that passing 0 as the gradLength automatically sets the shift mode to 0
    grouping pixels by 3, at a rate of 20ms

    ShiftingRainbowSeaSL shiftingRainSea(mainSegments, 10, 3, 30);
    Will shift through the rainbow, using shift mode 1, where the offsets are chosen to be in a range of 0 to 10
    grouping pixels by 3, at a rate of 20ms

Inputs:
    GradLength -- Used for shift mode 1, the maximum initial value of an offset
                 Note that by passing a gradLength of 0, you will set the effect to shiftMode 0
                 In shiftMode 0 the gradLength isn't used, but to make sure that if you set it back to shiftMode 0,
                 the gradLength will be set to random value between 20 - 40
    Grouping -- Min value of 1. The maximum number of consecutive pixels that can share the same offset
               (the offsets are grouped in randomly lengths up to the Grouping value)
    Rate -- The update rate (ms)

Functions:
    resetOffsets() -- Resets the offset array, re-calculating offsets for each pixel, will cause a jump if done mid-effect
    setShiftMode(newMode) -- Changes the shift mode of the effect, also resets the offset array
    setGradLength(newGradLength) -- Changes the gradLength to the specified value, resets the offset array (if in shift mode 1)
    setGrouping(newGrouping) -- Sets a new grouping value for the effect, also resets the offset array since that's where the grouping is set
    update() -- updates the effect

Other Settings:
    sat and val (default 255) -- the saturation and value settings for the rainbow
    shiftMode -- The shift mode of the effect, either 0, or 1: 0 for the offsets to be picked at any point in the rainbow
                                                               1 for the offsets to be picked between 0 and gradLength
    randomShift (default false) -- Turns on/off the random shift for the pixel offsets (see effect description above)
    shiftThreshold (default 15) -- Sets the probability threshold for incrementing a pixel's offset, out of "shiftBasis" (100), 
                                   with higher values being more likely. 15 seemed to look good in my tests.
    shiftStep (default 1, min 1) -- The maximum value (is chosen randomly) of the offset increment if the shiftThreshold is met
    shiftBasis (default 100) -- The shift probability threshold. 
                                A pixel's offset will shift if "random(shiftBasis) <= shiftThreshold".

Reference Vars:
    grouping -- (see constructor notes above) set this using setGrouping()
    shiftMode -- (see constructor notes above) set this using setShiftMode()
    gradLength -- (see constructor notes above) set this using setGradLength()
    cycleNum -- tracks how many update's we've done, resets every 255 cycles (each pixel will have gone through the rainbow once)

*/
class ShiftingRainbowSeaSL : public EffectBasePS {
    public:
        //Constructor for the effect. Note that by passing a gradLength of 0, you will set the effect to shift mode 0
        //In shift mode 0 the gradLength isn't used, but if you set it back to shift mode 0,
        //the gradLength will be set to random value between 20 - 40 to prevent crashing.
        ShiftingRainbowSeaSL(SegmentSetPS &SegSet, uint8_t GradLength, uint8_t Grouping, uint16_t Rate);

        //destructor
        ~ShiftingRainbowSeaSL();

        uint8_t
            sat = 255,
            val = 255,
            shiftThreshold = 15,
            shiftStep = 1,
            grouping,    //for reference, set this using setGrouping()
            shiftMode,   //for reference, set this using setShiftMode()
            gradLength;  //for reference, set this using setGradLength()

        uint16_t
            *offsets = nullptr,
            shiftBasis = 100, //random shift change scaling (random(shiftBasis) <= shiftThreshold controls shifting)
            cycleNum = 0;  //tracks how many update's we've done, max value of 255

        bool
            randomShift = false;

        void
            setGradLength(uint8_t newGradLength),
            setShiftMode(uint8_t newMode),
            setGrouping(uint16_t newGrouping),
            resetOffsets(),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0;

        CRGB
            color;

        uint8_t
            step;

        uint16_t
            numLines,
            numLinesMax = 0;  //used for tracking the memory size of the offset array

        void
            init(SegmentSetPS &SegSet, uint16_t Rate);
};

#endif
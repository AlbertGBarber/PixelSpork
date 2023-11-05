#ifndef ShiftingSeaSL_h
#define ShiftingSeaSL_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"
#include "Utils/shiftingSeaUtilsPS.h"

/*
Cycles each line of a segment set through a pattern of colors. Each segment line is given an random 
offset to place it somewhere mid-fade between the pattern colors, so the result is a shifting sea of colors. 

The pattern colors are taken from passed in palette. There are constructor options for just using a palette
(in which case a pattern will be created that matches the palette).

There are two shift modes (set with shiftMode): 
    0: Where the offsets are chosen randomly from any point of fading between any 
       two consecutive colors of the pattern (so the overall output is a mix of the pattern colors)

    1: Where the offsets are chosen randomly from the fade between the first and second colors
       in mode 1, the lines will all generally shift from color to color, 
       but some will be ahead of others, creating a varied look.

You can specify a grouping for the lines/pixels, which sets the offsets of consecutive pixels to be the same.
The number of lines/pixels grouped together is chosen randomly (up to the grouping amount).
This makes the effect more uniform, and may look better with larger patterns or segments sets

By default, once the offsets are set, they do not change. This can make the effect look a bit repetitive
to counter this, you can turn on random shifting, which will increment the offset of a line
by up to shiftStep (default 1) if a random threshold is met.

You can change how many gradient steps between colors there are on the fly.

In some cases the effect looks better if there is a "blank" inserted into the cycle
so that pixels will cycle to off every so often.
However most palettes don't include off as a color, so I've build the effect to automatically add "blanks" to
its shift pattern. This is controlled by the bgMode setting:

bgModes:
    0 -- No spaces (ex: {0, 1, 2, 3, 4}, where the values are palette indexes)
    1 -- One space added to the end of the pattern (ex: {0, 1, 2, 3, 4, 255})
    2 -- A space is added after each color (ex: {0, 255, 1, 255, 2, 255, 3, 255, 4, 255})

Note that 255 is used to mark the "blank" spaces in the pattern. You can also add spaces to your own input patterns.

You can also set the blank color (default is 0) using the *bgColor var.

Please note that this effect will not work with the colorModes of segDrawUtils::setPixelColor();
But you can find a rainbow version of the effect in ShiftingRainbowSeaPS.h

The effect is adapted to work on segment lines for 2D use, but you can keep it 1D by
passing in a SegmentSetPS with only one segment containing the whole strip.

Also note that the class needs a uint16_t array the length of the number of pixels in the segment in order to work
So if you are short on ram, you might not be able to run this!

This effect can is a little computationally heavy since you are calculating blends for each pixel or line.
For a similar effects you could try Lava or NoiseSL.

Example calls: 

    uint8_t pattern_arr = {0, 2, 1};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};
    ShiftingSeaSL shiftingSea(mainSegments, pattern, cybPnkPal_PS, 20, 0, 3, 40);
    Will shift through the colors of cybPnkPal_PS according to the pattern (color 0, then 2, then 1),
    with 20 steps between each shift, using shift mode 0
    grouping pixels by 3, at a rate of 40ms

    ShiftingSeaSL shiftingSea(mainSegments, cybPnkPal_PS, 20, 0, 3, 0, 40);
    Will shift through the colors of cybPnkPal_PS, with 20 steps between each shift, using shift mode 0.
    grouping pixels by 3, at a rate of 40ms.
    No "blank" spaces are added to the shift pattern (bgMode 0).

    ShiftingSeaSL shiftingSea(mainSegments, 4, 15, 1, 1, 2, 60);
    Will shift a random set of 4 colors, with 15 steps between each shift, using shift mode 1
    grouping pixels by 1, with "blank" spaces added between each shift color (bgMode 2) at a rate of 60ms.

Inputs:
    pattern (optional, see constructors) -- Used for making a strobe that follows a specific pattern (using colors from a palette) (see patternPS.h) 
    palette (optional, see constructors) -- The palette from which colors will be chosen
    numColors (optional, see constructors) -- The length of the randomly created palette used for the effect
    gradLength -- (max 255) the number of steps to fade from one color to the next
    shiftMode -- The shift mode of the effect, see Shift Modes in intro for list.
    grouping -- Min value of 1. The maximum number of consecutive pixels that can share the same offset
               (the offsets are grouped in randomly lengths up to the Grouping value)
    bgMode (optional, see constructors) -- Controls if "blank" spaces are added to the shift pattern
                                           See bgModes in intro for mode list.
    rate -- The update rate (ms)

Functions:
    resetOffsets() -- Resets the offset array, re-calculating offsets for each pixel, will cause a jump if done mid-effect
    setShiftMode(newMode) -- Changes the mode of the effect, also resets the offset array
    setGrouping(newGrouping) -- Sets a new grouping value for the effect, also resets the offset array since that's where the grouping is set
    setPaletteAsPattern() -- Sets the effect pattern to match the current palette, 
                             will also add "blank" spaces depending on bgMode (see intro)
    setBgMode(newBgMode) -- Sets the bgMode to control "blank" pattern spaces (See bgMode note in intro). 
                            Note that changing the bgMode also changes the shift pattern to use patternTemp
                            and re-writes patternTemp to a new pattern for the bgMode.
    update() -- updates the effect

Other Settings:
    *bgColor and bgColorOrig (default 0) -- The color used by "blank" spaces. By default the bgColor is pointed to bgColorOrig.
    randomShift (default false) -- Turns on/off the random shift for the pixel offsets (see effect description above)
    shiftThreshold (default 15) -- Sets the threshold for if a pixel offset will increment, out of 100, with higher values being more likly
                                   15 seemed to look good in my tests
    shiftStep (default 1, min 1) -- The maximum value (is chosen randomly) of the offset increment if the shiftThreshold is met

Reference Vars:
    grouping -- (see constructor vars above). Set using setGrouping().
    shiftMode -- (see constructor vars above). Set using setShiftMode().
    bgMode -- (see constructor vars above). Set using setBgMode().
    totalCycleLength -- The total number of possible offsets a pixel can have (one for each fade color)
    cycleNum -- Tracks how many update's we've done, resets at totalCycleLength (each pixel will have cycled through the pattern once)

*/
class ShiftingSeaSL : public EffectBasePS {
    public:
        //Constructor for effect with pattern and palette
        ShiftingSeaSL(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, uint8_t GradLength,
                      uint8_t ShiftMode, uint8_t Grouping, uint16_t Rate);

        //Constructor for effect with palette
        ShiftingSeaSL(SegmentSetPS &SegSet, palettePS &Palette, uint8_t GradLength, uint8_t ShiftMode, uint8_t Grouping,
                      uint8_t BgMode, uint16_t Rate);

        //Constructor for effect with randomly created palette
        ShiftingSeaSL(SegmentSetPS &SegSet, uint8_t NumColors, uint8_t GradLength, uint8_t ShiftMode, uint8_t Grouping,
                      uint8_t BgMode, uint16_t Rate);

        //destructor
        ~ShiftingSeaSL();

        uint8_t
            shiftThreshold = 15,
            shiftStep = 1,
            grouping,   //for reference, set this using setGrouping()
            shiftMode,  //for reference, set this using setShiftMode()
            bgMode,     //for reference, set this using setBgMode()
            gradLength;

        uint16_t
            *offsets = nullptr,
            totalCycleLength,  //the total number of possible offsets a pixel can have (one for each fade color), for reference
            cycleNum = 0;      //tracks how many update's we've done, max value of totalCycleLength, for reference

        bool
            randomShift = false;

        CRGB
            bgColorOrig = 0, //default "blank" color for spaces
            *bgColor = &bgColorOrig;

        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0};  //Must init structs w/ pointers set to null for safety

        patternPS
            *pattern = nullptr,
            patternTemp = {nullptr, 0, 0};  //Must init structs w/ pointers set to null for safety

        void
            setShiftMode(uint8_t newMode),
            setBgMode(uint8_t newBgMode),
            setGrouping(uint16_t newGrouping),
            resetOffsets(),
            setPaletteAsPattern(),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0;

        uint8_t
            patternLen,
            curColorIndex,
            nextColorIndex,
            gradStep;

        uint16_t
            numLines,
            numLinesMax = 0,  //used for tracking the memory size of the offset array
            curPatIndex,
            step = 0;

        CRGB
            color,
            currentColor,
            nextColor;

        void
            init(SegmentSetPS &SegSet, uint16_t Rate),
            setTotalCycleLen();
};

#endif
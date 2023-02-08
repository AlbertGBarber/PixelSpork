#ifndef ShiftingSeaSLPS_h
#define ShiftingSeaSLPS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"
#include "Utils/ShiftingSeaUtilsPS.h"

/*
Cycles each line of a segment set through a pattern of colors. Each segment line is given an random 
offset to place it somewhere mid-fade between the pattern colors, so the result is a shifting sea of colors. 

The pattern colors are taken from passed in palette. There are constructor options for just using a palette
(in which case a pattern will be created that matches the palette)

There are two modes: 
    0: Where the offsets are choosen randomly from any point of fading between any 
       two consecutive colors of the pattern (so the overall output is a mix of the pattern colors)
    1: Where the offsets are choosen randomly from the fade between the first and second colors
       in mode 1, the lines will all generally shift from color to color, 
       but some will be ahead of others, creating a varied look.

You can specify a grouping for the lines, this will set the offsets of consecutive pixels to be the same
the number of lines grouped together is choosen randomly (up to the grouping amount).
This makes the effect more uniform, and may look better with larger patterns or segments sets

By default, once the offsets are set, they do not change. This can make the effect look a bit repetitive
to counter this, you can turn on random shifting, which will increment the offset of a line
by up to shiftStep (default 1) if a random threshold is met.

You can change how many gradient steps between colors there are on the fly.

In some cases the effect looks better if there is a "blank" inserted into the cycle
so that pixels will cycle to off every so often.
However most palettes don't include off as a color, so I've build an off state into the effect directly.
This will not modify any palettes or patterns, the blank state is tacked on to the end of the pattern color cycle.
You can include the off state by setting addBlank to true.
You can also set the blank color (default is 0) using the *blankColor var.
Note that turning this on or off while the effect is running will cause colors to jump.

Please note that this effect will not work with the colorModes of segDrawUtils::setPixelColor();
But you can find a rainbow version of the effect in ShiftingRainbowSeaPS.h

The effect is adapted to work on segment lines for 2D use, but you can keep it 1D by
passing in a segmentSet with only one segment containing the whole strip.

Also note that the class needs a uint16_t array the length of the number of pixels in the segment in order to work
So if you are short on ram, you might not be able to run this!

This effect can is a little computationally heavy since you are caculating blends for each pixel or line.
For a similar effect you could try Lava or one of the other noise effects.

Example call: 

    uint8_t pattern_arr = {0, 2, 1};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr)};
    ShiftingSeaSL(mainSegments, &pattern, &palette1, 20, 0, 3, 40);
    Will shift through the colors of palette1 according to the pattern (color 0, then 2, then 1),
    with 20 steps between each shift, using mode 0
    grouping pixels by 3, at a rate of 40ms

    ShiftingSeaSL(mainSegments, &palette1, 20, 0, 3, 40);
    Will shift through the colors of palette1, with 20 steps between each shift, using mode 0
    grouping pixels by 3, at a rate of 40ms

    ShiftingSeaSL(mainSegments, 4, 15, 1, 1, 60);
    Will shift a random set of 4 colors, with 15 steps between each shift, using mode 1
    grouping pixels by 1, at a rate of 60ms

Inputs:
    pattern(optional, see constructors) -- Used for making a strobe that follows a specific pattern (using colors from a palette) (see patternPS.h) 
    palette (optional, see constructors) -- The palette from which colors will be choosen
    numColors (optional, see constructors) -- The length of the randonly created palette used for the effect
    gradLength -- (max 255) the number of steps to fade from one color to the next
    sMode -- The mode of the effect, either 0, or 1: 0 for the offsets to be picked between any two colors
                                                     1 for the offsets to be picked from between the first two colors
    grouping -- Min value of 1. The maximum number of consecutive pixels that can share the same offset
               (the offsets are grouped in randomly lengths up to the Grouping value)
    rate -- The update rate (ms)

Functions:
    resetOffsets() -- Resets the offset array, recaulating offsets for each pixel, will cause a jump if done mid-effect
    setMode(newMode) -- Changes the mode of the effect, also resets the offset array
    setGrouping(newGrouping) -- Sets a new grouping value for the effect, also resets the offset array since that's where the grouping is set
    setPaletteAsPattern() -- Sets the effect pattern to match the current palette
    update() -- updates the effect

Other Settings:
    addBlank (default false) -- Adds a blank color to the cycle. The blank color is stored as blankColor.
    *blankColor and blankColorOrig (default 0) -- The color used by addBlank. By default the blankColor is pointed to blankColorOrig.
    randomShift (default false) -- Turns on/off the random shift for the pixel offsets (see effect description above)
    shiftThreshold (default 15) -- Sets the threshold for if a pixel offset will increment, out of 100, with higher values being more likly
                                   15 seemed to look good in my tests
    shiftStep (default 1, min 1) -- The maximum value (is choosen randomly) of the offset increment if the shiftThreshold is met

Reference Vars:
    grouping -- (see notes above) set this using setGrouping()
    sMode -- (see notes above) set this using setMode()
    totalCycleLength -- The total number of possible offsets a pixel can have (one for each fade color)
    cycleNum -- Tracks how many update's we've done, resets at totalCycleLength (each pixel will have cycled through the pattern once)

*/
class ShiftingSeaSL : public EffectBasePS {
    public:
        //Constructor for effect with pattern and palette
        ShiftingSeaSL(SegmentSet& SegmentSet, patternPS *Pattern, palettePS* Palette, uint8_t GradLength, uint8_t Smode, uint8_t Grouping, uint16_t Rate);

        //Constructor for effect with palette
        ShiftingSeaSL(SegmentSet &SegmentSet, palettePS *Palette, uint8_t GradLength, uint8_t Smode, uint8_t Grouping, uint16_t Rate);  

        //Constructor for effect with randomly created palette
        ShiftingSeaSL(SegmentSet& SegmentSet, uint8_t NumColors, uint8_t GradLength, uint8_t Smode, uint8_t Grouping, uint16_t Rate);

        //destructor
        ~ShiftingSeaSL();

        SegmentSet 
            &segmentSet; 
        
        uint8_t
            shiftThreshold = 15,
            shiftStep = 1,
            grouping, //for reference, set this using setGrouping()
            sMode, //for reference, set this using setMode()
            gradLength;
        
        uint16_t
            totalCycleLength, //the total number of possible offsets a pixel can have (one for each fade color), for reference
            cycleNum = 0; //tracks how many update's we've done, max value of totalCycleLength, for reference
        
        bool
            randomShift = false,
            addBlank = false;
        
        CRGB 
            blankColorOrig = 0,
            *blankColor = &blankColorOrig;
        
        palettePS
            paletteTemp,
            *palette = nullptr;
        
        patternPS
            patternTemp,
            *pattern = nullptr;
        
        void
            setMode(uint8_t newMode),
            setGrouping(uint16_t newGrouping),
            resetOffsets(),
            setPaletteAsPattern(),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;

        CRGB
            color,
            currentColor,
            nextColor;

        uint8_t 
            patternLen,
            curColorIndex, 
            nextColorIndex,
            gradStep;

        uint16_t
            numLines,
            curPatIndex,
            *offsets = nullptr,
            step = 0;
        
        void
            init(uint16_t Rate),
            setTotalCycleLen();
};

#endif
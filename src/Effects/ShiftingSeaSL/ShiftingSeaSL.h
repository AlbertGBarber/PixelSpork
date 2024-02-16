#ifndef ShiftingSeaSL_h
#define ShiftingSeaSL_h

//TODO: place a cap on the random shift if in shift mode 1.
//      To do this, calculate the avg offset for each cycle, only allow a shift if the value is within some range of the avg
//      range => 1/3 totalCycleLength?

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"
#include "Utils/shiftingSeaUtilsPS.h"

/*
## Description:

Blends each pixel through a pattern of colors. 
Each pixel is given an random offset to place it somewhere mid-fade between two colors, 
so the result is a shifting sea of colors. The pattern colors are taken from passed in palette. 
There are constructor options for just using a palette (in which case a pattern will be created that matches the palette). 
You also have the option of using rainbow colors for the effect, see "Rainbow Mode" notes below.

While this effect may seem simple, there are a number of randomization options/settings that 
can help produce a more dynamic look. See the Inputs Guide below.

The effect is adapted to work on segment lines for 2D use. Each segment line will be a single color.

!!Does not support Color Modes (See segDrawUtils::SetPixelColor()).

The effect requires an array of uint16_t's sized to the number of lines in your segment set. 
It is allocated dynamically, and will need to be re-sized if you change the segment 
set using `resetOffsets()`. If you do this, be aware of memory fragmentation. 
See https://github.com/AlbertGBarber/PixelSpork/wiki/Effects-Advanced#managing-dynamic-memory-and-fragmentation 
for more details. 

This effect can is a little computationally heavy since you are calculating blends for each pixel/line. 
For a similar effects you could try Lava or NoiseSL.

## Inputs Guide & Notes:

    Note, that some of the effect's settings can only be changed through specific function calls, 
    and usually cause an effect reset. These settings are listed in the "Reference Vars" section below. 
    However, anything not listed there, including the gradient length between colors can be changed on the fly.  

    ### Shift Modes:
        To begin, you have two options for setting the initial offsets of the pixels. 
        Each option produces a fundamentally different look.

        The looks are selected using `shiftMode` in the constructors: 
            * 0 -- The offsets are chosen randomly from any point of fading between any 
                   two consecutive colors of the pattern (so the overall output is a mix of the pattern colors).

            * 1 -- The offsets are chosen randomly from the fade between the first and second colors in the pattern, 
                   meaning that the pixels will all generally shift from color to color, but some will be ahead of others, 
                   creating a varied look.
                
        You can change both the shift mode and offsets during runtime using `setShiftMode(uint8_t newMode)` 
        and `resetOffsets()`, but doing so will cause a "jump" in the effect.

        #### Shifting Offsets Over Time:
            By default, once the offsets are set, they do not change. 
            This can make the effect look a bit repetitive, as the same pixels are always in sync. 
            To counter this, you can turn on random shifting using "randomShift", 
            which will increment the offset of a pixel by up to a configurable `shiftStep` (default 1) 
            if a random threshold is met. 
            You can modify the likelihood of shifting by changing `shiftThreshold` and `shiftBasis` (default 15 and 100). 
            (see Other Settings below).

            Note that pixels are shifted individually, so any grouping (see below) will be ignored.

        #### Limiting Shifting:
            In shiftMode 1, all the offsets are set between 0 and gradLength to produce more unified color shifting.
            However, if random shifting is on, the offsets will drift over time, slowly reducing the unified look.
            Eventually this leads to the effect looking the same as shiftMode 0. 
            
            To keep the original look, but still allow variations via shifting, I've implemented a shift limiting system. 
            The system is toggled on automatically  if shiftMode 1 is set a constructor, or can be set using "limitShift". 
            By default it limits the shift range to one gradLength, so colors are not more that one color 
            ahead of the overall color. You can control the range via "shiftMax". 
            Note that the overall result is not perfect, and the effect will become more chaotic
            than at baseline, but it is much better than without. Also note that you should probably
            leave the shiftStep at it's default of 1.

            For shiftMode 0, there is no real point in limiting shifting, as the offsets are already completely random.

    ### Grouping:
        You can specify a "grouping" for the pixels, which gives lengths of pixels the same offset. 
        The number of pixels grouped together is chosen randomly, up to the "grouping" amount. 
        This makes the effect more uniform, and may look better with larger patterns or segments sets. 
        The grouping is included as part of the offsets calculation, but you can change it during runtime using 
        `setGrouping( uint16_t newGrouping )`. 
        Doing so will cause a "jump" in the effect as the offsets are recalculated. 

        ### Inserting Background Steps:

        In some cases, the effect looks better if there is a "blank" inserted into the gradient cycle so that pixels 
        will cycle to off every so often.

        However, most palettes don't include a blank color, so I've build the effect to automatically
        add "blanks" to the shift pattern. This is controlled by the `bgMode` setting, 
        which is included in the constructors.

        #### `bgMode`'s (uint8_t):
            Note that 255 is used to mark the "blank" spaces in the pattern. 
            You can also add spaces to your own input patterns.
            * 0 -- No spaces (ex: {0, 1, 2, 3, 4}, where the values are palette indexes).
            * 1 -- One space added to the end of the pattern (ex: {0, 1, 2, 3, 4, 255}).
            * 2 -- A space is added after each color (ex: {0, 255, 1, 255, 2, 255, 3, 255, 4, 255})

        By default, the blank color is set to 0, but you can change it using the `*bgColor` 
        setting (see Other Settings below).

        Can be changed during runtime using `setBgMode(uint8_t newMode)`. 
        Changing the `bgMode` also re-creates the shift pattern, storing it in the local `patternTemp` variable.

    ### Rainbow Mode:
        Setting `rainbowMode` to true switches the effect to use a rainbow gradient for colors. 
        The gradient is the typical 255 range of colors common in other rainbow effects and Color Modes. 
        This mode works with all the other effect settings, but the number of gradient steps is fixed to 255 and 
        the background mode and colors are ignored. However, you can set the rainbow's HSV saturation (`sat`) 
        and value (`val`). Imo it works best in shift mode 1 or with a large grouping value. 
        I also recommend an update rate of 30 - 60 due to the length of the gradient.
            
        `rainbowMode` is automatically set by the rainbow mode constructor (see below), 
        but may also be toggled on at anytime. When using the rainbow constructor,
        if shiftMode is 0, the offset range is automatically set to 255 to cover the whole rainbow, and 
        the gradLength is ignored. However, if shiftMode is 1, the constructor uses the gradLength
        to bound the initial offsets. Ie if I pass in 25 as the gradLength, the offsets will vary from 0 to 25.
        Once the offsets are setup, the gradLength is not used again, because the rainbow has a fixed
        length of 255.
        
        Also note that if using the constructor, 
        a random 3 color palette will be created for safety, so you can switch off rainbow mode without 
        the effect crashing due to a lack of palette.

Example calls: 

    uint8_t pattern_arr = {0, 2, 1};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};
    ShiftingSeaSL shiftingSea(mainSegments, pattern, cybPnkPal_PS, 20, 0, 3, 40);
    Will shift through the colors of cybPnkPal_PS according to the pattern (color 0, then 2, then 1),
    with 20 steps between each shift, using shift mode 0
    grouping pixels by 3, at a rate of 40ms

    ShiftingSeaSL shiftingSea(mainSegments, cybPnkPal_PS, 20, 0, 3, 0, 40);
    shiftingSea.randomShift = true; //place in Arduino setup()
    Will shift through the colors of cybPnkPal_PS, with 20 steps between each shift, using shift mode 0.
    grouping pixels by 3, no "blank" spaces are added to the shift pattern (bgMode 0).
    Because randomShift is turned on (via the extra line), the pixel offsets will vary over time.
    The effect updates at a rate of 40ms.

    ShiftingSeaSL shiftingSea(mainSegments, 4, 15, 1, 1, 2, 60);
    Will shift a random set of 4 colors, with 15 steps between each shift, using shift mode 1
    grouping pixels by 1, with "blank" spaces added between each shift color (bgMode 2) at a rate of 60ms.

    ShiftingSeaSL shiftingSea(mainSegments, 50, 0, 15, 30);
    Will do a shifting sea using rainbow colors (rainbowMode on)
    (note the lack of palette / color count as inputs)
    The gradient steps are set at 50, but will be ignored due to shift mode being 0 (the gradient length is 255) 
    (If shift mode was 1, 50 would be the range for the color offsets)
    The effect updates at a rate of 30ms.

Inputs:
    pattern (optional, see constructors) -- The color pattern the effect will use. 
                                            Note that any entries of 255 in the pattern will be set to the background color.
    palette (optional, see constructors) -- The palette from which colors will be chosen
    numColors (optional, see constructors) -- The length of the randomly created palette used for the effect
    gradLength -- The number of steps to fade from one color to the next.
    shiftMode -- The shift mode of the effect, see Shift Modes in Inputs Guide above. 
                 Can be changed later using `setShiftMode()`.
    grouping -- Min value of 1. The maximum number of consecutive pixels that can share the same offset, 
                see "Grouping" in Inputs Guide above. Can be changed later using `setGrouping()`.
    bgMode (optional, see constructors) -- Controls if "blank" spaces are added to the shift pattern. 
                                           See "Inserting Background Steps" in Inputs Guide above. 
                                           Can be changed later using `setBgMode()`.
    rate -- The update rate (ms)

Other Settings:
    *bgColor and bgColorOrig (default 0) -- The color used by "blank" spaces. By default the bgColor is pointed to bgColorOrig.
    randomShift (default false) -- Turns on/off the random shift for the pixel offsets (see "Shift Modes" above).
    shiftThreshold (default 15) -- Sets the probability threshold for incrementing a pixel's offset,out of "shiftBasis" (100), 
                                   with higher values being more likely. 15 seemed to look good in my tests.
    shiftBasis (default 100) -- The shift probability basis. 
                                A pixel's offset will shift if "random(shiftBasis) <= shiftThreshold".
    shiftStep (default 1, min 1) -- The maximum value (is chosen randomly) of the offset increment if the shiftThreshold is met
    limitShift (default false, set true if shiftMode is 1) -- If randomShift is on, this will help keep 
                                                              offsets from shifting too much. (See "Limiting Shifting" above)
    shiftMax (defaulted to gradLength) -- Used when limiting shifting, puts a cap on how far an offset can shift. (See "Limiting Shifting" above)
    rainbowMode (default false, but can be set by constructor) -- 
                If true, then the effect will use rainbow colors in place of palette colors. 
                (See "Rainbow Mode" notes above).
    sat (default 255) -- For rainbow mode; the rainbow's saturation value.
    val (default 255) -- For rainbow mode; the rainbow's "value" value.

Functions:
    resetOffsets() -- Resets the offset array, re-calculating offsets for each pixel, will cause a jump if done mid-effect
    setShiftMode(newMode) -- Changes the shift mode of the effect, also resets the offset array.
    setGrouping(newGrouping) -- Recalculates the "grouping" of the pixels. Resets the offset array to express the grouping.
    setPaletteAsPattern() -- Sets the effect pattern to match the current palette, 
                             will also add "blank" spaces depending on bgMode (see intro)
    setBgMode(newBgMode) -- Sets the bgMode to control "blank" pattern spaces.
                            See "Inserting Background Steps" in Inputs Guide above.
    update() -- updates the effect

Reference Vars:
    grouping -- (see constructor vars above). Set using setGrouping().
    shiftMode -- (see constructor vars above). Set using setShiftMode().
    bgMode -- (see constructor vars above). Set using setBgMode().
    totalCycleLength -- The total number of possible offsets a pixel can have (one for each fade value for each color in the pattern).
    cycleNum -- Tracks how many update's we've done, resets every totalCycleLength updates (each pixel will have cycled through the pattern once)

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

        //Constructor for rainbow mode
        ShiftingSeaSL(SegmentSetPS &SegSet, uint8_t GradLength, uint8_t ShiftMode, uint8_t Grouping, uint16_t Rate);

        //destructor
        ~ShiftingSeaSL();

        uint8_t
            shiftStep = 1,
            grouping,   //for reference, set this using setGrouping()
            shiftMode,  //for reference, set this using setShiftMode()
            bgMode,     //for reference, set this using setBgMode()
            gradLength,
            sat = 255,  //For rainbow mode; the rainbow's saturation value
            val = 255;  //For rainbow mode; the rainbow's "value" value

        uint16_t
            *offsets = nullptr,
            shiftThreshold = 15,  //Default random shift chance (out of shiftBasis)
            shiftBasis = 100,     //random shift change scaling (random(shiftBasis) <= shiftThreshold controls shifting)
            shiftMax,             //Used when limiting shifting, puts a cap on how far an offset can shift (defaulted to gradLength)
            totalCycleLength,     //the total number of possible offsets a pixel can have (one for each fade color), for reference
            cycleNum = 0;         //tracks how many update's we've done, max value of totalCycleLength, for reference

        bool
            randomShift = false,
            limitShift = false,
            rainbowMode = false;

        CRGB
            bgColorOrig = 0,  //default "blank" color for spaces
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
            step = 0,
            shiftCheck;

        bool
            canShift;

        CRGB
            color,
            currentColor,
            nextColor;

        void
            init(SegmentSetPS &SegSet, uint16_t Rate),
            setTotalCycleLen();
};

#endif
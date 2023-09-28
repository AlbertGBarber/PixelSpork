#ifndef StillPatternSLSeg_h
#define StillPatternSLSeg_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
//#include "MathUtils/mathUtilsPS.h"

/*
Effect Description:
Draws an unmoving color pattern at a set refresh rate.
Useful for backgrounds, or maybe combining with a palette utility to change colors.
Features multiple constructors for different pattern and palette combinations.
Allows you to add blank spaces to your pattern (any 255 entries in the pattern will be drawn as background)
and set the lengths of the pattern's color bands.

The patterns can be drawn in 1D, or in 2D, using segment lines, or whole segments (segModes).

segModes:
    0 -- The pattern will be drawn using segment lines (each line will be a single color)
    2 -- The pattern will be drawn using whole segment (each segment will be a single color)
    3 -- The pattern will be drawn linearly along the segment set (1D).

Supports color modes for both the main and background colors. The bgColor is a pointer, so you can bind it
to an external color variable.

Note that using 255 in your patterns will draw the background color.

Note that while each entry in the pattern is a uint8_t,
if you have a lot of colors, with long streamers, your patterns may be quite large
so watch your memory usage. Likewise, if you re-size the waves, the pattern may also be dynamically re-sized.
(see alwaysResizeObj_PS in Include_Lists -> GlobalVars, and the Effects Advanced Wiki Page -> Managing Memory Fragmentation)

You can freely change any of the effect's variables during runtime.

Example calls: 
    uint8_t pattern_arr = {0, 255, 255, 255, 1, 1, 255, 255};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};
    StillPatternSLSeg stillPattern(mainSegments, pattern, cybPnkPal_PS, 0, 0, 80);
    Will do pattern using the first two colors in the palette (based on the pattern)
    The [pattern will begin with 1 pixel of color 0, with three spaces after, 
    followed by 2 pixels of color 1, followed by 2 spaces.
    The bgColor is zero (off).
    The effect uses segMode 0, so the pattern will be drawn linearly (1D) on the segment set.
    The pattern will be re-drawn every 80ms.

    uint8_t pattern_arr = {1, 2, 3};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};
    StillPatternSLSeg stillPattern(mainSegments, pattern, cybPnkPal_PS, 3, 4, 0, 1, 120);
    Will draw a pattern using the first three colors of the palette (taken from the pattern)
    The pattern will have color bands of length 3, followed by 4 spaces, bgColor is 0 (off).
    The pattern will be drawn on segment lines (segMode is 1).
    The pattern will be re-drawn every 120ms.

    StillPatternSLSeg stillPattern(mainSegments, cybPnkPal_PS, 3, 4, CRGB::Red, 2, 40);
    Will draw a pattern using all the colors in cybPnkPal_PS, 
    The pattern will have color bands of length 3, with 4 spaces in between.
    The bgColor is red.
    The pattern will be drawn on whole segments (segMode is 2).
    The pattern will be re-drawn every 40ms.

    StillPatternSLSeg stillPattern(mainSegments, CRGB::Blue, 2, 2, CRGB::Red, 0, 140);
    Will do a pattern with blue color bands of length 2 and 2 spaces in between.
    The bgColor is red.
    The effect uses segMode 0, so the pattern will be drawn linearly (1D) on the segment set.
    The effect updates at a rate of 140ms

Constructor Inputs:
    pattern(optional, see constructors) -- The pattern used for the streamers, made up of palette indexes.
    palette(optional, see constructors) -- The repository of colors used in the pattern.
    color(optional, see constructors) -- Used for making a single color pattern.
    colorLength (optional, see constructors, max 255) -- The number pixels a color band is. Used for automated pattern creation.
    spacing (optional, see constructors, max 255) -- The number of pixels between each color band (will be set to bgColor).  
                                                     Used for automated pattern creation.
    bgColor -- The color of the spacing pixels. It is a pointer, so it can be tied to an external variable.
               By default it is bound to bgColorOrig.
    segMode -- Sets if the pattern will be drawn along segment lines, whole segments, or in 1D (see intro notes)
    rate -- The pattern re-draw rate (ms)

Functions:
    setPatternAsPattern(&inputPattern, colorLength, spacing) -- Takes an input pattern and creates a color pattern from it using the current palette
                                                                Ex: uint8_t pattern_arr = {1, 2, 3};
                                                                    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};
                                                                    setPatternAsPattern(pattern, 3, 4); 
                                                                    Will do a color pattern using the first three colors of the palette 
                                                                    (taken from the pattern).
                                                                    Each streamer will be length 3, followed by 4 spaces.
    setPaletteAsPattern(uint8_t colorLength, uint8_t spacing) -- Like the previous function, 
                                                                 but all of the current palette will be used for the pattern.                                                       
    update() -- updates the effect

Other Settings:
    colorMode (default 0) -- sets the color mode for the streamer pixels (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the spacing pixels (see segDrawUtils::setPixelColor)
*/
class StillPatternSLSeg : public EffectBasePS {
    public:
        //Constructor for using the passed in pattern and palette
        StillPatternSLSeg(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, CRGB BgColor, uint8_t SegMode, uint16_t Rate);

        //Constructor for building the pattern from the passed in pattern and the palette,
        //using the passed in colorLength and spacing
        StillPatternSLSeg(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, uint8_t ColorLength, uint8_t Spacing,
                      CRGB BgColor, uint8_t SegMode, uint16_t Rate);

        //Constructor for building the pattern using all the colors in the passed in palette,
        //using the colorLength and spacing for each color
        StillPatternSLSeg(SegmentSetPS &SegSet, palettePS &Palette, uint8_t ColorLength, uint8_t Spacing, CRGB BgColor,
                      uint8_t SegMode, uint16_t Rate);

        //Constructor for doing a single colored pattern, using colorLength and spacing
        StillPatternSLSeg(SegmentSetPS &SegSet, CRGB Color, uint8_t ColorLength, uint8_t Spacing, CRGB BgColor,
                      uint8_t SegMode, uint16_t Rate);

        ~StillPatternSLSeg();

        uint8_t
            segMode,
            colorMode,
            bgColorMode;

        CRGB
            bgColorOrig,
            *bgColor = nullptr;  //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)

        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0};  //Must init structs w/ pointers set to null for safety

        patternPS
            *pattern = nullptr,
            patternTemp = {nullptr, 0, 0};

        void
            setPatternAsPattern(patternPS &inputPattern, uint8_t colorLength, uint8_t spacing),
            setPaletteAsPattern(uint8_t colorLength, uint8_t spacing),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0;

        uint8_t
            patVal,
            modeOut;

        uint16_t
            patLen,
            patIndex,
            patIndexesToDraw;

        CRGB
            colorOut;

        void
            init(CRGB BgColor, SegmentSetPS &SegSet, uint16_t Rate),
            getNextPatColor();
};

#endif
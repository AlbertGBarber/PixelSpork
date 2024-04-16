#ifndef DrawPatternSLSeg_h
#define DrawPatternSLSeg_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
//#include "MathUtils/mathUtilsPS.h"

/*
Effect Description:
Draws a fixed color pattern at a set refresh rate.
Useful for backgrounds, or maybe combining with a palette utility to change colors.
Features multiple constructors for different pattern and palette combinations.
Allows you to add blank spaces to your pattern (any 255 entries in the pattern will be drawn as background)
and set the lengths of the pattern's color bands.

The patterns can be drawn in 1D, or in 2D, using segment lines, or whole segments (segModes).

segModes:
    0 -- The pattern will be drawn using segment lines (each line will be a single color)
    1 -- The pattern will be drawn using whole segment (each segment will be a single color)
    2 -- The pattern will be drawn linearly along the segment set (1D).

Supports color modes for both the main and background colors.

Note that using 255 in your patterns will draw the background color.

Note that while each entry in the pattern is a uint8_t,
if you have a lot of colors, with long streamers, your patterns may be quite large
so watch your memory usage. Likewise, if you re-size the waves, the pattern may also be dynamically re-sized.
(see alwaysResizeObj_PS in Include_Lists -> GlobalVars, and the Effects Advanced Wiki Page -> Managing Memory Fragmentation)

Also, remember that the pattern length is limited to 65,025 (uint16_t max), 
so make sure your (colorLength + spacing) * <num palette colors> is less than the limit.

You can freely change any of the effect's variables during runtime.

Example calls: 
    uint8_t pattern_arr = {0, 255, 255, 255, 1, 1, 255, 255};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};
    DrawPatternSLSeg drawPat(mainSegments, pattern, cybPnkPal_PS, 0, 2, 80);
    Will draw a pattern using the first two colors in the cybPnkPal_PS palette (based on the pattern above)
    The pattern will begin with 1 pixel of color 0, with three spaces after, 
    followed by 2 pixels of color 1, followed by 2 spaces.
    The background is black (0).
    The effect uses segMode 2, so the pattern will be drawn linearly (1D) on the segment set.
    The pattern will be re-drawn every 80ms.

    uint8_t pattern_arr = {0, 2, 1};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};
    DrawPatternSLSeg drawPat(mainSegments, pattern, cybPnkPal_PS, 3, 4, 0, 0, 120);
    Will draw a pattern using the first three colors of the cybPnkPal_PS palette (using the pattern above)
    The pattern will have color bands of length 3, followed by 4 spaces, background is black (0).
    The pattern will be drawn on segment lines (segMode is 0).
    The pattern will be re-drawn every 120ms.

    DrawPatternSLSeg drawPat(mainSegments, cybPnkPal_PS, 3, 4, CRGB::Red, 1, 40);
    Will draw a pattern using all the colors in cybPnkPal_PS palette, 
    The pattern will have color bands of length 3, with 4 spaces in between.
    The background is red.
    The pattern will be drawn on whole segments (segMode is 1).
    The pattern will be re-drawn every 40ms.

    DrawPatternSLSeg drawPat(mainSegments, CRGB::Blue, 2, 2, CRGB::Red, 2, 140);
    Will do a pattern with blue color bands of length 2 and 2 spaces in between.
    The background is red.
    The effect uses segMode 2, so the pattern will be drawn linearly (1D) on the segment set.
    The effect updates at a rate of 140ms

Constructor Inputs:
    pattern(optional, see constructors) -- The pattern used for the streamers, made up of palette indexes.
    palette(optional, see constructors) -- The repository of colors used in the pattern.
    color(optional, see constructors) -- Used for making a single color pattern. 
                                         The color will be placed in the effect's local palette, `paletteTemp`. 
                                         The local pattern, `patternTemp`, will be set to match the palette (ie a single entry for the single color).
    colorLength (optional, see constructors) -- The length of each color band. Used for automated pattern creation.
    spacing (optional, see constructors) -- The number of pixels between each color band (will be set to bgColor).  
                                            Used for automated pattern creation.
    bgColor -- The color of the spacing pixels. It is a pointer, so it can be tied to an external variable.
               By default it is bound to bgColorOrig.
    segMode -- Sets if the pattern will be drawn along segment lines, whole segments, or in 1D (see intro notes)
    rate -- The pattern re-draw rate (ms)

Other Settings:
    colorMode (default 0) -- sets the color mode for the streamer pixels (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the spacing pixels (see segDrawUtils::setPixelColor)

Functions:
    setPatternAsPattern(&inputPattern, colorLength, spacing) -- Takes an input pattern and creates a color pattern from it using the current palette
                                                                Ex: uint8_t pattern_arr = {1, 2, 3};
                                                                    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};
                                                                    setPatternAsPattern(pattern, 3, 4); 
                                                                    Will do a color pattern using the first three colors of the palette 
                                                                    (taken from the pattern).
                                                                    Each streamer will be length 3, followed by 4 spaces.
    setPaletteAsPattern(colorLength, spacing) -- Like the previous function, 
                                                 but all of the current palette will be used for the pattern.                                                       
    update() -- updates the effect

*/
class DrawPatternSLSeg : public EffectBasePS {
    public:
        //Constructor for using the passed in pattern and palette
        DrawPatternSLSeg(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, CRGB BgColor, uint8_t SegMode, uint16_t Rate);

        //Constructor for building the pattern from the passed in pattern and the palette,
        //using the passed in colorLength and spacing
        DrawPatternSLSeg(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, uint16_t ColorLength, uint16_t Spacing,
                      CRGB BgColor, uint8_t SegMode, uint16_t Rate);

        //Constructor for building the pattern using all the colors in the passed in palette,
        //using the colorLength and spacing for each color
        DrawPatternSLSeg(SegmentSetPS &SegSet, palettePS &Palette, uint16_t ColorLength, uint16_t Spacing, CRGB BgColor,
                      uint8_t SegMode, uint16_t Rate);

        //Constructor for doing a single colored pattern, using colorLength and spacing
        DrawPatternSLSeg(SegmentSetPS &SegSet, CRGB Color, uint16_t ColorLength, uint16_t Spacing, CRGB BgColor,
                      uint8_t SegMode, uint16_t Rate);

        ~DrawPatternSLSeg();

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
            setPatternAsPattern(patternPS &inputPattern, uint16_t colorLength, uint16_t spacing),
            setPaletteAsPattern(uint16_t colorLength, uint16_t spacing),
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
#ifndef StreamerFastPS_h
#define StreamerFastPS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

/* 
Similar to StreamerPS, but runs faster with some restrictions

Similar to StreamerSL, but runs faster with some restrictions.

Moves a set of colored "streamers" along the segment set. The effect uses a pattern for colors. 
You can either input a custom pattern, or the effect can build one for you with a set color lengths and spaces 
between each streamer.

This effect is adapted for 2D use. The streamer colors will be repeated down segment lines. 
For a version of this effect for whole segments, see SegWavesFast.

To make the effect "fast", it only ever calculates the color of the first pixel, then for each subsequent pixel, 
it copies the color of the next segment in line. This means it's only doing one color calculation per update, 
the rest is just copying what's already there.

However, this does lead to the following restrictions:
    1. The streamers do not blend as the shift across the segment set.
    2. Changing the palette on the fly will have a delayed effect on the colors. The existing colors will shift off the segments as new ones shift on. This prevents this effect from playing well with most Palette Utility Classes.
    3. The same restrictions as (2) apply to changing the pattern and most other effect settings.
    4. Changing the direction of the effect may break it temporarily.
    5. This effect is not compatible with Color Modes for either the streamers or the background.
    6. The effect should not be run alongside other effects on the same Segment Set due to it copying colors from LEDs.
    7. This effect does not play well with the EffectSetFaderPS utility.

Otherwise, the effect works the same as StreamerSL, and most of the inputs are identical.

## Inputs Guide & Notes:

    ### Patterns:
        Patterns work the same as with other effects; they are a pattern of palette array indexes. 
        ie a pattern of {0, 2, 1} would be the first three colors of a palette. 
        However, in this effect, to indicate a background pixel (set it to the `bgColor`) 
        we use 255 in the pattern. Make sure your palette doesn't have 255 colors (that would be waaaay to big!).

        For example, lets say we wanted to do the first two colors of our palette, each as length 4 streamers, 
        with 3 background pixels in between each. We would make a pattern as: 
        `{0, 0, 0, 0, 255, 255, 255, 1, 1, 1, 1, 255, 255, 255}`.

        For simple patterns like the previous example, I have written a few constructors that automate the 
        pattern creation for you, taking the length of the colored streamers, and the length of the 
        background spaces as arguments (see constructor notes below).

        Note that while each entry in the pattern is a uint8_t, if you have a lot of colors, with 
        long streamers and spaces, your patterns may be quite large, so watch your memory usage. 

        Any automatically generated patterns will be allocated dynamically. To avoid memory fragmentation, 
        when you create the effect, you should set your color length and spacing to the maximum values you 
        expect to use, and then call `setPatternAsPattern()` or `setPaletteAsPattern()` to resize 
        your streamers and spacing. See 
        https://github.com/AlbertGBarber/PixelSpork/wiki/Effects-Advanced#managing-dynamic-memory-and-fragmentation
        for more details. 

        Also, remember that the pattern length is limited to 65,025 (uint16_t max), 
        so make sure your `(colorLength + spacing) * <num palette colors>` is less than the limit. 

    ### Random Modes: 
        `randMode` controls how pattern colors are chosen. For `randMode`'s other than 0, 
        colors are chosen at random as they enter the first pixel, and are then shifted across the segment set. 

        `randMode` (default 0) (uint8_t): 
            * 0 -- Colors will be chosen in order from the pattern (not random).
            * 1 -- Colors will be chosen completely at random.
            * 2 -- Colors will be chosen at random from the pattern, but the same color won't be repeated in a row. (won't pick spaces).
            * 3 -- Colors will be chosen randomly from the pattern (allowing repeats). (won't pick spaces).
            
Example calls: 
    uint8_t pattern_arr = {0, 255, 255, 255, 1, 1, 255, 255};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};

    StreamerPS streamerFast(mainSegments, pattern, cybPnkPal_PS, 0, 120);
    Will do a set of streamers using the first two colors from the cybPnkPal_PS palette, following the pattern above
    The streamer will begin with 1 pixel of color 0, with three spaces after, 
    followed by 2 pixels of color 1, followed by 2 spaces.
    The background is blank (0).
    The effect updates at a rate of 120ms

    uint8_t pattern_arr = {0, 2, 1};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};

    StreamerPS streamerFast(mainSegments, pattern, cybPnkPal_PS, 0, 3, 4, 120);
    Will do streamers using the first three colors from the cybPnkPal_PS palette, following the pattern above.
    The background is blank (0).
    Each streamer will be length 3, followed by 4 spaces.
    The effect updates at a rate of 120ms

    StreamerPS streamerFast(mainSegments, cybPnkPal_PS, CRGB::Red, 3, 4, 120);
    Will do streamers using all the colors in cybPnkPal_PS.
    The background is red.
    each streamer will be length 3, with 4 spaces in between.
    The streamers will update at a 120ms rate

    StreamerPS streamerFast(mainSegments, CRGB::Blue, CRGB::Red, 2, 2, 0, 140);
    Will do blue streamers with length 2 and 2 spaces in between
    The background is red.
    The effect updates at a rate of 140ms.
 
Constructor Inputs:
    pattern(optional, see constructors) -- The pattern used for the streamers, made up of palette indexes 
    palette(optional, see constructors) -- The repository of colors used in the pattern
    color(optional, see constructors) -- Used for making a single color streamer
    colorLength (optional, see constructors) -- The length of a streamer. Used for automated pattern creation. 
    spacing (optional, see constructors) -- The number of pixels between each wave color (will be set to bgColor).  
                                            Used for automated pattern creation.  Used for automated pattern creation.
    bgColor -- The color of the spacing pixels. It is a pointer, so it can be tied to an external variable
    rate -- The update rate (ms)

Other Settings:
    randMode (default 0) -- (See randMode notes in intro)

Functions:
    reset() -- Restarts the streamer pattern.
    setPatternAsPattern(&inputPattern, colorLength, spacing) -- Takes an input pattern and creates a streamer pattern from it using the current palette
                                                                Ex: uint8_t pattern_arr = {1, 2, 3};
                                                                   patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};
                                                                   setPatternAsPattern(pattern, 3, 4) 
                                                                   Will do a streamer using the first three colors of the palette (taken from the pattern)
                                                                   Each streamer will be length 3, followed by 4 spaces
    setPaletteAsPattern(colorLength, spacing) -- Like the previous function, but all of the current palette will be used for the pattern                                                       
    update() -- updates the effect

Reference Vars:
    cycleNum -- Tracks how many patterns we've gone through, 
                resets every pattern length number of cycles (ie once we've gone through the whole pattern)

Flags:
    initFillDone -- Indicates if the strip has been pre-filled with the effect's color outputs 
                    This needs to happen before running the first update cycle
                    If false, preFill() will be called when first updating
                    Set true once the first update cycle has been finished

*/
class StreamerFastSL : public EffectBasePS {
    public:
        //Constructor for using the passed in pattern and palette for the streamer
        StreamerFastSL(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, CRGB BgColor, uint16_t Rate);

        //Constructor for building the streamer pattern from the passed in pattern and the palette, using the passed in colorLength and spacing
        StreamerFastSL(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, CRGB BgColor,
                       uint16_t ColorLength, uint16_t Spacing, uint16_t Rate);

        //Constructor for building a streamer using all the colors in the passed in palette, using the colorLength and spacing for each color
        StreamerFastSL(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, uint16_t ColorLength, 
                       uint16_t Spacing, uint16_t Rate);

        //Constructor for doing a single colored streamer, using colorLength and spacing
        StreamerFastSL(SegmentSetPS &SegSet, CRGB Color, CRGB BgColor, uint16_t ColorLength, uint16_t Spacing,
                       uint16_t Rate);

        ~StreamerFastSL();

        uint8_t
            randMode = 0;

        uint16_t
            cycleNum = 0;

        bool
            initFillDone = false;

        CRGB
            bgColorOrig,
            *bgColor = nullptr;  //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)

        patternPS
            *pattern = nullptr,
            patternTemp = {nullptr, 0, 0};  //Must init structs w/ pointers set to null for safety

        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0};  //Must init structs w/ pointers set to null for safety

        void
            setPatternAsPattern(patternPS &inputPattern, uint16_t colorLength, uint16_t spacing),
            setPaletteAsPattern(uint16_t colorLength, uint16_t spacing),
            reset(),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0;

        uint8_t
            nextPattern,
            prevPattern,
            randPat = 255; //Used to tracking the current pattern value for randModes, starts as spacing, but will be set in the first update().

        uint16_t
            numLines,
            numLinesLim,
            longestSeg,
            pixelNum;

        CRGB
            nextColor,
            pickStreamerColor(uint8_t nextPatternTemp);

        void
            initalFill(),
            init(CRGB BgColor, SegmentSetPS &SegSet, uint16_t Rate);
};

#endif
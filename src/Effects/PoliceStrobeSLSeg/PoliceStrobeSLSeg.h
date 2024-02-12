#ifndef PoliceStrobeSLSeg_h
#define PoliceStrobeSLSeg_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

/*
An effect to strobe a strip to mimic police lights, with some additional options
A strobe is a rapid blinking of light. 
An alternative to the general StrobeSLSeg.h effect.

The base constructor takes two colors, but you can optionally pass in a pattern and palette, or just a palette instead.

There are a few options for randomly choosing what pattern of strobes is used, 
how the strobe colors are chosen, when/if the strobing pauses, and more.

The effect is adapted to work on segment lines for 2D use, each segment line will be a solid color when pulsing.

You can change the effect's settings freely on the fly.

Strobe Modes (pulseMode):
    0: Pulse half the SegmentSetPS in each color (alternating halves and colors), then pulse each color on the whole SegmentSetPS
    1: Pulse half the SegmentSetPS in each color (alternating halves and colors)
    2: Pulse the whole SegmentSetPS in each color (alternating colors)

The strobe of each color will continue for a set number of on/off pulses (creating the strobe effect).

If using a pattern, all the pattern indexes will be cycled through before resetting 
For mode 0, this means all the colors will be strobe'd in halves, then strobe'd on the full strip

segMode (bool):
"segMode" controls if the strobe will pulse using segment lines or whole segments. True does whole segments.

randMode (default 0):
Sets how colors are chosen from the pattern/palette:
    0: Colors will be chosen from the palette in order (not random).
    1: Colors will be chosen completely at random (not using the palette).  
    2: Colors will be chosen randomly from the pattern, but the same color will not be chosen in twice.

Pausing:
    After a strobe cycle is finished (strob'd all colors) there is a user configurable pause time between cycles,
    after which, the strobe will restart. The pause can alternatively be configured to occur after every set of pulses, 
    controlled by `pauseEvery`. The length of the pause is controlled by `pauseTime` (ms).

Backgrounds:
    By default the background is filled in after the end of every pulse set and during a pause
    Both these can be disabled with the fillBg and fillBGOnPause flags respectively
    This causes the last pulse color to persist after the pulse set is done

    This effect is fully compatible with color modes, and the bgColor is a pointer, so you can bind it
    to an external color variable
    (see segDrawUtils::setPixelColor)

Notes:
    When using the dual color constructor option, the colors will be formed into a pattern and palette within the effect
    They are stored as paletteTemp and patternTemp
    So if you want to change the colors, you'll have to change them in paletteTemp.
    paletteUtilsPS::setColor(<your strobe effect name>->paletteTemp, <your new color>,  color index (0 or 1)); 

    When using the constructor with only a palette, a pattern matching the palette will be automatically created
    and stored in patternTemp.

Example calls: 
    PoliceStrobeSLSeg policeStrobe(mainSegments, CRGB::Red, CRGB::Blue, 0, 1, 0, 1, false, 200);
    Does a classic police set of lights
    Blinks each half of the strip for one pulse between red and blue, with 200ms per pulse.
    The background color is blank
    There is no pause between the cycles
    The strobes will be along segment lines (segMode is false)

    PoliceStrobeSLSeg policeStrobe(mainSegments, cybPnkPal_PS, CRGB:Purple, 4, 500, 0, true, 50);
    A more dynamic strobe
    Will strobe all the colors in the cybPnkPal_PS, with 4 pulses at 50ms each.
    The background color is purple
    Strobe mode 0 is used, so the strobe will alternate between strobing halves of the strip and the whole strip
    There is a 500ms pause between cycles 
    The strobes will be along whole segments (segMode is true)

    uint8_t pattern_arr = {0, 2, 1};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};

    PoliceStrobeSLSeg policeStrobe(mainSegments, pattern, cybPnkPal_PS, CRGB:green, 6, 300, 0, false, 50);
    Will strobe colors from the palette based on the pattern (ie colors 0, 2, and 1 in order), with 6 pulses at 50ms each
    The background color is green
    strobe mode 0 is used, so the strobe will alternate between strobing halves of the strip and the whole strip
    There is a 300ms pause between cycles 
    The strobes will be along segment lines (segMode is false)
 
Constructor Inputs:
    pattern(optional, see constructors) -- Used for making a strobe that follows a specific pattern (using colors from a palette) (see patternPS.h)  
    palette(optional, see constructors) -- Used for making a strobe from a specific palette (using the palette as the pattern)
    colorOne(optional, see constructors) -- Used for making a dual color strobe
    colorTwo(optional, see constructors) -- Used for making a dual color strobe
    bgColor -- The color between strobe pulses. It is a pointer, so it can be tied to an external variable
    numPulses -- The number of pulses per strobe
    pauseTime -- The pause time between strobe cycles (See pause notes in intro)
    pulseMode -- The type of strobe that will be used (see modes above)
    segMode -- Controls if the strobe will pulse using segment lines or whole segments. True does whole segments.
    rate -- The update rate of the strobe (ms)

Other Settings:
    colorMode (default 0) -- sets the color mode for the random pixels (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the background (see segDrawUtils::setPixelColor)
    pauseEvery (default false) -- If true, the effect will pause after every set of pulses, rather than after a whole strobe cycle
    fillBg (default true) -- flag to fill the background after each set of pulses
    fillBGOnPause (default true) -- flag to fill the background during each pause 
    randMode (default 0) -- Sets how colors will be chosen, (see randMode notes in intro)

Functions:
    setPaletteAsPattern() -- Sets the effect pattern to match the current palette
    reset() -- Restarts the effect
    update() -- updates the effect
    
Reference Vars:
    colorNum -- The pattern index of the color currently being pulsed (resets once every step in the pattern has been pulsed)

Flags:
    pause -- set true if a pause is active
*/
class PoliceStrobeSLSeg : public EffectBasePS {
    public:
        //Constructor for a traditional two color strobe
        PoliceStrobeSLSeg(SegmentSetPS &SegSet, CRGB ColorOne, CRGB ColorTwo, CRGB BgColor, uint8_t NumPulses,
                       uint16_t PauseTime, uint8_t PulseMode, bool SegMode, uint16_t Rate);

        //Constructor using both pattern and palette
        PoliceStrobeSLSeg(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, CRGB BgColor,
                       uint8_t NumPulses, uint16_t PauseTime, uint8_t PulseMode, bool SegMode, uint16_t Rate);

        //Constructor for using palette as the pattern
        PoliceStrobeSLSeg(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, uint8_t NumPulses,
                       uint16_t PauseTime, uint8_t PulseMode, bool SegMode, uint16_t Rate);

        ~PoliceStrobeSLSeg();

        bool
            fillBg = true,
            fillBGOnPause = true,
            paused = false,
            pauseEvery = false,
            segMode;

        uint8_t
            randMode = 0,
            pulseMode,
            numPulses,
            colorMode = 0,
            bgColorMode = 0;

        uint16_t
            colorNum = 0;  //pattern index of the color currently being pulsed

        unsigned long
            pauseTime;

        CRGB
            bgColorOrig,
            *bgColor = nullptr;  //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)

        palettePS
            *palette = nullptr,          //the palette used for the strobe colors
            paletteTemp = {nullptr, 0};  //Must init structs w/ pointers set to null for safety

        patternPS
            *pattern = nullptr,             //pattern of strobe colors (taken from the palette)
            patternTemp = {nullptr, 0, 0};  //Must init structs w/ pointers set to null for safety

        void
            reset(),
            setPaletteAsPattern(),
            update(void);

    private:
        unsigned long
            currentTime,
            pauseStartTime,
            prevTime = 0;

        uint8_t
            pulseCount = 1,
            palIndex,
            modeOut;

        uint16_t
            lightStart,
            lightEnd,
            halfLength,
            numLines;

        bool
            pulseBG = false,
            firstHalf = true,
            flashHalf;

        CRGB
            colorOut,
            colorTemp;

        void
            init(CRGB BgColor, SegmentSetPS &SegSet, uint16_t Rate),
            startPause(),
            pickColor();
};

#endif
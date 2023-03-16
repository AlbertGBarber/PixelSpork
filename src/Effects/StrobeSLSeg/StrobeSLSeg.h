#ifndef StrobeSLSeg_h
#define StrobeSLSeg_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

/*
An effect to strobe segment sets in various patterns, while switching between palette colors automatically
A strobe is a rapid blinking of light.

The strobe colors will be taken from a passed in pattern or palette.
There are constuctor options for single or randomly choosen colored strobes
The are a few options for randomly choosing the strobe colors from the palette or pattern.

Not matter the configuration, the strobe follows a color pattern, taking colors from a palette.
You have the option of suppling a specific pattern, or having the effect generate one for you to match a palette.

The effect is adapted for 2D use with multiple segments. 
Using a single segment will keep the effect 1D. (although strobe modes 0, 1, and 4 will do the same thing)

Strobe Modes:
    0: Strobe each segment in the set one at a time, the direction can be set, and it can be set to reverse after each cycle
    1: Strobe every other segment, alternating
    2: Strobe each segment line one at a time, the direction can be set, and it can be set to reverse after each cycle
    3: Strobe every other segment line, alternating
    4: Strobe all segs at once 

You can configure any combination of the modes to fire one after another.
Which modes get used depends on the mode bool values (segEach, segDual, etc, see constructor notes below)
Each mode that is selected will be used after the previous mode has finished (with a pause in between)
The effect will loop, so once the last mode has finished, we'll start again with the first mode
You can change which modes appear on the fly by changing the mode flags (see constructor inputs)
A mode is finished when a full strobe cycle in complete, see below:

The strobe of each segment will continue for a set number of on/off cycles (ie making the strobe)
A full cycle is when each of the segments has been strobed at least one time (gone through a set number of pulses)
and all colors of the pattern have been used at least once!! 

Color options and Strobe cycle behavior (set with newColor flag)
    A new color can be picked from the pattern either for each new set of pulses, or after a full strobe cycle
    The number of strobe cycles will always be enough to go through all the colors in the pattern,
    rounded up to a whole number of cycles
    For example, for a pattern of 3 colors with a segment set having 5 segments, doing strobe mode 0:
        If newColor is set true, 1 full strobe cycle (5 sets of pulses, one for each segment) will be done
        with each segment pulsing a new color from the pattern 
        However if the pattern was length 6, then two full strobe cycles would 
        happen so that all the colors in the pattern showed up

        If newColor is set false, then the number of full strobe cycles is the pattern length (number of colors)
        so that the all the segments are pulsed in each color one time
        ie for 5 colors, and mode 0, all the segments will be pulsed in one color before switching to the next. 
        After all the colors have been pulsed, the strobe cycle is over. 
    
    If you have a pattern of multiple colors, and want to multiple strobe modes, but go through all the colors before switching colors
    It's best to have a single color palette and pattern, and switch the colors depending on totalCycles.

!!The number of cycles is full dependent on the pattern length, even if colors are choosen at random

See the code of setCycleCountMax() to work out how many strobe cycles will happen based on your pattern
cycleCountMax is the value you want.

After a strobe cycle is finished there is a user configurable pause time between cycles
after which, the next mode will start.

Pausing:
    We pause for a set time (ms) after each full strobe cycle (or after every set of pulses of pauseEvery is set)

Backgrounds:
    By default the background is filled in after the end of every pulse set and duing a pause
    Both these can be disabled with the fillBG and fillBGOnPause flags respectively
    This causes the last pulse color to persist after the pulse set is done
    which can be used for some neat effects, like a scifi charging cycle "filling up" segments one at a time

This effect is fully compatible with color modes, and the bgColor is a pointer, so you can bind it
to an external color variable (see segDrawUtils::setPixelColor)

There are quite a lot of extra configuration options to help make more dynamic strobes

Example calls: 
    uint8_t pattern_arr = {0, 1, 4};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr)};
    StrobeSLSeg(ringSegments, pattern, palette1, 0, 4, 0, true, true, false, false, false, 50);
    setNewColorBool(true); (sets newColor flag)
    Will do a set of strobes using palette1 and strobe modes 0 and 1.
    The background is blank, there are 4 pulses per strobe with 50ms between each
    The pause time is 0
    The effect has been set to do a new color for each set of pulses

    StrobeSLSeg(ringSegments, palette1, 0, 4, 0, true, true, false, false, false, 50);
    setNewColorBool(true); (sets newColor flag)
    Will do a set of strobes using palette1 and strobe modes 0 and 1.
    The background is blank, there are 4 pulses per strobe with 50ms between each
    The pause time is 0
    The effect has been set to do a new color for each set of pulses

    StrobeSLSeg(ringSegments, palette1, CRGB::Red, 2, 500, true, false, false, false, true, 100);
    alternate = true;
    fillBG = false;
    fillBGOnPause = false;
    Will do a set of strobes using palette1 and strobe modes 0 and 4.
    The background is red, there are 2 pulses per strobe with 100ms between each
    The pause time is 500
    The effect has been set to alternate segment fill directions after each cycle (this only affects mode 0)
    The total number of strobe cycles will be equal to the palette length (because newColor was not set)
    The background will not be filled after each pulse cycle

    StrobeSLSeg(ringSegments, CRGB(CRGB:Blue), CRGB(CRGB::Red), 5, 0, false, true, false, true, false, 40);
    Will do a set of strobes in blue and strobe modes 1 and 3.
    The background is red, there are 5 pulses per strobe with 40ms between each
    The pause time is 0
    !!If using pre-build FastLED colors you need to pass them as CRGB( *color code* )

    StrobeSLSeg(ringSegments, 4, 0, 3, 0, false, true, false, false, false, 100);
    setNewColorBool(true); (sets newColor flag)
    randMode = 2; 
    Will do a set of strobes in using a palette of 4 random colors and strobe mode 1.
    The background is blank, there are 3 pulses per strobe with 60ms between each
    The pause time is 0
    Since the newColor flag has been set and randMode has been set to 2, 
    a new color from the pattern will be choosen for each pulse cycle
 
Constructor Inputs:
    pattern(optional, see constructors) -- Used to strobe a specific pattern of colors from a palette
    palette(optional, see constructors) -- Used for making a strobe from a specific palette (using the palette as the pattern)
    color(optional, see constructors) -- Used for making a single color strobe
    numColors (optional, see constructors) -- Used for making a strobe from a randomply generated palette of length numColors
    bgColor -- The color between strobe pulses. It is a pointer, so it can be tied to an external variable
    numPulses -- The number of pulses per strobe
    pauseTime -- The pause time between strobe cycles
    segEach -- If true, strobe mode 0 will be run
    segDual -- If true, strobe mode 1 will be run
    segLine -- If true, strobe mode 2 will be run
    segLineDual -- If true, strobe mode 3 will be run
    segAll -- If true, strobe mode 4 will be run
    rate -- The update rate (ms)

Functions:
    reset() -- Restarts the strobe using the first mode
    setPaletteAsPattern() -- Sets the effect pattern to match the current palette
    setNewColorBool(newColorBool) -- Changes the newColor flag
                                     The newColor flag determines if a new color is choosen every set of pulses
                                     or after every strobe cycle
    setPattern(&newPattern) -- Changes the pattern, also re-caculates the number of strobe cycles to do
    setPulseMode() -- Advances the pulse mode to the next mode (only call this if you manually want to change the mode)
    setCycleCountMax() -- Re-caculates how many strobe cycles to do based on the palette length (only need to call manually if you're doing something funky)                                                     
    update() -- updates the effect

Other Settings:
    colorMode (default 0) -- sets the color mode for the random pixels (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the background (see segDrawUtils::setPixelColor)
    fillBG (default true) -- flag to fill the background after each set of pulses
    fillBGOnPause (default true) -- flag to fill the background during each pause
    pauseEvery (default false) -- pause after every set of pulses rather than after every full strobe cycle
    direct (default true) -- The direction of the pulses for modes 0 and 2 (true starts at segment 0)
    alternate (default false) -- Set this to alternate the direction of pulses after each strobe cycle for modes 0 and 2
    randMode (default 0) -- Sets how colors are choosen from the palette
                        -- 0: Colors will be choosen from the palette in order (not random)
                        -- 1: Colors will be choosen completly at random (not using the palette)
                        -- 2: Colors will be choosen randomly from the palette, same color will not be choosen in a row

Reference Vars:
    newColor (default false) -- see Color options notes above, set using setNewColorBool()
    colorNum -- The pattern index of the color currently being pulsed
    pulseMode -- The current pulse mode, set using setPulseMode()
    totalCycles -- How many total strobe cycles we've been through.
                   Each sequential strobe mode finished is one strobe cycle.
                   Doesn't reset automatically, only if reset() is called.
Flags:
    paused -- set true if a pause is active

*/
class StrobeSLSeg : public EffectBasePS {
    public:

        //constructor for pattern and palette ver
        StrobeSLSeg(SegmentSet &SegmentSet, patternPS &pattern, palettePS &Palette, CRGB BgColor, uint8_t NumPulses, uint16_t PauseTime, 
                    bool SegEach, bool SegDual, bool SegLine, bool SegLineDual, bool SegAll, uint16_t Rate);

        //constructor for palette ver
        StrobeSLSeg(SegmentSet &SegmentSet, palettePS &Palette, CRGB BgColor, uint8_t NumPulses, uint16_t PauseTime, 
                    bool SegEach, bool SegDual, bool SegLine, bool SegLineDual, bool SegAll, uint16_t Rate);

        //constructor for single color ver
        //!!If using pre-build FastLED colors you need to pass them as CRGB( *color code* )
        StrobeSLSeg(SegmentSet &SegmentSet, CRGB Color, CRGB BgColor, uint8_t NumPulses, uint16_t PauseTime, 
                    bool SegEach, bool SegDual, bool SegLine, bool SegLineDual, bool SegAll, uint16_t Rate);
        
        //constructor for randomly generate palette ver
        StrobeSLSeg(SegmentSet &SegmentSet, uint8_t numColors, CRGB BgColor, uint8_t NumPulses, uint16_t PauseTime, 
                    bool SegEach, bool SegDual, bool SegLine, bool SegLineDual, bool SegAll, uint16_t Rate);

        ~StrobeSLSeg();  

        SegmentSet 
            &segmentSet; 
        
        bool
            fillBG = true, //flag to fill the background after each set of pulses
            fillBGOnPause = true, //flag to fill the background during each pause
            paused = false, //if a pause is active
            pauseEvery = false, 
            newColor = false, //reference only, set using setNewColorBool()
            direct = true,
            alternate = false,
            segEach,
            segDual,
            segLineDual,
            segLine,
            segAll;
        
        int8_t
            pulseMode = -1; //current pulse mode, for reference only

        uint8_t 
            colorNum = 0, //palette index of the color currently being pulsed
            randMode = 0,
            numPulses, 
            colorMode = 0,
            bgColorMode = 0;
        
        uint16_t
            pauseTime,
            totalCycles = 0;
        
        palettePS
            paletteTemp, //palette used for auto generate palettes
            *palette = nullptr; //the palette used for the strobe colors
        
        patternPS
            patternTemp,
            *pattern = nullptr;

        CRGB 
            bgColorOrig,
            *bgColor = nullptr; //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)
        
        void 
            reset(),
            setPulseMode(),
            setCycleCountMax(),
            setPaletteAsPattern(),
            setPattern(patternPS &newPattern),
            setNewColorBool(bool newColorBool),
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
            nextSeg,
            numSegs,    
            cycleLoopLimit,
            numLines,
            cycleNum = 0,
            cycleCountMax;
        
        bool
            boolTemp,
            pulseBG = false,
            firstHalf = true;
        
        CRGB
            colorOut,
            colorTemp;
        
        void
            startPause(),
            init(CRGB BgColor, uint16_t Rate),
            pickColor();
};

#endif
#ifndef StrobeSLSeg_h
#define StrobeSLSeg_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

/*
An effect to strobe segment sets in various patterns, while switching between palette colors automatically. 
You have a lot of options for customizing the strobe cycle, especially for 2D segment sets, 
and can set the effect to alternate through multiple strobe types. No matter the configuration, 
the strobe follows a color pattern, taking colors from a palette. 
You have the option of supplying a specific pattern, or having the effect generate one for you to match a palette. 
There are also a few options for randomizing the colors. 

The effect is adapted for 2D use with multiple segments. 
Different strobe modes will draw the strobes across whole segments or segment lines.

There's quite a lengthy set of notes on effect settings below, 
but it may be easier to understand just by trying the differed strobe and strobe color modes out.

Supports Color Modes for both the main and background colors.

For a more specific, but slightly easier to use strobe effect, see PoliceStrobeSLSeg.h

Inputs Guide & Notes:

    Strobe Modes:
        Strobe modes control how strobes are drawn. You can have multiple modes active at once, 
        they will be drawn one after another. 

            0 -- Strobe each segment in the set one at a time with a controllable direction (see Strobe Mode Directions below).
            1 -- Strobe each segment at a frequency of "sMode1Feq", alternating over time. 
                 (see Frequencies below for more info)
            2 -- Strobe each segment line one at a time with a controllable direction (see Strobe Mode Directions below).
            3 -- Strobe each segment line at a frequency of "sMode3Feq", alternating over time. (like mode 1, but for segment lines) 
            4 -- Strobe all segments at once.

        You can configure any combination of the modes to fire one after another. 
        Each mode is controlled by a specific boolean setting (`segEach`, `segDual`, etc, see Constructor Inputs below).
        If the mode's boolean is true, the mode will be used. Each active mode will trigger after the 
        previous mode has finished (with a pause in between). 
        The effect will loop, so once the last mode has finished, the effect will start again with the first mode. 
        You can change which modes appear on the fly by changing the mode flags.

    Frequencies:
        For modes 1 and 3, you have an additional frequency setting that controls the spacing of the pulsed lines/segments.
        For example, if the frequency is 2, every other line/segment will be pulsed, with the strobe'd set swapping
        after a strobe cycle, so that one half of segment set is strob'd and then the other. On the other hand, if 
        the frequency is 3, then every third line will be pulsed, swapping to the next third, and then the final third
        to finish the full strobe cycle. It's much easier to understand in person, so just try it out!

        By default both sMode1Feq and sMode3Feq are set to 2, so every other segment/line will be strobe'd.

        Note that if you change either frequency mid mode 1 or 3, you must call reset().

    Color Options and Strobe Cycle Behavior:
        The strobe of each segment will continue for a set number of on/off cycle/pulses (ie creating the strobe). 
        How the strobe colors are set, and how long each strobe cycle is depends on the `strobeColorMode` setting. 
        A full cycle is complete when each of the segments or segment lines has been strob'd at least one time, 
        while an individual strobe is one set of pulses only.
        The number of full cycles before a strobe mode is finished depends on the `strobeColorMode`.

        strobeColorMode (uint8_t):
            I'll describe each of the strobe color modes below, and give an example. 
            For all the examples, suppose we have a pattern of 3 colors with a segment set having 5 segments, 
            and we want to use strobe mode 0. 

            You can change the `strobeColorMode` during runtime using `setStrobeColorMode()`.

            0 -- Each set of pulses will be a different color, with the strobe mode looping until all colors 
                 have been done. For the example above, 1 full strobe cycle (5 sets of pulses, one for each segment) 
                 will be done with each segment pulsing a new color from the pattern. Because the pattern is only 
                 3 colors long, each segment is only strob'd once. However if the pattern was length 6, then two 
                 full strobe cycles would occur, so that all the colors in the pattern were used.

            1 -- The strobe goes through a full cycle for each pattern color. In other words the number of 
                 full strobe cycles is the pattern length, so that the all the segments are pulsed in each color one
                 time. For the example above, the segments will be go through 3 full cycles, with the segments pulsing 
                 one color at a time. 
                
            2 -- The strobe will do exactly 1 full cycle using 1 color from the pattern. 
                 For the example above, the segments will be pulsed in 1 color for only 1 full cycle, 
                 like a slice of color mode 1. This mode is most useful when using multiple strobe modes, so each 
                 will use a different color from the pattern.

        `randMode`'s (uint8_t) (default 0):
            Sets how colors are chosen from the pattern:
            0 -- Colors will be chosen from the pattern in order (not random).
            1 -- Colors will be chosen completely at random (not using the pattern or palette).
            2 -- Colors will be chosen randomly from the pattern, same color will not be chosen in a row.
            3 -- Colors will be chosen randomly from the pattern, allowing repeats.

    Note that the number of full cycles is dependent on the pattern length, even if colors are chosen at random, 
    see `strobeColorMode` above.

    You can also check `setCycleCountMax()` in the code to determine how many strobe cycles there will based 
    on your pattern, strobe mode, and strobe color mode.

    Strobe Mode Directions and Some Quirks:
        All strobe modes, except "4", have a direction, "direct". For modes 1 and 3 it controls what group of segments/lines 
        is strobe'd first. For modes 0 and 2 it sets the start point of the strobe to either the first/last segment/line.
        This setting is shared across all modes (I didn't want a direction setting for each mode!). If you want different
        modes to have different settings, you use the "strobeMode" reference var to track the current mode and set the 
        direction accordingly.

        By default direct is set to true. (First segment/line to last).

        The direction can also be set to alternate between full strobe cycles using "alternate". By default, the direction
        will flip after each full cycle, but this can lead to some quirks when working with multiple strobe modes 
        depending on the strobe color mode.
        For example, with two modes, and color mode 2, each mode will flip the direction, so overall, 
        their directions will remain static (but reversed from one another). 
        To help combat this, I've allowed you to adjust of often the direction is flipped using "altFreq" (defaulted to 1). 
        The default of 1 means the direction is always flipped, but 2 would flip it every other cycle, 3, every third, etc. 

        Note that the direction is only flipped after a full strobe cycle (see note in Color Options above). This
        can lead to some complex interactions depending on the color mode and how many full cycles your strobe will do.
        Rather than trying to predict the exact output, it's probably easier just to test the effect with various 
        altFreq values. Understand that the system is not perfect, and there are some flipping combinations that are not 
        possible (such as flipping 2 out of 3 modes only), but it's simpler than having separate settings for each mode.
        (You can always manually track the strobe mode and set the direction manually).

    Pausing:
        The effect can be set to pause for a set time (`pauseTime`) between full strobe cycles or individual strobes, 
        controlled by `pauseEvery`.

            * If `pauseEvery` is true, the effect will pause after each individual strobe cycle. 
              For example, if our full strobe cycle consists of 3 individual strobes, one for each segment in set, 
              the effect will pause after each strobe. 

            * If `pauseEvery` is false (default), the effect will pause after each full strobe cycle. 

    Background Color Setting: 
        By default the background is filled after the end of every strobe cycle, and during a pause, 
        to clear the final strobe pulse. You can control this behavior with the `fillBg` and `fillBGOnPause` 
        flags respectively (both default true). 

        * `fillBg` -- If true, the background will be filled after each set of strobe pulses, 
                      and at the end of a full strobe cycle.
        
        * `fillBGOnPause` -- If true, the background will be filled whenever the effect pauses (see Pausing above).

        Disabling these causes the last pulse color to persist after a strobe is done, which can be 
        used for some neat effects, like a sci-fi charging cycle "filling up" segments one at a time.

        Note that the effect always enters a pause, even if the pause time is 0, so if `pauseEvery` is true, 
        `fillBGOnPause` will control the fill after every strobe cycle. 

Example calls: 
    Note that most of these examples work best with a 2D segment set. The second works well in 1D.

    uint8_t pattern_arr = {0, 2, 1};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};
    StrobeSLSeg strobe(mainSegments, pattern, cybPnkPal_PS, 0, 4, 0, 2, true, true, false, false, false, 50);
    Will do a set of strobes using colors from cybPnkPal_PS palette, following the pattern above.
    The background is blank, there are 4 pulses per strobe with 50ms between each
    The pause time is 0ms
    The effect uses strobe color mode 2, and strobe modes 0 and 1.

    StrobeSLSeg strobe(mainSegments, cybPnkPal_PS, 0, 6, 0, 1, false, false, false, true, true, 30);
    Will do a set of strobes using colors from cybPnkPal_PS palette.
    The background is blank, there are 6 pulses per strobe with 30ms between each
    The pause time is 0ms
    The effect uses strobe color mode 1, and strobe modes 3 and 4.

    StrobeSLSeg strobe(mainSegments, cybPnkPal_PS, CRGB::Red, 2, 500, 0, true, false, false, false, true, 100);
    alternate = true; //put in Arduino Setup()
    altFreq = 2: //put in Arduino Setup()
    fillBg = false; //put in Arduino Setup()
    fillBGOnPause = false; //put in Arduino Setup()
    Will do a set of strobes using colors from cybPnkPal_PS palette.
    The background is red, there are 2 pulses per strobe with 100ms between each
    The pause time is 500ms.
    The effect uses strobe color mode 0, and strobe modes 0 and 4.
    The effect has been set to alternate segment fill directions after each cycle.
    The background will not be filled after each pulse cycle

    StrobeSLSeg strobe(mainSegments, CRGB:Blue, CRGB::Red, 5, 0, 0, false, true, false, true, false, 40);
    Will do a set of strobes in blue.
    The background is red, there are 5 pulses per strobe with 40ms between each
    The pause time is 0ms.
    The strobe modes 1 and 3. 
    (Note that the strobe color mode is excluded, because the result is the same for each mode. It is default to 0)
 
Constructor Inputs:
    pattern(optional, see constructors) -- Used to strobe a specific pattern of colors from a palette
    palette(optional, see constructors) -- Used for making a strobe from a specific palette (using the palette as the pattern)
    color(optional, see constructors) -- Used for making a single color strobe. 
    numColors (optional, see constructors) -- Used for making a strobe from a randomly generated palette of length numColors
    bgColor -- The color between strobe pulses. It is a pointer, so it can be tied to an external variable
    numPulses -- The number of pulses per strobe cycle.
    pauseTime -- The pause time between full strobe cycles 
                 (or individual strobe cycles if pauseEvery is true, see Pausing Notes above)
    strobeColorMode (optional, see constructors) -- (See Color Options in Inputs Guide above).
                                                    Can be changed later using setStrobeColorMode(). 
                                                    Note that it is excluded from the single color constructor 
                                                    (defaulted to 0) because the result is the same for all modes.
    segEach  --    If true, strobe mode 0 will be run.
    segDual  --    If true, strobe mode 1 will be run.
    segLine  --    If true, strobe mode 2 will be run.
    segLineDual -- If true, strobe mode 3 will be run.
    segAll   --    If true, strobe mode 4 will be run.
    rate -- The update rate (ms)

Other Settings:
    colorMode (default 0) -- sets the color mode for the random pixels (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the background (see segDrawUtils::setPixelColor)
    fillBg (default true) -- When true, the segment set will be filled with the background color after each complete set 
                             of strobe pulses. If false, the background will only be filled when strobing 
                             (ie to create the strobe effect). (see Background section in Inputs Guide above).
    fillBGOnPause (default true) -- flag to fill the background during each pause (see Background notes in Inputs Guide)
    pauseEvery (default false) -- When true, the segment set will be filled with the background color when each pause 
                                  starts. If false, the background will not be filled when pausing. 
                                  (see Background section in Inputs Guide above).
    direct (default true) -- The direction of the pulses for strobe modes 0 and 2. When true, the pulses will move 
                             from the first to last segment, the opposite when false. 
                             (See Directions section in Inputs Guide above).
    alternate (default false) -- If true, the direction of pulses for each strobe mode (except 4) will 
                                 alternate after each full strobe cycle. 
                                 (See Directions section in Inputs Guide above).
    altFreq (default 1, min 1) -- Only active if "alternate" is true. Controls the frequency at which the direction is flipped
                                  1 is after every full cycle, 2 is after every other, etc. (See Directions section in Inputs Guide above).
    sMode1Freq (default 2, min 1) -- Sets the segment frequency for strobe mode 1 (defaulted to 2; every other segment).
                                     (See Frequencies in intro). If you change this mid mode, you must reset().
    sMode3Freq (default 2, min 1) -- Sets the segment frequency for strobe mode 3 (defaulted to 2; every other segment).
                                     (See Frequencies in intro). If you change this mid mode, you must reset().
    randMode (default 0) -- Sets how strobe colors are chosen. (See randMode notes in Inputs Guide above).

Functions:
    reset() -- Restarts the strobe using the first mode
    setStrobeColorMode(newMode)  -- Changes the `strobeColorMode` setting. (See Color Options in Inputs Guide above). 
                                    Also restarts the current strobe mode.
    setPaletteAsPattern() -- Sets the effect pattern to match the current effect palette. Also restarts the current strobe mode.
    setPattern(&newPattern) -- Changes the effect's color pattern. Also restarts the current strobe mode.
    setStrobeMode() -- Advances the pulse mode to the next mode (only call this if you manually want to change the mode)
    setCycleCountMax() -- Re-calculates how many strobe cycles to do based on the pattern length (only need to call manually if you're doing something funky)                                                     
    update() -- Updates the effect.

Reference Vars:
    strobeColorMode -- (See Color Options in Inputs Guide above). Set using setStrobeColorMode().
    colorNum -- The pattern index of the color currently being pulsed.
    strobeMode -- The current strobe mode, automatically advanced using setStrobeMode().
    totalCycles -- How many full strobe cycles we've been through.
                   Resets only when `reset()` is called.
                   
Flags:
    paused -- Set true if a pause is active.

*/
class StrobeSLSeg : public EffectBasePS {
    public:

        //Constructor for pattern and palette ver
        StrobeSLSeg(SegmentSetPS &SegSet, patternPS &pattern, palettePS &Palette, CRGB BgColor, uint8_t NumPulses,
                    uint16_t PauseTime, uint8_t StrobeColorMode, 
                    bool SegEach, bool SegDual, bool SegLine, bool SegLineDual, bool SegAll, 
                    uint16_t Rate);

        //Constructor for palette ver
        StrobeSLSeg(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, uint8_t NumPulses, uint16_t PauseTime,
                    uint8_t StrobeColorMode, bool SegEach, bool SegDual, bool SegLine, bool SegLineDual, bool SegAll, 
                    uint16_t Rate);

        //Constructor for single color ver
        StrobeSLSeg(SegmentSetPS &SegSet, CRGB Color, CRGB BgColor, uint8_t NumPulses, uint16_t PauseTime,
                    bool SegEach, bool SegDual, bool SegLine, bool SegLineDual, bool SegAll, 
                    uint16_t Rate);

        //Constructor for randomly generate palette ver
        StrobeSLSeg(SegmentSetPS &SegSet, uint8_t numColors, CRGB BgColor, uint8_t NumPulses, uint16_t PauseTime, 
                    uint8_t StrobeColorMode, bool SegEach, bool SegDual, bool SegLine, bool SegLineDual, bool SegAll, 
                    uint16_t Rate);

        ~StrobeSLSeg();

        bool
            fillBg = true,         //flag to fill the background after each set of pulses
            fillBGOnPause = true,  //flag to fill the background during each pause
            paused = false,        //if a pause is active
            pauseEvery = false,
            direct = true,
            alternate = false,
            segEach,
            segDual,
            segLineDual,
            segLine,
            segAll;

        int8_t
            strobeMode = -1;  //current pulse mode, for reference only

        uint8_t
            numPulses,
            colorMode = 0,
            bgColorMode = 0,
            randMode = 0,
            strobeColorMode, //reference only, set using setNewColorBool()
            sMode1Freq = 2,  //Sets the segment frequency for strobe mode 1 (defaulted to 2; every other segment)
            sMode3Freq = 2,  //Sets the segment frequency for strobe mode 3 (defaulted to 2; every other segment line)
            altFreq = 1;

        uint16_t
            pauseTime,
            colorNum = 0,  //pattern index of the color currently being pulsed
            totalCycles = 0;

        palettePS
            *palette = nullptr,          //the palette used for the strobe colors
            paletteTemp = {nullptr, 0};  //palette used for auto generate palettes, init to null for safety

        patternPS
            patternTemp = {nullptr, 0, 0},  //Must init structs w/ pointers set to null for safety
            *pattern = &patternTemp;        //the strobe color pattern (using colors from the palette) 
                                            //(need to bind this to prevent a crash during init)

        CRGB
            bgColorOrig,
            *bgColor = nullptr;  //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)

        void
            reset(),
            setStrobeMode(),
            setCycleCountMax(),
            setPaletteAsPattern(),
            setPattern(patternPS &newPattern),
            setStrobeColorMode(uint8_t newMode),
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
            numLines,
            cycleNum = 0,
            cycleLoopLimit,
            cycleCountMax;

        bool
            atCycleLim,
            pulseBG = false;

        CRGB
            colorOut,
            colorTemp;

        void
            startPause(),
            init(CRGB BgColor, SegmentSetPS &SegSet, uint16_t Rate),
            pickColor();
};

#endif
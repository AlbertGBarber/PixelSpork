#ifndef GlimmerSL_h
#define GlimmerSL_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
/*
The goal of the effect is to fade a random set of leds in and out, between the glimmerColor and the bgColor
(note that the bgColor is the dominant color in the effect)

Each led fades a random amount towards glimmerColor, but all leds fade together (their fades finish at the same time)
the fade amounts are restricted between fadeMin and fadeMax.

The effect can be set so that one or two sets of leds are fading in/out at any one time
With one set, the pixels are faded in then out before choosing a new set,
while with two sets, one set is faded in while the other is faded out.

TwoPixelSets setting:
    True: Two sets of pixels will be fading at once, with one set fading in while the other is fading out.
    False: Only one set of pixels will be faded at a time, with the set being fully faded in then out before a new set is picked.


Both glimmerColor and bgColor are pointers, so you can bind them to external color vars.
By default, they are bound to glimColorOrig and bgColorOrig during construction.

The effect is adapted to work on segment lines for 2D use, but has a 1D operating mode controlled
by the bool, `lineMode`.

lineMode (bool):
    true -- The effect will use whole segment lines for fading.
    false -- The effect will use individual pixels for fading, 
             which is useful for using multi-segment color modes, while still only glimmering individual pixels.

Supports colorModes for both the glimmer and background colors.

You can change most settings on the fly, with the exception to the number of fading pixels,
`numGlims`, and the `twoPixelSets` setting.

Note that the effect needs one uint16_t array and one uint8_t array in order to work
With one set of leds, the array lengths are the number of fading leds
With two sets, the length is doubled.

Example calls: 
    GlimmerSL glimmer(mainSegments, 8, CRGB::Blue, 0, true, 10, 50);
    Will choose 8 pixels to cycle to/from off to CRGB::Blue with a blank background
    Two sets of pixels will be faded in/out at one time, going through 10 fade steps, with 50ms between each step

    GlimmerSL glimmer(mainSegments, 10, CRGB::Blue, CRGB::Red, false, 20, 100, 255, 60);
    Will choose 10 pixels to to cycle to/from CRGB::Red to CRGB::Blue (red is the background)
    One set of pixels will be faded in then out before a new set is chosen
    The range of the fades will be between 100 and 255, with 20 fade steps, with 60ms between each step

Constructor Inputs:
    numGlims -- The number of random pixels chosen for fading for each cycle, can be set later using setNumGlims().
    glimmerColor -- The color the pixels will be faded towards
                    Note that the effect's glimmerColor is a pointer, by default it's bound to glimColorOrig
    bgColor -- The color of the background, this is the color that not-fading pixels will be
    twoPixelSets -- Sets if one or two sets of pixels will be fading in/out at once (true for two sets)
                    Can be set later using setTwoSets().
    fadeMin (optional, max 255, default 50) -- The minimum amount that a pixel will fade by
    fadeMax (optional, max 255, default 255) -- The maximum amount that a pixel will fade by
    fadeSteps -- The number of steps it takes a pixel to fade (one step every update cycle)
    rate -- The update rate (ms)

Other Settings:
    colorMode (default 0) -- sets the color mode for the random pixels (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the background (see segDrawUtils::setPixelColor)
    lineMode (default true) -- If false, sets each glimmer to use a individual pixel, rather than a whole line
                               Only really useful if you want multi-segment color modes with point glimmers
    fillBg (default false) -- sets the background to be redrawn every cycle, useful for bgColorModes that are dynamic

Functions:
    setNumGlims(newNumGlims) -- Changes the number of pixels fading,
                                If the new number of pixels is different than the current numGlims,
                                the glimmer arrays will be re-created, and the effect will be reset
                                (otherwise, nothing will happen)
    setTwoSets(newSetting) -- Changes the TwoPixelSets variable, if different, it will reset the current fade cycle
    reset() -- Resets the effect, restarting it. Fills in the background to clear any existing pixels.
    update() -- updates the effect
    
Reference Vars:
    numGlims -- The number of glimmering particles, set using setNumGlims()
    twoPixelSets -- See constructor inputs, set with setTwoSets(newSetting).
    step -- Tracks the fade step we're on, max value is fadeSteps, is reset to 0 once a fade is finished

Flags: 
    fadeIn -- Tracks if the current set of pixels is fading in or not, for two sets, this variable will be toggled mid-cycle.

*/
class GlimmerSL : public EffectBasePS {
    public:
        //Constructor using default fade in and out values
        GlimmerSL(SegmentSetPS &SegSet, uint16_t NumGlims, CRGB GlimmerColor, CRGB BgColor,
                  bool TwoPixelSets, uint8_t FadeSteps, uint16_t Rate);

        //Constructor for setting maximum fade in and out values
        GlimmerSL(SegmentSetPS &SegSet, uint16_t NumGlims, CRGB GlimmerColor, CRGB BgColor,
                  bool TwoPixelSets, uint8_t FadeSteps, uint8_t FadeMin, uint8_t FadeMax, uint16_t Rate);

        ~GlimmerSL();

        uint8_t
            colorMode = 0,
            bgColorMode = 0,
            fadeSteps,
            step = 0,
            fadeMin = 50,
            fadeMax = 255,
            *totFadeSteps = nullptr;

        uint16_t
            *fadePixelLocs = nullptr,
            numGlims;  //For reference only!, Set using setNumGlims()

        bool
            fadeIn,
            lineMode = true,
            fillBg = false,
            twoPixelSets;

        CRGB
            glimColorOrig,
            *glimmerColor = nullptr,  //glimmerColor is a pointer so it can be tied to an external variable if needed (such as a palette color)
            bgColorOrig,
            *bgColor = nullptr;  //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)

        void
            reset(void),
            setNumGlims(uint16_t newNumGlims),
            setTwoSets(bool newSetting),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0;

        uint16_t
            glimArrLen,
            glimArrLenMax = 0,  //used for tracking the memory size of the glimmer arrays
            numLines,
            numSegs,
            pixelNum;

        bool
            firstFade = false;

        pixelInfoPS
            pixelInfo = {0, 0, 0, 0};

        CRGB
            startColor,
            targetColor,
            fadeColor,
            getFadeColor(uint16_t glimNum);

        void
            init(CRGB GlimmerColor, CRGB BgColor, SegmentSetPS &SegSet, uint16_t Rate),
            setupPixelArray(),
            fillPixelArray(),
            advancePixelArray();
};

#endif
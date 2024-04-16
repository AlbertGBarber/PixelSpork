#ifndef GlimmerSL_h
#define GlimmerSL_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

/*
This effect fades a random group of pixels in and out between a color and the background, 
choosing different pixels for each fade. 
This is similar to the Twinkle series of effects, but use groups of pixels instead of fading them individually. 
The colors can be picked randomly from a palette, or be set to a single color. 

The amount each pixel fades is randomized to between `fadeMin` and `fadeMax` (defaulted to 30 and 255), 
but all pixels fade together, so that their fades end at the same time. 
This helps produce a more varied, shimmery look, although the overall effect is quite subtle. 

The effect is adapted for 2D use, with each "glimmer" being extended along segment lines. 
The effect can be made 1D using `lineMode` (see below).

Supports Color Modes for both the glimmer and background colors.

You can change most settings on the fly, with the exception to the number of fading pixels, `numGlims`, 
and the `twoPixelSets` setting, which must be changed via function calls.

Note that the effect creates one uint16_t array and two uint8_t arrays. 
These arrays are allocated dynamically, so, to avoid memory fragmentation, when you create the effect, 
you should set `numGlims` to the maximum value you expect to use. 
See https://github.com/AlbertGBarber/PixelSpork/wiki/Effects-Advanced#managing-dynamic-memory-and-fragmentation for more details. 
Note that when fading two sets of pixels (see below), the lengths of the arrays will be `numGlims` * 2; 
one length for each set of pixels.

    Two Pixel Sets:
        The effect can be set so that one or two sets of pixels are fading in/out at any one time. 
        With one set, the pixels are faded in then out before choosing a new set, 
        while with two sets, one set is faded in while the other is faded out.

        This is controlled by the `twoPixelSets` setting:
            * true -- Two sets of pixels will be fading at once, with one set fading in while the other is fading out.
            * false -- Only one set of pixels will be faded at a time, with the set being fully 
                       faded in then out before a new set is picked.

    Line Mode: 
        The `lineMode` (bool) setting controls if the glimmering pixels are draws in 2D along segment lines, 
        or in 1D as individual pixels.

        * true (default) -- The effect will use whole segment lines for fading.
        * false -- The effect will use individual pixels for fading, which is useful for using multi-segment color modes, 
                   while still only glimmering individual pixels.

Example calls: 
    GlimmerSL glimmer(mainSegments, 8, cybPnkPal_PS, 0, true, 10, 50);
    Will choose 8 pixels to cycle to/from colors from the cybPnkPal_PS palette
    with a blank background.
    Two sets of pixels will be faded in/out at one time, 
    going through 10 fade steps, with 50ms between each step.

    GlimmerSL glimmer(mainSegments, 10, CRGB::Blue, CRGB::Red, false, 20, 20, 150, 60);
    Will choose 10 pixels to to cycle to/from CRGB::Red to CRGB::Blue (red is the background).
    One set of pixels will be faded in then out before a new set is chosen.
    The range of the fades will be between 20 and 150, 
    with 20 fade steps, with 60ms between each step.

Constructor Inputs:
    numGlims -- The number of random pixels chosen for fading for each cycle, can be set later using setNumGlims().
    palette (optional, see constructors) -- The set of colors the effect will use. 
                                            The colors of the fading pixels will be picked randomly from the palette, 
                                            changing after each fade. 
                                            It is a pointer.
    Color -- A single color the pixels will be faded towards. 
             The color will be placed into the effect's local palette, `paletteTemp` for use.
    bgColor -- The color of the background, this is the color that not-fading pixels will be
    twoPixelSets -- Controls if one or two sets of pixels will be fading in/out at once (see notes in Intro), 
                    Can be set later using setTwoSets().
    fadeMin (optional, max 255, default 30) -- The minimum amount that a pixel will fade by. 
                                               Higher means that pixels will be faded further to the glimmer colors. 
    fadeMax (optional, max 255, default 255) -- The maximum amount that a pixel will fade by.
                                                Higher means that pixels will be faded further to the glimmer colors. 
    fadeSteps -- The number of steps to fade a pixel (one step every update cycle).
    rate -- The update rate (ms)

Other Settings:
    colorMode (default 0) -- The color mode for the glimmering pixels (see segDrawUtils::setPixelColor).
    bgColorMode (default 0) -- The color mode for the background pixels (see segDrawUtils::setPixelColor).
    lineMode (default true) -- If true, glimmers will be drawn along segment lines (2D), 
                               if false, glimmers will be drawn on individual pixels (1D), 
                               (see notes in Intro).
    fillBg (default false) -- sets the background to be redrawn every cycle, useful for bgColorModes that are dynamic

Functions:
    setNumGlims(newNumGlims) -- Changes the number of pixels fading,
                                If the new number of pixels is different than the current numGlims,
                                the glimmer arrays will be re-created, and the effect will be reset
                                (otherwise, nothing will happen)
    setTwoSets(newSetting) -- Changes the TwoPixelSets variable, if different, it will reset the current fade cycle
    reset() -- Resets the effect, restarting it. Fills in the background to clear any glimmering pixels.
    update() -- updates the effect
    
Reference Vars:
    numGlims -- The number of glimmering particles, set using setNumGlims()
    twoPixelSets -- See constructor inputs, set with setTwoSets(newSetting).
    step -- Tracks the fade step we're on, max value is fadeSteps, is reset to 0 once a fade is finished

Flags: 
    fadeIn -- Is true when the glimmering pixels are fading in, false when they are fading out. 
              Not useful when fading two sets because it is toggled mid-update.

*/
class GlimmerSL : public EffectBasePS {
    public:
        //Constructor with palette, using default fade in and out values
        GlimmerSL(SegmentSetPS &SegSet, uint16_t NumGlims, palettePS &Palette, CRGB BgColor,
                   bool TwoPixelSets, uint8_t FadeSteps, uint16_t Rate);

        //Constructor with palette and setting maximum fade in and out values
        GlimmerSL(SegmentSetPS &SegSet, uint16_t NumGlims, palettePS &Palette, CRGB BgColor,
                  bool TwoPixelSets, uint8_t FadeSteps, uint8_t FadeMin, uint8_t FadeMax, uint16_t Rate);

        //Constructor with single color, using default fade in and out values
        GlimmerSL(SegmentSetPS &SegSet, uint16_t NumGlims, CRGB Color, CRGB BgColor,
                  bool TwoPixelSets, uint8_t FadeSteps, uint16_t Rate);

        //Constructor with single color and setting maximum fade in and out values
        GlimmerSL(SegmentSetPS &SegSet, uint16_t NumGlims, CRGB Color, CRGB BgColor,
                  bool TwoPixelSets, uint8_t FadeSteps, uint8_t FadeMin, uint8_t FadeMax, uint16_t Rate);

        ~GlimmerSL();

        uint8_t
            colorMode = 0,
            bgColorMode = 0,
            fadeSteps,
            step = 0,
            fadeMin = 30,
            fadeMax = 255;

        uint16_t
            numGlims;  //For reference only!, Set using setNumGlims()

        bool
            fadeIn,
            lineMode = true,
            fillBg = false,
            twoPixelSets;

        CRGB
            bgColorOrig,
            *bgColor = nullptr;  //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)

        palettePS
            *palette,
            paletteTemp;

        //Glimmer pixel tracking arrays
        //These are public, but you shouldn't ever need to access them!
        uint8_t
            *glimColorArr = nullptr, //palette color indexes of the glimmering pixels
            *totFadeSteps = nullptr; //The fade steps for each glimmering pixel
        uint16_t
            *fadePixelLocs = nullptr; //locations of glimmering pixels

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
            init(CRGB BgColor, SegmentSetPS &SegSet, uint16_t Rate),
            setupPixelArray(),
            fillPixelArray(),
            advancePixelArray();
};

#endif
#ifndef GlimmerSL_h
#define GlimmerSL_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
/*
The goal of the effect is to fade a random set of leds in and out, between the glimmerColor and the bgColor
(note that the bgColor is the dominant color in the effect)
each led fades a random amount towards glimmerColor, but all leds fade together (their fades finish at the same time)
the fade amounts are restricted between fadeMin and fadeMax
The effect can be set so that one or two sets of leds are fading in/out at any one time
With one set, it is faded in then out be for choosing a new set of leds
while with two sets, one set is being faded in while the other is being faded out
Note that the effect needs one uint16_t and one uint8_t array in order to work
With one set of leds, the array lengths are the number of fading leds
With two sets, the length is doubled

Both glimmerColor and bgColor are pointers, so you can bind them to external color vars.
By default, they are bound to colorOrig and bgColorOrig during construction.

The effect is adapted to work on segment lines for 2D use, but you can keep it 1D by
passing in a SegmentSet with only one segment containing the whole strip.
Or you can set lineMode to false, which makes each glimmer an individual pixel 
(useful for using multi-segment color modes, while still only glimmering individual pixels)

TwoPixelSets setting:
    True: Two sets of leds with be driven at once, with one set fading in while the other is fading out
    False: Only one set of leds with be driven at a time, with the set being fully faded in then out before a new set is picked

Example calls: 
    GlimmerSL glimmer(mainSegments, 8, CRGB::Blue, 0, true, 10, 50);
    Will choose 8 pixels to cycle to/from off to CRGB::Blue 
    Two sets of pixels will be faded in/out at one time, going through 10 fade steps, with 50ms between each step

    GlimmerSL glimmer(mainSegments, 10, CRGB::Blue, CRGB::Red, false, 20, 100, 255, 60);
    Will choose 10 pixels to to cycle to/from CRGB::Red to CRGB::Blue 
    One set of pixels will be faded in then out before a new set is chosen
    The range of the fades will be between 100 and 255, with 20 fade steps, with 60ms between each step

Constructor Inputs:
    numGlims -- The amount of random pixels chosen for fading
    glimmerColor -- The color the pixels will be faded towards
                    Note that the effect's glimmerColor is a pointer, by default it's bound to colorOrig
    BgColor -- The color of the background, this is the color that not-fading pixels will be
    twoPixelSets -- (bool) sets if one or two sets of pixels will be fading in/out at once (true for two sets)
    fadeMin (optional, max 255, default 50) -- The minimum amount that a pixel will fade by
    fadeMax (optional, max 255, default 255) -- The maximum amount that a pixel will fade by
    fadeSteps -- The number of steps it takes a pixel to fade (one step every update cycle)
    rate -- The update rate (ms)

Functions:
    setNumGlims(newNumGlims) -- Changes the number of pixels fading,
                                If the new number of pixels is different than the current numGlims,
                                the glimmer arrays will be re-created, and the effect will be reset
                                (otherwise, nothing will happen)
    setTwoSets(newSetting) -- Changes the TwoPixelSets variable, if different, it will reset the current fade cycle
    setupPixelArray() and fillPixelArray() -- Both generate a new set of pixels for fading, you should not need to call these
    update() -- updates the effect

Other Settings:
    colorMode (default 0) -- sets the color mode for the random pixels (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the background (see segDrawUtils::setPixelColor)
    lineMode (default true) -- If false, sets each glimmer to use a individual pixel, rather than a whole line
                               Only really useful if you want multi-segment color modes with point glimmers
    fillBG (default false) -- sets the background to be redrawn every cycle, useful for bgColorModes that are dynamic
    fadeMin  and fadeMax -- (see Constructor inputs)

Flags: 
    fadeIn -- Tracks is the set of leds is fading in or not, for two sets, this variable will be toggled mid-set

Reference Vars:
    numGlims -- The number of glimmering particles, set using setNumGlims()
    step -- Tracks the fade step we're on, max value is fadeSteps, is reset to 0 once a fade is finished

Notes:
    Requires uint16_t array and uint8_t arrays of length up to 2*numGlims to work, make sure you have the memory for them!
    Likewise, changing the number of glimmers, or setting the effect to use two sets of glimmers, requires
    the arrays to be re-created, so you need to call setNumGlims(newNumGlims) or setTwoSets(newSetting)
*/
class GlimmerSL : public EffectBasePS {
    public:
        //Constructor using default fade in and out values
        GlimmerSL(SegmentSet &SegSet, uint16_t NumGlims, CRGB GlimmerColor, CRGB BgColor, bool TwoPixelSets, uint8_t FadeSteps, uint16_t Rate);  
        
        //Constructor for setting maximum fade in and out values
        GlimmerSL(SegmentSet &SegSet, uint16_t NumGlims, CRGB GlimmerColor, CRGB BgColor, bool TwoPixelSets, uint8_t FadeSteps, uint8_t FadeMin, uint8_t FadeMax, uint16_t Rate);

        ~GlimmerSL();
        
        SegmentSet 
            &SegSet; 
        
        uint8_t 
            colorMode = 0,
            bgColorMode = 0,
            fadeSteps,
            step = 0,
            fadeMin = 50,
            fadeMax = 255;

        uint16_t
            numGlims; //For reference only!, Set using setNumGlims()
        
        bool 
            fadeIn,
            lineMode = true,
            fillBG = false,
            twoPixelSets;
        
        CRGB 
            colorOrig,
            *glimmerColor = nullptr, //glimmerColor is a pointer so it can be tied to an external variable if needed (such as a palette color)
            bgColorOrig,
            *bgColor = nullptr; //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)
        
        void 
            setupPixelArray(),
            fillPixelArray(),
            setNumGlims(uint16_t newNumGlims),
            setTwoSets(bool newSetting),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        uint8_t
            *totFadeSteps = nullptr;
        
        uint16_t
            *fadePixelLocs = nullptr,
            glimArrLen,
            glimArrLenMax = 0, //used for tracking the memory size of the glimmer arrays
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
            getFadeColor(uint8_t glimNum);
        
        void 
            init(CRGB GlimmerColor, CRGB BgColor, uint16_t Rate),
            advancePixelArray();
};

#endif
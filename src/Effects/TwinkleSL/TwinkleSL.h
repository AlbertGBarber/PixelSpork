#ifndef TwinkleSL_h
#define TwinkleSL_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

/*
Fades sets of randomly chosen segment lines in and out (like FastLED TwinkleFox)
The color of the lines will be set to a single color, chosen randomly, of picked from a palette 
The amount of fade in and out steps are controlled by FadeInSteps, and FadeOutSteps
Please note that the total number of lines colored at one time will be numTwinkles * (FadeInSteps + FadeOutSteps)
Just run an example and you'll see what I mean

The effect is adapted to work on segment lines for 2D use, but you can keep it 1D by
passing in a SegmentSetPS with only one segment containing the whole strip.

This effect is fully compatible with color modes, and the bgColor is a pointer, so you can bind it
to an external color variable

Note that you cannot change most of the effect variables on the fly without resetting the effect.

Also note that the maximum sum of the fade in and out steps is 255. So you can have 128 fade in and out steps,
254 fade in steps and 1 fade out, etc.

Example call: 
    TwinkleSL twinkle(mainSegments, CRGB::Red, CRGB::Blue, 2, 4, 4, 70);
    Will choose 2 lines each cycle to fade to/from red each cycle, using a blue background, 
    with 4 fade in and out steps, at a rate of 70ms

    TwinkleSL twinkle(mainSegments, cybPnkPal_PS, 0, 3, 1, 6, 60);
    Will choose 3 lines each cycle to fade to/from colors chosen from the palette, using a blank background,
    with 1 fade in and 6 fade out steps, at a rate of 60ms

    TwinkleSL twinkle(mainSegments, 0, 4, 2, 2, 80);
    Will choose 4 lines each cycle to fade to/from random colors, using a blank background, 
    (note this sets randMode = 1)
    with 2 fade in and 2 fade out steps, at a rate of 80ms

Constructor Inputs:
    palette(optional, see constructors) -- the palette from which colors will be chosen randomly
    color(optional, see constructors) -- the color that the randomly chosen pixels will be set to
    numTwinkles -- The amount of random pixels chosen each cycle 
    bgColor -- The color of the background, this is what pixels will fade to and from
    fadeInSteps and FadeOutSteps -- The number of steps taken to fade pixels in and out (min value of 1, max of 255)
    rate -- The update rate (ms)

Other Settings:
    colorMode (default 0) -- sets the color mode for the random pixels (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the background (see segDrawUtils::setPixelColor)
    randMode (default 0) -- sets how colors will be picked
                            0: Picks colors from the palette
                            1: Picks colors at random
    fillBg (default false) -- sets the background to be redrawn every cycle, useful for bgColorModes that are dynamic

Functions:
    setSteps(newFadeInSteps, newFadeOutSteps) -- Sets the number of fade in and out steps
                                                 (Will reset the effect if the new total number of steps (fadeInSteps + fadeOutSteps) 
                                                 is greater than the current number of steps)
    setSingleColor(Color) -- Sets the effect to use a single color for the pixels, will restart the effect
    reset() -- Resets the startup variables, you probably don't need to ever call this
    setNumTwinkles(newNumTwinkles) -- sets an new number of pixels to be chosen each cycle, 
                                      (Will reset the effect if the new number of pixels is different than the current number)
    update() -- updates the effect

reference Vars:
    numTwinkles -- How many random pixels are chosen each cycle, set with setNumTwinkles()
    fadeInSteps and fadeOutSteps -- The number of steps taken to fade pixels in and out (min value of 1, max of 255)
                                    Both are set at the same time using setSteps();

Notes:
    In order for the effect to work, it needs to keep track of all the active random pixels until they fade out
    To do this it uses a pair of 2D uint16_t and CRGB arrays of size numTwinklesMax by (FadeInSteps + FadeOutSteps)
    (numTwinklesMax is the maximum numTwinkles ever used, if you don't change numTwinkles, it will be numTwinkles)
    This takes up a good amount of ram, so try to limit your pixel number and fade lengths
    In practice, fewer pixels with longer fades will keep ram minimal, while also looking good
    If you don't have enough ram, check the TwinkleLowRam version of this effect
    
    You cannot dynamically change the fade lengths or number of random pixels without restarting the effect
    It would be very complicated to allow this due to having to resize/copy the arrays, while also fading away
    all active pixels fully
    (it might look ok to initialize the arrays with random values, but idk)
    You can do this with the TwinkleLowRam version of this effect, but it is more limited 
*/
class TwinkleSL : public EffectBasePS {
    public:
        //Constructor for a full palette effect
        TwinkleSL(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, uint16_t numTwinkles, uint8_t FadeInSteps,
                  uint8_t FadeOutSteps, uint16_t Rate);

        //Constructor for a using a single color
        TwinkleSL(SegmentSetPS &SegSet, CRGB Color, CRGB BgColor, uint16_t numTwinkles, uint8_t FadeInSteps,
                  uint8_t FadeOutSteps, uint16_t Rate);

        //Constructor for choosing all colors at random
        TwinkleSL(SegmentSetPS &SegSet, CRGB BgColor, uint16_t numTwinkles, uint8_t FadeInSteps,
                  uint8_t FadeOutSteps, uint16_t Rate);

        //destructor
        ~TwinkleSL();

        uint8_t
            randMode = 0,
            colorMode = 0,
            bgColorMode = 0;

        //step vars
        uint8_t
            fadeInSteps = 0,   //Set using setSteps(), for reference only!!
            fadeOutSteps = 0;  //Set using setSteps(), for reference only!!

        uint16_t
            numTwinkles;  //for reference, set with setNumTwinkles()

        CRGB
            bgColorOrig,
            *bgColor = nullptr;  //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)

        bool
            fillBg = false;

        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0};  //Must init structs w/ pointers set to null for safety

        void
            setSteps(uint8_t newFadeInSteps, uint8_t newFadeOutSteps),
            setSingleColor(CRGB Color),
            reset(),
            setNumTwinkles(uint16_t newNumTwinkles),
            initTwinkleArrays(),
            deleteTwinkleArrays(),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0;

        uint8_t
            paletteLength,
            step,
            totalSteps = 0,
            totFadeStepsMax = 0,  //used for tracking the memory size of the led and color index arrays
            totFadeSteps;

        uint16_t
            numSegs,
            numLines,
            lineNum,
            pixelNum,
            numTwinklesMax = 0,  //used for tracking the memory size of the led and color index arrays
            **ledArray = nullptr;

        bool
            startUpDone = false;

        CRGB
            **colorIndexArr = nullptr,
            twinkleColor,
            colorTarget,
            pickColor();

        void
            init(uint8_t FadeInSteps, uint8_t FadeOutSteps, CRGB BgColor, SegmentSetPS &SegSet, uint16_t Rate),
            incrementTwinkleArrays();
};

#endif
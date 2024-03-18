#ifndef TwinkleSL_h
#define TwinkleSL_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

/*
Fades random pixels in and out (like FastLED TwinkleFox). The color of the pixels can be set to a single color, 
chosen randomly, or picked from a palette. 
The number of fade in and out steps is set by `fadeInSteps` and `fadeOutSteps`, 
allowing you full customization over how the effect pixels look. 
Try a single fade in step, with 5+ fade out steps for a "rain drop" type look.

The effect is adapted to work on segment lines for 2D use. Each line will be a single color.

Supports Color Modes for both the main and background colors.

Note that the effect always spawns one fading pixel with every update cycle, 
while fading any existing twinkles out by one step. This means that the total number of twinkles active at 
one time will be `numTwinkles * (FadeInSteps + FadeOutSteps)`. 
So even a low `numTwinkles`, will lead to quite a lot of lit pixels. 
Just run an example and you'll see what I mean.

For a more complex version of this effect that offers more control over the twinkle density, see Twinkle2SLSeg.h. 
For more lightweight, faster version of this effect see TwinkleFastSL.h

Note that adjusting most of the effect's settings will reset the effect.

    `randMode` (default 0) (uint8_t):
        Sets how the twinkle colors will be picked.
        * 0 -- Picks colors randomly from the palette.
        * 1 -- Picks colors totally at random.

**Warning**, for the effect to work, it needs to keep track of all the active twinkles as they fade in/out. 
To do this it uses a pair of _dynamically_ created 2D uint16_t and CRGB arrays of 
size `numTwinkles * (fadeInSteps + fadeOutSteps`). 
This can take up a good amount of ram, so be aware of your available memory. 
To avoid memory fragmentation, when you create the effect, 
you should set set numTwinkles, fadeInSteps, and fadeOutSteps to the maximum value you expect to use. 
See https://github.com/AlbertGBarber/PixelSpork/wiki/Effects-Advanced#managing-dynamic-memory-and-fragmentation
for more details. 

Example call: 
    TwinkleSL twinkle(mainSegments, cybPnkPal_PS, 0, 3, 1, 6, 60);
    Will choose 3 pixels each cycle to fade to/from colors from the cybPnkPal_PS palette.
    The background is blank.
    The pixels will have 1 fade in and 6 fade out steps, 
    updating at a rate of 60ms.

    TwinkleSL twinkle(mainSegments, CRGB::Red, CRGB::Blue, 2, 4, 4, 70);
    Will choose 2 pixels each cycle to fade to/from red.
    The background is blue.
    The pixels will 4 fade in and out steps, 
    updating at a rate of 70ms.

Constructor Inputs:
    palette(optional, see constructors) -- the palette from which colors will be chosen randomly
    color(optional, see constructors) -- the color that the randomly chosen pixels will be set to
    bgColor -- The color of the background, this is what pixels will fade to and from
    numTwinkles -- The number of random pixels chosen for twinkling each cycle. Can be changed later using `setNumTwinkles()`.
    fadeInSteps (min 1) -- The number of blend steps to fade a pixel in. Can be changed later using setSteps().
    fadeOutSteps (min 1) -- The number of blend steps to fade a pixel out. Can be changed later using setSteps().
    rate -- The update rate (ms)

Other Settings:
    colorMode (default 0) -- sets the color mode for the random pixels (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the background (see segDrawUtils::setPixelColor)
    randMode (default 0) -- (See randMode notes in intro)
    fillBg (default false) -- sets the background to be redrawn every cycle, useful for bgColorModes that are dynamic

Functions:
    setSingleColor(Color) -- Sets the effect to use a single color for the pixels. 
    setNumTwinkles(newNumTwinkles) -- sets a new number of pixels to be twinkled each cycle, 
                                      (Will reset the effect if the new number of pixels is different than the current number)
    setSteps(newFadeInSteps, newFadeOutSteps) -- Sets the number of fade in and out steps.
                                                 (Will reset the effect if the new total number of steps (fadeInSteps + fadeOutSteps) 
                                                 is greater than the current total)
    reset() -- Restarts the effect. Clears any existing twinkles.
    update() -- updates the effect

reference Vars:
    numTwinkles -- (See Constructor Inputs above), set with setNumTwinkles().
    fadeInSteps -- (See Constructor Inputs above), set using setSteps().
    fadeOutSteps -- (See Constructor Inputs above), set using setSteps().
*/
class TwinkleSL : public EffectBasePS {
    public:
        //Constructor for a full palette effect
        TwinkleSL(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, uint16_t numTwinkles, uint8_t FadeInSteps,
                  uint8_t FadeOutSteps, uint16_t Rate);

        //Constructor for a using a single color
        TwinkleSL(SegmentSetPS &SegSet, CRGB Color, CRGB BgColor, uint16_t numTwinkles, uint8_t FadeInSteps,
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
            step;

        uint16_t
            totalSteps = 0,
            totFadeStepsMax = 0,  //used for tracking the memory size of the led and color index arrays
            totFadeSteps,
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
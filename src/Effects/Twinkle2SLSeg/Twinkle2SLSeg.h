#ifndef Twinkle2SLSeg_h
#define Twinkle2SLSeg_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "twinkleStarPS.h"

/*
Fades sets of randomly chosen segment lines, segments, or pixels in and out smoothly (like FastLED TwinkleFox). 
The effect gives you a lot of options to customize how the pixels fade in and out and also how they spawn and 
group together. The color of the pixels can be set to a single color, chosen randomly, of picked from a palette. 
See the Inputs Guide below for more details.

This effect is very similar to TwinkleSL.h, but aims to give you more options on how the twinkling looks by 
allowing you to adjust the spawn chance of the twinkles. 
It also gives you the freedom to adjust all effect settings (except `numTwinkles`, see below) on the fly, 
so you can have the effect change over time without resetting.

Supports Color Modes for both the main and background colors. 
Note that when drawing along whole segments (`segMode` 1), the effect excludes color modes 1, 6, 2, 7, 3, 8.

The effect is adapted to work on segment lines and whole segments for 2D use. 
It can also be set to 1D mode (see `segMode`'s below for more info).

Note that the effect requires an array of TwinkleStars to function (See twinkleStar.h). 
The twinkles are allocated dynamically, so, to avoid memory fragmentation, when you create the effect, 
you should set `numTwinkles` to the maximum value you expect to use. 
See https://github.com/AlbertGBarber/PixelSpork/wiki/Effects-Advanced#managing-dynamic-memory-and-fragmentation
for more details.

Note that the effect does require slightly more programming space and ram than TwinkleSL.h. 
The ram needed is in proportion to the number of twinkles.

## Inputs Guide and Notes:

    Twinkle Spawning and Control:
        The effect works by storing a set of twinkles, which are either active or inactive. 
        When active, the twinkles fade up to their target color and then fade out again, 
        setting themselves inactive once fully faded out. Inactive twinkles have a set chance of spawning with each update cycle.
        The spawn chance is the same for all twinkles. This gives you a lot of freedom in tweaking the twinkle density, 
        and can even serve as a replacement for adjusting the number of twinkles (so you don't need to reset()). 
        However, it does come with some quirks as I'll explain with the settings below:

        * `numTwinkles` -- The maximum number of twinkles that can be active on the strip at one time. 
                           Basically sets how dense your twinkles are, although this is also influenced by `spawnChance`. 
                           You can change `numTwinkles` using `setNumTwinkles()`, but will reset the effect.

        * `spawnChance` and `spawnBasis` -- The chance than an inactive twinkle will become active in a given 
                                            update cycle. If `random(spawnBasis) <= spawnChance` a twinkle will spawn. 
                                            By default, `spawnBasis` is set to 1000 to allow sub 1% probabilities. 
                                            `spawnChance` is critical because it no only influences how dense your 
                                            twinkles are, but also how likely they are to spawn together. 
                                            For instance, 30 twinkles with a 10% spawn chance will probably look similar 
                                            to 10 with a 50% spawn chance. But the latter set will be more likely to 
                                            spawn in groups. Note that even a 50% spawn chance will slowly have groups 
                                            become quite random, while 80% will keep them grouped up, 
                                            but with a bit of randomness. The grouping is also influenced by the fade 
                                            in/out ranges (see below). Getting the look you want will probably 
                                            require some experimentation.

        * `fadeInSteps` and `fadeOutSteps` -- **Min value of 1 for both!** 
                                              These set how many gradient steps are taken to fade a twinkle in and out. 
                                              Larger values will mean longer fades, with pixels sticking around for longer. 
                                              By varying the two values you can approximate square waves, triangle waves, etc. 
                                              Try a fade in step of 1 with a longer fade out step for a "falling rain" type look. 
                                              I recommend values between 4-10.
                
        * `fadeInRange` and `fadeOutRange` -- Whenever a twinkle spawns, its fade in and out steps are calculated as: 
                                              fadeInSteps + random(fadeInRange) (same for fading out but with the "out" variables).
                                              So `fadeInSteps` sets a minimum number of fade steps while `fadeInRange` 
                                              sets a cap on the random number of extra steps added. 
                                              This can give the twinkles a more varied look and helps to prevent 
                                              twinkles from grouping up. Setting these to 0 means than all twinkles 
                                              will fade exactly the same. I recommend a value in proportion to your 
                                              fade in/out steps, maybe double each a most?

        * `limitSpawning` (default false) -- If true, only one new twinkle will be allowed to spawn each update cycle. 
                                             This is a kind-of brute force way of completely prevent twinkles from 
                                             grouping up, but puts a limit on the possible twinkle density. 
                                             Makes it easier to exactly match the look of TwinkleSL.h. 

Other Effect Settings:

    `segMode` (uint8_t):
        The effect is adapted to work on segment lines or whole segments for 2D use, and on single pixels (1D).

        This is controlled by the `segMode` setting:
            * 0 -- Twinkles will be drawn on whole segment lines. Each line will be a solid color.
            * 1 -- Twinkles will be drawn on whole segments. Excludes Color Modes 1, 6, 2, 7, 3, 8.
            * 2 -- Twinkles will be drawn on individual pixels.

    `randMode` (uint8_t) (default 0):
        Sets how twinkle colors will be picked.
        * 0 -- Twinkle colors are picked randomly from the palette.
        * 1 -- Twinkle colors fully random.

Example call: 

    Twinkle2SLSeg twinkle2(mainSegments, cybPnkPal_PS, 0, 12, 500, 3, 2, 4, 5, 0, 70);
    Will choose 12 segment lines (segMode 0) to fade to/from colors from the cybPnkPal_PS palette 
    The background in blank. 
    There is a 50% chance an inactive line will become active each cycle (500/1000)
    There are 3 fade in and 4 fade out steps with ranges of 2 and 5 respectively
    The effect updates at a rate of 70ms.

    Twinkle2SLSeg twinkle2(mainSegments, CRGB::Red, CRGB::Blue, 3, 1000, 2, 0, 6, 0, 1, 60);
    Will choose 3 whole segments (segMode 1) to fade to/from red, using a blue background, 
    There is a 100% chance an inactive segment will become active each cycle (1000/1000)
    There are 2 fade in and 6 fade out steps with ranges of 0 and 0 respectively
    The effect updates at a rate of 60ms.

Constructor Inputs:
    palette(optional, see constructors) -- The palette from which colors will be chosen randomly
    color(optional, see constructors) -- The color that the twinkles will be set to
    bgColor -- The color of the background, this is what twinkles will fade to and from
    numTwinkles -- The maximum amount of random twinkles that can be active at one time. 
                   Can be changed later using `setNumTwinkles()`.
    fadeInSteps and fadeOutSteps -- The number of blend steps taken to fade twinkles in and out (min value of 1, max of 255)
    fadeInRange and fadeOutRange -- The amount of variation for the fade in and out steps (see Inputs Guide)
    segMode -- Sets if twinkles will be drawn on segment lines, whole segments or individual pixels
               (See segMode notes above) Can be changed later using setSegMode().
    Rate -- The update rate (ms)

Other Settings:
    colorMode (default 0) -- sets the color mode for the random twinkles (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the background (see segDrawUtils::setPixelColor)
    randMode (default 0) -- (See randMode notes in intro)
    fillBg (default false) -- sets the background to be redrawn every cycle, useful for bgColorModes that are dynamic
    limitSpawning -- Limits the twinkles so that only one new one can become active per update cycle (see inputs guide above)
    spawnBasis (default 1000) -- The spawn probability threshold. 
                                 A twinkle will spawn if "random(spawnBasis) <= spawnChance". (see Inputs Guide above)

Functions:
    setSteps(newFadeInSteps, newFadeOutSteps) -- Sets the number of fade in and out steps
                                                 You can also set the steps directly as long as you don't set them below 1
    setSingleColor(Color) -- Sets the effect to use a single color for the twinkles. 
                             The color will be stored in the effect's local palette, `paletteTemp`
    reset() -- Sets all twinkles to inactive and fills in the background.
    setNumTwinkles(newNumTwinkles) -- Sets the maximum amount of random twinkles that can be active at one time,
                                      will restart the effect if the new number is different from the current number.
    setSegMode(newSegMode) -- Sets the `segMode` (See `segMode` notes in Inputs Guide) 
                              (will reset the effect if the new `segMode` is different than the current).
    update() -- updates the effect
    
Reference Vars:
    numTwinkles -- (see Input Guide above) set using setNumTwinkles()
    segMode -- (see Input Guide above) set using setSegMode()
    
*/
class Twinkle2SLSeg : public EffectBasePS {
    public:
        //Constructor for a full palette effect
        Twinkle2SLSeg(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, uint16_t NumTwinkles, uint16_t SpawnChance, 
                      uint8_t FadeInSteps, uint8_t FadeInRange, uint8_t FadeOutSteps, uint8_t FadeOutRange, 
                      uint8_t SegMode, uint16_t Rate); 

        //Constructor for a using a single color
        Twinkle2SLSeg(SegmentSetPS &SegSet, CRGB Color, CRGB BgColor, uint16_t NumTwinkles, uint16_t SpawnChance,
                      uint8_t FadeInSteps, uint8_t FadeInRange, uint8_t FadeOutSteps, uint8_t FadeOutRange,
                      uint8_t SegMode, uint16_t Rate);

        //destructor
        ~Twinkle2SLSeg();
        
        uint8_t
            segMode, //For reference only, use setSegMode()
            randMode = 0,
            colorMode = 0,
            bgColorMode = 0;

        //Step vars
        uint8_t 
            fadeInSteps,
            fadeInRange,
            fadeOutRange,
            fadeOutSteps; 
        
        uint16_t
            spawnChance,
            spawnBasis = 1000, //spawn change scaling (random(spawnBasis) <= spawnChance controls spawning)
            numTwinkles; //for reference, set with setNumTwinkles()
        
        bool 
            limitSpawning = false,
            fillBg = false;
        
        CRGB 
            bgColorOrig,
            *bgColor = nullptr; //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)
        
        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0}; //Must init structs w/ pointers set to null for safety
        
        twinkleSetPS 
            *twinkleSet = nullptr,
            twinkleSetTemp = {nullptr, 0, 0}; //Must init structs w/ pointers set to null for safety
        
        void 
            setSteps(uint8_t newFadeInSteps, uint8_t newFadeOutSteps),
            setSingleColor(CRGB Color),
            reset(),
            setNumTwinkles(uint16_t newNumTwinkles),
            setSegMode(uint8_t newSegMode),
            initTwinkleArrays(),
            deleteTwinkleSet(),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        uint8_t
            twinkleFadeInSteps,
            twinkleFadeOutSteps;

        uint16_t
            twinkleLoc,
            numLines,
            numSegs,
            numLeds,
            pixelNum,
            twinkleStep;
        
        bool 
            fadeIn = true,   
            spawnOk = true;
        
        CRGB
            twinkleColor,
            colorTarget,
            colorOut;
        
        twinkleStarPS 
            *twinklePtr = nullptr,
            **twinkleArr = nullptr;

        pixelInfoPS
            pixelInfo = {0, 0, 0, 0};

        CRGB 
            getFadeColor();
        
        void
            spawnTwinkle(uint8_t twinkleNum),
            drawLineTwinkle(),
            drawSegTwinkle(),
            drawPixelTwinkle(),
            init(uint8_t FadeInSteps, uint8_t FadeOutSteps, CRGB BgColor, SegmentSetPS &SegSet, uint16_t Rate);
};

#endif
#ifndef Twinkle2SLPS_h
#define Twinkle2SLPS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "twinkleStarPS.h"

/*
Fades sets of randomly chosen segment lines in and out (like FastLED TwinkleFox)
The effect gives you a lot of options to customize how the pixels fade in and out and how they group together
The color of the pixels be set to a single color, chosen randomly, of picked from a palette 
The amount of fade in and out steps are controlled by fadeInSteps, fadeOutSteps, fadeinRange, and fadeOutRange
The maximium possible number of segment lines that can be colored at one time is numTwinkles
With each pixel having a set spawn chance.
See the inputs guide below for more details.

This effect is very similar to TwinklePS, but aims to give you more options on how the twinkling looks.
Likewise, it allows you to adjust all the variables (except possibly numTwinkles, see below) on the fly,
so you can have the effect change over time.

The effect is adapted to work on segment lines for 2D use, but you can keep it 1D by
passing in a segmentSet with only one segment containing the whole strip.

Note that the effect does require slightly more programing space and ram than TwinklePS
The ram needed is in proportion to the number of twinkles.

This effect is fully compatible with color modes, and the bgColor is a pointer, so you can bind it
to an external color variable.

Inputs Guide:
    This guide will focus on how you can adjust the twinkling rates and groupings using the following inputs:
    You should know that the effect works by storing a set of twinkles, which are either active or inactive.
    When active the twinkles fade up to their target color and then fade out again, finaly setting themselves inactive
    once fully faded out. 
        1) numTwinkles: The maximum number of twinkles that can be active on the strip at one time.
                        Basically sets how dense your twinkles are, although this is also influenced by spawnChance.

        2) spawnChance: The chance than an inactive twinkle will become active in a given update cycle.
                        Is a percent out of 100, with 100 being certain to spawn.
                        This is a super important variable because it no only influences how dense your twinkles are,
                        but also how likley they will be to spawn together. For instance, 30 twinkles with a 10% spawn chance
                        will probably look similar to 10 with a 50% spawn chance. But the latter set will be more likely to 
                        spawn in groups. Note tha even a 50% spawn chance will slowly have groups become quite random, while 
                        80% will keep them grouped up, but with a bit of randomnness.

        3) fadeInSteps and fadeOutSteps: These set how many gradient steps are taken to fade a twinkle in and then fade it 
                                        out again. Larger values will mean longer fades, with pixels sticking around for longer
                                        By varying the two values you can approximate square waves, tringle waves, etc
                                        Min value of these is 1. Recommend between 4-10.
        
        3) fadeInRange and fadeOutRange: Whenever a twinkle spawns, its fade in and out steps are calculated as:
                                         fadeInSteps + random(fadeInRange). So fadeInSteps sets a minium number of fade steps
                                         while fadeInRange set a cap on the random number of extra steps added. This mainly helps
                                         to prevent twinkles from grouping up, by adding randomness to how long they fade in/out.
                                         Setting these to 0 means than all twinkles will fade exactly the same.
                                         I recommend a value in proportion to your fade in/out steps,
                                         maybe double each a most?

        4) limitSpawing (default false): If true, only one new twinkle will be allowed to spawn each update cycle. This will completly
                                         prevent twinkles from grouping up, but puts a limit on the total number of pixels that 
                                         can be active at one time because if you take 10 cycles to fade in-out total, then only 10 
                                         new twinkles can spawn before the first pixel is up again. 

An extra note: You technically cannot change the numTwinkles on the fly, since it requires re-sizing an array, but you 
              can set the inital number of twinkles higher than you need, and then adjust the length of the twinkleSetTemp
              to be lower. The effect will only draw twinkles from the set up to it's length. You should turn on
              filBG, so that if you reduce the number of twinkles you don't have any that are left in mid-fade on the strip.
              youEffectName.twinkleSet->length = x;

Finally, remember that the effect works on segment lines, so all pixels in a line will be lit,
their colors will match the twinkle color (or be set according to the colorMode)

Example call: 
    Twinkle2SLPS(mainSegments, CRGB::Red, CRGB::Blue, 12, 50, 3, 2, 4, 5, 70);
    Will choose 10 pixels each cycle to fade to/from red each cycle, using a blue background, 
    There is a 50% chance an inactive pixel will become active each cycle
    There are 3 fade in and 4 fade out steps with ranges of 2 and 5 respectivly
    The effect upates at a rate of 70ms

    Twinkle2SLPS(mainSegments, &palette1, 0, 8, 100, 2, 0, 6, 0, 60);
    Will choose 8 pixels each cycle to fade to/from colors from palette1, using a blank background, 
    There is a 100% chance an inactive pixel will become active each cycle
    There are 2 fade in and 6 fade out steps with ranges of 0 and 0 respectivly
    The effect upates at a rate of 60ms

    Twinkle2SLPS(mainSegments, 0, 12, 20, 2, 0, 2, 0, 80);
    Will choose 12 pixels each cycle to fade to/from random colors, using a blank backgound, 
    (note this sets randMode = 1)
    There is a 20% chance an inactive pixel will become active each cycle
    There are 2 fade in and 2 fade out steps with ranges of 0 and 0 respectivly
    The effect upates at a rate of 80ms

Constructor Inputs:
    palette(optional, see constructors) -- the palette from which colors will be choosen randomly
    color(optional, see constructors) -- the color that the randomly choosen pixels will be set to
    numTwinkles -- The amount of random pixels choosen each cycle 
    bgColor -- The color of the background, this is what pixels will fade to and from
    fadeInSteps and fadeOutSteps -- The number of steps taken to fade pixels in and out (min value of 1, max of 255)
    fadeInRange and fadeOutRange -- The amount of variation for the fade in and out steps (see Inputs Guide)
    Rate -- The update rate (ms)

Functions:
    setSteps(newfadeInSteps, newfadeOutSteps) -- Sets the number of fade in and out steps, will restart the effect
                                                 You can also set the steps directly as long as you don't set them below 1
    setSingleColor(Color) -- Sets the effect to use a single color for the pixels
    reset() -- Sets all twinkles to inactive and fills in the background
    setNumTwinkles(newNumTwinkles) -- Sets an new number of pixels to be choosen each cycle, will restart the effect
                                      You can avoid calling this if you do the trick from my extra note in the inputs guide
    update() -- updates the effect

Other Settings:
    colorMode (default 0) -- sets the color mode for the random pixels (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the background (see segDrawUtils::setPixelColor)
    randMode (default 0) -- sets how colors will be picked
                            0: Picks colors from the palette
                            1: Picks colors at random
    fillBG (default false) -- sets the background to be redrawn every cycle, useful for bgColorModes that are dynamic
    limitSpawing -- Limits the twinkles so that only one new one can become active per update cycle (see inputs guide above)

Notes:
    The twinkles are stored in a twinkleSet as part of the effect. The set is public to 
    allow editing of its length (see note at end of inputs guide). 
    The more twinkles you have, the larger the twinkleSet is, so watch your memory usage.
    For more on twinkle sets see twinkleStart.h
*/
class Twinkle2SLPS : public EffectBasePS {
    public:
        //Constructor for a full palette effect
        Twinkle2SLPS(SegmentSet &SegmentSet, palettePS *Palette, CRGB BgColor, uint16_t NumTwinkles, uint8_t SpawnChance, 
                   uint8_t FadeInSteps, uint8_t FadeInRange, uint8_t FadeOutSteps, uint8_t FadeOutRange, uint16_t Rate); 

        //Constructor for a using a single color
        Twinkle2SLPS(SegmentSet &SegmentSet, CRGB Color, CRGB BgColor, uint16_t NumTwinkles, uint8_t SpawnChance,
                   uint8_t FadeInSteps, uint8_t FadeInRange, uint8_t FadeOutSteps, uint8_t FadeOutRange, uint16_t Rate);
        
        //Constructor for choosing all colors at random
        Twinkle2SLPS(SegmentSet &SegmentSet, CRGB BgColor, uint16_t NumTwinkles, uint8_t SpawnChance, 
                   uint8_t FadeInSteps, uint8_t FadeInRange, uint8_t FadeOutSteps, uint8_t FadeOutRange, uint16_t Rate);

        //destructor
        ~Twinkle2SLPS();

        uint16_t
            numTwinkles;

        CRGB 
            bgColorOrig,
            *bgColor; //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)

        bool 
            limitSpawing = false,
            fillBG = false;
        
        uint8_t 
            randMode = 0,
            spawnChance,
            colorMode = 0,
            bgColorMode = 0;

        uint8_t 
            fadeInSteps,
            fadeInRange,
            fadeOutRange,
            fadeOutSteps;

        SegmentSet 
            &segmentSet; 
        
        palettePS
            paletteTemp,
            *palette;
        
        twinkleSetPS 
            *twinkleSet,
            twinkleSetTemp;
        
        void 
            setSteps(uint8_t newfadeInSteps, uint8_t newfadeOutSteps),
            setSingleColor(CRGB Color),
            reset(),
            setNumTwinkles(uint16_t newNumTwinkles),
            initTwinkleArrays(),
            deleteTwinkleSet(),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        uint8_t
            numSegs,
            twinkleFadeInSteps,
            twinkleFadeOutSteps;

        uint16_t
            twinkleLoc,
            numLines,
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
            *twinklePtr,
            **twinkleArr;
        
        void
            spawnTwinkle(uint8_t twinkleNum),
            init(uint8_t FadeInSteps, uint8_t FadeOutSteps, CRGB BgColor, uint16_t Rate);
};

#endif
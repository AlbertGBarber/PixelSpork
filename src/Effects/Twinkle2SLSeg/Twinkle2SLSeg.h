#ifndef Twinkle2SLSeg_h
#define Twinkle2SLSeg_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "twinkleStarPS.h"

/*
Fades sets of randomly chosen segment lines or segments in and out (like FastLED TwinkleFox)
The effect gives you a lot of options to customize how the pixels fade in and out and how they group together
The color of the pixels be set to a single color, chosen randomly, of picked from a palette 
The amount of fade in and out steps are controlled by fadeInSteps, fadeOutSteps, fadeinRange, and fadeOutRange
The maximium possible number of segment lines that can be colored at one time is numTwinkles
With each pixel having a set spawn chance.
See the inputs guide below for more details.

This effect is very similar to TwinkleSL, but aims to give you more options on how the twinkling looks.
Likewise, it allows you to adjust all the variables (except possibly numTwinkles, see below) on the fly,
so you can have the effect change over time.

segMode:
The effect is adapted to work on segment lines, whole segments for 2D use, or on single pixels (1D)
This is contolled by the segMode var:
    0: Twinkles will be drawn on segment lines
    1: Twinkles will be drawn on whole segments
    2: Twinkles will be drawn on individual pixels

Note that the effect does require slightly more programing space and ram than TwinklePS
The ram needed is in proportion to the number of twinkles.

This effect is almost fully compatible with color modes, and the bgColor is a pointer, so you can bind it
to an external color variable. 
When drawing along segments (segMode is 1), the effect works with all color modes except 1, 6, 2, 7, 3, 8.

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

Example call: 
    Twinkle2SLSeg(mainSegments, CRGB::Red, CRGB::Blue, 12, 50, 3, 2, 4, 5, 0, 70);
    Will choose 12 segment lines (segMode 0) to fade to/from red each cycle, using a blue background, 
    There is a 50% chance an inactive line will become active each cycle
    There are 3 fade in and 4 fade out steps with ranges of 2 and 5 respectively
    The effect upates at a rate of 70ms

    Twinkle2SLSeg(mainSegments, palette1, 0, 8, 100, 2, 0, 6, 0, 1, 60);
    Will choose 8 segments (segMode 1) each cycle to fade to/from colors from palette1, using a blank background, 
    There is a 100% chance an inactive segment will become active each cycle
    There are 2 fade in and 6 fade out steps with ranges of 0 and 0 respectively
    The effect upates at a rate of 60ms

    Twinkle2SLSeg(mainSegments, 0, 12, 20, 2, 0, 2, 0, 3, 80);
    Will choose 12 individual pixels (segMode 2) to fade to/from random colors, using a blank backgound, 
    (note this sets randMode = 1)
    There is a 20% chance an inactive line will become active each cycle
    There are 2 fade in and 2 fade out steps with ranges of 0 and 0 respectively
    The effect upates at a rate of 80ms

Constructor Inputs:
    palette(optional, see constructors) -- The palette from which colors will be choosen randomly
    color(optional, see constructors) -- The color that the twinkles will be set to
    numTwinkles -- The maxmimum amount of random twinkles that can be active at one time
    bgColor -- The color of the background, this is what twinkles will fade to and from
    fadeInSteps and fadeOutSteps -- The number of steps taken to fade twinkles in and out (min value of 1, max of 255)
    fadeInRange and fadeOutRange -- The amount of variation for the fade in and out steps (see Inputs Guide)
    segMode -- Sets if twinkles will be drawn on segment lines, whole segments or individual pixels
               (See segMode notes above)
    Rate -- The update rate (ms)

Functions:
    setSteps(newfadeInSteps, newfadeOutSteps) -- Sets the number of fade in and out steps
                                                 You can also set the steps directly as long as you don't set them below 1
    setSingleColor(Color) -- Sets the effect to use a single color for the twinkles
    reset() -- Sets all twinkles to inactive and fills in the background
    setNumTwinkles(newNumTwinkles) -- Sets the maxmimum amount of random twinkles that can be active at one time,
                                      will restart the effect if the new number is different from the current number
                                      You can avoid calling this if you do the trick from my extra note in the inputs guide
    setSegMode(newSegMode) -- Sets if twinkles will be drawn on segment lines, whole segments or individual pixels
                              (See segMode notes above) (will reset the effect if value is different from current value)
    update() -- updates the effect

Other Settings:
    colorMode (default 0) -- sets the color mode for the random twinkles (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the background (see segDrawUtils::setPixelColor)
    randMode (default 0) -- sets how colors will be picked
                            0: Picks colors from the palette
                            1: Picks colors at random
    fillBG (default false) -- sets the background to be redrawn every cycle, useful for bgColorModes that are dynamic
    limitSpawing -- Limits the twinkles so that only one new one can become active per update cycle (see inputs guide above)

Reference Vars:
    numTwinkles -- (see notes above) set using setNumTwinkles()
    segMode -- (see notes above) set using setSegMode()
    
Notes:
    The twinkles are stored in a twinkleSet as part of the effect. The set is public to 
    allow editing of its length (see note at end of inputs guide). 
    The more twinkles you have, the larger the twinkleSet is, so watch your memory usage.
    For more on twinkle sets see twinkleStart.h
*/
class Twinkle2SLSeg : public EffectBasePS {
    public:
        //Constructor for a full palette effect
        Twinkle2SLSeg(SegmentSet &SegmentSet, palettePS &Palette, CRGB BgColor, uint16_t NumTwinkles, uint8_t SpawnChance, 
                      uint8_t FadeInSteps, uint8_t FadeInRange, uint8_t FadeOutSteps, uint8_t FadeOutRange, 
                      uint8_t SegMode, uint16_t Rate); 

        //Constructor for a using a single color
        Twinkle2SLSeg(SegmentSet &SegmentSet, CRGB Color, CRGB BgColor, uint16_t NumTwinkles, uint8_t SpawnChance,
                      uint8_t FadeInSteps, uint8_t FadeInRange, uint8_t FadeOutSteps, uint8_t FadeOutRange,
                      uint8_t SegMode, uint16_t Rate);
        
        //Constructor for choosing all colors at random
        Twinkle2SLSeg(SegmentSet &SegmentSet, CRGB BgColor, uint16_t NumTwinkles, uint8_t SpawnChance, 
                      uint8_t FadeInSteps, uint8_t FadeInRange, uint8_t FadeOutSteps, uint8_t FadeOutRange, 
                      uint8_t SegMode, uint16_t Rate);

        //destructor
        ~Twinkle2SLSeg();

        uint16_t
            numTwinkles; //for refrence, set with setNumTwinkles()

        CRGB 
            bgColorOrig,
            *bgColor = nullptr; //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)

        bool 
            limitSpawing = false,
            fillBG = false;
        
        uint8_t
            segMode, //For reference only, use setSegMode()
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
            *palette = nullptr;
        
        twinkleSetPS 
            *twinkleSet = nullptr,
            twinkleSetTemp;
        
        void 
            setSteps(uint8_t newfadeInSteps, uint8_t newfadeOutSteps),
            setSingleColor(CRGB Color),
            reset(),
            setNumTwinkles(uint16_t newNumTwinkles),
            initTwinkleArrays(),
            setSegMode(uint8_t newSegMode),
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
            pixelInfo{0, 0, 0, 0};

        CRGB 
            getFadeColor();
        
        void
            spawnTwinkle(uint8_t twinkleNum),
            drawLineTwinkle(),
            drawSegTwinkle(),
            drawPixelTwinkle(),
            init(uint8_t FadeInSteps, uint8_t FadeOutSteps, CRGB BgColor, uint16_t Rate);
};

#endif
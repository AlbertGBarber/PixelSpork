#ifndef AddGlitterPS_h
#define AddGlitterPS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

/*
A utility effect that lights up random pixels to add "glitter" to an existing effect. 
The utility is specifically built to run on the same segment set as another effect 
and includes several mechanisms to help sync the effects.

The utility only supports one color for the glitter, 
but the color is a pointer, so you can bind it externally (see Constructor Inputs for more).

Supports Color Modes for the glitter colors.

**Warning!** The utility never removes any glitter, it assumes that the partnered effect will wipe 
any old glittering pixels out when it updates (you many need to turn on `fillBg` in your effect). 
This may clash with various "fast" effects, and any effects that rely on reading or adding pixel colors. 
Test any effects before assuming it works.

Note that the utility uses an uint16_t array sized to the number of glitter particles, `glitterNum`. 
It is allocated dynamically, so, to avoid memory fragmentation, when you create the effect, 
you should set `glitterNum` to the maximum value you expect to use. 
See https://github.com/AlbertGBarber/PixelSpork/wiki/Effects-Advanced#managing-dynamic-memory-and-fragmentation 
for more details. 

    Glitter Modes:
        There are two glitter modes that set how the glitter pixels are drawn:
        `glitterMode` (uint8_t):
            * 0 -- All the glitter pixels get new locations each update.
            * 1 -- The glitter pixels are replaced one by one with new locations at each update, so that one pixel turns off and a new one turns on. 

    Update Rate Syncing:
        The utility has two rates: the `glitterRate` and the overall update `rate`. 
        The `glitterRate` controls how often the glittering pixels change,
        while the `rate` sets how often the glitter pixels are re-drawn. 
        In practice, you bind your effect's update rate and the utility's `rate` to be the same, 
        so that the effect and the utility draw at the same time, preventing them from over-writing each other. 
        Meanwhile, you use `glitterRate` to control the look of the glitter. 

        For example, lets say have an effect that updates every 100ms, and we want to add glitter, 
        spawning it every 1000ms. The utility's `rate` would be 100ms, the same as the effect's, 
        so that the glitter would be re-drawn in sync with the effect. Meanwhile, our `glitterRate` would be 1000ms, 
        controlling how often the glitter pixels change location.

        Note that the utility assumes that the glitter spawn rate is slower or equal to the effect's update rate. 
        If `glitterRate` is the faster, then it will keep filling the segment set with glitter.

        Both `glitterRate` and `rate` are pointers like in other effects, so you can bind them externally. 

        Note that while the glitter spawning and drawing parts of the utility update independently, 
        it may be helpful to make it a multiple of your effect's rate. 
        For example, if your effect's rate is 80, glitter rates could be 160, 240, 320, etc.
        
    Updating and Showing:
        Like a normal effect, the utility does call a `show()` function to draw the glittering pixels. 
        To prevent the glitter from being written over, it should be drawn after any other effects sharing the 
        segment set. This means that the utility should be always be updated _AFTER_ your effects. 
        You should also set the other effect's `showNow` variable to false to reduce redundant drawing.

        In other words your update setup should be similar to:
        yourEffect.update();//<---- with showNow = false;
        glitter.update(); //<---- showNow = true, so the pixels are displayed.

Example call: 
    //Since the utility needs an effect to work with, we'll use a basic Rainbow Cycle effect for this example
    RainbowCyclePS rainbowCycle(mainSegments, 30, true, 80); 
    Will draw rainbows of length 30, moving towards the end of the segment set, at 80ms

    //Create the glitter utility
    AddGlitterPS addGlitter(mainSegments, CRGB::White, 10, 0, 800, 80);
    Will drawn 10 white glitter pixels on the segment set every 80ms.
    Glitter mode 0 is used, so the pixels all be set on/off at once.
    New glitter locations will be set every 800ms.
    
    //Some setup code (put in Arduino setup() function)
    addGlitter.rate = rainbowCycle.rate; //bind the utility rate pointer to the effect's so they sync
    rainbowCycle.showNow = false; //We'll show() using just the glitter utility.
    
    //The update functions (put in the Arduino loop() function)
    rainbowCycle.update();
    glitter.update();

Constructor Inputs:
    glitterColor -- The color of the glitter particles. 
                    It is a pointer, so you can externally. 
                    By default it is bound to the utility's local color var, glitterColorOrig.
    glitterNum -- How many glitter pixels there are. Can be changed later using setGlitterNum().
    glitterMode -- How the glitter pixels will be turned on/off (see Glitter Modes above).
    glitterRate -- How often new particle locations are set (ms). Like rate, it is a pointer, so you can bind it externally.
    rate -- The update rate (ms) (should be the same as the rate in the effect you want to add glitter to)

Other Settings:
    colorMode (default false) -- The color mode used for the glitter pixels, see (see segDrawUtils::setPixelColor)
    active (default true) -- If false, the utility will be disabled (updates() will be ignored).

Functions:
    setGlitterNum(newNum) -- Sets the number of glitter pixels, will wipe out the current set.
    update() -- updates the effect.

Reference Vars:
    glitterNum -- The number of glitter pixels, use setGlitterNum() to set.
*/
class AddGlitterPS : public EffectBasePS {
    public:
        AddGlitterPS(SegmentSetPS &SegSet, CRGB GlitterColor, uint16_t GlitterNum, uint8_t GlitterMode,
                     uint16_t GlitterRate, uint16_t Rate);

        ~AddGlitterPS();

        uint8_t
            glitterMode,
            colorMode = 0;

        uint16_t
            *glitterLocs = nullptr,
            *glitterRate = nullptr,
            glitterRateOrig,
            glitterNum;  //for reference, use setGlitterNum() to set,

        CRGB
            glitterColorOrig,
            *glitterColor = nullptr;

        void
            setGlitterNum(uint16_t newNum),
            update(void);

    private:
        unsigned long
            currentTime,
            prevGlitterTime = 0,
            prevTime = 0;

        uint16_t
            glitterNumMax = 0,
            pixelNum,
            numLeds;

        void
            advanceGlitterArr(),
            fillGlitterArr();
};

#endif
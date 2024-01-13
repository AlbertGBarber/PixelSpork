#ifndef BreathPS_h
#define BreathPS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
//#include "MathUtils/mathUtilsPS.h"

/*
Classic breathing/heartbeat effect. The segment set slowly fades between a main color and a background color
The effect is set to use multiple colors based on a given pattern and palette.
The effect will cycle through the colors with each "breath"
There are also a few random color modes and a rainbow mode (more on those below)

Based on code by found ldirko (https://www.reddit.com/user/ldirko/)
code found here: https://pastebin.com/gqxUnmMa

The background color is a pointer so you can bind it to an external variable.

The speed of the breathing is set by breathFreq.
I've set this to be a pointer like the effect update rate.
So you can bind it externally. 
By default the breathFreq will be bound to breathFreqOrig, 
so change that if you want to change the speed directly.

I recommend a breathFreq between 5 and 20, otherwise it gets quite slow.

Your update rate should not be too much more than your breathing freq, or the effect will look choppy.

You can also set how far the breath fades by setting maxBreath (default 255) to mimic more of a pulse kind of look
by preventing the breath from fading all the way to the background.
Note that this works best when using a single color, otherwise the effect will look "jumpy" since colors will change without fully fading.

Note that you can change variables freely while the effect is running.

Does not work with color modes from segDrawUtils::setPixelColor().

randModes:
    0: Colors will be chosen in order from the pattern (not random)
    1: Colors will be chosen completely at random
    2: Colors will be chosen randomly from the pattern (not allowing repeats)
    3: Colors will be chosen randomly from the pattern (allowing repeats)
    4: Colors will be from the rainbow (the hue is offset by hueRate each time a color is chosen)

Example calls: 
    uint8_t pattern_arr = {0, 1, 2};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};
    BreathPS breath(mainSegments, pattern, cybPnkPal_PS, 0, 10, 50);
    Does a breathing cycle using the colors from cybPnkPal_PS, following the pattern above
    The background is blank
    The breathFreq is 10, the effect updates at 50ms

    BreathPS breath(mainSegments, cybPnkPal_PS, 0, 5, 50);
    Does a breathing cycle using the colors from cybPnkPal_PS in order
    The background is blank
    The breathFreq is 5, the effect updates at 50ms

    BreathPS breath(mainSegments, CRGB::Red, CRGB::Blue, 200, 5, 50);
    Does a breathing cycle in red only
    The background is blue
    The maximum fade amount is capped at 200
    The breathFreq is 5, the effect updates at 50ms

    BreathPS breath(mainSegments, 0, 25, 10, 50);
    Does a breathing cycle using a rainbow
    The background is blank
    The rainbow hue advances 25 steps with each breath
    (the hueRate is how the rainbow's color changes with each breath, out of 255)
    The breathFreq is 10, the effect updates at 50ms

Constructor Inputs
    pattern(optional, see constructors) -- A pattern is struct made from a 1-d array of palette indexes ie {0, 1, 3, 6, 7} 
                                          and the length of the array 
                                          (see patternPS.h)                                     
    palette(optional, see constructors) -- The repository of colors used in the pattern, or can be used as the pattern itself
    color(optional, see constructors) -- Will do the breathing in a single color. 
                                         Note that the color will be placed in a palette called paletteTemp.
                                         If you pass in 0 as the color then the effect will switch to randMode 1 (full random)
    maxBreath (optional, default 255) -- How far the breath color will fade towards the background
                                         Only used as a constructor input when creating a single color breath
                                         Using lower than 255 with multiple colors causes the colors to jump with each breath.
                                         !!Must be greater than minBreath (default 60, see other settings below)
    hueRate(optional, default 20) -- How much the rainbow's color changes with each breath, out of 255
                                     Only used for the rainbow randColor mode (4)
    bgColor -- The color that the breaths will fade to (usually blank).
    breathFreq -- The speed of the breathing, between 5 and 20 works well.
    Rate -- update rate (ms)

Functions:
    setPaletteAsPattern() -- Sets the effect pattern to match the current palette
    reset() -- Restarts the effect
               Note that the effect may start mid breath after resetting.
    update() -- updates the effect

Other Settings:
    bgColorOrig -- The default target of the effect's bgColor pointer variable.
                   Will be set to the passed in bgColor from the constructor. 
    randMode (default 0, unless set by a constructor) -- see mode notes above
    minBreath (default 60, min 0) -- The minimum breath fade amount. Should be less than maxBreath (see constructors above)
                                     60 was taken from the original code by ldirko.
    breathFreqOrig -- The default target of the effect's breathFreq pointer variable.
                      Will be set to the passed in breathFreq from the constructor
                      (see breathFreq notes above)
    breathEndOffset (default 5) -- The brightness threshold difference from maxBreath for changing breath colors,
                                   shouldn't need to change this, see comments in update() function
    rainbowSat (default 255) -- The saturation of the rainbow mode 
    rainbowVal (default 255) -- The value of the rainbow mode (as part of setting the rainbow color)

Reference vars:
    breathCount -- The number of breath cycles we've gone through. Does not have a capped value.
*/
class BreathPS : public EffectBasePS {
    public:

        //Constructor for using a pattern and palette
        BreathPS(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, CRGB BgColor, uint8_t BreathFreq,
                 uint16_t Rate);

        //Constructor for using palette as pattern
        BreathPS(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, uint8_t BreathFreq, uint16_t Rate);

        //Constructor for a single color breath (pass in 0 as the color to trigger randMode 2, fully random)
        BreathPS(SegmentSetPS &SegSet, CRGB color, CRGB BgColor, uint8_t MaxBreath, uint8_t BreathFreq, uint16_t Rate);

        //Constructor for rainbow mode
        BreathPS(SegmentSetPS &SegSet, CRGB BgColor, uint8_t HueRate, uint8_t BreathFreq, uint16_t Rate);

        ~BreathPS();

        uint8_t
            breathFreqOrig,
            *breathFreq = nullptr,  //pointer to the breath frequency (dby default it's bound to the the Rate)
            minBreath = 60,         //The minimum breath fade amount (0 is min, should be less than maxBreath)
            maxBreath = 255,        //How far the breath color will fade towards the background (255 is max)
            breathEndOffset = 5,    //The offset from maxBreath after which a new color is chosen (see comments in update() code)
            rainbowSat = 255,
            rainbowVal = 255,
            hueRate = 20,  //How much the hue is incremented by for randMode 4
            randMode = 0;  //see description above

        uint16_t
            breathCount = 0;  //How many total breath cycles we've gone through

        CRGB
            bgColorOrig,
            *bgColor = nullptr;  //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)

        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0};  //Must init structs w/ pointers set to null for safety

        patternPS
            *pattern = nullptr,
            patternTemp = {nullptr, 0, 0};  //Must init structs w/ pointers set to null for safety;

        void
            setPaletteAsPattern(),
            reset(void),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0;

        CRGB
            colorOut,
            breathColor;  //the color we are fading to

        uint8_t
            hue = 0,  //for rainbow mode
            bWave,
            breathEndVal,
            breath;

        uint16_t
            palIndex = 0,
            patternIndex = 0;

        bool
            lockColor = false;

        void
            getNextColor(),
            init(CRGB BgColor, uint8_t BreathFreq, SegmentSetPS &SegSet, uint16_t Rate);
};

#endif
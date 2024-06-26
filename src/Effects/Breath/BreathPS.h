#ifndef BreathPS_h
#define BreathPS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
//#include "MathUtils/mathUtilsPS.h"

/*
Classic breathing/heartbeat effect. 
The segment set slowly fades between a main color and a background color. 
The effect cycles through colors following a pattern/palette. 
There a options for a single color, random colors, a rainbow mode.

The breathing is based on code by ldirko (https://www.reddit.com/user/ldirko/), 
found here: https://pastebin.com/gqxUnmMa.

The effect is not compatible with Color Modes.

The speed of the breathing is set by `breathFreq`. 
Like the effect update rate, it is pointer  so you can bind it externally. 
By default the `breathFreq` will be bound to the effect's local `breathFreqOrig`.

I recommend a `breathFreq` between 5 and 20 (higher is slower).

Your update rate should not be too much more than your breathing freq, or the effect will look choppy.

You can also control how far the breath fades using `maxBreath` (default 255).
Lower values will mimic more of a pulse kind of look by preventing the breath from 
fading all the way to the background. 
Note that this works best when using a single color, 
otherwise the effect will look "jumpy" as colors will change without fully fading.

In rainbow mode (`randMode` 4), the effect cycles through colors following the rainbow. 
`hueRate` (default 20) sets how many steps the rainbow hue is incremented by with each breath, 
controlling how fast the effect cycles through the rainbow.

randModes:
    0: Colors will be chosen in order from the pattern (not random)
    1: Colors will be chosen completely at random
    2: Colors will be chosen randomly from the pattern (not allowing repeats)
    3: Colors will be chosen randomly from the pattern (allowing repeats)
    4: Colors will be from the rainbow (the hue is offset by hueRate each time a color is chosen)

Example calls: 
    uint8_t pattern_arr = {0, 2, 1};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};

    BreathPS breath(mainSegments, pattern, cybPnkPal_PS, 0, 10, 50);
    Does a breathing cycle using the colors from the cybPnkPal_PS palette, 
    following the pattern above
    The background is blank
    The breathFreq is 10, the effect updates at 50ms

    BreathPS breath(mainSegments, cybPnkPal_PS, 0, 5, 50);
    Does a breathing cycle using the colors from the cybPnkPal_PS palette in order
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
    color(optional, see constructors) -- Sets a single color for the breathing. 
                                         Note that the color will be placed in the effect's local palette, "paletteTemp".
                                         If you pass in 0 as the color then the effect will switch to randMode 1 (full random)
    bgColor -- The color that the breaths will fade to (usually blank).                       
    maxBreath (optional, default 255) -- How far the breath color will fade towards the background
                                         Only used as a constructor input when creating a single color breath
                                         Using less than 255 with multiple colors causes the colors to jump with each breath.
                                         !!Must be greater than minBreath (default 60, see other settings below)
    hueRate(optional, default 20) -- Only used for the rainbow mode: `randMode` 4. 
                                     Sets how many steps the rainbow hue is incremented by with each breath, 
                                     controlling how fast the effect cycles through the rainbow.
    breathFreq -- The speed of the breathing, between 5 and 20 works well (higher is slower). 
                  Is a pointer like the update rate, by default it's bound to the effect's local variable "breathFreqOrig".
    Rate -- update rate (ms)

Other Settings:
    bgColorOrig -- The default target of the effect's bgColor pointer variable.
                   Will be set to the passed in bgColor from the constructor. 
    randMode (default 0, unless set by a constructor) -- see mode notes above
    minBreath (default 60, min 0) -- The maximum amount (out of 255) that the breath colors will fade _from_ the background.
                                     At 0 the breaths will start fully at the breathing color. 
                                     60 was taken from the original code by ldirko. 
                                     **Must** be less than `maxBreath` (see constructors above).
    breathEndOffset (default 5) -- The brightness threshold difference from "maxBreath" for changing breath colors,
                                   shouldn't need to change this, see comments in update() function.
    sat (default 255) -- The HSV saturation of the rainbow mode (randMode 4).
    val (default 255) -- The HSV "value" value of the rainbow mode (randMode 4).

Functions:
    setPaletteAsPattern() -- Sets the effect pattern to match the current palette.
    reset() -- Restarts the effect.
               Note that the effect may start mid breath after resetting.
    update() -- updates the effect

Reference vars:
    breathCount -- The number of breath cycles we've gone through. Must be reset manually by calling reset().
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
            sat = 255,              //saturation value for rainbow mode (randMode 4)
            val = 255,              //"value" value for rainbow mode (randMode 4)
            hueRate = 20,           //How much the hue is incremented by for rainbow mode (randMode 4)
            randMode = 0;           //see description in intro

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
#ifndef CrossFadeCyclePS_h
#define CrossFadeCyclePS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

/* 
Cross fades the entire SegmentSetPS from one solid color to the next
following either a pattern and palette, a palette alone, or using random colors
each fade is done in a specified number of steps, at a specified rate
see constructors below for inputs

The number of fades we've done is recorded by fadeCount.

You can set the effect to pause between between fades for a fixed length of time.

Does not work with color modes from segDrawUtils::setPixelColor()

randModes are: (default 0)
    0: Colors will be chosen in order from the pattern (not random)
    1: Colors will be chosen completely at random
    2: Colors will be chosen randomly from the palette (not allowing repeats)

Example calls: 
    uint8_t pattern_arr = {0, 2, 1};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};
    CrossFadeCyclePS crossFadeCycle(mainSegments, pattern, cybPnkPal_PS, 40, 30);
    Will fade from color 0, to color 2, to color 1 of the palette
    taking 40 steps for each fade, with 30ms between steps

    CrossFadeCyclePS crossFadeCycle(mainSegments, cybPnkPal_PS, 40, 30);
    Will fade through the colors of the palette in order
    taking 40 steps for each fade, with 30ms between steps

    CrossFadeCyclePS crossFadeCycle(mainSegments, 40, 30);
    Will fade from one random color to the next
    taking 40 steps for each fade, with 30ms between steps

Constructor Inputs
    pattern(optional, see constructors) -- A pattern is struct made from a 1-d array of palette indexes ie {0, 1, 3, 6, 7} 
                                          and the length of the array 
                                          (see patternPS.h)                                     
    palette(optional, see constructors) -- The repository of colors used in the pattern, or can be used as the pattern itself
    steps -- How many steps for each fade
    rate -- update rate (ms)

Other Settings:
    randMode (default 0) -- see mode note above
    pauseTime (default 0) -- The time that the effect will wait for between fades (ms)

Functions:
    setPaletteAsPattern() -- Sets the effect pattern to match the current palette
    reset() -- restarts the effect
    update() -- updates the effect

Reference Vars:
    fadeCount -- The number of fade cycles we've gone through.

Flags:
    paused -- If true then the effect is paused, it will begin fading again after pauseTime has passed

*/
class CrossFadeCyclePS : public EffectBasePS {
    public:
        //Constructor for pattern and palette
        CrossFadeCyclePS(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, uint8_t Steps, uint16_t Rate);

        //Constructor for palette as the pattern
        CrossFadeCyclePS(SegmentSetPS &SegSet, palettePS &Palette, uint8_t Steps, uint16_t Rate);

        //Constructor for random colors
        CrossFadeCyclePS(SegmentSetPS &SegSet, uint8_t Steps, uint16_t Rate);

        ~CrossFadeCyclePS();

        uint8_t
            steps,         //total steps per fade
            randMode = 0;  //see description above

        uint16_t
            pauseTime = 0,
            fadeCount;  //how many total fades we've done

        bool
            paused = false;

        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0};  //Must init structs w/ pointers set to null for safety

        patternPS
            *pattern = nullptr,
            patternTemp = {nullptr, 0, 0};  //Must init structs w/ pointers set to null for safety

        void
            setPaletteAsPattern(),
            reset(void),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0;

        uint8_t
            patternIndex = 0,  //What index we're fading to in the pattern
            palIndex = 0,      //the index of the pattern that we are currently fading away from, mainly used for shuffle()
            currentStep = 0;   //current step of the current fade

        CRGB
            startColor,  //the color we are fading from
            colorOut,
            nextColor;  //the color we are fading to

        void
            getNextColor(),
            init(SegmentSetPS &SegSet, uint16_t Rate);
};

#endif
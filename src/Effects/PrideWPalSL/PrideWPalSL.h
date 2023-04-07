#ifndef PrideWPalSL_h
#define PrideWPalSL_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

/*
An effect based on Pride2015 by Mark Kriegsman (https://gist.github.com/kriegsman) and
work done by Jason Coon: https://github.com/jasoncoon/fibonacci-demoreel 
It has been adapted to work with segments
This version of the effect draws bands of color and brightness along segment lines,
creating ever-changing waves of color that shift and blend.
This is basically the same as the original Pride 2015, even though it works with segment lines
Passing in a SegmentSet with one segment with all your leds in order will produce the original effect
(set briDirect = true and randomBriInc = false)

The effect is different from PrideWPalSL because the colors move along the segment lines, rather then along the segments
Just try it out and you'll see what I mean

This effect includes code for doing the original Pride2015 with rainbows
and using a custom palette for colors

In adapting the effect for segment lines, I've had to change some of the 
wave inputs from the original Pride2015 to get a good look
I've turned the inputs into variables that you can change as you like
I'll explain what each of them does below
I've also included a default and random option
The default will use values hand-picked by me to look nice
while the random option will choose them from a set of ranges, 
and should also look nice, and will mean every instance of this effect will be different

You can change these variables on the fly, but doing so will probably cause the effect to "jump"

Inputs guide:
    This guide will focus on using a palette, since all the variables effect it
    The rainbow mode is more of an "as-is", where I don't expect people to change the variables
    Overall the effect has two components:
        1) Waves of varying brightness that rotate around the segment and shift in length/intensity with time
        2) Waves of palette colors that rotate around the segment and shift in length with time
    The brightness waves can be set to either rotate with or contra to the colors
    
    Brightness wave settings: 
        1) brightDepthMin and brightDepthMax: These control the max and min brightnesses the waves reach
                                              By default the max is 224 and the min 100, which leans more towards the 
                                              dark side. A lower min value will keep the segments at a "flat" brightness for longer
        2) briThetaInc16Min and briThetaInc16Max: As the effect runs, thr brightness waves divide between smaller, more numerous
                                                  waves and larger, but fewer waves. The briTheta max and min values indirectly
                                                  set the maximum and minimum number of waves. The defaults for these values are
                                                  20 and 40, which vary the waves from ~2-4. I give you the option to randomize these
                                                  on effect creation to create a unique effect. You can find the ranges for these
                                                  in init(); Also see randomizeBriInc().
        3) briThetaFreq: Has a default value of 250. Sets how quickly we cycle through the brightness waves. 
                         I give you the option to randomize this on effect creation to create a unique effect. 
                         You can find the ranges for this in init(), also see randomizeBriFreq().
        4) briDirect: Sets the direction of the brightness waves relative to the color waves. True will have the waves
                      rotate in the same direction, while false will have then reverse. This is a constructor option
                      so there's no default, but the true gives you the original look from Mark's code.
        
    Color change settings: 
        1) hueChangeMin and hueChangeMax: Sets the minimum and maximum shifts for colors in the palette. This effects
                                          how quickly colors will change and shift. The defaults for these are 5 and 9. 
                                          The actual shift value will vary between the min and max over time.
                                          Keeping these both low will produce a slow shift.
        2) gradLength: Sets how many steps we take to blend from one color to another. Default is 20. Lower numbers will
                       increase the speed that colors appear, but I wouldn't set it below 10ish, or the shift will become more
                       jagged. The effect of this is pretty subtle, so you may just want to leave it alone.
                       Does not effect the rainbow mode.

    Note that I only have functions for randomizing briThetaInc16Min/briThetaInc16Max and briThetaFreq. These produce the most 
    dramatic change in the effect, and both have a wide range of inputs where the effect still works. You can vary the other
    variables, but the effects will be more subtle

Example calls:
    PrideWPalSL(mainSegments, true, false, 80);
    Will do a rainbow effect, where the brightness waves move with the colors
    The brightness wave values will not be randomized, defaults will be used
    The effect updates at 80ms
    (the original effect by Mark)

    PrideWPalSL(ringSegments, 4, false, true, 80);
    Will do an effect using 4 random colors (in a randomly generated palette)
    The brightness waves will move opposite to the colors
    The brightness wave values will be randomized, see init() for the ranges
    The effect updates at 80ms

    PrideWPalSL(ringSegments, palette1, true, true, 80);
    Will do an effect using colors from palette1
    The brightness waves will move with the colors
    The brightness wave values will be randomized, see init() for the ranges
    The effect updates at 80ms

    PrideWPalSL(ringSegments, palette1, true, 20, 120, 250, 350, 20, 40, 3, 7, 60);
    Will do an effect using colors from palette1
    The brightness waves will move with the colors
    Colors will shift using 20 steps to blend the colors
    The brightnessThetaInc16 will vary from 120 to 250
    The briThetaInc16 will vary from 20 to 40 with a briThetaFreq of 350
    The hueInc will vary from 3 to 7
    The effect updates at 60ms

Constructor inputs: 
    palette (optional, see constructors) -- A custom palette passed to the effect
    numColors (optional, see constructors) -- How many colors will be in the randomly created palette
    briDirect -- Sets if the brightness waves move with or against the colors, true is with
    randomBriInc -- Set to true will randomize briThetaInc16 min and max, and the briThetaFreq
                    See init() for the ranges
    rate -- The update rate (ms) note that this is synced with all the particles.

    For the constructor will all the inputs, the variables in order are:
        GradLength, BrightDepthMin, BrightDepthMax, BriThetaFreq, BriThetaInc16Min,
        BriThetaInc16Max, HueChangeMin, HueChangeMax
    You can read the details of these below in Other Settings

Functions:
    randomizeBriInc(briThetaMinMin, briThetaMinMax, briThetaMaxMin, briThetaMaxMax ) -- 
                                Randomizes the min and max values of briThetaInc16 from within the passed in ranges
                                First pair are range for the briThetaInc16Min and second are for briThetaInc16Max
                                !!Warning, make sure that briThetaMinMax is less than briThetaMaxMin, so that
                                briThetaInc16Min is always less than briThetaInc16Max
    randomizeBriFreq( briFreqMin, briFreqMax ) -- Randomizes the briThetaFreq to be between the two passed in values
    update() -- updates the effect 

Other Settings (see Inputs Guide for more info on most vars):
    prideMode (default false, set automatically by constructor) -- If true, a rainbow will be used for colors instead of a palette
    brightDepthMin (default 100) -- The minimum value of brightnessThetaInc16
    brightDepthMax (default 224) -- The maximum value of brightnessThetaInc16
    briThetaInc16Min (default 25) -- The minimum value of briThetaInc16
    briThetaInc16Max (default 40) -- The Maximum value of briThetaInc16
    briThetaFreq (default 250) -- The frequency that briThetaInc16 changes at
    hueChangeMin (default 5) -- The minimum value of hueInc
    hueChangeMax (default 9) -- The maximum value of hueInc
    gradLength (default 20) -- How many gradient steps between palette colors (not used for rainbow)
*/
class PrideWPalSL : public EffectBasePS {
    public:
        //constructor for rainbow mode
        PrideWPalSL(SegmentSet &SegSet, bool BriDirect, bool RandomBriInc, uint16_t Rate);

        //constructor for palette input
        PrideWPalSL(SegmentSet &SegSet, palettePS &Palette, bool BriDirect, bool RandomBriInc, uint16_t Rate);  

        //constructor for making a random palette
        PrideWPalSL(SegmentSet &SegSet, uint8_t numColors, bool BriDirect, bool RandomBriInc, uint16_t Rate);

        //constructor with inputs for all main variables
        PrideWPalSL(SegmentSet &SegSet, palettePS &Palette, bool BriDirect, uint8_t GradLength, 
                              uint8_t BrightDepthMin, uint8_t BrightDepthMax, uint16_t BriThetaFreq, 
                              uint8_t BriThetaInc16Min, uint8_t BriThetaInc16Max, uint8_t HueChangeMin, 
                              uint8_t HueChangeMax, uint16_t Rate);

        ~PrideWPalSL();

        SegmentSet 
            &SegSet; 
        
        //commented values are values from Mark's code
        uint8_t
            brightDepthMin = 100, //96 //how much the brightness varies by, min sets how long we stay in a flat color
            brightDepthMax = 224, //224
            briThetaInc16Min = 25, //How many "waves" we have
            briThetaInc16Max = 40,
            hueChangeMin = 5, //5 //sets how fast the colors shift
            hueChangeMax = 9; //9

        uint16_t 
            gradLength = 20, //How many gradient steps to shift colors
            briThetaFreq = 203; //203 //how long we spend transitioning through waves
        
        bool
            briDirect,
            prideMode = false;
        
        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0}; //Must init structs w/ pointers set to null for safety

        void 
            randomizeBriInc(uint8_t briThetaMinMin, uint8_t briThetaMinMax, uint8_t briThetaMaxMin, uint8_t briThetaMaxMax ),
            randomizeBriFreq( uint16_t briFreqMin, uint16_t briFreqMax ),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        int8_t 
            briDirectMult;
        
         uint8_t
            sat8,
            bri8,
            hue8,
            brightDepth,
            index,
            msMultiplier;
        
        uint16_t
            numSteps,
            deltaTime,
            sPseudoTime = 0,
            sHue16 = 0,
            hue16,
            hueInc16,
            brightnessTheta16,
            brightnessThetaInc16,
            b16,
            bri16,
            h16_128,
            pixelNum,
            lineNum,
            numSegs,
            numLines;
    
        CRGB
            newColor,
            colorOut;
        
        void
            init(bool RandomBriInc, uint16_t Rate);
};

#endif
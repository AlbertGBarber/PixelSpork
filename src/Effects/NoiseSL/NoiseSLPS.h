#ifndef NoiseSLPS_h
#define NoiseSLPS_h

//TODO -- add background color?

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

/* 
An effect that maps 2D noise to a segment set by drawing the noise onto the segment lines
This is easy because segment set line always form a full matrix of size numLines x numSegs
Much of this code is based on the FastLED noise example here: https://github.com/FastLED/FastLED/blob/master/examples/NoisePlusPalette/NoisePlusPalette.ino
See more about noise here: https://github.com/s-marley/FastLED-basics/tree/main/6.%20Noise

This effect is a sort of 2D noise playground
Overall it produces spots of color that shift and morph across the segment set
You have a number of options for tuning the noise an how it changes through time
The output colors can either be taken from a palette, or set to one of two rainbow modes

The effect also works well on a strip with a only a single segment (ie just one strip)

For an alternate noise effect, see LavaPS or Noise16

Please note that this effect stores an array of uint8_t's of size (length of longest segment) * (number of segments in set)
So make sure you have enough memory to run it

Note that I don't fully understand how everything in this effect works,
but based on testing I've written an input guide below:

Inputs guide:
    When you create the effect you'll have five values to set: blendSteps, scaleBase, scaleRange, speed, and Rate
        Blending:
        blendSteps sets how many long the gradient is between each color in the palette. Doesn't need to be super large
        Anything between 20 and 60 will look good

        Scale:
        scaleBase and scaleRange control how large color areas are; how "zoomed-in" the noise is.
        Smaller values => more zoomed in, larger patches of color
        To add more variation to the effect, the scale can vary with time. This is controlled by the scaleRange value.
        scaleBase is the minimum scale value, while scaleBase + scaleRange is the maximum.
        A target scale value will be set by adding scaleBase to randomly picked value up to scaleRange: scaleBase + random(scaleRange)
        With a target set, the scale will move towards the target value by one step each update cycle
        This avoids color "jumps" by keeping the transition gradual. 
        Once the target is reached, a new target value is picked and the process begins again
        If you don't want any shifting, simply set scaleRange = 0.
        The values for scaleBase and scaleRange will vary based on the size of your segmentSet
        But I recommend starting with a scaleBase of 10 - 20, and a scaleRange of 80.

        Effect speed:
        Both speed and update rate effect how fast the effect looks
        Higher speed => faster, while lower rate => faster
        You probably want to start with rate between 50-80 (100+ may look choppy)
        And a speed value of 10 - 50

        Rainbows and shifting hues:
        There are three color modes in the effect. 
        These are set by cMode:
            0: Use the input palette for colors
            1: Noise is mapped directly to the rainbow
              So any color can show up anywhere, but they will follow the rainbow gradient (red goes to orange to yellow, etc )
            2: Noise is mapped to a limited section of the rainbow
              So you'll get one-ish color across the whole strip
              To shift the colors make sure the rotateHue is on!
        Both rainbow modes were adapted from here: https://github.com/FastLED/FastLED/blob/master/examples/Noise/Noise.ino
        rotateHue and iHue (default True):
        The noise that FastLED produces tends to be grouped in the middle of the range 
        While the effect does try to strech this out, 
        it still doesn't usually hit colors at either end of the palette
        So, for example using cMode 1, you'll get more yellow, green, and blue than red and purple
        because they're in the middle of the rainbow
        To mitigate this we can slowly offset the color center over time,
        so our rainbow will slowly morph to being more red, yellow, green to more purple, red, yellow, etc
        rotateHue controls if the offsetting is on or off, while iHue is the offset value (0 - 255)
        rotateHue is on (true) by default, since it looks good with all modes

Example calls: 
    NoiseSLPS(mainSegments, 4, 20, 20, 60, 10, 80);
    Will produce a noise effect with a palette of 4 randomly choosen colors
    There are 20 blend steps between each color
    A base scaling value of 20 will be applied with a range of 60 (max scale is 80)
    The speed is 10
    The effect updates at 80ms
    
    NoiseSLPS(mainSegments, &palette1, 40, 5, 95, 20, 80);
    Will produce a noise effect with using colors from palette1
    There are 40 blend steps between each color
    A base scaling value of 5 will be applied with a range of 95 (max scale is 100)
    The speed is 20
    The effect updates at 80ms

Constructor inputs: 
    palette (optional, see constructors) -- A custom palette passed to the effect, the default is the 
                                          lava colors palette encoded below
    numColors (optional, see constructors) -- How many colors will be in the randomly created palette
    blendSteps -- Sets how many steps are used to blend between each color
                 Basically changes how fast the colors blend
                 Between 20 - 60 looks good
    scaleBase -- Sets how "zoomed-in" the noise is
                 (test it out yourself to see what I mean) (see inputs guide above)
    scaleRange -- The range for the variation of the scale (see inputs guide above)
    Rate -- The update rate (ms) note that this is synced with all the particles.

Other settings:
    cMode -- The color mode of the effect (see inputs guide above)
    rotateHue (default true) -- sets if the noise center will be offset over time (see inputs guide above)
    ihue (default 0) -- The position of the noise center, is automatically adjusted if rotateHue is on
                       But you can also set if yourself to a specific value if needed (see inputs guide above)

Functions:
    setupNoiseArray() -- Creates the array for storing the noise (will be matrix of uint8_t's, numLines x numSegs)
                        Only call this if you change what segmentSet the effect is on
    update() -- updates the effect 

Notes:
    !!Do NOT set the scale directly after turning on shiftScale
    if you do, be sure to adjust scaleTarget = scale 
*/
class NoiseSLPS : public EffectBasePS {
    public:

        //Constuctor for randomly generated palette
        NoiseSLPS(SegmentSet &SegmentSet, uint8_t numColors, uint16_t BlendSteps, uint16_t ScaleBase, uint16_t ScaleRange, uint16_t Speed, uint16_t Rate);

        //Constructor using palette
        NoiseSLPS(SegmentSet &SegmentSet, palettePS *Palette, uint16_t BlendSteps, uint16_t ScaleBase, uint16_t ScaleRange, uint16_t Speed, uint16_t Rate); 

        ~NoiseSLPS();

        SegmentSet 
            &segmentSet; 
        
        uint8_t 
            ihue = 0,
            cMode = 0;
        
        uint16_t
            blendSteps,
            speed, 
            scaleBase,
            scaleRange;
    
        bool 
            rotateHue = true;
        
        palettePS 
            paletteTemp,
            *palette;

        void 
            setupNoiseArray(),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        int8_t 
            scaleStep = 0;
        
        uint8_t
            *noise,
            noiseData,
            oldData,
            newData,
            bri,
            dataSmoothing;
        
        uint16_t
            totBlendLength,
            pixelNum,
            numLines,
            numSegs,
            colorIndex,
            scale,
            scaleTarget,
            x,
            y,
            z,
            noiseStart,
            noiseIndex,
            joffset,
            ioffset;
        
        CRGB 
            colorOut;
        
        void
            init(uint16_t Rate),
            fillnoise8(),
            mapNoiseSegsWithPalette(),
            setShiftScale();
};

#endif
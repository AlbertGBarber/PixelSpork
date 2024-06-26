#ifndef NoiseSL_h
#define NoiseSL_h

//TODO: inoise8() has it's real range limited to 0-128,
//      use inoise16() instead, but shift out the extra bits: uint8_t n = inoise16(x << 8, y << 8) >> 8;?

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

/* 
An effect that maps 2D Perlin noise to a segment set by drawing the noise onto the segment lines 
(made simpler because a segment set acts like a matrix with dimensions `numLines x numSegs`). 
Much of this code is based on the FastLED noise example here: 
(https://github.com/FastLED/FastLED/blob/master/examples/NoisePlusPalette/NoisePlusPalette.ino). 
Overall, this effect is a sort of 2D noise playground, 
producing spots of color that shift and morph across the segment set. 
You have a number of options for tuning the noise an how it changes through time. 
The output colors can either be taken from a palette, or set to one of two rainbow modes. 
You can read more about FastLED noise here: (https://github.com/s-marley/FastLED-basics/tree/main/6.%20Noise).

The effect also works well on a strip with a only a single segment (ie just one strip)

The noise colors will be blended towards a background color (except for in rainbow modes)
The bgColor is a pointer, so you can bind it to an external color variable.
By default it is bound to bgColorOrig, which is set to 0 (blank color).
Changing the background color will "tint" the overall output, since all the colors are blended towards the background color as part of the noise.
You can also set the effect to use one of the background color modes (see segDrawUtils::setPixelColor).

Please note that this effect stores an array of uint8_t's of size (length of longest segment) * (number of segments in set)
So make sure you have enough memory to run it!

Since this effect is more of a playground, I encourage you to experiment with different input values.
Note that I don't fully understand how everything in this effect works (mainly the noise smoothing),
but based on testing I've written an input guide below:

Inputs guide:
    When you create the effect you'll have five values to set: blendSteps, scaleBase, scaleRange, speed, and Rate
        Blending:
            blendSteps sets how long the gradient is between each color in the palette. 
            Doesn't need to be super large, anything between 20 and 60 will look good.

        Scale:
            scaleBase and scaleRange control how large color areas are; how "zoomed-in" the noise is.
            Smaller values => more zoomed in, larger patches of color
            To add more variation to the effect, the scale can vary with time. This is controlled by the scaleRange value.
            scaleBase is the minimum scale value, while scaleBase + scaleRange is the maximum.
            A target scale value will be set by adding scaleBase to randomly picked value up to scaleRange: scaleBase + random(scaleRange)
            With a target set, the scale will move towards the target value by one step each update cycle
            This avoids color "jumps" by keeping the transition gradual. 
            Once the target is reached, a new target value is picked and the process begins again.
            If you don't want any shifting, simply set scaleRange = 0.
            The values for scaleBase and scaleRange will vary based on the size of your SegmentSetPS
            But I recommend starting with a scaleBase of 10 - 20, and a scaleRange of 80.

        Effect speed:
            Both speed and update rate effect how fast the effect looks
            Speed effects how quickly the noise changes, the update rate is how often the effect updates.
            Higher speed => faster, while lower rate => faster
            You probably want to start with rate between 50-80 (100+ may look choppy)
            And a speed value of 10 - 50.

        cMode:
            There are three color modes in the effect (note that these are not the same as general Color Modes)
            These are set by cMode:
                0: Use the input palette for colors
                1: Uses the palette for colors, but maps into roughly one color at a time.
                   So you'll get one-ish color across the whole strip, with the color blending towards the next color in palette
                   To shift the colors make sure the hueCycle is on! 
                   It may be helpful to set the blendSteps larger than normal to slow down the blending,
                   setting it so that the <<palette length>> * blendSteps ~= 250+ works well.
                2: Noise is mapped directly to the rainbow
                   So any color can show up anywhere, but they will follow the rainbow gradient (red goes to orange to yellow, etc )
                   Ignores the background color.
                3: Noise is mapped to a limited section of the rainbow (like mode 1)
                   So you'll get one-ish color across the whole strip, with the color blending towards the next color in the rainbow, ie red to orange, etc
                   To shift the colors make sure the hueCycle is on! Ignores the background color.
            Both rainbow modes were adapted from here: https://github.com/FastLED/FastLED/blob/master/examples/Noise/Noise.ino

        hueCycle and hue:
            The noise that FastLED produces tends to be grouped in the middle of the range 
            While the effect does try to stretch this out, it still doesn't usually hit colors at either end of the palette
            So, for example using cMode 2 (rainbow mode 1), you'll get more yellow, green, and blue than red and purple 
            because they're in the middle of the rainbow.
            To mitigate this we can slowly offset the color center over time,
            so our rainbow will slowly morph to being more red, yellow, green to more purple, red, yellow, etc
            hueCycle controls if the offsetting is on or off.
            hueCycle is on (true) by default, since it looks good with all modes. 
            When cycling, the hue is incremented by 1 every update cycle. 
            The hue values range from 0 - 255 for cMode's 0, 2, 3, 
            and 0 - <<palette length>> * blendSteps for cMode 1.

Example calls: 
    NoiseSL noiseSL(mainSegments, 4, 20, 20, 60, 10, 1, 80);
    Will produce a noise effect with a palette of 4 randomly chosen colors
    There are 20 blend steps between each color
    A base scaling value of 20 will be applied with a range of 60 (max scale is 80)
    The speed is 10
    cMode is 1
    The effect updates at 80ms
    
    NoiseSL noiseSL(mainSegments, cybPnkPal_PS, 40, 5, 95, 20, 0, 80);
    Will produce a noise effect with using colors from cybPnkPal_PS
    There are 40 blend steps between each color
    A base scaling value of 5 will be applied with a range of 95 (max scale is 100)
    The speed is 20
    cMode is 0
    The effect updates at 80ms

Constructor inputs: 
    palette (optional, see constructors) -- A custom palette passed to the effect
    numColors (optional, see constructors) -- How many colors will be in the randomly created palette
    blendSteps -- Sets how many steps are used to blend between each color
                  Basically changes how fast the colors blend
                  Between 20 - 60 looks good
    scaleBase -- Sets how "zoomed-in" the noise is
                 (test it out yourself to see what I mean) (see inputs guide above)
    scaleRange -- The range for the variation of the scale (see inputs guide above)
    speed -- How fast the colors blend (higher is faster) (See inputs guide above)
    cMode -- The color mode of the effect (see inputs guide above)
    Rate -- The update rate (ms) note that this is synced with all the particles. 

Other settings:
    bgColorMode (default 0) -- Sets the color mode for blend color target (background) (see segDrawUtils::setPixelColor)
    hueCycle (default true) -- sets if the noise center will be offset over time (see inputs guide above)
    hue (default 0) -- The position of the noise center, is automatically adjusted if hueCycle is on
                       But you can also set if yourself to a specific value if needed (see inputs guide above)
                       Range is (0 - 255) for `cMode`'s 0, 2, 3, and (0 - <<palette length>> * blendSteps) for cMode 1.
    bgColorOrig (default 0) -- The default color of the background (bound to the bgColor pointer by default)
    *bgColor (default bound to bgColorOrig) -- The color of the background, is a pointer so it can be bound to an external variable

Functions:
    setupNoiseArray() -- Creates the array for storing the noise data (will be matrix of uint8_t's, numLines x numSegs)
                         Only call this if you change the segment set dimensions.
    update() -- updates the effect
    
*/
class NoiseSL : public EffectBasePS {
    public:
        //Constructor for randomly generated palette
        NoiseSL(SegmentSetPS &SegSet, uint8_t numColors, uint16_t BlendSteps, uint16_t ScaleBase,
                uint16_t ScaleRange, uint16_t Speed, uint8_t CMode, uint16_t Rate);

        //Constructor using palette
        NoiseSL(SegmentSetPS &SegSet, palettePS &Palette, uint16_t BlendSteps, uint16_t ScaleBase,
                uint16_t ScaleRange, uint16_t Speed, uint8_t CMode, uint16_t Rate);

        ~NoiseSL();

        uint8_t
            *noise = nullptr,
            cMode = 0,
            bgColorMode = 0;

        uint16_t
            blendSteps,
            hue = 0,
            speed,
            scaleBase,
            scaleRange;

        bool
            hueCycle = true;

        CRGB
            bgColorOrig = 0,          //default background color (blank)
            *bgColor = &bgColorOrig;  //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)

        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0};  //Must init structs w/ pointers set to null for safety

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
            numPointsMax = 0,  //used for tracking the memory size of the noise array
            colorIndex,
            scale,
            scaleTarget,
            x,
            y,
            z,
            noiseStart,
            noiseIndex,
            j_offset,
            i_offset;

        CRGB
            colorTarget,
            colorOut;

        void
            init(SegmentSetPS &SegSet, uint16_t Rate),
            fillNoise8(),
            mapNoiseSegsWithPalette(),
            setShiftScale();
};

#endif
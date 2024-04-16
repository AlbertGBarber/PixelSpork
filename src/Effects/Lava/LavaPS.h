#ifndef LavaPS_h
#define LavaPS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "ColorUtils/colorUtilsPS.h"

/* 
An effect based on the lava.ino code created by Scott Marley here: https://github.com/s-marley/FastLED-basics/tree/main/6.%20Noise/lava
See more about noise here: https://github.com/s-marley/FastLED-basics/tree/main/6.%20Noise
A fairly simple, but great looking effect that uses FastLED Perlin noise functions to produce a lava-esque effect
You can also supply your own palette for the effect to add your own colors, 
choose to have a palette of random color be created, or use the rainbow for the colors.

You can customize how the effect looks by adjusting the blendSteps and blendScale values
I encourage playing with these, since they can change the effect a lot.

The effect is 1D, so it always treats the strip as a single line. In my testing, 
the effect already tended to look 2D-like, so I saw no reason to expand it to be truly 2D. 

Note that by default, the effect uses the lavaPal_PS palette from paletteList.h.

Also note that this effect tends to look better at a higher refresh rate (lower rate),
which may be hard to hit depending on your setup.

For a similar effect with some more options, see NoiseSL.

Rainbow Mode:
    The effect also has a rainbow mode, where the rainbow is used for the noise colors. 
    This is turned on/off using rainbowMode.

hueCycle:
    Because the noise function values tend to fall in the center of the noise range, you tend not to see 
    the start/end colors of a palette/rainbow. 
    To fix this, you can set hueCycle to true, which cycles a hue offset over time,
    adjusting the center of the noise so you see all the colors.
    By default the offset fully cycles once a minute (in rainbow mode), but you can change the rate using hueRate.
    Note that hueRate is a pointer, by default it's bound to hueRateOrig, but you can bind it to an external var if needed.

    Note that if you want to change the offset, but don't want it to change over time 
    you can set the offset directly by setting "hue". Note that for the rainbow the range is 0-255,
    while for a palette the range is 0 to (<palette length> * blendSteps).

Example calls: 
    LavaPS lava(mainSegments, 30);
    Will do a lava effect, 
    using the default lavaPal_PS,
    with default blendSteps (30) and blendScale (80) values
    updating at 30ms

    LavaPS lava(mainSegments, 10, 20, 60);
    Will do a lava effect, with 20 blendSteps
    and a blendScale of 20
    at an update rate of 30ms

    LavaPS lava(mainSegments, cybPnkPal_PS, 10, 80, 60);
    Will do a lava effect using the cybPnkPal_PS palette for colors, 
    with 10 blendSteps and a blendScale of 80
    at an update rate of 60ms

    LavaPS lava(mainSegments, 3, 20, 80, 30);
    Will do a lava effect with a palette of 3 randomly chosen colors
    with 20 blendSteps and a blendScale of 80
    at an update rate of 30ms

Constructor inputs: 
    palette (optional, see constructors) -- A custom palette passed to the effect, is a pointer. 
                                            Will be bound to the lavaPal_PS if omitted.
    numColors (optional, see constructors) -- How many colors will be in the randomly created palette
    blendSteps (optional, default 30) -- Sets how many steps are used to blend between each color
                                         Basically changes how fast the colors blend
                                         Between 20 - 100 looks good.  
                                         The default is taken from the original code by Scott Marley.
    blendScale (optional, default 80) -- Sets how large the blended color areas are
                                         (test it out yourself to see what I mean)
                                         Between 10 - 100 looks good -> larger => smaller areas
                                        The default is taken from the original code by Scott Marley.
    rate -- The update rate (ms)

Other Settings:
    brightnessScale (default 150) -- Sets the noise range for the brightness
                                     It doesn't seem to change much in my testing 
    rainbowMode (default false) -- If true, rainbow colors will be used for the lava (see rainbow mode notes above)
    sat (default 255) -- Saturation value for rainbow mode
    val (default 255) -- "value" value for rainbow mode
    hueCycle (default false) -- If true, the center of the noise will shift over time (see hueCycle notes above)
    hueRateOrig (default 235) -- Default hue shifting time (ms), does a complete hue cycle ~every min for a rainbow
    *hueRate (default bound to hueRateOrig) -- The hue shifting time (ms). It's a pointer so you can bind it externally
    paletteTemp -- Storage for any randomly created palettes 
                   (will be bound to the effect palette if the random color constructor was used)

Functions:
    update() -- updates the effect 

Reference vars:
    hue -- The amount to offset the noise center by, (see hueCycle notes above)

*/
class LavaPS : public EffectBasePS {
    public:
        //Constructor for effect using default lava palette, blendSteps and blendScale
        LavaPS(SegmentSetPS &SegSet, uint16_t Rate);

        //Constructor for effect with default lava palette, but custom scale and steps
        LavaPS(SegmentSetPS &SegSet, uint16_t BlendSteps, uint16_t BlendScale, uint16_t Rate);

        //Constructor for effect using colors from palette
        LavaPS(SegmentSetPS &SegSet, palettePS &Palette, uint16_t BlendSteps, uint16_t BlendScale, uint16_t Rate);

        //constructor for a randomly created palette
        LavaPS(SegmentSetPS &SegSet, uint8_t numColors, uint16_t BlendSteps, uint16_t BlendScale, uint16_t Rate);

        ~LavaPS();

        uint8_t
            sat = 255,  //saturation value for rainbow mode
            val = 255;  //"value" value for rainbow mode

        uint16_t
            blendSteps = 30,
            blendScale = 80,
            brightnessScale = 150,
            hueRateOrig = 235,        //Default hue shifting time (ms), does a complete hue cycle ~every min (only relevant for rainbow mode)
            *hueRate = &hueRateOrig,  //The hue shifting time (ms), by default it's bound to hueRateOrig, but it's a pointer so you can bind it externally
            hue = 0;  //offset for center of the noise (see hueCycle notes in intro)

        bool
            hueCycle = false,
            rainbowMode = false;

        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0};  //Must init structs w/ pointers set to null for safety

        void
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0,
            prevHueTime = 0;

        uint8_t
            brightness;

        uint16_t
            pixelCount = 0,
            index,
            totBlendLength,
            totSegLen,
            numSegs,
            pixelNum;

        CRGB
            colorOut;

        void
            init(SegmentSetPS &SegSet, uint16_t Rate);
};

#endif
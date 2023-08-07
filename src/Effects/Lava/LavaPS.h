#ifndef LavaPS_h
#define LavaPS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "ColorUtils/colorUtilsPS.h"

/* 
An effect based on the lava.ino code created by Scott Marley here: https://github.com/s-marley/FastLED-basics/tree/main/6.%20Noise/lava
See more about noise here: https://github.com/s-marley/FastLED-basics/tree/main/6.%20Noise
A fairly simple, but great looking effect that uses FastLED noise functions to produce a lava-esque effect
You can also supply your own palette for the effect to add your own colors
or choose to have a palette of random color be created.

You can customize how the effect looks by adjusting the blendSteps and blendScale values
I encourage playing with these, since they can change the effect a lot.

Note that by default, the effect uses the lavaPal palette from paletteList.h.

For a similar effect with some more options, see NoiseSL.

Rainbow Mode:
    The effect also has a rainbow mode, where the rainbow is used for the noise colors. 
    This is turned on/off using rainbowMode.
    Because the noise function values tend to fall in the center of the noise range, the rainbow mode
    cycles a hue offset over time to help show all the rainbow colors. 
    By default the offset fully cycles once a minute, but you can change the rate using hueRate.
    Note that hueRate is a pointer, by default it's bound to hueRateOrig, but you can bind it to an external var if needed.

    Note that it's not possible to fully halt the hue offset change (it really looks best if it shifts).
    However, you can set the hueOffset manually, or set the hueRate to 65535, which will increase the hue once every 65 sec (very slow).

Example calls: 
    LavaPS lava(mainSegments, 10);
    Will do a lava effect, updating at 10ms

    LavaPS lava(mainSegments, 40, 20, 10);
    Will do a lava effect, with 40 blendSteps
    and a blendScale of 20 (see inputs for info below)
    at an update rate of 10ms

    LavaPS lava(mainSegments, cybPnkPal, 40, 80, 10);
    Will do a lava effect using cybPnkPal for colors, 
    with 40 blendSteps
    and a blendScale of 80
    at an update rate of 10ms

    LavaPS lava(mainSegments, 3, 40, 80, 10);
    Will do a lava effect with a palette of 3 randomly chosen colors
    with 40 blendSteps
    and a blendScale of 80
    at an update rate of 10ms

Constructor inputs: 
    palette (optional, see constructors) -- A custom palette passed to the effect, is a pointer. 
                                            Will be bound to the lavaPal if omitted.
    numColors (optional, see constructors) -- How many colors will be in the randomly created palette
    blendSteps (optional, see constructors) -- Sets how many steps are used to blend between each color
                                               Basically changes how fast the colors blend
                                               Between 20 - 100 looks good
    blendScale (optional, see constructors) -- Sets how large the areas that a blended are
                                               (test it out yourself to see what I mean)
                                               Between 10 - 100 looks good -> larger => smaller areas
    rate -- The update rate (ms)

Functions:
    update() -- updates the effect 

Other Settings:
    brightnessScale (default 150) -- Sets the noise range for the brightness
                                     It doesn't seem to change much in my testing 
    rainbowMode (default false) --  If true, rainbow colors will be used for the lava (see rainbow mode notes above)
    rainSat (default 255) -- Saturation value for rainbow mode
    rainVal (default 255) -- "value" value for rainbow mode
    hueRateOrig (default 235) -- Default hue shifting time (ms), does a complete hue cycle ~every min (only relevant for rainbow mode)
    *hueRate (default bound to hueRateOrig) -- The hue shifting time (ms). It's a pointer so you can bind it externally (only relevant for rainbow mode)
    paletteTemp -- Storage for any randomly created palettes 
                   (will be bound to the effect palette if the random color constructor was used)

Reference vars:
    hueOffset -- The amount of offset during rainbow mode (resets at 255) (see rainbow mode notes above)

*/
class LavaPS : public EffectBasePS {
    public:
        //Constructor for effect using default lava palette, blendSteps and blendScale
        LavaPS(SegmentSet &SegSet, uint16_t Rate);  

        //Constructor for effect with default lava palette, but custom scale and steps
        LavaPS(SegmentSet &SegSet, uint16_t BlendSteps, uint16_t BlendScale, uint16_t Rate);  

        //Constructor for effect using colors from palette
        LavaPS(SegmentSet &SegSet, palettePS &Palette, uint16_t BlendSteps, uint16_t BlendScale, uint16_t Rate);
        
        //constructor for a randomly created palette
        LavaPS(SegmentSet &SegSet, uint8_t numColors, uint16_t BlendSteps, uint16_t BlendScale, uint16_t Rate);
    
        ~LavaPS();

        SegmentSet 
            &SegSet; 
        
        uint8_t 
            rainSat = 255, //saturation value for rainbow mode
            rainVal = 255, //"value" value for rainbow mode
            hueOffset = 0; //offset for the hue in rainbow mode (helps keeps the colors)
        
        uint16_t
            blendSteps = 30,
            blendScale = 80,
            brightnessScale = 150,
            hueRateOrig = 235, //Default hue shifting time (ms), does a complete hue cycle ~every min (only relevant for rainbow mode)
            *hueRate = &hueRateOrig; //The hue shifting time (ms), by default it's bound to hueRateOrig, but it's a pointer so you can bind it externally
        
        bool
            rainbowMode = false;
        
        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0}; //Must init structs w/ pointers set to null for safety
            
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
            init(uint16_t Rate);
};

#endif
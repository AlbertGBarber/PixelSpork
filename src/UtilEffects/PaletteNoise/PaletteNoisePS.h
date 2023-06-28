#ifndef PaletteNoisePS_h
#define PaletteNoisePS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
//#include "MathUtils/mathUtilsPS.h"

/*
Effect Description:
Generates a palette where the colors vary over time using FastLED noise. 
You pick the size of the palette, a base hue, and how much the hue can vary by, and the util does the rest.
This allows you to create "themed or mood" palettes, such as a an ocean or forrest palette, where all the 
colors vary between different blues or greens. 
These palettes work well with a number of effects, notably Lava, RollingWaves, NoiseSL, Rain. 

The output palette is stored in the Util's noisePalette.

You can also set the palette to generate complimentary colors (dual, triad, tetrad, etc)
(although these don't stay fully complimentary all the time due to the noise variations)

The palette can also be set to vary its base hue over time, shifting the palette through the rainbow
while still generating noise based colors.
The rate change is tied to hueRate, which is a pointer, so it can be bound externally.

Note that you are free to change most of the Utils vars on the fly, with exception to the number of palette colors
You must call setupPalette(numColors) to change it.

Sometimes the palette colors can flicker at lower brightnesses, setting FastLED.setDither(0) can help with this.

Inputs Guide:
Getting the colors to vary in the way you want can be a little tricky.
The name of the game is finding a set of settings that give you a good variety of colors, 
but don't shift the hue too far away from your main color.
Overall you have five main settings for adjusting the colors:
    hueBase and hueMax: 
        The util generates colors using the FastLED HSV colors (hue, saturation, and value)
        You can read more about this color space online, but basically the "hue" determines what color you're at,
        while the saturation and value control how much white and black are mixed into the color. 
        FastLED maps the hue/sat/val range to 0 - 255 from the more common 0 - 360 range.
        A rough color to hue map is:
            Red/Orange: 240 to ~10 
            Orange/Yellow: 10 to ~40
            Yellow/Green: 40 to ~90
            Green/Blue: 90 to ~150
            Blue/Purple: 150 to ~180
            Purple/Red: 180 to 240
        So a color with hue 240 with a saturation of 100, and a value of 255 will be a pale red/pink
        (saturation and value use a reverse scale, so at 255 the value has no effect, while at 0, the color would be fully black)
        
        For the Util, the hueBase is the minimum hue the palette colors can have, while the hueMax is the maximum
        The util generates colors by treating the noise as a percentage of the hueMax, and then adding that to the hueBase
        ie hue = hueBase + map8(noiseData, 0, hueMax)
        In general, I recommend hueMax to between 30 - 100.
        Note that the noise tends to fall more towards the middle of the range, so you're baseHue + 1/2 * hueMax will be the most common color.
        Overall, larger hueMax's will generate a wider range of colors, but the colors will trend father from the baseHue
        so reds will become more orange, etc.

        If you want to have a tighter range of colors, I recommend tweaking the satMin and valMin, as explained below.

    satMin (default 220) and valMin (default 180):
        These values control how much white and black can be added to the hue color via noise (saturation and value). 
        Their effect is minimized at 255 and maximized at 0. 
        Like with the hueMax, satMin and valMin sets the lowest value they can be, ranging from satMin/valMin to 255. 
        The defaults are set to give modest changes in bright/darkness, and work best with a hueMax of 60+.
        If you want to have a tighter hue, but keep a good variety of colors, lower their values to the 100 - 150 range.
        Note that valMin is governed by its own noise function, to prevent it moving in tandem with the saturation.
        
    The right combo of hueMax, satMin, and valMin will probably take some experimentation to determine.

    compliment:
        A boolean, that if true, will set the palette colors to be compliments of the base hue, which means they will
        be equally spaced across the hue spectrum. This should generate color sets that are different but "look good"
        together.
        Note that the noise effects each palette color separately, so for larger hueMax's, the palette colors will not 
        stay fully complimentary as they shift. 

Example calls: 
    
    PaletteNoisePS paletteNoise(4, 160, 60, false, 15, 60);
    Will generate a palette with a 4 colors, using a base hue of 160 and a range of 60
    The colors will not be complimentary
    The noise changes with a speed scale of 15
    The hue will not shift over time (hueRate is defaulted to 0)
    The palette colors update at a rate of 60ms

    PaletteNoisePS paletteNoise(2, 0, 40, true, 8, 500, 60);
    Will generate a palette with a 2 colors, using a base hue of 0 and a range of 40
    The colors will be complimentary
    The noise changes with a speed scale of 8
    The hue will shift over time at a rate of 500ms
    The palette colors update at a rate of 60ms

    PaletteNoisePS paletteNoise(3, 130, 40, 150, 120, false, 10, 0, 60);
    These settings generate a nice ocean-like palette
    Will generate a palette with a 3 colors, using a base hue of 130 and a range of 40
    The saturation min and value min are set to 150 and 120 respectively
    The colors will not be complimentary
    The noise changes with a speed scale of 10
    The hue will not shift over time (hueRate is 0)
    The palette colors update at a rate of 60ms

Constructor Inputs:
    numColors -- How many colors in the output noise palette
    hueBase -- (See Inputs Guide above)
    hueMax -- (See Inputs Guide above)
    satMin (optional, default 220) -- (See Inputs Guide above)
    valMin (optional, default 220)  -- (See Inputs Guide above)
    compliment -- (See Inputs Guide above)
    blendSpeed -- The speed factor at which the palette colors change, 
                  recommend between 5 and 20, smaller is faster
    hueRate (optional, default 0, no shift) -- The rate of change of hueBase in ms (at 0 the hue won't change)
                                               Is a pointer, by default is bound to hueRateOrig
    rate -- The util update rate (ms)

Functions:
    setupPalette(numColors) -- Changes the number of colors in the palette. Note that this re-creates the noisePalette
                               so any external pointers to the noisePalette's colors will become invalid.

Other Settings:
    colorScale (default 200) -- How "zoomed in" the color noise is. 
                                At lower values the palette colors can be too similar, so we want this pretty high.
    briScale (default 100) -- Same as the colorScale, but for the HSV Value (darkness added)
                              we use separate a separate noise output for the Value to increase variation in colors.
    briSpeed (default 10) -- Same as the blendSpeed, but for the HSV Value (darkness added)
    hueRateOrig (default 0) -- The default address the hueRate points to

*/
class PaletteNoisePS : public EffectBasePS {
    public:
        //Constructor with hueRate of 0 and default satMin/valMin values
        PaletteNoisePS(uint8_t numColors, uint8_t HueBase, uint8_t HueMax, bool Compliment, uint16_t BlendSpeed, uint16_t Rate);  

        //Constructor with hueRate setting and default satMin/valMin values
        PaletteNoisePS(uint8_t numColors, uint8_t HueBase, uint8_t HueMax, bool Compliment, uint16_t BlendSpeed, 
                       uint16_t HueRate, uint16_t Rate);  

        //Constructor with saturation and value min settings and  hueRate setting
        PaletteNoisePS(uint8_t numColors, uint8_t HueBase, uint8_t HueMax, uint8_t SatMin, uint8_t ValMin, bool Compliment,
                       uint16_t BlendSpeed, uint16_t HueRate, uint16_t Rate); 

        ~PaletteNoisePS();
    
        uint8_t
            satMin = 220, //how light the colors get (higher is less bright) 150
            valMin = 180, //how dark the colors get (higher is less dark) 100
            hueBase,
            hueMax;
        
        uint16_t
            colorScale = 200,
            briScale = 100,
            blendSpeed, //higher -> slower blend
            briSpeed = 10;
        
        uint16_t
            hueRateOrig = 0, //Will be set to the inital hue rate as passed into constructors
            *hueRate = &hueRateOrig; //Is initially pointed to hueRateOrig, but can be pointed to an external variable if wanted
        
        bool
            compliment;

        palettePS //The output palette for the util
            noisePalette = {nullptr, 0}; //Must init structs w/ pointers set to null for safety, will be filled in during construction

        void
            setupPalette(uint8_t numColors),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0,
            prevHueTime = 0;
        
        uint8_t
            compStep,
            noiseData,
            noiseData2,
            hueAdj,
            sat,
            val;
        
        void
            init(uint16_t numColors, uint16_t HueRate, uint16_t Rate);
};

#endif
#ifndef PaletteNoisePS_h
#define PaletteNoisePS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
//#include "MathUtils/mathUtilsPS.h"

/*
Generates a palette where the colors vary over time using FastLED Perlin noise
(https://github.com/s-marley/FastLED-basics/tree/main/6.%20Noise). 
You pick the size of the palette, a base hue, and how much the hue can vary by, and the utility does the rest. 
This allows you to create "themed or mood" palettes, 
such as a an ocean or forest palette, where all the colors vary between different blues or greens. 

These palettes work well with a number of effects, notably Lava, RollingWaves, NoiseSL, Rain. 

See palettePS.h for more info on palettes.

The output palette of the utility is named `noisePalette`.

You can use it in your effects like:
    //Sets an effect's palette to your Palette Noise's output palette
    //Note that this assumes that the effect's "palette" var is a pointer (true in most effects)
    //So we need to pass the Palette Noise's palette by address (using the "&")
    yourEffect.palette = &yourPaletteNoise.noisePalette;

You can also pass `&yourPaletteNoise.noisePalette` as part of the effect's constructor.

Note that you are free to change most of the utility's settings on the fly, 
with exception to the number of noise palette colors, which you must call `setupPalette()` to change.

Sometimes the palette colors can flicker at lower brightness values, setting `FastLED.setDither(0)` can help with this.

    Other Features:
        * You can set the utility to generate a set of complimentary palette colors (dual, triad, tetrad, etc) 
          that vary over time, although the noise variations will force them in and out of being truly complimentary.

        * The utility can be set to vary the noise palette's base hue over time, 
          shifting the palette through the rainbow while still generating noise based colors. 
          The rate change is tied to `hueRate`, which is a pointer, 
          so it can be bound externally (like the utility's update rate).

Inputs Guide:
    Getting the colors to vary in the way you want can be a little tricky.
    The name of the game is finding a set of settings that give you a good variety of colors, 
    but don't shift the hue too far away from your main color.
    Overall you have five main settings for adjusting the colors:
        hueBase and hueMax: 
            The util generates colors using the FastLED HSV colors (hue, saturation, and value)
            You can read more about this color space online, but basically the "hue" what your "base" color is,
            while the saturation and value control how much white and black are mixed into the color. 
            FastLED maps the hue/sat/val range to 0 - 255 from the more common 0 - 360 range.
            A rough color to hue map is:
                Red/Orange: 240 to ~10 
                Orange/Yellow: 10 to ~40
                Yellow/Green: 40 to ~90
                Green/Blue: 90 to ~150
                Blue/Purple: 150 to ~180
                Purple/Red: 180 to 240
            So a color with a hue of 240, saturation of 100, and a value of 255 will be a pale red/pink
            (saturation and value use a reverse scale, so at 255 the value has no effect, 
            while at 0, the color would be fully black)
            
            For the Util, "hueBase" is the minimum hue the palette colors can have, while "hueMax" is the maximum
            The util generates colors by treating the noise as a percentage of "hueMax", and then adding that to "hueBase".
            ie "outputHue = hueBase + map8(noiseData, 0, hueMax);"
            In general, I recommend hueMax to between 30 - 100.
            Note that the noise tends to fall more towards the middle of the range, 
            so "baseHue + 1/2 * hueMax" will be the most common color.
            Overall, larger hueMax's will generate a wider range of colors, 
            but the colors will trend father from the baseHue.

            If you want to have a tighter range of colors, I recommend tweaking the satMin and valMin, as explained next.

        satMin (default 220) and valMin (default 180):
            These values (saturation and value) control how much white and black can be added to the hue color via noise. 
            Their effect is minimized at 255 and maximized at 0. 
            Like with "hueMax", "satMin" and "valMin" sets the lowest value they can be, 
            ranging from "satMin or valMin" to 255. 
            The defaults are set to give modest changes in bright/darkness, and work best with a hueMax of 60+.
            If you want to have a tighter hue, but keep a good variety of colors, 
            lower `satMin` and `valMin` to the 100 - 150 range.
            
            Note that `satMin` is fluctuated using the same noise as the palette colors, 
            while `valMin` has its own noise function. This helps prevent the values from moving in tandem, 
            producing a better variation of colors.
            
        The right combo of hueMax, satMin, and valMin will probably take some experimentation to determine.

        Other Inputs:
            Noise speed:
                The speed at which the palette colors shift is governed by `blendSpeed` and `briSpeed`. 
                `blendSpeed` controls how fast the hue and saturation change. This is a multiplier, 
                so there's not a good real-world approximation I can give you. 
                I recommend a speed between 5 and 20. Smaller values mean faster color changes.

                `briSpeed` controls how fast the HSV "value" changes. This is default to 10, 
                and is separate from the hue and saturation multiplier to help produce a better variation of colors.

            `colorScale` (default 200) and `briScale` (default 100):
                These settings control how large the noise color patches are, or how "zoomed-in" the Perlin noise is.`
                colorScale` applies to the hue and saturation noise function, while `briScale` 
                applies to the HSV "value" value. For both these, we want the noise pretty zoomed-out so that 
                colors blend over a decent range of pixels. 
                See FastLED's noise https://github.com/s-marley/FastLED-basics/tree/main/6.%20Noise notes for more info.

            `compliment` (bool):
                If true, will set the palette colors to be compliments of the base hue, 
                which means they will be equally spaced across the hue spectrum. 
                This should generate color that are different but "look good" together. 
                Note that the noise effects each palette color separately, so for larger `hueMax`'s, 
                the palette colors will not stay fully complimentary as they shift. 

Example calls:   
    PaletteNoisePS paletteNoise(4, 160, 60, false, 15, 0, 60);
    Will generate a palette of 4 colors, using a base hue of 160 and a range of 60
    The colors will not be complimentary
    The noise changes with a speed scale of 15
    The hue will not shift over time (hueRate is 0)
    The palette colors update at a rate of 60ms

    PaletteNoisePS paletteNoise(2, 0, 40, true, 8, 500, 60);
    Will generate a palette of 2 colors, using a base hue of 0 and a range of 40
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
    numColors -- The number of colors in the output noise palette. Can be changed later using `setupPalette()`.
    hueBase -- The base hue for the palette colors (See Inputs Guide above).
    hueMax -- The maximum variation for the palette hue (See Inputs Guide above).
    satMin (optional, default 220) -- The palette color's HSV "saturation" minimum (See Inputs Guide above).
    valMin (optional, default 180)  -- The palette color's HSV "value" minimum (See Inputs Guide above).
    compliment -- If true, the utility will generate complimentary colors for the noise palette (See Inputs Guide above).
    blendSpeed -- The speed factor at which the palette colors change, 
                  recommend between 5 and 20, smaller is faster (See Inputs Guide above).
    hueRate (0 for no shift) -- The rate of change of hueBase in ms (at 0 the hue won't change)
                                Is a pointer, by default is bound to hueRateOrig.
    rate -- The util update rate (ms)

Outputs:
    noisePalette -- The output palette for the noise colors.

Other Settings:
    colorScale (default 200) -- How "zoomed in" the color noise is (See Inputs Guide above).
    briScale (default 100) -- Same as the colorScale, but for the HSV Value (darkness added).
    briSpeed (default 10) -- Same as the blendSpeed, but for the HSV Value (darkness added). higher -> slower.
    hueRateOrig (default 0) -- The default address the hueRate points to.
    active (default true) -- If false, the utility will be disabled (updates() will be ignored)

Functions:
    setupPalette(numColors) -- Changes the number of colors in the palette. 
                               Note that if the new length is longer than the noise palette's 
                               maximum length (the longest it has been while the utility is running), 
                               the noise palette's memory will be re-allocated.
    update() -- updates the utility.

*/
class PaletteNoisePS : public EffectBasePS {
    public:
        //Constructor with hueRate setting and default satMin/valMin values
        PaletteNoisePS(uint8_t numColors, uint8_t HueBase, uint8_t HueMax, bool Compliment, uint16_t BlendSpeed,
                       uint16_t HueRate, uint16_t Rate);

        //Constructor with saturation and value min settings
        PaletteNoisePS(uint8_t numColors, uint8_t HueBase, uint8_t HueMax, uint8_t SatMin, uint8_t ValMin, bool Compliment,
                       uint16_t BlendSpeed, uint16_t HueRate, uint16_t Rate);

        ~PaletteNoisePS();

        uint8_t
            satMin = 220,  //how light the colors get (higher is less bright) 150
            valMin = 180,  //how dark the colors get (higher is less dark) 100
            hueBase,
            hueMax;

        uint16_t
            colorScale = 200,
            briScale = 100,
            blendSpeed,  //higher -> slower blend
            briSpeed = 10;

        uint16_t
            hueRateOrig = 0,          //Will be set to the inital hue rate as passed into constructors
            *hueRate = &hueRateOrig;  //Is initially pointed to hueRateOrig, but can be pointed to an external variable if wanted

        bool
            compliment;

        palettePS                         //The output palette for the util
            noisePalette = {nullptr, 0};  //Must init structs w/ pointers set to null for safety, will be filled in during construction

        void
            setupPalette(uint8_t numColors),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0,
            prevHueTime = 0;

        uint8_t
            paletteLenMax = 0,
            compStep,
            noiseData,
            noiseData2,
            hueAdj,
            sat,
            val;

        void
            init(uint16_t numColors, uint16_t HueRate, uint16_t Rate),
            getNoisePalColors();
};

#endif
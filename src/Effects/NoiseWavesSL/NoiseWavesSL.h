#ifndef NoiseWavesSL_h
#define NoiseWavesSL_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

/*
Uses FastLED Perlin noise to produce varying colors with waves of brightness that both move and 
grow/shrink across the segment lines with time.
There are various setting to control waves and colors, explained in the Inputs guide below.
The effect requires a palette for colors, or can generate one randomly for you.

The noise colors will be blended towards a background color. 
Changing the background color will "tint" the overall output, 
because all the colors are blended towards the background color as part of the effect.
Supports Color Modes for the background color only.

You can change the variables freely, although this may result in jumps in the effect.

Inputs guide:
    Overall the effect has two components:
        1) Waves of varying brightness that shift randomly across the segment lines while growing and shrinking with time
        2) Spots of palette colors that grow and change with time
    
    Brightness wave settings: 
        The brightness wave is generated using a cos() where the frequency and phase are set based on noise functions
        The scale settings determine how fast the waves move and how large they are
        1) phaseScale: Changes how fast the waves move. Must be greater than 0. Recommend between 3 - 15.
                       Note that because the waves a noise based, their speed varies over time. phaseScale sets a cap on how 
                       fast they move. Higher is slower. The effect has diminishing returns at higher values.
        2) freqScale: Sets how long the waves are. Must be greater than 0. Recommend between 3 - 10.
                      The waves will vary in size over time, but this sets a minimum they will reach.
                      Higher means longer waves.
        
    Color change settings: 
        1) blendScale: Sets how "zoomed-in" the color noise is. Ie it determines how large the patches of color will be
                       Recommend values 10 - 60+. Higher values produce smaller patches.
        2) blendSpeed: How fast the colors blend with time. Must be greater than 0.
                       This is not a constructor setting, and is defaulted to 10. Lower values will make faster blends
        3) blendSteps: How many steps there are between colors. 
                       This is not a constructor setting since the change to the effect isn't very noticeable 
                       unless below a certain value, which causes the colors to become coarse.
                       Defaulted to 30.

    Cycling Colors:
        For FastLED noise, most of the noise output falls in the center of its range. 
        This means that for a given palette, you tend to only see the colors in the middle of the palette. 
        To counteract this, the noise center can be offset over time, cycling colors into view. 
        The end result is more pretty than without imo.

        The offset cycling is controlled by the settings below. Note that the cycling is on by default:

*       1) hue: Offsets the center of the FastLED noise, allowing you to shift the "center" color of the palette. 
                By default it is incremented during every update cycle, but can also be set to a fixed value 
                with cycling turned off. 

        2) hueCycle (default true): Controls the hue offset cycling.
                                    When off (false) the hue offset will be fixed to its current value.
                    

Example calls: 

    NoiseWavesSL noiseWaves(mainSegments, 3, 0, 30, 8, 3, 80);
    Will produce an effect using 3 random colors
    The background is blank
    The blend scale is 30, while the phaseScale is 8 and the freqScale is 3
    The produces small waves that shift more slowly, with larger color patches.
    (even at the slowest the waves still move pretty quick)
    The effect updates at 80ms
    
    NoiseWavesSL noiseWaves(mainSegments, cybPnkPal_PS, 0, 10, 2, 9, 80);
    Will produce an effect using colors from cybPnkPal_PS
    The background is blank
    The blend scale is 10, while the phaseScale is 2 and the freqScale is 9
    The produces large waves that shift quickly
    The effect updates at 40ms

Constructor inputs: 
    palette (optional, see constructors) -- A custom palette passed to the effect
    numColors (optional, see constructors) -- How many colors will be in the randomly created palette
    bgColor --  The color of the background pixels
    blendScale -- Sets how "zoomed-in" the noise is. (See Inputs Guide above)
    phaseScale -- Changes how fast the waves move. Must be greater than 0. (See Inputs Guide above)
    freqScale -- Sets how long the waves are. Must be greater than 0. (See Inputs Guide above)
    rate -- The update rate (ms)

Other Settings:
    bgColorMode (default 0) -- sets the color mode for the spacing pixels (see segDrawUtils::setPixelColor)
    blendSpeed (default 10) -- How fast the colors blend with time. Must be greater than 0. Lower => faster.
    blendSteps (default 30) -- How many steps there are between colors. Doesn't really change anything in the effect
                               unless you make it very small.
    hueCycle (default true) -- Controls the hue offset cycling. (See Inputs Guide above).
    hue (default 0) -- Offsets the center of the FastLED noise, allowing you to shift the "center" color of the palette.
                       (See Inputs Guide above).

Functions:
    update() -- updates the effect 
*/
class NoiseWavesSL : public EffectBasePS {
    public:
        //Constructor with palette
        NoiseWavesSL(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, uint16_t BlendScale,
                     uint8_t PhaseScale, uint8_t FreqScale, uint16_t Rate);

        //Constructor with randomly generated palette
        NoiseWavesSL(SegmentSetPS &SegSet, uint8_t numColors, CRGB BgColor, uint16_t BlendScale,
                     uint8_t PhaseScale, uint8_t FreqScale, uint16_t Rate);

        ~NoiseWavesSL();

        uint8_t
            bgColorMode = 0,
            phaseScale,
            freqScale,
            blendSpeed = 10,
            hue = 0; //Offsets the center of the noise to shift palette colors, called hue for consistency with other effects

        uint16_t
            blendScale,
            blendSteps = 30;
        
        bool 
            hueCycle = true;

        CRGB
            bgColorOrig,
            *bgColor = nullptr;  //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)

        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0};  //Must init structs w/ pointers set to null for safety

        void
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0;

        uint8_t
            freqCounter,
            noisePhase,
            index,
            bri;

        uint16_t
            numLines,
            numSegs,
            colorIndex,
            pixelNum,
            totBlendLength;

        CRGB
            colorTarget,
            colorOut;

        void
            init(CRGB BgColor, SegmentSetPS &SegSet, uint16_t Rate);
};

#endif
#ifndef PaletteSliderPS_h
#define PaletteSliderPS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

/* 
Lets say you have a palette that contains a bunch of colors ot match a theme, like christmas, halloween, etc
You also have an effect that you'd like to use the palette colors in, but may be not all the colors at once
Like maybe you'd only like to have two colors at a time, such as red/green for christmas, and then cycle through pairs of colors
That's what this utility is intended to do
You feed it a palette with all your colors, a pattern of how you want the colors to blend, and
the number of colors you want at once, and the utility will create a palette that cycles through your input palette
colors according to the pattern.
So for the above example, you'd pass in your christmas palette, and the length of the output palette: 2.
(we'll assume that the pattern matches the palette, see constructors below)
and the utility would make a palette that cycles through two colors at a time.

There are two modes of shifting through colors, using a single shift, and shifting the whole palette length:
    When single shifting:
        The palette colors all follow the same pattern, with each one being offset from the others
        For example, if I had a palette of {red, green, blue, orange} and a slider palette of length 2, 
        and the pattern is just the palette in order
        The first set of slider palette colors would be {red, green} transitioning to {green, blue} 
        then to {blue, orange}, etc. So each color follows the one after it.
    When shifting the whole palette length:
        The palette colors transition as a full group, with each color targeting the color that is a palette length away from it
        Using the same example palettes from the single shifting:
        The slider palette would transition as {red, green} goes to {blue, orange} goes back to {red, green}
        The colors transition in pairs
        If the slider palette were length 3, it would go {red, green, blue} -> {orange, red, green} -> {blue, orange, red}
        The target color is 3 away from the starting color.
You can switch which mode you're using at any time.

You can also set the palette to pause for a time after each transition by setting the pauseTime var (in ms)

The output palette of the utility is "sliderPalette". Pass it to your effects to use it.

You cannot change the length of the slider palette without resetting it
This is because it needs to be reallocated in memory
If you do want to change the length on the fly, initialize it as something large, and then 
manually reduce it's length by setting its length property.

The update rate is a pointer, and can be bound externally, like in other effects.

Example calls: 
    PaletteSliderPS paletteSlider(cybPnkPal, 2, 20, true, 80);
    Creates a slider palette of length 2, using cybPnkPal as the target palette
    The pattern will match the cybPnkPal in order.
    Each transition will take 20 blend steps
    The colors will be cycled in singles (see single shift explanation above)
    The blend will update at 80ms

    uint8_t pattern_arr = {0, 1, 3, 2};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr)};
    PaletteSliderPS paletteSlider(cybPnkPal, pattern, 2, 20, false, 80);
    Creates a slider palette of length 2, using cybPnkPal as the target palette
    The pattern will match the pattern above.
    Each transition will take 20 blend steps
    The colors will be cycled in whole palette lengths (see single shift explanation above)
    so the palette will blend from {0, 1} to {3, 2} and back again
    The blend will update at 80ms

Constructor Inputs:
    pattern(optional, see constructors) -- The pattern of the colors, created using indexes from the palette
                                           (see patternPS.h)                                     
    paletteTarget -- The repository of colors used by the pattern, or can be used as the pattern itself
    sliderPalLen -- The length of the output sliderPalette (how many colors it has)
    blendSteps -- The total number of steps taken to blend between the palette colors.
    singleShift -- Sets if the colors will cycle using single shifts, or shift by the whole palette
                   (see single shift explanation above)
    rate -- The update rate of the blend (ms)

Functions:
    setPaletteAsPattern() -- Sets the effect pattern to match the current palette.
    makeSliderPalette(paletteLength) -- Creates a new slider palette of the specified length, also resets the utility
    reset() -- Restarts the effect
    update() -- updates the effect

Other Settings:
    pauseTime (default 0) -- Sets a time (ms) that the sliderPalette will be pause for after finishing a transition before starting the next

Reference Vars:
    patternIndex -- How many full palette blends we've done, resets every time we've finished the pattern

Flags:
    paused -- Set when the effect is paused after transitioning between palettes indexes
              Is reset to false once the pause time has passed

*/
class PaletteSliderPS : public EffectBasePS {
    public:
        PaletteSliderPS(palettePS &PaletteTarget, patternPS &Pattern, uint16_t SliderPalLen, uint16_t BlendSteps, bool SingleShift, uint16_t Rate);  

        PaletteSliderPS(palettePS &PaletteTarget, uint16_t SliderPalLen, uint16_t BlendSteps, bool SingleShift, uint16_t Rate);

        ~PaletteSliderPS();

        uint16_t
            blendSteps,
            pauseTime = 0,
            patternIndex = 0; //How many full palette blends we've done, for reference only
        
        bool
            paused = false,
            singleShift;
        
        CRGB 
            *sliderPalColArr = nullptr; //for reference only. Storage array for the slider palette colors

        palettePS
            *paletteTarget = nullptr, //The palette being slid across
            sliderPalette = {nullptr, 0}; //The output palette of the utility, is filled in during the inital update cycle
        
        patternPS
            *pattern = nullptr,
            patternTemp = {nullptr, 0}; //used when using the palette as the pattern, init as empty
 
        void 
            setPaletteAsPattern(),
            makeSliderPalette(uint16_t paletteLength),
            reset(),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0,
            pauseStartTime;
        
        uint8_t 
            ratio,
            startIndex,
            endIndex;
        
        uint16_t
            patternStep,
            sliderPalLen,
            blendStep = 0;
        
        CRGB 
            startColor,
            endColor;
        
        void
            init(uint16_t Rate);
};

#endif
#ifndef PaletteSliderPS_h
#define PaletteSliderPS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

/* 
Lets say you have a palette that contains a bunch of themed colors, like christmas, halloween, etc
You also have an effect that you'd like to use the palette colors in, but may be not all the colors at once
Maybe you'd only like to have two colors at a time, such as red/green for christmas, and then cycle through pairs of colors.
This utility automates blending between sets of colors in a palette.
You feed it a palette with all your colors, a pattern of the order you want the colors to blend, and
the number of colors you want at once in the output palette, 
and the utility will create a palette that cycles through your input palette colors according to the pattern.

So for the above example, you'd pass in your christmas palette, and the length of the output palette: 2.
(we'll assume that the pattern matches the palette)
and the utility would make a palette that cycles through two colors at a time in order.

The output palette of the utility is "sliderPalette". Pass it to your effects to use it.

There are two modes of shifting through colors, using a single shift, and shifting the whole palette length:
    When single shifting:
        The palette colors all follow the same pattern, with each one being offset from the others
        For example, if I had a palette of {red, green, blue, orange} and a slider palette of length 2, 
        and the pattern is just the palette in order
        The first set of slider palette colors would be {red, green} transitioning to {green, blue} 
        then to {blue, orange}, etc. So each color follows the one after it.
        Think of it like you're sliding a window along the palette, each color moves into the window, 
        and out of it one at a time.
    When shifting the whole palette length:
        The palette colors transition as a full group, with each color targeting the color that is a palette length away from it
        Using the same example palettes from the single shifting:
        The slider palette would transition as {red, green} goes to {blue, orange} goes back to {red, green},
        transitioning in pairs.
        If the slider palette were length 3, it would go {red, green, blue} -> {orange, red, green} -> {blue, orange, red}
        The target color is 3 away from the starting color.
You can switch which mode you're using at any time.

You can have the blending pause either at the beginning or end of each blend, with a configurable
"pause time". When paused, the palette will not be blended until the pause ends.

You cannot change the length of the slider palette without resetting it because it needs to be reallocated in memory.
If you do want to change the length on the fly, initialize it as something large, and then 
manually reduce it's length by setting its length property.

The update rate is a pointer, and can be bound externally, like in other effects.

Example calls: 
    PaletteSliderPS paletteSlider(cybPnkPal_PS, 2, true, false, 20, 80);
    Creates a slider palette of length 2, using cybPnkPal_PS as the target palette.
    The pattern will match the cybPnkPal_PS in order.
    The colors will be cycled in singles (see single shift explanation above).
    The utility will pause the blend after each blend is finished (startPaused is false).
    (also pauseTime is default to 0, so they'll be no pause).
    Each blend will take 20 blend steps with 80ms between each step.

    uint8_t pattern_arr = {0, 1, 3, 2};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};

    PaletteSliderPS paletteSlider(cybPnkPal_PS, pattern, 2, false, true, 20, 80);
    paletteSlider.setPauseTime(6000); //Sets the pause time between each palette blend in ms.

    Creates a slider palette of length 2, using cybPnkPal_PS as the target palette
    The pattern will match the "pattern" above.
    The colors will be cycled in whole palette lengths (see single shift explanation above)
    so the palette will blend from {0, 1} to {3, 2} and back again.
    The utility will start paused (startPaused is true), and pause for 6000ms, before blending. 
    Each blend will take 20 blend steps with 80ms between each step.

Constructor Inputs:
    pattern(optional, see constructors) -- The pattern of the colors, created using indexes from the palette
                                           (see patternPS.h)                                     
    paletteTarget -- The repository of colors used by the pattern, or can be used as the pattern itself.
    sliderPalLen -- The length of the output sliderPalette (how many colors it has).
    blendSteps -- The total number of steps taken to blend between the palette colors.
    singleShift -- Sets if the colors will cycle using single shifts, or shift by the whole palette
                   (see single shift explanation above).
    startPaused -- If true, then the blend cycle will start paused, blocking the initial blend for pauseTime time. 
                   Allows you to use the inital palette colors for a time and then start blending. 
    rate -- The update rate of the blend (ms).

Functions:
    setPaletteAsPattern() -- Sets the effect pattern to match the current palette.
    makeSliderPalette(paletteLength) -- Creates a new slider palette of the specified length, also resets the utility.
    reset() -- Restarts the effect.
    update() -- updates the effect.

Other Settings:
    pauseTime (default 0) -- Sets a time (ms) that the sliderPalette will be pause 
                             for after finishing a blend before starting the next.

Reference Vars:
    patternIndex -- What index we're at in the pattern, for reference only. 
    cycleNum -- How many full palette blends we've done, resets every "numCycles" cycles.
                (ie once all possible blend palettes have been cycled through).
    numCycles -- How many blends the palette will go through before repeating 
                 (not always the same as just the pattern length!).   
                 It is re-calculated after each blend, so if you change the palette length, singleShift, 
                 or the pattern length, it will be changed.

Flags:
    paused -- Set when the effect is paused after transitioning between palettes indexes.
              Is reset to false once the pause time has passed.
    firstPauseDone -- Set true when the first pause is finished (probably only useful if "startPaused" is true).
                      Is reset whenever reset() is called.

*/
class PaletteSliderPS : public EffectBasePS {
    public:
        PaletteSliderPS(palettePS &PaletteTarget, patternPS &Pattern, uint16_t SliderPalLen, bool SingleShift,
                        bool StartPaused, uint16_t BlendSteps, uint16_t Rate);

        PaletteSliderPS(palettePS &PaletteTarget, uint16_t SliderPalLen, bool SingleShift, bool StartPaused,
                        uint16_t BlendSteps, uint16_t Rate);

        ~PaletteSliderPS();

        uint16_t
            blendSteps,
            pauseTime = 0,
            patternIndex = 0,  //what index we're at in the pattern, for reference only
            numCycles,         //How many blends the palette will go through before repeating
            cycleNum = 0;      //How many full palette blends we've done, resets every "numCycles" cycles, for reference only

        bool
            paused = false,
            startPaused,
            pauseDone = false,
            firstPauseDone = false,  //set once the first pause is done (probably only useful if "startPaused" is true)
            singleShift;

        CRGB
            *sliderPalColArr = nullptr;  //for reference only. Storage array for the slider palette colors

        palettePS
            *paletteTarget = nullptr,      //The palette being slid across
            sliderPalette = {nullptr, 0};  //The output palette of the utility, is filled in during the inital update cycle

        patternPS
            *pattern = nullptr,
            patternTemp = {nullptr, 0, 0};  //used when using the palette as the pattern, init as empty

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
            sliderPalLenMax = 0,
            sliderPalLen,
            ratio = 0,
            startIndex,
            endIndex;

        uint16_t
            patternStep,
            blendStep = 0;

        CRGB
            startColor,
            endColor;

        void
            init(uint16_t Rate),
            blendPaletteColors(),
            setNumCycles();
};

#endif
#ifndef PaletteSliderPS_h
#define PaletteSliderPS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

/* 
A utility that takes an input palette and produces an output palette that is a "window" of the input. 
The window shifts across the input palette over time, smoothly blending between its colors. 
For example, lets say you have a palette that contains a bunch of themed colors, 
like for Christmas, Halloween, etc. You also have an effect that you'd like to use with the palette, 
but not use all the colors at once. 
Maybe you'd only like to have two colors at a time, such as red/green for Christmas, 
and then cycle through pairs of colors. 
This utility allows you to do that by creating an output palette that shifts through sections of the input over time.

To set up the utility, you feed it a input palette (see palettePS.h), 
a pattern (see patternPS.h) with order you want the colors to blend (this can be created automatically), 
and the number of colors you want in the output palette, 
and the utility will create a palette that cycles through your input palette colors according to the pattern.

The output palette of the utility is named `sliderPalette`.

You can use it in your effects like:
    //Sets an effect's palette to your palette slider's output palette
    //Note that this assumes that the effect's "palette" var is a pointer (true in most effects)
    //So we need to pass the palette slider's palette by address (using the "&")
    yourEffect.palette = &yourPaletteSlider.sliderPalette;

You can also pass `yourPaletteSlider.sliderPalette` as part of the effect's constructor.

    Color Shifting Modes:
        There are two modes of shifting through palette colors, using a "single shift", and shifting the whole palette length (controlled by `singleShift`):
            * When single shifting:
                The palette colors all follow the same pattern, with each one being offset from the others. 
                For example, if I had a palette of {red, green, blue, orange} 
                and a slider palette of length 2 (assume the pattern is just the palette in order). 
                The first set of slider palette colors would be {red, green} 
                transitioning to {green, blue} then to {blue, orange}, etc. 
                So each color follows the one after it. 
                
                Think of it like you're sliding a window along the palette, 
                each color moves into the window, and out of it one at a time.

            * When shifting the whole palette length:
                The palette colors transition as a full group, with each color targeting the color that is a 
                palette length away from it. Using the same example palettes from the single shifting: 
                the slider palette would transition as {red, green} to {blue, orange} then back to {red, green}, 
                transitioning in pairs. 
                If the slider palette were length 3, it would go 
                {red, green, blue} -> {orange, red, green} -> {blue, orange, red}. 
                The target color is 3 away from the starting color.
                
                Think of this like you're taking snapshots of the palette, 
                picking a new set of colors for each snapshot.

        Note that you can switch between shifting modes freely during run time.

    Other Notes:
        * The utility always loops, wrapping around the input palette.

        * You can use a custom pattern to "re-order" your input palette's colors. 
          For example, with a palette of { red, green, blue } and a pattern of {0, 2, 1} 
          the utility would treat the palette as being { red, blue, green }. 
          Note that the pattern is optional, if it is omitted, a pattern matching the order of the 
          palette will be created for you.

        * You can have the blending pause either at the beginning or end of each blend, 
          with a configurable pause time, `pauseTime`. When paused, the palettes will not be blended, 
          giving you time to reflect/absorb the current palette (and effect) colors, before cycling to a new palette. 

        * You cannot change the length of the slider palette without resetting the utility 
          (it needs have its memory checked and possibly reallocated). 
          If you do need to change the length on the fly, you can initialize the palette with the highest 
          length you want, and then manually reduce its `length`. This ensures that your palette always 
          has enough memory. You'll also need to call `setPaletteAsPattern()` or manually adjust your pattern. 
          Note that this is an advanced and possibly dangerous technique, 
          you should read the 
          https://github.com/AlbertGBarber/PixelSpork/wiki/Effects-Advanced#managing-dynamic-memory-and-fragmentation 
          wiki entry first.

Example calls: 
    PaletteSliderPS paletteSlider(cybPnkPal_PS, 2, true, false, 0, 20, 80);
    Creates a slider palette of length 2, using the cybPnkPal_PS palette as the target palette.
    The pattern will match the order of the cybPnkPal_PS palette.
    The colors will be cycled in singles (see single shift explanation above).
    The utility will pause the blend after each blend is finished (startPaused is false).
    The pause time is 0ms.
    Each blend will take 20 blend steps with 80ms between each step.

    uint8_t pattern_arr = {0, 2, 1};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};

    PaletteSliderPS paletteSlider(cybPnkPal_PS, pattern, 2, false, true, 3000, 20, 80);
    Creates a slider palette of length 2, using the cybPnkPal_PS palette as the target palette.
    The pattern will match the "pattern" above.
    The colors will be cycled in whole palette lengths (see single shift explanation above)
    so the palette will blend from {0, 2} to {1, 0} to {2, 1} back to {0, 2}
    The utility will start paused (startPaused is true), and pause for 3000ms, before blending. 
    Each blend will take 20 blend steps with 80ms between each step.

Constructor Inputs:                                 
    inputPalette -- The palette that the utility will blend through.
    pattern(optional, see constructors) -- The order the utility will use of the input palette's colors (See notes in intro).
    sliderPalLen -- The length of the output palette, `sliderPalette`. Ie, how many colors will have.
    singleShift -- If true, the palette colors will cycle using single shifts, 
                   otherwise they will shift by the whole palette 
                   (see "Color Shifting Modes" above).
    startPaused -- If true, then the blend cycle will start paused, blocking the initial blend for pauseTime time. 
                   Allows you to use the initial palette colors for a time and then start blending. 
    pauseTime -- Sets a time (ms) that the palette blending will be pause for after finishing a blend.
    blendSteps -- The total number of steps taken to blend between the palette colors.
    rate -- The update rate of the blend (ms).

Outputs:
    *sliderPalette -- The output palette for the color blend.

Other Settings:
    active (default true) -- If false, the utility will be disabled (updates() will be ignored).
                            
Functions:
    setPaletteAsPattern() -- Sets the palette color pattern to match the input palette.
    makeSliderPalette(paletteLength) -- Changes the length of the slider palette, also `reset()`'s the utility. 
                                        Note that if the new length is longer than the silder palette's 
                                        maximum length (the longest it has been while the utility is running), 
                                        the slider palette's memory will be re-allocated.
    reset() -- Restarts the utility.
    update() -- updates the utility.

Reference Vars:
    patternIndex -- What index we're at in the pattern, changes by 1 if singleShift is true, 
                    and by the slider palette length if false.
     for reference only. 
    cycleNum -- How many full palette blends we've done, resets every "cycleLen" cycles.
                (ie once all possible blend palettes have been cycled through).
    cycleLen -- How many blends the palette will go through before repeating 
                 (not always the same as the pattern length!).   
                 It is re-calculated after each blend, so if you change the palette length, singleShift, 
                 or the pattern length, it will be changed.

Flags:
    paused -- Set true when the utility is paused. Is reset to false once the pause time has passed.
    firstPauseDone -- Set true when the first pause is finished (probably only useful if "startPaused" is true).
                      Is reset to false whenever reset() is called.

*/
class PaletteSliderPS : public EffectBasePS {
    public:
        //Constructor with Pattern
        PaletteSliderPS(palettePS &InputPalette, patternPS &Pattern, uint16_t SliderPalLen, bool SingleShift,
                        bool StartPaused, uint16_t PauseTime, uint16_t BlendSteps, uint16_t Rate);

        //Constructor with just palette (the pattern will match the palette)
        PaletteSliderPS(palettePS &InputPalette, uint16_t SliderPalLen, bool SingleShift, bool StartPaused,
                        uint16_t PauseTime, uint16_t BlendSteps, uint16_t Rate);

        ~PaletteSliderPS();

        uint16_t
            blendSteps,
            pauseTime = 0,
            patternIndex = 0,  //what index we're at in the pattern, for reference only
            cycleLen,         //How many blends the palette will go through before repeating
            cycleNum = 0;      //How many full palette blends we've done, resets every "cycleLen" cycles, for reference only

        bool
            paused = false,
            startPaused,
            pauseDone = false,
            firstPauseDone = false,  //set once the first pause is done (probably only useful if "startPaused" is true)
            singleShift;

        CRGB
            *sliderPalColArr = nullptr;  //for reference only. Storage array for the slider palette colors

        palettePS
            *inputPalette = nullptr,      //The palette being slid across
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
#ifndef PaletteBlenderPS_h
#define PaletteBlenderPS_h

#include "Include_Lists/PaletteFiles.h"
#include "Effects/EffectBasePS.h"
#include "MathUtils/mathUtilsPS.h"

/* 
A utility that takes a starting and ending palette and produces a palette that is blended over a set time from 
the start towards the end palette. You also have options for looping the blend so that it repeats 
and randomizing the palette colors for each loop. 
This allows you to produce an endless set of blending colors, which you can then plug into effects. 

See the "Palettes and Util Classes" code example for a working usage of the utility.

See https://github.com/AlbertGBarber/PixelSpork/wiki/Palette-Basics for info on palettes. 
Also see the PaletteCyclePS.h utility for blending through a set of palettes.

The utility's blended output palette is stored locally as 'blendPalette'. 

You can use it in your effects like:

    //Sets an effect's palette to your palette blender's output palette
    //Note that this assumes that the effect's "palette" var is a pointer (true in most effects)
    //So we need to pass the palette blender's palette by address (using the "&")
    yourEffect.palette = &yourPaletteBlender.blendPalette;

You can also pass `&yourPaletteBlender.blendPalette` as part of the effect's constructor.

    Feature Notes:
        * **Overall, I recommend only blending between palettes of equal lengths.** 
          The length of the blended output palette will be the length of the longest input palette. 
          For example, if I blend from a palette of 5 colors into a palette with 3, 
          the output palette will have 5 colors. This makes it easier when looping and helps maintain 
          compatibility with effects. Note however, that this means that sometimes the output 
          palette will have repeats of colors. Consider the previous example, 
          since our output palette will 5 colors, but the ending palette only has 3, 
          two of the end palette colors will be repeated in the output. 
          The repeats will follow the order of the ending palette, so it shouldn't be noticeable with most effects.

        * The blend can be set to looped (set using `looped`). 
          When looping, once the blend palette has reached the current ending palette, 
          the utility swaps the starting and ending palette and resets itself, setting itself 
          to blend back to the original starting palette. 
          For example, if I had a two single color palettes of red and blue, when looping, 
          the output palette would blend from red to blue, back to red, then back to blue, etc.

        * When looping, the start/end palettes can be set (using `randomize`) 
          to have their colors randomized each cycle. Note that this permanently changes the 
          colors of the input start and end palette, so be sure you aren't using the 
          palettes in other effects. If you do randomize, you may want to call `reset()` just 
          before you first update the utility. 
          This will refill the blended output palette's colors, and 
          can help prevent "jumps" where an input palette changes after the utility is setup, 
          but before it starts updating. 

        * You can have the blend pause either at the beginning or end of the blend, 
          with a configurable pause time, `pauseTime`. This is mainly used when looping. 
          When paused, the palettes will not be blended, giving you time to reflect/absorb the 
          current palette (and effect) colors, before cycling to a new palette. 

    Other Notes:

        * The start and end palettes are not modified by the utility unless `randomize` is active.

        * When looping, if you're not starting paused, you can use `step == 0` to track when the blend cycle restarts. 
          If you are starting paused, you can use `(step == 0) && !paused && !firstPauseDone` instead. 
          This can be useful if you want to trigger effect changes when once the utility has finished a blend.

        * If you have set an external variable pointing to one of the colors in the `blendPalette` 
          (such as a background for an effect) be warned that the `blendPalette` is allocated dynamically. 
          If the `blendPalette` is ever re-sized and re-allocated, any pointers to the palette will break, 
          and need to be re-bound. However, the palette is only re-allocated if its size needs to change 
          for a new blend (ie new start/end palettes are set that don't have the same max size as the previous pair), 
          which you control.

        * The blend palette is allocated dynamically. Be aware of memory fragmentation. 
          See (https://github.com/AlbertGBarber/PixelSpork/wiki/Effects-Advanced#managing-dynamic-memory-and-fragmentation
          for more details. 

Example calls: 
    PaletteBlenderPS PaletteBlender(startPalette, endPalette, false, 50, 100);
    Blends from the start palette to the end palette once (looping is false),  
    using 50 steps, with 100ms between each step
    Note, you'll have to create your own start and end palettes.

    PaletteBlenderPS PaletteBlender(startPalette, endPalette, true, true, false, 0, 50, 100);
    Blends from the start palette to the end palette once, 
    looping and randomizing the palettes with each loop cycle.
    (looping and randomize are true).
    The utility will pause for 0ms after every cycle 
    (startPaused is false, and pauseTime is 0)
    Uses 50 steps, with 100ms between each step.
    Will produce a palette of ever-changing colors.
    Note, you'll have to create your own start and end palettes.

Constructor Inputs:
    startPalette -- The palette of colors the output blend palette will start as. 
                    Can be changed later using a `reset()` function (see Functions below).
    endPalette -- The palette of colors the output blend palette will end as.
                  Can be changed later using a `reset()` function (see Functions below).
    looped -- If set true, the utility will reset and restart once a blend has ended, 
              switching the start and end palettes (see note on looping in intro).
    randomize (optional, default false) --  If true, will randomize the colors of the end palette.
                                            Combine this with looped to produce constantly changing palettes.
                                            Note that this will permanently modify the end palette 
                                            so make sure you aren't using it elsewhere! 
    pauseTime (optional, default 0) -- Sets a time (ms) that the blendPalette will be pause at after finishing 
                                       a transition before starting the next
                                       Only relevant if looping. (See notes in intro).
    startPaused (optional default false) -- If true, then the utility will pause before the blend begins
                                            instead of after it is over. 
                                            Allows you to use the starting palette for a time
                                            before it is blended to the final palette. Mainly used when looping. 
                                            The inital pause will be triggered on the first update().             
    totalSteps (max 255) -- The total number of steps taken to blend between the palettes
    rate -- The update rate of the blend (ms)

Output Vars:
   blendPalette -- The blended palette created by the utility. See notes in intro on how to use it in effects. 

Other Settings:
    compliment (default false) -- Only relevant when randomizing -
                                  If true, randomized palettes will only generate complimentary colors, 
                                  which means they will be equally spaced across the hue spectrum (see the HSV color space). 
                                  This should generate color sets that are different but "look good" together.
    active (default true) -- If false, the utility will be disabled (updates() will be ignored).
                                  
Functions:
    reset() -- Restarts the blend (all settings and palettes stay the same)
    reset(StartPalette, EndPalette) -- Restarts the blend with a new start/end palette, with the same steps and update rate
    reset(StartPalette, EndPalette, TotalSteps, Rate) -- Restarts the blend with all new settings.
    setupBlendPalette(blendPaletteLength) -- Creates the blend palette, used only by PaletteCycle, not for general use!
    update() -- updates the utility.

Reference Vars:
    step -- The current blend step. See "Other Notes" above about using this to track the palette blend.

Flags:
    blendEnd -- Set true when the blend has finished, causes a pause to begin.
    paused -- Set true when the effect is paused. Is reset to false once the pause is over.
    pauseDone -- Set true when a pause is finished. If looping, it resets with each loop.

*/
class PaletteBlenderPS : public EffectBasePS {
    public:
        //Base constructor
        PaletteBlenderPS(palettePS &StartPalette, palettePS &EndPalette, bool Looped, uint8_t TotalSteps,
                         uint16_t Rate);

        //Constructor including randomize and pausing options
        PaletteBlenderPS(palettePS &StartPalette, palettePS &EndPalette, bool Looped, bool Randomize, bool StartPaused, 
                         uint16_t PauseTime, uint8_t TotalSteps, uint16_t Rate);

        ~PaletteBlenderPS();

        uint8_t
            totalSteps;  //The total number of steps taken to blend between the palettes, you can change this on the fly

        uint16_t
            step,  //the current step, mainly for reference, do not set manually
            pauseTime = 0;

        bool
            looped,
            randomize = false,
            compliment = false,
            paused = false, //set true while paused
            startPaused = false,
            pauseDone = false, //Set true when a pause is finished, if looping, resets with each loop, or with reset()
            blendEnd = false; //set when the blend is finished

        CRGB
            *blendPalette_arr = nullptr;

        palettePS
            blendPalette = {nullptr, 0};  //the output palette from the blend (is filled in when the class is constructed)

        palettePS
            *startPalette = nullptr,
            *endPalette = nullptr;

        void
            reset(palettePS &StartPalette, palettePS &EndPalette),  //resets just the colors (also starts the blend again)
            reset(palettePS &StartPalette, palettePS &EndPalette, uint8_t TotalSteps, uint16_t Rate),  //resets all vars
            reset(), //resets the loop vars, restarting the blend from the beginning
            setupBlendPalette(uint8_t blendPaletteLength),   //used only by PaletteCycle, not for general use!
            update();

    private:
        unsigned long
            currentTime,
            pauseStartTime = 0,
            prevTime = 0;

        uint8_t
            blendPaletteMaxLen = 0,
            ratio = 0;

        CRGB
            startColor,
            endColor,
            newColor;

        palettePS
            *palTempPtr = nullptr;
        
        void
            blendPaletteColors(),
            loopPalettes();
};

#endif
#ifndef PaletteCyclePS_h
#define PaletteCyclePS_h

#include "Effects/EffectBasePS.h"
#include "UtilEffects/PaletteBlender/PaletteBlenderPS.h"
#include "Palette_Stuff/palettePS.h"

/* 
A utility that takes an array of palettes and blends from one palette to the next over time. 
An automated, multi palette version of the PaletteBlenderPS.h utility. 
Like Palette Blender, the cycle can be set to loop and also randomize the palette colors as it loops. 

To create a palette array, you'll need to use a `paletteSet`. See palette.h for info.

The utility's blended output palette is stored locally as 'cyclePalette'. 
Its length and number of colors will be equal to the longest palette in the cycle palette array.

'cyclePalette' is a pointer, so passing it to effects is a little different than a normal palette:
    //Sets an effect's palette to your palette blender's output palette
    //Note that this assumes that the effect's "palette" var is a pointer (true in most effects)
    yourEffect.palette = yourPaletteCycle.cyclePalette;

You can also pass `*yourPaletteCycle.cyclePalette` as part of the effect's constructor.

    Feature Notes:
        * _**Overall, I recommend only cycling between palettes of equal lengths.**_ 
          The length of the blended output palette will be the length of the longest input palette. 
          For example, if I blend from a palette of 5 colors into a palette with 3, the output palette 
          will have 5 colors. This makes it easier when looping and helps maintain compatibility with effects. 
          Note however, that this means that sometimes the output palette will have repeats of colors. 
          Consider the previous example, since our output palette will 5 colors, but the ending palette only 
          has 3, two of the end palette colors will be repeated in the output. The repeats will follow the order 
          of the ending palette, so it shouldn't be noticeable with most effects.

        * The palette cycle can be set to loop (using `looped`). When looping, once the cycle has reached the 
          final palette, it will blend back to the starting palette, repeating the cycle.

        * Instead of following the palette array in order, the utility can be set (using `shuffle`) 
          to choose the next palette from the array at random. The chosen palette will never be the same as 
          the current palette (for obvious reasons!). 

        * The palettes can be set (using `randomize`) to have their colors randomized each cycle. 
          Note that this permanently changes the colors of the cycle array palettes, 
          so be sure you aren't using the palettes in other effects. 
          If you do randomize, you may want to call `reset()` just before you first update the utility. 
          This will refill the blended output palette's colors, and can help prevent "jumps" 
          where an input palette changes after the utility is setup, but before it starts updating. 

        * You can have the blend pause either at the beginning or end of each palette blend, 
          with a configurable pause time, `pauseTime`. This is mainly used when looping. 
          When paused, the palettes will not be blended, giving you time to reflect/absorb 
          the current palette (and effect) colors, before cycling to a new palette. 

    Other Notes:
        * The utility uses an instance of the PaletteBlenderPS.h utility to do the blends, 
          automatically swapping palettes and resetting it in the background for the cycle. 
          The instance is created dynamically as part of the utility's construction. 
          It is public and can be accessed as `PB`. Some utility settings must be changed 
          in the blender instance directly. I mostly provide pass-through functions for 
          changing these settings, but any missing settings can be accessed like: 
          `<your PaletteCycle instance>.PB->someSetting`.

        * The utility's `cyclePalette` is pointed to the palette blender instance's `blendPalette`.

        * Any quirks from the Palette Blender will also apply to this utility, 
          see the blender's "Other Notes" entry.

Example calls: 
    To declare palette set, use the paletteSet struct (see palettePS.h):
    palettePS *paletteArr[] = { &palette1, &palette2, etc};
    paletteSetPS paletteSet = {paletteArr, SIZE(paletteArr), SIZE(paletteArr)};
    Note you'll have to create your own palette1 and palette2

    PaletteCyclePS paletteCycle(paletteSet, true, false, false, false, 0, 50, 80);
    Blends between each palette in the array in order, looping back to the first palette at the end
    The palettes are not randomized or shuffled,
    the blend does not start paused, and the pause time is 0ms
    each blend takes 50 steps, with 80ms between each step

    PaletteCyclePS paletteCycle(paletteSet, true, false, true, true, 3000, 50, 80);
    Blends between each palette in the array in order, looping back to the first palette at the end
    The palettes are not randomized, but are shuffled.
    The palette blending will start paused, with a pause time of 3000ms,
    so the initial palette will be constant until 3000ms have passed from the first update.
    each blend takes 50 steps, with 80ms between each step

Constructor Inputs:
    *paletteSet -- The pointer to the Palette Set used for the blends.
    looped -- If true, will reset the blend cycle once it has ended, cycling back to the starting palette.
    randomize -- If true, will randomize the next palette in the cycle,
                 note that this will permanently modify palettes, so make sure you aren't using them elsewhere!
                 Combine this with looped, to produce constantly changing palettes.
    shuffle -- If true, a random palette from the set will be chosen for each blend (will not be the same as the current palette)
               When not looping, the number of palettes blended will still be the number of palettes in the set.
               Works, with randomize, but isn't really useful.
    startPaused -- If true, then the blend cycle will start paused, blocking the first palette blend for pauseTime time.
                   Note that this setting is a member of the Palette Blender instance and not in the cycle utility itself. 
                   (see paletteBlenderPS for more) 
    pauseTime -- The time (ms) that the utility will pause for between blends. Can be changed later using `setPauseTime()`. 
                 Note that this setting is a member of the Palette Blender instance and not in the utility itself.
    totalSteps (max 255) -- The total number of steps taken to blend between the palettes
    rate -- The update rate of the blend (ms)

Output Vars:
   *cyclePalette -- The blended palette created by the utility. See notes in intro on how to use it in effects. 

Other Settings:
    compliment (default false) -- Only relevant when randomizing -
                                  If true, randomized palettes will only generate complimentary colors, 
                                  which means they will be equally spaced across the hue spectrum (see the HSV color space). 
                                  This should generate color sets that are different but "look good" together.
    active (default true) -- If false, the utility will be disabled (updates() will be ignored).
    *PB -- The PaletteBlendPS instance. (See Other Notes above).

Functions:
    reset() -- Restarts the blend cycle (all settings and palettes stay the same)
    reset(&newPaletteSet) -- Restarts the blend cycle with a new palette set, but with the same steps and update rate
    setTotalSteps(newTotalSteps) -- Changes the number of blend steps. (set in the Palette Blender instance)
    setPauseTime(newPauseTime) -- Changes the pause time (ms) between blends (set in the Palette Blender instance)
    setStartPaused(bool newStartPaused) -- Changes the "startPaused" setting in the Palette Blender instance, see PaletteBlenderPS.
    update() -- Updates the utility.
                                      
Reference Vars:
    cycleNum -- How many palette blends have been completed. Must be reset manually by calling reset();

Flags:
    done -- Set true when the utility has blended through all the palettes in the palette set. Not set when looping.
*/
class PaletteCyclePS : public EffectBasePS {
    public:
        PaletteCyclePS(paletteSetPS &PaletteSet, bool Looped, bool Randomize, bool Shuffle, bool StartPaused, 
                       uint16_t PauseTime, uint8_t TotalSteps, uint16_t Rate);

        ~PaletteCyclePS();

        uint8_t
            cycleNum = 0;  //for reference

        bool
            randomize,
            shuffle,
            compliment = false,
            done = false,
            looped;

        paletteSetPS
            *paletteSet = nullptr;

        palettePS
            *cyclePalette = nullptr;  //ouput palette

        PaletteBlenderPS
            *PB = nullptr;  //PaletteBlenderPS instance

        void
            reset(),
            reset(paletteSetPS &newPaletteSet),
            setTotalSteps(uint8_t newTotalSteps),
            setPauseTime(uint16_t newPauseTime),
            setStartPaused(bool newStartPaused),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0;

        uint8_t
            paletteSetLen,
            maxPaletteLength = 0,
            indexGuess,
            nextIndex,
            currentIndex = 0;
};

#endif
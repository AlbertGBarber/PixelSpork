#ifndef PaletteCyclePS_h
#define PaletteCyclePS_h

#include "Effects/EffectBasePS.h"
#include "UtilEffects/PaletteBlender/PaletteBlenderPS.h"
#include "Palette_Stuff/palettePS.h"

/* 
Takes an array of palettes and blends from one palette to the next
the palettes are blended at the passed in rate (ms) in the passed in totalSteps steps
the resulting palette can be accessed as "cyclePalette", it will be the length of the longest of the palettes in the array

The blend can be looped (so it repeats), or just set to happen once. 

You can have the blend pause either at the beginning or end of the blend, with a configurable
"pause time". This is mainly used when looping. When paused, the palettes will not be blended.

When looping, the palettes can be set to be randomized, which will blend to a random palette each cycle.

Create your array of palettes using a paletteSet (see palettePS.h)

Pass cyclePalette to your effects to use the blended palette. Note that it is a pointer (unlike in other palette utilities)
So passing it will look slightly different, usually *<your PaletteCyclePS instance>.cyclePalette should work.

The PaletteCyclePS update rate is a pointer, and can be bound externally, like in other effects

The utility uses an instance of paletteBlenderPS to do the blends. 
The instance is public and can be accessed as "PB". This is the only way to get certain settings.
ie "<your PaletteCycle instance>.PB->totalSteps" fetches the total number of blend steps.  
Note that the utility includes some functions for setting the more common paletteBlender settings.

Notes:
    To make sure this works well with various effects, we manually set the length of the blendPalette to be the same each cycle
    by pre-determining the longest palette in the palette set and setting the blendPalette's length to match
    (the length usually would change depending on the lengths of the start and end palettes. which might change each cycle)
    This should be ok because palettes wrap. So the blendPalette might have some repeated colors, but will still
    be the correct blend overall. 

    This produces a palette with a consistent length, that should play nice will all effects.

    One downside of this is that with some effects, your colors may not be represented equally.
    For example, with 2 palettes one length 2 and one length 3, the blended palette will be length 3.
    But when blending to the palette with 2 colors, the blend palette will be <<Color 1, Color 2, Color 1>>
    In some effects this will lead to color 1 being twice as prevalent as color 2. 

    The other downside is that you're always doing the maximum number of blend calculations even if your palettes are much smaller than the max
    For example, we have a 5, 2, and 3 length palettes in the set
    The maximum length is 5, so our cycle/blendPalette will be length 5, even when we're blending between the 2 and 3 length palettes
    The calculations for the 3 and 2 blends will be done 5 times, instead of the 3 if we setup the blend normally.

    Most palettes aren't very long, so it shouldn't be a huge issue, but something to be aware of!

    The PaletteBlendPS instance (PB) is public, but don't mess with it unless you know what you're doing!

Example calls: 
    To declare palette set, use the paletteSet struct (see palettePS.h):
    palettePS *paletteArr[] = { &palette1, &palette2, etc};
    paletteSetPS paletteSet = {paletteArr, SIZE(paletteArr), SIZE(paletteArr)};
    Note you'll have to create your own palette1 and palette2

    PaletteCyclePS paletteCycle(paletteSet, true, false, false, false, 50, 80);
    Blends between each palette in the array in order, looping back to the first palette at the end
    The palettes are not randomized or shuffled, and the blend does not start paused
    each blend takes 50 steps, with 80ms between each step

    PaletteCyclePS paletteCycle(paletteSet, true, true, false, true, 50, 80);
    paletteCycle.setPauseTime(6000); //Sets the pause time between each palette blend in ms.
    Blends between each palette in the array in order, looping back to the first palette at the end
    The palettes are randomized, but not shuffled.
    The palette blending will start paused, with a pause time of 3000ms,
    so the initial palette will be constant until 3000ms have passed from the first update.
    each blend takes 50 steps, with 80ms between each step

Constructor Inputs:
    *paletteSet -- The pointer to the set of palettes used for the blends
    looped -- If true, it will reset the blend once it has ended, cycling back to the start palette
    randomize -- If true, will randomize the next palette in the cycle,
                 note that this will permanently modify palettes, so make sure you aren't using them elsewhere!
                 Combine this with looped, to produce constantly changing palettes.
    shuffle -- If true, a random palette from the set will be chosen for each blend (will not be the same as the current palette)
               If looping is false, the number of palettes blended will still be the number of palettes in the set.
               Works, with randomize, but isn't really useful.
    startPaused -- If true, then the blend cycle will start paused, blocking the first palette blend for pauseTime time. 
                   (see paletteBlenderPS for more)
    totalSteps (max 255) -- The total number of steps taken to blend between the palettes
    rate -- The update rate of the blend (ms)

Functions:
    reset(&newPaletteSet) -- Restarts the blend with a new palette set, with the same steps and update rate
    reset() -- Restarts the blend (all settings and palettes stay the same)
    setTotalSteps(newTotalSteps) -- changes the total number of steps used in the blends (set in the PB instance, see PaletteBlenderPS)
    setPauseTime(newPauseTime) -- Changes the pause between blends (set in the PB instance, see PaletteBlenderPS)
    setStartPaused(bool newStartPaused) -- Changes the "startPaused" setting in the PB instance, see PaletteBlenderPS.
    update() -- updates the effect

Other Settings:
    compliment (default false) -- Only relevant when randomizing -
                                  If true, randomized palettes will only generate complimentary colors, 
                                  which means they will be equally spaced across the hue spectrum (see the HSV color space). 
                                  This should generate color sets that are different but "look good" together.
                                     
Reference Vars:
    cycleNum -- How many palette blends have been finished. Must be reset manually by calling reset();

Flags:
    done -- Set when we've blended through all the palettes in the palette array (if not looping)
*/
class PaletteCyclePS : public EffectBasePS {
    public:
        PaletteCyclePS(paletteSetPS &PaletteSet, bool Looped, bool RandomizePal, bool Shuffle, bool StartPaused, 
                       uint8_t TotalSteps, uint16_t Rate);

        ~PaletteCyclePS();

        uint8_t
            cycleNum = 0;  //for reference

        bool
            randomizePal,
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
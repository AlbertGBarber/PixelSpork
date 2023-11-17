#ifndef PaletteBlenderPS_h
#define PaletteBlenderPS_h

#include "Include_Lists/PaletteFiles.h"
#include "Effects/EffectBasePS.h"
#include "MathUtils/mathUtilsPS.h"

/* 
Takes two palettes, a start and and end one, and gives you a palette that is blended over time from the start towards the end
the palette is blended at the passed in rate (ms) in the passed in totalSteps steps
the resulting palette can be accessed as "blendPalette", it will be the length of the longest of the passed in palettes
The blend can be looped (so it repeats), or just set to happen once.

You can have the blend pause either at the beginning or end of the blend, with a configurable
"pause time". This is mainly used when looping. When paused, the palettes will not be blended.

When looping, the palettes can be set to be randomized, which will blend to a random palette each cycle.
If you do randomize, you may want to call reset() when you first start to use the utility, which will
set the blended palette colors up for the inital cycle. This can help prevent "jumps" where an input palette 
changes after the utility is setup, but before it starts updating. 

Pass "blendPalette" to your effects to use the blended palette.

The PaletteBlenderPS update rate is a pointer, and can be bound externally, like in other effects.

Notes:
    The passed in start and end palettes are not modified by the blend unless randomize is active.

    When looping, if your're not starting paused, you can use "step == 0" to track when the cycle restarts.
    If you are starting paused, you can use "(step == 0) && !paused && !firstPauseDone" instead.

    If you have set an external variable pointing to one of the colors in the blendPalette (such as a bgColor for an effect)
    be warned that the blandPalette is allocated on the fly using new
    It is only re-allocated if it's size needs to change for a new blend 
    (ie new start/end palettes are set that don't have the same max size as the previous pair)
    So if you do set new palettes, either make sure they have the same max size a the previous pair,
    or re-bind you variable's pointer.

Example calls: 
    PaletteBlenderPS PaletteBlender(startPalette, endPalette, false, 50, 100);
    Blends from the start palette to the end palette once (no looping), using 50 steps, with 100ms between each step
    Note, you'll have to create your own start and end palettes

    PaletteBlenderPS PaletteBlender(startPalette, endPalette, true, true, 50, 100);
    Blends from the start palette to the end palette once, looping and randomizing the palettes with each loop cycle.
    Uses 50 steps, with 100ms between each step.
    Will produce a palette of ever-changing colors.
    Note, you'll have to create your own start and end palettes

Constructor Inputs:
    startPalette -- The palette the blendPalette will start as
    endPalette -- The palette that the blendPalette will end as
    looped -- set true, it will reset the blend once it has ended, switching the start and end palettes
    randomize (optional, default false) --  Randomize will randomize the end palette.
                                            Combine this with looped to produce constantly changing palettes.
                                            Note that this will permanently modify the end palette 
                                            so make sure you aren't using it elsewhere!              
    totalSteps (max 255) -- The total number of steps taken to blend between the palettes
    rate -- The update rate of the blend (ms)

Functions:
    reset(StartPalette, EndPalette) -- Restarts the blend with a new start/end palette, with the same steps and update rate
    reset(StartPalette, EndPalette, TotalSteps, Rate) -- Restarts the blend with all new vars
    reset() -- restarts the blend (all settings and palettes stay the same)
    setupBlendPalette(blendPaletteLength) -- //creates a blend palette, used only by PaletteCycle, not for general use!
    update() -- updates the effect

Other Settings:
    pauseTime (default 0) -- Sets a time (ms) that the blendPalette will be pause at after finishing a transition before starting the next
                             Only relevant if looping.
    startPaused (default false) -- If true, then the utility will pause before the blend begins
                                   instead of after it is over. Allows you to use the starting palette for a time
                                   before it is blended to the final palette. Mainly used when looping. 
                                   The inital pause will be triggered on the first update().
    compliment (default false) -- Only relevant when randomizing -
                                  If true, randomized palettes will only generate complimentary colors, 
                                  which means they will be equally spaced across the hue spectrum (see the HSV color space). 
                                  This should generate color sets that are different but "look good" together.

Flags:
    blendEnd -- Set when the blend has ended, causes the pause to start
    paused -- Set when the effect is paused after transitioning between palettes
              Is reset to false once the pause time has passed
    pauseDone -- Set true when a pause is finished, if looping, 
                      resets with each loop and whenever reset() is called

*/
class PaletteBlenderPS : public EffectBasePS {
    public:
        //Base constructor
        PaletteBlenderPS(palettePS &StartPalette, palettePS &EndPalette, bool Looped, uint8_t TotalSteps,
                         uint16_t Rate);

        //Constructor including a randomize option
        PaletteBlenderPS(palettePS &StartPalette, palettePS &EndPalette, bool Looped, bool Randomize, 
                         uint8_t TotalSteps, uint16_t Rate);

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
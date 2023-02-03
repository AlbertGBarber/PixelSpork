#ifndef PaletteCyclePS_h
#define PaletteCyclePS_h

#include "Effects/EffectBasePS.h"
#include "UtilEffects/PaletteBlender/PaletteBlenderPS.h"

//NEEDS TESTING!!!!

//TODO:
//-- Add a shuffle function, so the next palette is choosen at random
//-- Add direction controls?

//struct used to hold a palette array and it's length in one spot
//also provides code for getting palette objects
struct paletteSetPS {
    palettePS **paletteArr = nullptr;
    uint8_t length;

    palettePS *getPalette(uint8_t index){
        return paletteArr[ mod8(index, length) ];
    };
};

/* Takes an array of palettes and blends from one palette to the next
the palettes are blended at the passed in rate (ms) in the passed in totalSteps steps
the resulting palette can be accessed as "cyclePalette", it will be the length of the longest of the palettes in the array
The blend can be looped (so it repeats), or just set to happen once
A hold time can be set, that will pause the the cycle once a blend has finished before starting the next
While looping, the palettes can be set to be randomized, which will blend to a random palette each time

pass cyclePalette to your effects to use the blended palette 

The PaletteCyclePS update rate is a pointer, and can be bound externally, like in other effects

Uses an instance of paletteBlenderPS to do the blends. 
The instance is public and can be accessed as "PB"

The hold time and totalSteps are variables of the PaletteBlenderPS instance
so they use setter functions as shown below

Example calls: 
    to declare palette array, use paletteSet struct (above):
    palettePS *paletteArr[] = { &palette1, &palette2, etc};
    paletteSetPS paletteSet = {paletteArr, SIZE(paletteArr)};

    to declare a paletteCycle
    PaletteCyclePS(&paletteSet, true, 50, 80);
    Blends between each palette in the array in order, looping back to the first palette at the end
    each blend takes 50 steps, with 80ms between each step

Constructor Inputs:
    *paletteSet -- The pointer to the set of palettes used for the blends
    looped -- set true, it will reset the blend once it has ended, cycling back to the start palette
    totalSteps (max 255) -- The total number of steps taken to blend between the palettes
    rate -- The update rate of the blend (ms)

Functions:
    reset(*newPaletteSet) -- Restarts the blend with a new palette set, with the same steps and update rate
    reset() -- Restarts the blend (all settings and palettes stay the same)
    setTotalSteps(newTotalSteps) -- changes the total number of steps used in the blends (set in the PB instance, see PaletteBlenderPS)
    getTotalSteps() -- Returns the number of steps being used for the blend (set in the PB instance, see PaletteBlenderPS)
    setHoldTime(newHoldTime) -- Changes the pause between blends (set in the PB instance, see PaletteBlenderPS)
    update() -- updates the effect

Other Settings:
    randomize (default false) --  Will randomize the each palette in the cycle, every cycle
                                  note that this will permanently modify palettes
                                  so make sure you aren't using them elsewhere!
                                  Combine this with looped, to produce constantly changing palettes

Flags:
    cycleEnd -- Set when we've blended through all the palettes in the palette array 

Reference Vars:
    nextIndex -- The index of the palette we're shifting towards in the paletteSet (will reset to 0 if looped, once all the palettes have been cycled)
    currentIndex -- The index of the palette we're shifting from in the paletteSet

Notes:
To make sure this works well with various effects, we manually set the length of the blendPalette to be the same each cycle
by pre-determining the longest palette in the palette set and setting the blendPalette's length to match
(the length usually would change depending on the lengths of the start and end palettes. which might change each cycle)
This should be ok because palettes wrap. So the blendPalette might have some repeated colors, but will still
be the correct blend overall. 

This produces a palette with a consistent length, that should play nice will all effects

The downside is that you're always doing the maximum number of blend calculations even if your palettes are much smaller than the max
for example, we have a 5, 2, and 3 length palette in the set
The maximum length is 5, so our cycle/blendPalette will be length 5, even when we're blending between the 2 and 3 length palettes
The caculations for the 3 and 2 blends will be done 5 times, instead of the 3 if we setup the blend normally

Most palettes aren't very long, so it shouldn't be a huge issue, but something to be aware of!

the PaletteBlendPS instance (PB) is public, but don't mess with it unless you know what you're doing 
*/
class PaletteCyclePS : public EffectBasePS {
    public:
        PaletteCyclePS(paletteSetPS *PaletteSet, bool Looped, uint8_t TotalSteps, uint16_t Rate);  

        ~PaletteCyclePS();
        
        uint8_t
            nextIndex, //For reference only!!
            currentIndex = 0, //For reference only!!
            getTotalSteps();

        bool 
            cycleEnd = false,
            randomize = false,
            looped = false;
        
        paletteSetPS
            *paletteSet = nullptr;
                                 
        palettePS
            *cyclePalette = nullptr; //ouput palette
        
        PaletteBlenderPS
            *PB = nullptr; //PaletteBlenderPS instance

        void 
            reset(),
            reset(paletteSetPS *newPaletteSet),
            setTotalSteps(uint8_t newTotalSteps),
            setHoldTime(uint16_t newHoldTime),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        uint8_t
            paletteLength,
            maxPaletteLength = 0;
};

#endif
#ifndef PaletteBlenderPS_h
#define PaletteBlenderPS_h

//TODO:
//-- Add new blend version that lets you exclude some indexes from blending 
//   (for randomizing when you want a const color), forced same length palettes, excluding colors at end?

#include "PaletteFiles.h"
#include "Effects/EffectBasePS.h"
#include "MathUtils/mathUtilsPS.h"

/* Takes two palettes, a start and and end one, and gives you a palette that is blended over time from the start towards the end
the palette is blended at the passed in rate (ms) in the passed in totalSteps steps
the resulting palette can be accessed as "blendPalette", it will be the length of the longest of the passed in palettes
The blend can be looped (so it repeats), or just set to happen once
A hold time can be set, that will pause the loop once a blend is finished
While looping, the palettes can be set to be randomized, which will blend to a random palette each time

pass blendPalette to your effects to use the blended palette 

The PaletteBlenderPS update rate is a pointer, and can be bound externally, like in other effects

Example calls: 
    PaletteBlenderPS(startPalette, endPalette, false, 50, 100);
    Blends from the start palette to the end palette once (no looping), using 50 steps, with 100ms between each step

Constructor Inputs:
    startPalette -- The palette the blendPalette will start as
    endPalette -- The palette that the blendPalette will end as
    Looped -- set true, it will reset the blend once it has ended, switching the start and end palettes
    TotalSteps (max 255) -- The total number of steps taken to blend between the palettes
    Rate -- The update rate of the blend (ms)

Functions:
    reset(StartPalette, EndPalette) -- Restarts the blend with a new start/end palette, with the same steps and update rate
    reset(StartPalette, EndPalette, TotalSteps, Rate) -- Restarts the blend with all new vars
    reset() -- restarts the blend (all settings and palettes stay the same)
    setupBlendPalette(blendPaletteLength) -- //creates a blend palette, used only by PaletteCycle, not for general use!
    update() -- updates the effect

Other Settings:
    holdTime (default 0) -- Sets a time (ms) that the blendPalette will be pause at after finishing a transiton before starting the next
                           Only relevant if looped is true
    randomize (default false) -- //randomize will randomize the end palette note that this will permanently modify the end palette
                                  so make sure you aren't using it elsewhere!
                                  Combine this with looped, to produce constantly changing palettes

Flags:
    blendEnd -- Set when the blend has ended, causes the hold to start
    holdActive -- Set when the class is in the holding time after transitioning between palettes
                 Is reset to false once the hold time has passed

Notes:
The passed in start and end palettes are not modified by the blend unless randomize is active

If you have set an external variable pointing to one of the colors in the blendPalette (such as a bgColor for an effect)
be warned that the blandPalette is allocated on the fly using new
It is only re-allocated if it's size needs to change for a new blend 
(ie new start/end palettes are set that don't have the same max size as the previous pair)
So if you do set new palettes, either make sure they have the same max size a the previous pair,
or re-bind you variable's pointer
*/
class PaletteBlenderPS : public EffectBasePS {
    public:
        PaletteBlenderPS(palettePS *StartPalette, palettePS *EndPalette, bool looped, uint8_t TotalSteps, uint16_t Rate);
        
        ~PaletteBlenderPS();

        uint8_t
            step, //the current step, mainly for refrence, not intended to be manually set
            totalSteps; //The total number of steps taken to blend between the palettes, you can change this on the fly
        
        uint16_t 
            holdTime = 0;
        
        bool
            looped,
            randomize = false,
            holdActive = false,
            blendEnd = false;
        
        palettePS
            blendPalette; //the output palette from the blend
        
        palettePS
            *startPalette = nullptr,
            *endPalette = nullptr;
        
        void
            reset(palettePS *StartPalette, palettePS *EndPalette), //resets just the colors (also starts the blend again)
            reset(palettePS *StartPalette, palettePS *EndPalette, uint8_t TotalSteps, uint16_t Rate), //resets all vars
            reset(), //resets the loop vars, restarting the blend from the beginning
            setupBlendPalette(uint8_t blendPaletteLength), //used only by PaletteCycle, not for general use!
            update();
    
    private:
        unsigned long
            currentTime,
            holdStartTime = 0,
            prevTime = 0;

        CRGB
           *blendPalette_arr = nullptr,
           startColor,
           endColor,
           newColor;
        
        uint8_t 
            blendPaletteLength = 0;
};

#endif
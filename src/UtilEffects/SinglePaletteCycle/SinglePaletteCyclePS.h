#ifndef SinglePaletteCyclePS_h
#define SinglePaletteCyclePS_h

//NOT TESTED!!!

#include "Effects/EffectBasePS.h"
#include "UtilEffects/PaletteBlender/PaletteBlenderPS.h"

/* 
Takes a single palette and blends between it's own colors depending on the blend mode
for example, you could shift all of the palette's colors forward by one
the palettes are blended at the passed in rate (ms) in the passed in totalSteps steps
the resulting palette can be accessed as "cyclePalette", it's length will vary depending on the mode
The blend is always looped, so it will repeat indefinitly 
A hold time can be set, that will pause the the cycle once a blend has finished before starting the next
Modes can be changed mid-cycle
Some modes have a diection setting (see below)

The input palette is never modified by the blends, and can be changed at will

pass cyclePalette to your effects to use the blended palette 

The PaletteCyclePS update rate is a pointer, and can be bound externally, like in other effects

Uses an instance of paletteBlenderPS to do the blends. 
The instance is public and can be accessed as "PB"

The hold time and totalSteps are variables of the PaletteBlenderPS instance
so they use setter functions as shown below

Examples below use a palette of  {blue, red, green}
Blend Modes:
    0 -- Cycles the entire palette by foward/backward by one step each cycle. 
        ie for direct = true, {blue, red, green} => {green, blue, red} => {red, green, blue} , etc
    1 -- Like mode 0, but a random color is inserted at the beginning (end if direct is false) each cycle, (initially starts as palette)
        ie for direct = true, {rand1, red, green} => {rand4, rand1, red} => {rand5, rand4, rand1}, etc
    2 -- shuffles the palette each cycle
        ie {blue, red, green} could go to {red, blue, green}, or {blue, green, red}, etc 
        direct has no effect
    3 -- Makes the palette length 1, cycles through each color of the palette
        ie for direct = true and palette {blue, red, green} it will be {blue} => {red} => {green} => {blue}, etc
    4 -- Same as mode 3, but chooses the next color randomly from the palette (will not be the current color)
    5 -- Same as mode 3, but the next color is choosen completely randomly (not from the palette)

Example calls: 
    SinglePaletteCyclePS(palette, 0, 50, 80);
    Blends the palette using blend mode 0
    each blend takes 50 steps, with 80ms between each step

Constructor Inputs:
    palette -- The pointer to the palette used in the blend
    blendMode -- The mode used to blend the palette (see blend modes above). You should be able to change this on the fly
    totalSteps (max 255) -- The total number of steps taken to blend between the palettes
    rate -- The update rate of the blend (ms)

Functions:
    switchPalette() -- Adjusts the current start/end palettes used for the current blend. Should only be called if you manually changed the palette
    setPalette(*palette) -- Restarts the blend with a new palette, with the same steps and update rate, and mode
    reset() -- Restarts the blend (all settings and palettes stay the same)
    setTotalSteps(newTotalSteps) -- changes the total number of steps used in the blends (set in the PB instance, see PaletteBlenderPS)
    getTotalSteps() -- Returns the number of steps being used for the blend (set in the PB instance, see PaletteBlenderPS)
    setHoldTime(newHoldTime) -- Changes the pause between blends (set in the PB instance, see PaletteBlenderPS)
    update() -- updates the effect

Other Settings:
    direct (default true)-- direction setting for some blendModes, will either cycle the palette backwards or forwards

Public Vars:
   *paletteOrig -- The pointer to the input palette (as supplied when you created the class instance)
                   You can change it by calling setPalette(), but this resets the cycle
                   To avoid reseting, you can change it manually, but you may need to call switchPalette()
   cyclePalette -- The output palette for the blend, length is set depending on mode (see notes below for more)
   *PB -- The PaletteBlendPS instance, public for reference 

Reference Vars:
    cycleNum -- How many blend cycle's we've done, resets every palette length number of cycles (all the palette colors have been shifted once)
    current/nextPalette -- starting/end palette for the current blend

Notes:
If you switch modes, the output palette may change length, either from 1 to the input palette length or vise versa
Be aware that this may break some effects, if they don't adjust for the palette length mid-cycle. 
This is also true if you change the input palette, depending on the mode

the PaletteBlendPS instance  (PB) is public, but don't mess with it unless you know what you're doing
*/
class SinglePaletteCyclePS : public EffectBasePS {
    public:
        SinglePaletteCyclePS(palettePS *Palette, uint8_t BlendMode, uint8_t TotalSteps, uint16_t Rate);

        ~SinglePaletteCyclePS();
        
        uint8_t
            blendMode,
            cycleNum = 0, //the current cycle of the blend we're on (max value of palette length), for reference only
            getTotalSteps();

        CRGB 
            *paletteColorArr1 = nullptr, //storage for the start/end palette colors, for reference
            *paletteColorArr2 = nullptr;
        
        bool 
            direct = true;
                                 
        palettePS
            *paletteOrig = nullptr,
            cyclePalette; //ouput palette
        
        palettePS
            currentPalette, //starting palette for the current blend
            nextPalette; //ending palette for the current blend
        
        PaletteBlenderPS
            *PB = nullptr; //PaletteBlenderPS instance

        void 
            switchPalette(),
            setPalette(palettePS *palette),
            setTotalSteps(uint8_t newTotalSteps),
            setHoldTime(uint16_t newHoldTime),
            reset(),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        int8_t 
            stepDirect;
        
        uint8_t 
            paletteLength = 0,
            prevMode = 0;
        
        uint16_t
            currentIndex;
};

#endif
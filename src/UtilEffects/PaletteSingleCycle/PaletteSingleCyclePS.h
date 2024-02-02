#ifndef PaletteSingleCyclePS_h
#define PaletteSingleCyclePS_h

#include "Effects/EffectBasePS.h"
#include "UtilEffects/PaletteBlender/PaletteBlenderPS.h"

/* 
Takes an input palette and produces an ouput palette that blends between the input's colors.
Has various "blend modes" for producing different types of color blends.
For example, you could shift all of the palette's colors forward over time, cycling their position in the palette.
Note that the input palette is not modified by this utility, and can be changed at will 
(although their changes will only show up after the current blend)

The palettes are blended at the passed in rate (ms) with a set number of steps for each blend. 

The output palette can be accessed as "cyclePalette", it's length will vary depending on the mode
The blend is always looped, so it will repeat indefinitely.

You can have the blend pause either at the beginning or end of the blend, with a configurable "pause time". 

Modes can be changed mid-cycle, palettes can too, but the new palette will be blended in as part of the cycle
To force a new palette, you can call reset().

Some modes have a direction setting (see below)

You can track what blend color you're on via cycleNum.

Pass "cyclePalette" to your effects to use the blended palette.

The utility uses an instance of paletteBlenderPS to do the blends. 
The instance is public and can be accessed as "PB". This is the only way to get certain settings.
ie "<your PaletteCycle instance>.PB->totalSteps" fetches the total number of blend steps.  
Note that the utility includes some functions for setting the more common paletteBlender settings.

Notes:
    If you switch modes, the output palette may change length, either from 1 to the input palette length or vise versa
    Be aware that this may break some effects, if they don't adjust for the palette length mid-cycle. 
    This is also true if you change the input palette, depending on the mode.

Blend Modes:
Examples below use a palette of  {blue, red, green}

    0 -- Cycles the entire palette by forward/backward by one step each cycle. 
         ie for direct = true, {blue, red, green} => {green, blue, red} => {red, green, blue} , etc
    1 -- Like mode 0, but a random color is inserted at the beginning (end if direct is false) each cycle, (initially starts as palette)
         ie for direct = true, {rand1, red, green} => {rand4, rand1, red} => {rand5, rand4, rand1}, etc
    2 -- Randomizes the whole palette each cycle for a palette with 3 random colors: {rand1, rand2, rand3} -> {rand4, rand5, rand6}, etc
         direct has no effect.
    3 -- shuffles the palette each cycle
         ie {blue, red, green} could go to {red, blue, green}, or {blue, green, red}, etc 
         Note that the same palette order can repeat, the likely-hood depends on the palette length.
         DO not use this for a palette with only 2 colors!
         direct has no effect.
    4 -- Makes the palette length 1, cycles through each color of the palette
         ie for direct = true and palette {blue, red, green} it will be {blue} => {red} => {green} => {blue}, etc
    5 -- Same as mode 4, but chooses the next color randomly from the palette (will not be the current color). direct has no effect.
    6 -- Same as mode 4, but the next color is chosen completely randomly (not from the palette). direct has no effect.

Example calls: 
    PaletteSingleCyclePS palSingleCycle(cybPnkPal_PS, 0, true, false, 50, 80);
    Blends the palette using blend mode 0.
    The palette will be blended forward, and will start blending immediately (startPaused is false)
    (pauseTime is default 0, so they'll be no pause)
    Each blend takes 50 steps, with 80ms between each step

Constructor Inputs:
    palette -- The pointer to the palette used in the blend
    blendMode -- The mode used to blend the palette (see blend modes above). You should be able to change this on the fly
    direct -- direction setting for some blendModes, will either cycle the palette backwards or forwards
    startPaused -- If true, then the blend cycle will start paused, blocking the first palette blend for pauseTime time. 
                   (see paletteBlenderPS for more)
    totalSteps (max 255) -- The total number of steps taken to blend between the palettes
    rate -- The update rate of the blend (ms)

Other Settings:
   *inputPalette -- The pointer to the input palette (as supplied when you created the class instance)
                   You can change it by calling setPalette(), but this resets the cycle
                   To avoid resetting, you can change it manually, but you may need to call switchPalette()
   cyclePalette -- The output palette for the blend, length is set depending on mode (see notes below for more)
   *PB -- The PaletteBlendPS instance, public for reference 

Functions:
    switchPalette() -- Adjusts the current start/end palettes used for the current blend. Should only be called if you manually changed the palette
    reset() -- Restarts the blend cycle (all settings and palettes stay the same)
    setTotalSteps(newTotalSteps) -- changes the total number of steps used in the blends (set in the PB instance, see PaletteBlenderPS)
    setPauseTime(newPauseTime) -- Changes the pause between blends (set in the PB instance, see PaletteBlenderPS)
    setStartPaused(bool newStartPaused) -- Changes the "startPaused" setting in the PB instance, see PaletteBlenderPS.
    update() -- updates the effect

Reference Vars:
    cycleNum -- How many blend cycle's we've done, resets every palette length number of cycles (all the palette colors have been shifted once)
    current/nextPalette -- starting/end palette for the current blend
*/
class PaletteSingleCyclePS : public EffectBasePS {
    public:
        //Constructor with custom direction setting
        PaletteSingleCyclePS(palettePS &InputPalette, uint8_t BlendMode, bool Direct, bool StartPaused, 
                             uint8_t TotalSteps, uint16_t Rate);

        ~PaletteSingleCyclePS();

        uint8_t
            blendMode,
            cycleNum = 0;  //the current cycle of the blend we're on (max value of palette length), for reference only

        bool
            direct;

        CRGB
            *paletteColorArr1 = nullptr,  //storage for the start/end palette colors
            *paletteColorArr2 = nullptr;

        palettePS
            *inputPalette = nullptr,
            cyclePalette = {nullptr, 0};  //ouput palette (is filled in during the first update cycle)

        palettePS
            currentPalette = {nullptr, 0},  //starting palette for the current blend (is filled in during class construction)
            nextPalette = {nullptr, 0};     //ending palette for the current blend (is filled in during class construction)

        PaletteBlenderPS
            *PB = nullptr;  //PaletteBlenderPS instance

        void
            switchPalette(),
            setTotalSteps(uint8_t newTotalSteps),
            setPauseTime(uint16_t newPauseTime),
            setStartPaused(bool newStartPaused),
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
            paletteLenMax = 0,
            *indexOrder = nullptr; //an array used to track the palette order when shuffling (see blend modes)

        uint16_t
            currentIndex = 0;
        
        void 
            initPaletteColors();
};

#endif
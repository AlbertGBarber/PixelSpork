#ifndef PaletteSingleCyclePS_h
#define PaletteSingleCyclePS_h

#include "Effects/EffectBasePS.h"
#include "UtilEffects/PaletteBlender/PaletteBlenderPS.h"

/* 
A utility class that takes an input palette and produces an output palette that shifts the input's colors over time. 
For example, you could shift all of the inputs palette's colors forward over time, cycling their position in the palette. 
There a variety of "blend modes" for configuring how the palette colors are shifted. 
Importantly, the input palette is not modified by this utility, all the color shifts are tracked within the utility. 
Likewise, the input palette can be changed at will (although the changes will be delayed s the blends are processed).

See palette.h for info about palettes.

The utility's blended output palette is stored locally as 'cyclePalette'. 
Its length and number of colors will be set according to the "blend mode" (see below).

'cyclePalette' is a pointer, so passing it to effects is a little different than a normal palette:
    //Sets an effect's palette to your palette blender's output palette
    //Note that this assumes that the effect's "palette" var is a pointer (true in most effects)
    yourEffect.palette = yourPaletteSingleCycle.cyclePalette;

You can also pass `*yourPaletteSingleCycle.cyclePalette` as part of the effect's constructor.

    Blend Modes:
        Controls how the input palette's colors will be shifted for the output palette.
        Set using `blendMode`.
        Examples below use a palette of {blue, red, green}
            0 -- Cycles the entire palette forward/backward by one step each cycle. 
                 ie for direct = true, {blue, red, green} => {green, blue, red} => {red, green, blue} , etc
            1 -- Like mode 0, but a random color is inserted at the beginning (end if direct is false) each cycle, 
                 (initially starts as the input palette)
                 ie for direct = true, {rand1, red, green} => {rand4, rand1, red} => {rand5, rand4, rand1}, etc, 
                 where "rand#" are random colors
            2 -- Randomizes the whole palette each cycle. 
                 For a palette with 3 random colors: 
                 {rand1, rand2, rand3} -> {rand4, rand5, rand6}, etc
                 direct has no effect.
            3 -- Shuffles the input palette each cycle
                 ie {blue, red, green} could go to {red, blue, green}, or {blue, green, red}, etc 
                 Note that the same palette order can repeat, the likely-hood depends on the palette length.
                 DO not use this for a palette with only 2 colors!
                 direct has no effect.
            4 -- Makes the output palette a single color, cycling through each color in the input palette.
                 ie for direct = true and an input palette {blue, red, green} 
                 the output will be {blue} => {red} => {green} => {blue}, etc.
            5 -- Same as mode 4, but chooses the next color randomly from the input palette (will not be the current color). 
                 direct has no effect.
            6 -- Same as mode 4, but the next color is chosen completely randomly (not from the palette). direct has no effect.

        For blend modes 0, 1, 2, 3 the output palette length will be the same as the input. 
        For modes 4, 5, 6 the output palette length will be 1.

    Utility Feature Notes:
        * The blend is always looped, so it will repeat indefinitely.

        * You can have the blend pause either at the beginning or end of each palette blend, 
          with a configurable pause time, `pauseTime`. This is mainly used when looping. 
          When paused, the palettes will not be blended, giving you time to reflect/absorb 
          the current palette (and effect) colors, before cycling to a new palette. 

        * The blend modes can be changed mid-cycle, along with the input palette, but the new palette will 
          be blended in over time. To force a new palette, you can call reset(). 
          Note that changing the blend mode may change the output palette's length, see "Blend Modes" above. 
          Be aware that this may break some effects if they don't adjust for the palette length changes during updates. 
          This is also true if you change the input palette for blend modes where the input and output palettes 
          are the same length (0, 1, 2, 3). The easiest way to check this is to test your effect. 

        * Some blend modes have a direction setting (`direct`), which controls the direction the palette colors are 
          shifted. If `direct` is true the color will shift forward, so color 0 becomes color 1, etc 
          and visa versa if `direct` is false.

    Other Notes:
        * You can track what blend stage you're on using `cycleNum`. 
          This resets every input palette length number of cycles.

        * The utility uses an instance of the PaletteBlenderPS utility to do the blends, 
          automatically swapping palettes and resetting it in the background for the cycle. 
          The instance is created dynamically as part of the utility's construction. 
          It is public and can be accessed as `PB`. 
          Some utility settings must be changed in the blender instance directly. 
          I mostly provide pass-through functions for changing these settings, 
          but any missing settings can be accessed like: `<your PaletteSingleCycle instance>.PB->someSetting`.

        * The utility's `cyclePalette` is pointed to the palette blender instance's `blendPalette`.

        * Any quirks from the Palette Blender will also apply to this utility, see the blender's "Other Notes" entry.

Example calls: 
    PaletteSingleCyclePS palSingleCycle(cybPnkPal_PS, 0, true, false, 0, 50, 80);
    Cycles through the cybPnkPal_PS palette using blend mode 0.
    The palette will be blended forward (direct is true),
    and will start blending immediately (startPaused is false)
    The pause time between blends is 0.
    Each blend takes 50 steps, with 80ms between each step.

Constructor Inputs:
    inputPalette -- The input palette used for blending.
    blendMode -- Sets how the input palette's colors will be shifted (see "Blend Modes" above).
    direct -- The direction used for some blend modes, will either cycle the palette backwards or forwards. 
              (true is forwards).
    startPaused -- If true, then the blend cycle will start paused, blocking the first blend for `pauseTime` time. 
                   Can be changed later using `setStartPaused()`. 
                   Note that this setting is a member of the Palette Blender instance and not in the utility itself.
    pauseTime -- The time (ms) that the utility will pause for between blends. Can be changed later using `setPauseTime()`. 
                 Note that this setting is a member of the Palette Blender instance and not in the utility itself.
    totalSteps (max 255) -- The total number of steps taken to blend between the palettes. 
                            Can be changed later using `setTotalSteps()`. 
                            Note that this setting is a member of the Palette Blender instance and not in the utility itself.
    rate -- The update rate of the blend (ms)

Outputs:
   *cyclePalette -- The output palette for the blend, the length depends on the blend mode (see notes above for more).

Other Settings:
   active (default true) -- If false, the utility will be disabled (updates() will be ignored).
   *PB -- The PaletteBlendPS instance. (See Other Notes above).

Functions:
    reset() -- Restarts the blend cycle (all settings stay the same).
    switchPalette() -- Sets the internal start/end palettes used for the current blend. 
                       Should only be called if you manually change the input palette.
    setTotalSteps(newTotalSteps) -- Changes the number of blend steps. (set in the Palette Blender instance)
    setPauseTime(newPauseTime) -- Changes the pause time (ms) between blends (set in the Palette Blender instance)
    setStartPaused(bool newStartPaused) -- Changes the "startPaused" setting in the Palette Blender instance, see PaletteBlenderPS.
    update() -- updates the utility.

Reference Vars:
    cycleNum -- How many blend cycles the utility has completed.
                This resets every input palette length number of cycles (all the palette colors have been shifted once).
    current/nextPalette -- starting/end palette for the current blend
*/
class PaletteSingleCyclePS : public EffectBasePS {
    public:
        //Constructor with custom direction setting
        PaletteSingleCyclePS(palettePS &InputPalette, uint8_t BlendMode, bool Direct, 
                            bool StartPaused, uint16_t PauseTime, uint8_t TotalSteps, uint16_t Rate);

        ~PaletteSingleCyclePS();

        uint8_t
            blendMode,
            cycleNum = 0;  //the current cycle of the blend we're on (max value of palette length), for reference only

        bool
            direct;

        palettePS
            *inputPalette = nullptr,
            *cyclePalette = nullptr;  //ouput palette (is bound to the palette blender instance's blendPalette)

        palettePS
            currentPalette = {nullptr, 0},  //starting palette for the current blend (is filled in during class construction)
            nextPalette = {nullptr, 0};     //ending palette for the current blend (is filled in during class construction)

        CRGB
            *paletteColorArr1 = nullptr,  //storage for the start/end palette colors
            *paletteColorArr2 = nullptr;

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
            *indexOrder = nullptr;  //an array used to track the palette order when shuffling (see blend modes)

        uint16_t
            currentIndex = 0;

        void
            initPaletteColors();
};

#endif
#ifndef PlasmaSL_h
#define PlasmaSL_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

/*

An effect based on the PlasmaSL.ino code by Andrew Tuline found 
here: (https://github.com/atuline/FastLED-Demos/blob/master/PlasmaSL/PlasmaSL.ino). 
Uses two moving waves to blend a set of palette colors together.

The effect is controlled by frequency and phase values for each of the two waves. 
These control where the waves meet and how the colors blend. When creating the effect, 
you have the option to set them to default values (from Andrew's original effect), or have them set randomly. 
The phases can be configured to drift randomly over time, 
which helps prevent the effect from repeating as much. (More on that below)

There are a few issues with this effect at lower brightness (<50), 
where the LEDs seem to blink. The brightness is controlled by a wave. 
The blinking happens when the wave frequency is too low. 
You can adjust the frequency and the maximum dimming using `briFreq` and `briRange`. 
I've defaulted these to a pair of values that seem to work okay at lower brightness, 
but you may need to adjust them. Andrew Tuline's defaults are 9 and 96 respectively.

You can change all of the effect's settings on the fly, but they will probably cause a "jump" in the effect.

The effect is adapted to work on segment lines for 2D use.

Finally, I note that I've never been completely happy with how this effect turned out, 
it just doesn't seem to have enough variation for me (maybe I've implemented it wrong?). 
Probably looks better on a 1D strip than in 2D. For alternative, 
but more computationally intensive effects, check out the "noise" series. 

Inputs Guide & Notes:

    Shifting Phases with Time:
        To produce a more varied effect you can opt to have the wave phases shift with time by setting `randomize` = true.
            * `phase1Base` and `phase1Range` control the shifting of wave 1.
            * `phase2Base` and `phase2Range` control the shifting of wave 2.
            *  phase1Base is the minimum phase value, while phase1Base + phase1Range is the maximum. 
               (Same for the wave 2 settings)

        To randomize the phase, a target phase value will be set by adding the "phase base" to 
        a randomly picked value up to the "phase range" ie, "phase1Base + random(phase1Range)"". 
        With a target set, the phase will move towards the target value by one step each update cycle. 
        This avoids color "jumps" by keeping the transition gradual. Once the target is reached, 
        a new target value is picked and the process begins again.
            
        The default is to use the full range of 255 to vary the phases where:
            * Both phase bases are set to a default value of 0.
            * Both phase ranges are set to a default value of 255.

        You can adjust either of these on the fly. If you don't want any shifting, simply set randomize = false. 
        However, note that the to phases will stay at what ever value they currently have.

        !!Do NOT set either phases directly after turning on randomize.
        If you do, be sure to adjust `phase1 == targetPhase1` and `phase2 == targetPhase2`.

        There are also an additional pair of phases that always vary using a sin() function. 
        You don't need to touch these, but know that their frequencies are randomly adjusted 
        by a small amount on startup if `randomize` is true. These are the `phaseWave1` and `phaseWave2` 
        variables in the code. Giving them a slight variation helps produce a more unique effect.

    Random Frequencies:
        If "randomize" is true in the constructor, the frequencies for the waves will be randomized once (between 10 and 30) 
        but they will not be shifted over time because it causes jumps in the effect. 
        You can randomize the frequencies at any time by calling `randomizeFreq(freqMin, freqMax)` with a min and max value.

        If you choose not to randomize, the freqs will be set to defaults (matching original values from Andrew Tuline):
            * `freq1` = 23.
            * `freq2` = 15.
    
Example calls: 

    PlasmaSL plasma(mainSegments, 3, 50, true, 80);
    Will do a the effect using 3 randomly chosen colors
    with 50 blend steps between each color
    and the freqs and phases being randomized (randomize is true)
    The effect updates every 80ms

    PlasmaSL plasma(mainSegments, cybPnkPal_PS, 80, false, 40);
    Will do a the effect using colors from the cybPnkPal_PS palette
    with 80 blend steps between each color
    and the freqs and phases are not randomized
    (set to defaults, and will not shift over time)
    The effect updates every 40ms

Constructor inputs: 
    palette (optional, see constructors) -- A custom palette passed to the effect, the default is the 
                                           lava colors palette encoded below
    numColors (optional, see constructors) -- How many colors will be in the randomly created palette
    blendSteps -- Sets how many steps are used to blend between each color
                  Basically changes how fast the colors blend
                  Between 20 - 100 looks good
    randomize -- Set to true will randomize and shift the phase values over time (and also randomize the freqs once)
    rate -- The update rate (ms) note that this is synced with all the particles.

Other Settings:
    freq1 (default 23) -- The frequency used for the first wave.
    freq2 (default 15) -- The frequency used for the second wave.
    pAdj1 (default 0) -- Random factor for the wave 1 phase calculation (randomized during construction if randomize is true)
    pAdj2 (default 0) -- Random factor for the wave 2 phase calculation (randomized during construction if randomize is true)
    phase1Base (default 0) -- The minimum value of the wave 1 phase (see Shifting Phases with Time above).
    phase2Base (default 0) -- The minimum value of the wave 2 phase (see Shifting Phases with Time above).
    phase1Range (default 255) -- The range for the variation of wave 1 phase (see Shifting Phases with Time above).
    phase2Range (default 255) -- The range for the variation of wave 2 phase (see Shifting Phases with Time above).
    briFreq (default 15) -- The frequency at which the brightness changes
    briRange (default 75) -- The maximum reduction in brightness (min is 0)

Functions:
    randomizeFreq(freqMin, freqMax) -- Sets both wave frequencies to be a random value between the passed in min and max.
                                       (Causes a jump in the effect).
    update() -- updates the effect 
*/
class PlasmaSL : public EffectBasePS {
    public:
        //Constructor for effect with palette
        PlasmaSL(SegmentSetPS &SegSet, palettePS &Palette, uint16_t BlendSteps, bool Randomize, uint16_t Rate);

        //Constructor for effect with randomly chosen palette
        PlasmaSL(SegmentSetPS &SegSet, uint8_t numColors, uint16_t BlendSteps, bool Randomize, uint16_t Rate);

        ~PlasmaSL();

        //Brightness vars
        uint8_t
            briFreq = 15,   //9
            briRange = 75;  //96
        
        //Base wave controlling vars
        uint8_t
            freq1 = 23,
            freq2 = 15,
            pAdj1 = 0,
            pAdj2 = 0;

        //vars for shifting phase
        uint8_t
            phase1Base = 0,
            phase2Base = 0,
            phase1Range = 255,
            phase2Range = 255;

        uint16_t
            blendSteps;

        bool
            randomize;

        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0};  //Must init structs w/ pointers set to null for safety

        void
            randomizeFreq(uint8_t freqMin, uint8_t freqMax),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0,
            holdStartTime = 0;

        int8_t
            phase1Step = 0,
            phase2Step = 0;

        uint8_t
            phaseWave1,
            phaseWave2,
            phase1,
            phase2,
            phase1Target,
            phase2Target,
            brightness;

        uint16_t
            numLines,
            lineNum,
            colorIndex,
            totBlendLength;

        CRGB
            colorOut;

        void
            shiftPhase(uint8_t *phase, uint8_t *phaseTarget, int8_t *phaseStep, uint8_t phaseBase, uint8_t phaseRange),
            init(SegmentSetPS &SegSet, uint16_t Rate);
};

#endif
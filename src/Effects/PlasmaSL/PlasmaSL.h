#ifndef PlasmaSLPS_h
#define PlasmaSLPS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

/*
An effect based on the PlasmaSL.ino code by Andrew Tuline found here: https://github.com/atuline/FastLED-Demos/blob/master/PlasmaSL/PlasmaSL.ino
Uses two moving waves to blend a set of palette colors together

The effect is controlled by two frequency and phase values for the two waves, 
Changing these changes where the waves meet and how the colors blend
These are set to a pair of default values, or you can set them yourself, or have then set randomly 
For the phases, you have the option to have them shift automatically over time randomly
which helps prevent the effect from repeating too often.

There are a few issues with this effect at lower brightnesses (<50), where the leds seem to blink. 
The brightness is controlled by a wave. The blinking happens when the wave frequency is too low
You can adjust the freqency and the maxium dimming using briFreq and briRange
I've pre-set these to a pair of values that seem to work ok at lower brightnesses
But you may need to adjust them. Andrew Tuline defaults are 9 and 96 respectively

The effect is adapted to work on segment lines for 2D use, but you can keep it 1D by
passing in a segmentSet with only one segment containing the whole strip.

Shifting Phases with Time:
    To produce a more varied effect you can opt to have the wave phases shift with time
    Setting randomize = true will set the phases to shift
    phase1Base and phase1Range control the shifting of wave 1
    phase2Base and phase2Range control the shifting of wave 2
    phase#Base is the minimum phase value, while phase#Base + phase#Range is the maximum.
    (# is 1 or 2)
    A target phase value will be set by adding phase#Base to randomly picked value up to phase#Range: phase#Base + random(phase#Range)
    With a target set, the phase will move towards the target value by one step each update cycle
    This avoids color "jumps" by keeping the transition gradual. 
    Once the targett is reached, a new target value is picked and the process begins again
    The default is to use the full range of 255 to vary the phases
    Both phase#Bases are set to a default value of 0
    Both phase#Ranges are set to a default value of 2555
    You can adjust either of these on the fly
    If you don't want any shifting, simply set randomize = false,
    However, note that the to phases will stay at what ever value they currently have.

    !!Do NOT set either phases directly after turning on randomize.
    If you do, be sure to adjust phase1 == targetPhase1 and phase2 == targetPhase2

    If randomize is true in the constuctor, the freqencies for the waves,
    will also be randomized once (between 10 and 30) but they will not be 
    shifted over time because it causes jumps in the effect

    You can randomize the frequencies at any time by calling randomizeFreq(freqMin, freqMax) with a min and max value

    If you choose not to randomize the freqs will be set to defaults (matching original values from Andrew Tuline)
    freq1 = 23;
    freq2 = 15;

    There are also an additional pair of phases that always vary using a sin(). You don't need to touch these,
    but know that their frequencies are randomly adjusted by a small amount on startup if randomize is true
    These use the phaseWave1 and phaseWave2 variables in the code.
    
Example calls: 

    PlasmaSL(mainSegments, 2, 50, true, 80);
    Will do a PlasmaSL effect using 3 randomly choosen colors
    with 50 blend steps between each color
    and the freqs and phases being randomized 
    It will update every 80ms

    PlasmaSL(mainSegments, &palette1, 80, false, 40);
    Will do a PlasmaSL effect using colors from palette1
    with 80 blend steps between each color
    and the freqs and phases are not randomized
    It will update every 40ms

Constructor inputs: 
    palette (optional, see constructors) -- A custom palette passed to the effect, the default is the 
                                           lava colors palette encoded below
    numColors (optional, see constructors) -- How many colors will be in the randomly created palette
    blendSteps -- Sets how many steps are used to blend between each color
                  Basically changes how fast the colors blend
                  Between 20 - 100 looks good
    randomize -- Set to true will randomize and shift the phase values over time (and also randomize the freqs once)
    rate -- The update rate (ms) note that this is synced with all the particles.
vb
Functions:
    randomizeFreq(freqMin, freqMax) -- Sets both wave frequencies to be a random value between the passed in min and max
    update() -- updates the effect 

Other Settings:
    freq1 (default 23) -- The first frequency used in the wave caculation
    freq1 (default 15) -- The second frequency used in the wave caculation
    pAdj1 (default 0) -- Random factor for the phaseWave1 calculation
    pAdj2 (default 0) -- Random factor for the phaseWave2 calculation
    phase1Base (default 0) -- The minimum value of phase1 (see Shifting Phases with Time above)
    phase2Base (default 0) -- The minimum value of phase2 (see Shifting Phases with Time above)
    phase1Range (default 255) -- The range for the variation of phase1 (see Shifting Phases with Time above)
    phase1Range (default 255) -- The range for the variation of phase2 (see Shifting Phases with Time above)
    briFreq (default 15) -- The frequency at which the brightness changes
    briRange (default 75) -- The maximum reduction in brightness (min is 0)
*/
class PlasmaSL : public EffectBasePS {
    public:

        //Constructor for effect with palette
        PlasmaSL(SegmentSet &SegmentSet, palettePS *Palette, uint16_t BlendSteps, bool Randomize, uint16_t Rate); 

        //Constructor for effect with randomly chosen palette
        PlasmaSL(SegmentSet &SegmentSet, uint8_t numColors, uint16_t BlendSteps, bool Randomize, uint16_t Rate);

        ~PlasmaSL();

        SegmentSet 
            &segmentSet; 

        uint8_t
            freq1 = 23, 
            freq2 = 15, 
            pAdj1 = 0,
            pAdj2 = 0;

        //brightness vars
        uint8_t
            briFreq = 15, //9
            briRange = 75; //96
        
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
            paletteTemp,
            *palette = nullptr;

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
            init(uint16_t Rate);
};

#endif
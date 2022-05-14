#ifndef PlasmaPS_h
#define PlasmaPS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

//An effect based on the plasma.ino code by Andrew Tuline found here: https://github.com/atuline/FastLED-Demos/blob/master/plasma/plasma.ino
//Uses two moving waves to blend a set of pallet colors together

//The effect is controlled by two frequency values for the two waves, 
//changing these changes where the waves meet and how the colors blend
//These are set to a pair of default values, or you can set them youself, or you can 
//have them shift automatically over time randomly

//There are a few issues with this effect at lower brightnesses (<50), where the leds seem to blink. 
//The brightness is controlled by a wave. The blinking happens when the wave frequency is too low
//You can adjust the freqency and the maxium dimming using briFreq and briRange
//I've pre-set these to a pair of values that seem to work ok at lower brightnesses
//But you may need to adjust them. Andrew Tuline defaults are 9 and 96 respectivly

//Shifting Frequencies with Time:
    //To produce a more varied effect you can opt to have the wave frequencies shift with time
    //Setting randomize = true will set the frequencies to shift
    //freq1Base and freq1Range control the shifting of wave 1
    //freq2Base and freq2Range control the shifting of wave 2
    //freq#Base is the minimum scale value, while freq#Base + freq#Range is the maximum.
    //(# is 1 or 2)
    //A target freq value will be set by adding freq#Base to randomly picked value up to freq#Range: freq#Base + random(freq#Range)
    //With a target set, the freq will move towards the target value by one step each update cycle
    //This avoids color "jumps" by keeping the transition gradual. 
    //Once the targett is reached, a new target value is picked and the process begins again
    //Both freq#Bases are set to a default value of 10
    //Both freq#Ranges are set to a default value of 40
    //You can adjust either of these on the fly
    //If you don't want any shifting, simply set randomize = false,
    //However, note that the to freqs will stay at what ever value they currently have

    //Randomize will also randomize the intial phases for the waves, but they will not be shifted over time
    //(only happens if randomize is true in the constructor)

    //!!Do NOT set either freqs directly after turning on randomize
    //if you do, be sure to adjust freq1 == targetFreq1 and freq2 == targetFreq2

    //If you choose not to randomize the freqs will be set to defaults (matching original values from Andrew Tuline)
    //freq1 = 23,
    //freq2 = 15,
    
//Example calls: 

    //PlasmaPS(mainSegments, 3, 50, true, 80);
    //Will do a plasma effect using 3 randomly choosen colors
    //with 50 blend steps between each color
    //and the freqs and phases being randomized 
    //It will update every 80ms

    //PlasmaPS(mainSegments, &pallet1, 80, false, 40);
    //Will do a plasma effect using colors from pallet1
    //with 80 blend steps between each color
    //and the freqs and phases are not randomized
    //It will update every 40ms

//Constructor inputs: 
    //pallet (optional, see constructors) -- A custom pallet passed to the effect, the default is the 
    //                                       lava colors pallet encoded below
    //numColors (optional, see constructors) -- How many colors will be in the randomly created pallet
    //blendSteps -- Sets how many steps are used to blend between each color
    //              Basically changes how fast the colors blend
    //              Between 20 - 100 looks good
    //randomize -- Set to true will randomize and shift the freq values over time (and also randomize the phases once)
    //Rate -- The update rate (ms) note that this is synced with all the particles.

//Functions:
    //update() -- updates the effect 

//Other Settings:
    //freq1 (default 23) -- The first frequency used in the wave caculation
    //freq1 (default 15) -- The second frequency used in the wave caculation
    //pAdj1 (default 0) -- Random factor for the phase1 calculation
    //pAdj2 (default 0) -- Random factor for the phase2 calculation
    //freq1Base (default 0) -- The minimum value of freq1 (see Shifting Frequencies with Time above)
    //freq2Base (default 0) -- The minimum value of freq2 (see Shifting Frequencies with Time above)
    //freq1Range (default 40) -- The range for the variation of freq1 (see Shifting Frequencies with Time above)
    //freq1Range (default 40) -- The range for the variation of freq2 (see Shifting Frequencies with Time above)
    //briFreq (default 15) -- The frequency at which the brightness changes
    //briRange (default 75) -- The maximum reduction in brightness (min is 0)
class PlasmaPS : public EffectBasePS {
    public:

        PlasmaPS(SegmentSet &SegmentSet, uint8_t numColors, uint16_t BlendSteps, bool Randomize, uint16_t Rate);

        PlasmaPS(SegmentSet &SegmentSet, palletPS *Pallet, uint16_t BlendSteps, bool Randomize, uint16_t Rate); 

        ~PlasmaPS();

        SegmentSet 
            &segmentSet; 

        uint8_t
            freq1 = 23, //23
            freq2 = 15, //15
            pAdj1 = 0,
            pAdj2 = 0;

        //brightness vars
        uint8_t
            briFreq = 15, //9
            briRange = 75; //96
        
        //vars for shifting frequency
        uint8_t
            freq1Base = 10,
            freq2Base = 10,
            freq1Range = 40,
            freq2Range = 40;
        
        uint16_t
            blendSteps;
        
        bool 
            randomize;
        
        palletPS
            palletTemp,
            *pallet;

        void 
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        int8_t 
            freq1Step = 0,
            freq2Step = 0;
        
        uint8_t
            numSegs,
            phase1,
            phase2,
            freq1Target,
            freq2Target,
            brightness;
        
        uint16_t
            colorIndex,
            totSegLen,
            totBlendLength,
            pixelNum;
        
        CRGB 
            colorOut;

        void
            shiftFreq(uint8_t *freq, uint8_t *freqTarget, int8_t *freqStep, uint8_t freqBase, uint8_t freqRange),
            init(uint16_t Rate);
};

#endif
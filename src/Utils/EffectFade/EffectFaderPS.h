#ifndef EffectFaderPS_h
#define EffectFaderPS_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WConstants.h"
#include "WProgram.h"
#include "pins_arduino.h"
#endif

#include "Effects/EffectBasePS.h"

//A class for fading effects in and out, like when a song fades out at it's end
//useful for smoothly transitioning between effects
//(although blending directly to the next effect(s) would be better, it is far more complicated
// and would probably require a whole frame buffer for each effect, taking far too much memory)

//Usage:
//Takes an array of effects and its length (like EffectGroupPS), a direction, and a time length (ms)
//the effects are then all either dimmed to a minimum value (default is set below in class def),
//or brightened from the minimum value to their original brightness over the time length
//direction = true will do a brighten, while false will dim
//it assumes that the supplied segments are already set to their regular brightness
//(ie, you do not need to pre-adjust them to the fade starting point)

//To update the fade, call update(), the default update rate is 20ms
//You can reset the fade to use a new set of effects, or to just restart the current fade
//using the two reset() functions

//the minimum brightness is set via minBrightness
//the udpate rate is set by the rate var (in ms), default is 20ms (set in class def)
//the active flag turns the fade on/off (stopping mid fade is not the best, since the brightnesses are not reset)
//the done flag will be set when a fade is complete
//the started flag will be set when a fade has started (is not reset when done)

//Notes:
//DO NOT call update if the effect group array is empty, you will crash
//Don't mess with the maxBrightness, minBrightness, runTime, or the *origBrightness_arr array
//while the fade is running

class EffectFaderPS {

    public:
        EffectFaderPS(EffectBasePS **effGroup, uint8_t newNumEffects, bool direction, uint16_t RunTime);

        bool
            direct, //true -> increasing brightness
            done = false, //is the fade finished?
            started = false, //has a fade been started?
            active = true; //is the fade allowed to function?

        uint8_t
            minBrightness = 10,
            *origBrightness_arr,
            maxBrightness;
        
        uint16_t
            runTime,
            rate = 40;
        
        void
            reset(),
            resetBrightness(),
            reset(EffectBasePS **newGroup, uint8_t newNumEffects, bool direction),
            update();

    private:

        uint8_t
            numEffects;

        unsigned long
            startTime,  
            currentTime,
            prevTime = 0;

        int16_t 
            result;
        
        uint16_t
            stepRate;
        
        EffectBasePS
            **group;
        
        SegmentSet
            *getSegPtr(uint8_t effectNum),
            *segmentSet;

        void
            resetSegVars();
};

#endif
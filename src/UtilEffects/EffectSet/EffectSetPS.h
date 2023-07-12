
#ifndef EffectSetPS_h
#define EffectSetPS_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WConstants.h"
#include "WProgram.h"
#include "pins_arduino.h"
#endif

#include "Effects/EffectBasePS.h"

//TODO:
//-- Add ability to store a callback function ptr, would be called when EffectSet finishes

/*
Takes an array of pointers to effects or utils, and allows you to manage them as a group
(calling updates, destructors, etc) (see below for how to setup an effect array)
This is useful for compactly handling multiple effects and utils, by only calling one update() function
instead of one for each effect/util.

The EffectSet has a run time limit (in ms) or can be told to run forever.
This allows you to limit how long the effect set runs for.
After the run time limit is reached the effects/utils will not be updated and the "done" flag will be set.
So you can run effect(s) for a fixed time, and then stop, allowing you to switch in new effects and reset, or do whatever.

EffectSetPS is also designed to work with EffectSetFaderPS, allowing you to fade effects in/out as they start and stop.
(see more info in EffectSetFaderPS.h)

Updating Effects:
    To update the effects, call the EffectSet's update() function.
    Effects/utils will be updated in order matching their placement in the array (see below)
    so if you have multiple effects on one segment, make sure the order is correct to prevent overwriting the wrong effect
    the first time you call the update function, the "started" flag will be set, and a start time (ms) will be recorded.

Destructing Effects: 
    I've designed the the general way to operate the library is that you create/destroy effects on the fly.
    This helps keep memory usage down, since you don't need to every effect's variables all at once.
    EffectSet is designed with this in mind, and includes functions for destructing the effects in the group
    and setting new effects. 

    However, you probably don't want to destruct and re-create your utils 
    (since things like faders, palette blending, etc can be used across multiple effects)
    So, to make it easy to avoid destructing your utils, I've added a effectDestLimit, which limits
    the you to only destructing items AT and above the effectDestLimit index in the array.
    For example:
    If the effectDestLimit is 2 and the number of effects in the array is 5 -> [0, 1, 2, 3, 4]
    then effects/utils at indexes 2, 3, and 4 will be destructed, while those at 0 and 1 will not.

    destructEffsAftLim() is the function to call to destruct the effects using the limit.
    There are also other destructor functions (see below)

Notes:
    To allow multiple effects to be held in the array, they all inherit from (and have the type of) EffectBasePS.
    So if you access any effects via the effect array, you'll only be able to access the vars listed in EffectBasePS.h
    Ie if you effect has a var called "fillBG" you won't be able to access it from here b/c "fillBG" is not in EffectBasePS.
    Instead you'll need a pointer of the same type as your effect:
    Ie if your effect is FireworksPS, you'll need a pointer like FireworksPS *yourPointer, which you manage yourself.

    The length of the effectSet array, this is public so you can do shenanigans,
    like only having a single array for all your effects, but manipulating the "length" 
    so that you only use part of it at one time
    Ie if you some times have 5 effects/utils, mostly only have 2, you could make an array
    of length 5, but change the length in the set to 2 as needed.

    If you want to create an effect array on the fly do:
    effArray = new EffectBasePS*[<<numEffects>>];
    You should initialize the array members to nullptr's at first.

Creating An Effect Array:
    Say I have one PaletteBlender util and one StreamerSL effect.
    I am using the palette from the PaletteBlender in the StreamerSL, so I'd like to group them together in a set.

    The palette blender was created with a direct variable like:
    <Before Arduino setup()>
        PaletteBlenderPS PB(cybPnkPal, palette2, true, 30, 100);

    While the StreamerSL was created with a pointer using new:
    <Before Arduino setup()>
        StreamerSL *strem;
    <In Arduino loop()>
        <some logic to prevent the Streamer from being re-created every loop>
        strem = new StreamerSL(mainSegments, PB.blendPalette, 1, 2, 0, 30, 20);
    
    To group these together I'll create an effect array before the Arduino setup function with space for my util and effect.
        EffectBasePS *effArray[2] = {nullptr, nullptr};
        Note that the array is an array of pointers to effects/utils.

    Because the PaletteBlender has a direct variable, we can add it to the array in the Arduino Setup function:
        effArray[0] = &PB;

    We can add the StreamerSL to the array right after it's created in the Arduino loop():
        effArray[1] = strem;
    
    Note that we use & for the PaletteBlender, because all the effArray elements must be pointers (using & puts a pointer to PB in the array).
    strem is already a pointer, so we can just set it directly.

    For a full example of this in code see <EffectGroup example in the library>

    To manage the effect and util, you'd probably want to put the array into an EffectGroup.
    Examples of setting one up are shown below:

Example calls:
    EffectBasePS *effArray[2] = {nullptr, nullptr};
    EffectSetPS effectSet(effectArr, SIZE(effectArr), 10000);
    Creates an effect set using the passed in effectArr
    The run time for the set is 10000ms
    
    EffectBasePS *effArray[2] = {nullptr, nullptr};
    EffectSetPS effectSet(effectArr, SIZE(effectArr), 2, 0);
    Creates an effect set using the passed in effectArr, with a destruct limit of 2
    The passed in run time is 0, this will tell the set to run indefinitely 
    (the "infinite" flag is set true if you pass a 0 as the run time)

Constructor Inputs:
    **effectArr -- The array of effect pointers for the set, see notes above)
    numEffects -- The length of the effect array, ie how many effects/utils there are.
    effectDestLimit (optional, default 0) -- The destruct starting index for the effect array 
                                             (see Destructing Effects above)
    runTime (ms) -- How long the effect set should update effects for. 
                    If 0 is passed in, the infinite flag will be set true.
                
Other Settings:
    infinite (default false) -- If true, the effect set will update forever, regardless of the runTime setting

Functions:
    reset() -- Resets the time settings of the effect set (started and done), essentially restarting the effect set
    setNewSet(**newEffectArr, newNumEffects) -- Changes the effect set to use the passed in effect array and length
    setEffect(*newEffect, effectNum) -- Changes the effect pointer at the specified index of the set's effect array
    destructAllEffects() -- Calls the destructor for ALL effects, IGNORES the destruct limit
    destructEffsAftLim() -- Calls the destructor for all effects in the effect array AT AND AFTER the current effectDestLimit (see Destructing Effects above)
    destructEffsAftLim(limit) -- Calls the destructor for all effects in the effect array AT AND AFTER the passed in limit (see Destructing Effects above)
    destructEffect(effectNum) -- Calls the destructor for the effect in the effect array at the passed in index 
    updateEffect(effectNum) -- Updates the effect in the effect array at the passed in index
    update() -- Updates all the effects in the set, while also tracking the set's run time
                Will set the "done" flag once the run time has elapsed
    getEffectPtr(num) -- Returns the pointer to the effect in the effect array at the passed in index.
                         Note that the return type will be EffectBasePS (see Notes above)

Reference Vars:
    startTime -- The time (ms) the first update() was called
    timeElapsed -- The time elapsed (ms), since the first update was called

Flags:
    started (default false) -- Set true if the update() for the set has been called (the startTime will be set)
    done (default false) -- Set true if the effect set has reached the run time. Prevents any updating of the set's effects.
*/
class EffectSetPS {
    public:
        //Basic Constructor
        EffectSetPS(EffectBasePS **EffectArr, uint8_t NumEffects, uint16_t RunTime);
        
        //Constructor with an effect destruct limit (see notes above)
        EffectSetPS(EffectBasePS **EffectArr, uint8_t NumEffects, uint8_t EffectDestLimit, uint16_t RunTime);

        EffectBasePS
            **effectArr = nullptr,
            *getEffectPtr(uint8_t num);

        uint8_t 
            numEffects, //length of the effectSet array, this is public so you can do shenanigans,
                        //like only having a single array for all effects, but manipulating the "length" 
                        //so that you only use the bit you need
            effectDestLimit = 0; //The destruct limit for the effect array (see notes above)

        unsigned long
            runTime, //run time of the effect effectArr (ms), passing 0 will set it as infinite
            startTime = 0, //The time (ms) the first update() was called, for reference
            timeElapsed = 0; //the elapsed time (ms), for reference

        bool 
            infinite = false, //if set, the effect effectArr will run indefinitely
            started = false, //has the effect effectArr started (only relevant if not infinite)
            done = false; //has the effect effectArr finished (if not infinite)
        
        void 
            reset(void),
            setNewSet(EffectBasePS **newEffectArr, uint8_t newNumEffects),
            setEffect(EffectBasePS *newEffect, uint8_t effectNum),
            destructAllEffects(void),
            destructEffsAftLim(void),
            destructEffsAftLim(uint8_t limit),
            destructEffect(uint8_t effectNum),
            updateEffect(uint8_t effectNum),
            update(void);
        
    private:
        unsigned long
            currentTime;
        
        void 
            init();
};

#endif


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
This utility allows you to group and manage a set of effects and/or utilities (calling updates, setting a run time, etc).
Specifically, updating the utility updates all of the grouped effects and utilities, 
reducing the tediousness of updating them individually. 
Also integrates with the EffectSetFaderPS utility for streamlining effect transitions.

See the Effect Set code example at https://github.com/AlbertGBarber/PixelSpork/wiki/Cycling-Using-EffectSets,
for a working implementation of an effect set.

    Creating An Effect Set and Array Example:
        Say I have a PaletteBlender utility and a StreamerSL effect. 
        Also say that I am using the Palette Blender's blended palette for the colors in the Streamer effect, 
        so that the shift over time. 
        I'd like to use an effect set to group the effect and utility so I can update them together.

        To make things a little more complicated the Palette Blender and Streamer are created differently:
        The Palette Blender, named "PB", is created with a direct variable (not a pointer) like:

            //<Before Arduino setup()>
            //Create a palette blender instance named "PB".
            PaletteBlenderPS PB(firePalBlue_PS, firePal_PS true, 30, 100);
            //Note that the palette's I've picked don't matter, so I just used some pre-made ones

        While the Streamer is created dynamically with a pointer (`strem`) using `new`:

            //<Before Arduino setup()>
            //create a pointer for our streamer effect, we'll create the effect instance dynamically during runtime.
            StreamerSL *strem; 

            //<In Arduino loop()>
            //<some logic to prevent the Streamer from being re-created every loop> (see example on Temporary Effects wiki page)
            //Create the streamer instance dynamically using "new", using the blend palette as a input.
            strem = new StreamerSL(mainSegments, PB.blendPalette, 1, 2, 0, 30, 20);
            
        Before we can add the effect and utility to an effect set, we need to create an array to store them:
 
            EffectBasePS *effArray[2] = {nullptr, nullptr};
            //Note that the array is an array of pointers to effects/utils.
           
        All effect sets require an effect array -- this is how the effects are grouped together. 
        In the example above, the array is length 2; one space for the Palette Blender and the Streamer. 

        Note that the effect array is an array of pointers to effects.
        This mainly changes how you add effects to the array (as I'll show below), but is useful to know.
        Also note that we initialize the array with `nullptr`'s, which is a safety measure to prevent crashes. 
        The effect set is programmed to automatically skip over any `nullptr` array entries.

        Now we need to add the Streamer and Palette Blender to the array:

            //<<in the Arduino setup() function>>
            //Because the Palette Blender is a direct variable, we add it to the array via its address (using "&"):
            effArray[0] = &PB;

            //Because the Streamer variable is already a pointer, we can add it to the array directly:
            effArray[1] = strem;

        Finally, with the array complete, we can create the effect set:

            EffectSetPS effectSet(effectArr, SIZE(effectArr), 10000); //run time of 10000ms
 
        That's it. Now whenever we call `effectSet.update()`, it will update the Streamer and Palette Blender. 
        We can also change their run time by setting `effectSet.runTime`.

    Updating Effects:
        To update the utility's effects, call the effect set's `update()` function. 
        Effects/utilities will be updated according to their order in the set's effect array 
        (see "Creating an Effect Array" above). If you have multiple effects on one segment set, 
        make sure the order is correct to prevent overwriting the wrong effect. 
        The first time you call the update function, the "started" flag will be set, 
        and a start time (ms) will be recorded.

    Setting A Run Time:
        You can set a fixed run time for the utility using `runtime`. 
        After the run time is reached the effects/utilities will not be updated and the set's `done` flag will be set. 
        So you can run effect(s) for a fixed time, and then stop, allowing you to switch in new effects,
        change the run time, reset, or do whatever.

        You can also set the effects to run continuously either by setting the `infinite` setting to true, 
        or by passing 0 as the `runTime` in the effect set constructor.

    Destructing Dynamic Effects: 
        Note, if you are not using any dynamically allocated effects, you can ignore this section. 
        (You are only dynamically allocating effects if you use the `new` keyword when creating them).

        Effect sets include functions for destructing dynamically allocated effects, freeing their memory. 
        However, in many cases, only some of your effects will be dynamic. 
        This is particularly true for utilities, which are often shared between multiple effects. 
        Destructing an pre-allocated effect/utility will usually cause a crash, so it is critical to carefully 
        manage what effects/utilities you are destructing. 

        So, to make it easy to avoid destructing your pre-allocated effect/utilities, 
        I've added a limit to effect sets, `effectDestLimit`, and a helper function, `destructEffsAftLim()`. 
        When calling the function, any effects/utilities **_AT and AFTER_** the limit in the set's effect array 
        will be destructed. Any before the limit will be untouched. So you put all your pre-allocated effects 
        and utilities at the beginning of the array, up to the limit, 
        and use the rest of the array for your dynamically allocated effects. 

        For Example:
            If the `effectDestLimit` is 2 and the number of effects in the array is 5, ie {0, 1, 2, 3, 4}, 
            then effects at indexes 2, 3, and 4 will be destructed, while those at 0 and 1 will not.

    Extra Notes:
        * To allow multiple effects to be held in the array, they all inherit from (and have the type of) `EffectBasePS`. So if you access any effects via the effect array, you'll only be able to access the variables listed in [Effect Base](https://github.com/AlbertGBarber/PixelSpork/wiki/The-Effect-Base-Class).

          For example, if you effect has a `fillBg` setting you won't be able to access 
          it through the effect array b/c `fillBg" is not in `EffectBasePS`.

          Instead you'll need a pointer of the same type as your effect. For example, 
          if you have a FireworksPS effect, to access it you'll need a pointer like `FireworksPS *yourPointer`, 
          which you manage yourself.

        * The length of the effect array is public so you can do shenanigans, 
          like only having a single array for all your effects, but manipulating the `length` so that 
          you only use part of it at one time. Ie if you sometimes have 5 effects, but mostly only have 2, 
          you could make an array of length 5, but change the `length` in the set to 2 as needed.

        * If you want to create an effect array on the fly do: `effArray = new EffectBasePS*[<<numEffects>>];` 
          You should initialize the array members to `nullptr`'s after creation.

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
    runTime (ms) -- How long the effect set will run for.
                    If 0 is passed in, the infinite flag will be set true.
                
Other Settings:
    infinite (default false) -- If true, the effect set will update forever, regardless of the runTime setting

Functions:
    reset() -- Resets the time settings of the effect set (started and done), restarting it.
    setNewSet(**newEffectArr, newNumEffects) -- Changes the effect array of the effect set to the passed in array.
    setEffect(*newEffect, effectNum) -- Changes the effect pointer at the specified index of the set's effect array, 
                                        essentially replacing the current effect.
    destructAllEffects() -- Calls the destructor for ALL effects and/or utilities, IGNORES the destruct limit
    destructEffsAftLim() -- Calls the destructor for all effects in the effect array 
                            AT AND AFTER the current effectDestLimit (see Destructing Effects above)
    destructEffsAftLim(limit) -- Calls the destructor for all effects in the effect array AT AND AFTER the passed in limit (see Destructing Effects above)
    destructEffect(effectNum) -- Calls the destructor for the effect in the effect array at the passed in index 
    getEffectPtr(num) -- Returns the pointer to the effect in the effect array at the passed in index.
                         Note that the return type will be EffectBasePS (see Notes above)
    updateEffect(effectNum) -- Updates the effect in the effect array at the passed in index
    update() -- Updates all the effects in the set, while also tracking the set's run time
                Will set the "done" flag once the run time has elapsed

Reference Vars:
    startTime -- The time (ms) the first update() was called.
    timeElapsed -- The time elapsed (ms), since the first update() was called.

Flags:
    started (default false) -- Set true if first the update() for the set has been called (the startTime will be set).
    done (default false) -- Set true if the effect set has reached the run time. Prevents updating the set's effects.
*/
class EffectSetPS {
    public:
        //Basic Constructor
        EffectSetPS(EffectBasePS **EffectArr, uint8_t NumEffects, uint16_t RunTime);

        //Constructor with an effect destruct limit (see "Destructing Dynamic Effects" above)
        EffectSetPS(EffectBasePS **EffectArr, uint8_t NumEffects, uint8_t EffectDestLimit, uint16_t RunTime);

        EffectBasePS
            **effectArr = nullptr,
            *getEffectPtr(uint8_t num);

        uint8_t
            numEffects,           //length of the effectSet array, this is public so you can do shenanigans,
                                  //like only having a single array for all effects, but manipulating the "length"
                                  //so that you only use the bit you need
            effectDestLimit = 0;  //The destruct limit for the effect array (see notes above)

        unsigned long
            runTime,          //run time of the effect effectArr (ms), passing 0 will set it as infinite
            startTime = 0,    //The time (ms) the first update() was called, for reference
            timeElapsed = 0;  //the elapsed time (ms), for reference

        bool
            infinite = false,  //if set, the effect effectArr will run indefinitely
            started = false,   //has the effect effectArr started (only relevant if not infinite)
            done = false;      //has the effect effectArr finished (if not infinite)

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

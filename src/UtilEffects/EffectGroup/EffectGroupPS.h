
#ifndef EffectGroupPS_h
#define EffectGroupPS_h

//#define NO_EFADE_PS

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WConstants.h"
#include "WProgram.h"
#include "pins_arduino.h"
#endif

#include "Effects/EffectBasePS.h"
#ifndef NO_EFADE_PS
#include "UtilEffects/EffectFade/EffectFaderPS.h"
#endif

//TODO:
//-- Add ability to store a callback function ptr, would be called when group finishes

// EffectBasePS *effArray[] = {&effectInstanceName1, &effectInstanceName2, etc } // example of an effect group array

// Takes an array of effects, and calls their update functions together
// useful for compactly handling effects over multiple segmentSets
// It is given a run time limit (in ms) or can be told to run infinitly
// pass 0 as RunTime in the constructor to set the effects to run infinitly
//(or you can set the infinite flag later)
// after the run time limit is reached the effects will not be updated and the "done" flag will be set
// useful for effects based around triggered events

// to update the effects, call the EffectGroup's update() function
// effects will be updated in order matching their placement in the array
// so if you have multiple effects on one segment, make sure the order is correct to prevent overwriting the wrong effect
// the first time you call the update function, the "started" flag will be set

// you may adjust the effect group by calling setGroup() (see class for details)
// this will also reset all the time vars and flags by calling reset()

//Fader Useage:
//By default the effect group creates an effect fader (see EffectFaderPS for details)
//The fader will fade the effects in the effect group in as the group starts, and then fade them out again as the group reaches it's run time
//the fade in and out can be turned on and off using the fadeIn and fadeOut flags
//The fader has a run time of fadeRunTime
//By default the run time is set to 0 and the fader is inactive
//You can turn on the fader by calling setupFader(), this will also reset the fader, so don't call it mid fade
//You can also overload the effect group's constructor with the fade time (see class for details),
//which will setup the fader automatically
//The EffectFaderPS object can be accessed via the effectFader pointer
//By default effects will fade in, but not out when infinite is turned on

//Removing the Fader to conserve memory:
//   *The effect fader class takes up quite a lot of program memory (6% on an Arduino Nano/Uno) 
//    so you can entierly exclude the fader by adding #define NO_EFADE_PS above your #include PixelStrip2 in you .ino sketch
//    this prevents you from ever having a fade native to an effect group, but you can still define on elsewhere 
//    (although I'm not sure why you would, since you'd pay the memory price anyway)

//The above is also why this class is fully in the header file (the .cpp file just has a commented out version of the class)
//This is because the Arduino compiler compiles .cpp files seperatly to your .ino sketch, so the NO_EFADE_PS flag
//in the .ino file is not visible to the .cpp file, causing the EffectFaderPS functions to be included
//however .h files are added inline to the .ino sketch, so by keeping everything in the .h file
//all of the EffectFaderPS bits are removed at compile time.
class EffectGroupPS {
public:
    uint8_t 
        //length of the effectGroup array, this is public so you can do shenanigans,
        //like only having a single array for all effects, but manipulating the "length" 
        //so that you only use the bit you need
        numEffects; 

    uint16_t 
        fadeRunTime = 0, //run time of the fader (ms), defaults to 0, so the fader will not be used
        runTime; //run time of the effect group (ms), passing 0 will set it as infinite

    bool infinite = false, //if set, the effect group will run indefinitly
         fadeInStarted = false, //flag for if the initial fade in has started
         fadeOutStarted = false, //flag for if the final fade out has started
         fadeIn = true, // flag to allow the fade in to happen
         fadeOut = true, //flag to allow the fade out to happen
         started = false, //has the effect group started (only relevant if not infinite)
         done = false; //has the effect group finished (if not infinite)

    // constructor for running effects up to a time limit (RunTime)
    // note that if RunTime is passed in as 0, infinite will be set to true
    // numEffects is the length of the effGroup array
    EffectGroupPS(EffectBasePS** effGroup, uint8_t NumEffects, uint16_t RunTime): 
    runTime(RunTime), numEffects(NumEffects), group(effGroup)
    {
        runTimeCheck();
        reset();
    };

#ifndef NO_EFADE_PS
    //constructor for setting the effect group with a fader
    EffectGroupPS(EffectBasePS** effGroup, uint8_t NumEffects, uint16_t RunTime, uint16_t FadeRunTime): 
    fadeRunTime(FadeRunTime), runTime(RunTime), numEffects(NumEffects), group(effGroup) 
    {
        runTimeCheck();
        reset();
    };

    //pointer to the EffectFaderPS object created by the effect group
    //is public for external access
    EffectFaderPS  *effectFader = nullptr;

    ~EffectGroupPS(){
        delete effectFader;
    }

    //sets a new fade run time and also resets the fader
    //the fader's direction is initally set to false because this keeps us at max brightness
    //(if effectFader.update() was called eroneously, the brightness would stay the same)
    void setupFader(uint16_t newFadeRunTime) {
        fadeRunTime = newFadeRunTime;
        fadeInStarted = false;
        fadeOutStarted = false;
        if (!effectFader) { //if a fader has not been created, do so
            delete effectFader;
            effectFader = new EffectFaderPS(group, numEffects, false, fadeRunTime);
        } else { //if we have an effect fader, reset it with the new run time
            effectFader->reset(group, numEffects, false);
            effectFader->runTime = fadeRunTime;
        }
        if (fadeRunTime == 0) {
            effectFader->active = false;
        }
    };
#endif
    
    //used by contructors to set infinite to true if runTime is 0
    //as a shorthand from setting infinite to true in a new statement
    void runTimeCheck(void){
        if (runTime == 0) {
            infinite = true;
        }
    };

    //resets the effect group to start over
    //and also resets the fader
    void reset(void) {
        done = false;
        started = false;
        #ifndef NO_EFADE_PS
        setupFader(fadeRunTime);
        #endif
    };

    //set a new effect group and resets
    void setGroup(EffectBasePS** newGroup, uint8_t newNumEffects) {
        group = newGroup;
        numEffects = newNumEffects;
        reset();
    };

    //Calls the update function of all the effect group array members
    //also handles the fader
    void update(void) {
        if (!done) {
            currentTime = millis();
            // if this is the first time we've updated, set the started flag, and record the start time
            // also sets up the inital fade in (if being used) and sets the fadeInStarted flag
            if (!started) {
                startTime = currentTime;
                started = true;
            #ifndef NO_EFADE_PS
                //sets up the fade in, which resets the fader
                //shouldn't ever be called more than once, but 
                //the fadeInStarted flag adds an extra guard
                if (!fadeInStarted) { 
                    fadeInStarted = true;
                    if (!fadeIn) {
                        effectFader->done = true;
                    } else {
                        effectFader->direct = true;
                        effectFader->reset();
                    }
                }
            #endif
            }

            #ifndef NO_EFADE_PS
            //update the effect fader
            if (effectFader->active && !effectFader->done) {
                effectFader->update();
            }

            //if we're in the time between the length of the fade, and the run time of the effect group
            //we need to start fading out, so set the fader direction, and reset it
            //we set the fadeOutStarted, so that we don't reset the fader if it finishes before the effect group is done
            if (!infinite && !fadeOutStarted && (currentTime - startTime) >= runTime - fadeRunTime) {
                fadeOutStarted = true;
                if (!fadeOut) {
                    effectFader->done = true;
                } else {
                    effectFader->direct = false;
                    effectFader->reset();
                }
            }
            #endif

            // if we've not reached the time limit (or we're running indefinitly)
            // call all the effects' update functions
            if (infinite || (currentTime - startTime) <= runTime) {
                for (int i = 0; i < numEffects; i++) {
                    (group[i])->update();
                }
            } else {
                done = true;
                #ifndef NO_EFADE_PS
                //a small safe guard for if the effect group finishes before the fader
                //so that the original brightness of the effects is restored
                if(!effectFader->done){
                    effectFader->resetBrightness();
                }
                #endif
            }
        }
    };

    //deletes all the effects in the effect array (for freeing memory) 
    void destructAllEffects(void){
        for (int i = 0; i < numEffects; i++) {
            delete group[i];
        }
    };

    //deletes the effect at the specified index in the effect array (for freeing memory)
    void destructEffects(uint8_t index){
        delete group[index];
    };

private:
    unsigned long
         startTime, 
         currentTime;

    EffectBasePS** 
        group;
};

#endif

//hypotetical struct for the effectGroup array, not functional due to pointer stuff
// struct EffectArray {
//   EffectBase *effectArr;
//   uint8_t length;
//   void updateEffect(uint8_t num){
//       effectArr[num]->update();
//   };
// } ;
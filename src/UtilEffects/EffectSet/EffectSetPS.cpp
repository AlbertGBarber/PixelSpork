#include "EffectSetPS.h"

//Basic Constructor
//Note that if RunTime is passed in as 0, infinite will be set to true, so that effects will run indefinitely
EffectSetPS::EffectSetPS(EffectBasePS **EffectArr, uint8_t NumEffects, uint16_t RunTime):
    runTime(RunTime), numEffects(NumEffects), effectArr(EffectArr)
    {
        init();
    }

//Constructor with an effect destruct limit, for when you have utility effects you don't want to deconstruct.
//Note that if RunTime is passed in as 0, infinite will be set to true, so that effects will run indefinitely
EffectSetPS::EffectSetPS(EffectBasePS **EffectArr, uint8_t NumEffects, uint8_t EffectDestLimit, uint16_t RunTime):
    runTime(RunTime), numEffects(NumEffects), effectArr(EffectArr), effectDestLimit(EffectDestLimit)
    {
        init();
    }

//Initializes core vars 
//Note that if the runTime is 0, the infinite flag is set true
//so that effects will run indefinitely
void EffectSetPS::init(){
    infinite = false;
    if(runTime == 0){
        infinite = true;  
    }
    reset();
};

//Resets the time tracking vars, restarting the effect set
void EffectSetPS::reset(void){
    done = false;
    started = false;
};

//Sets the effect set to use a new array of effects
//(basically like calling the constructor again, but keeping the same runTime, etc)
void EffectSetPS::setNewSet(EffectBasePS **newEffectArr, uint8_t newNumEffects){
    effectArr = newEffectArr;
    numEffects = newNumEffects;
    reset();
};

//Sets the effect at the effectNum in the effect array to the passed in effect
//Ie changes one effect to another
void EffectSetPS::setEffect(EffectBasePS *newEffect, uint8_t effectNum){
    effectArr[effectNum] = newEffect;
};

//Calls the update() function of the specified effect in the effect array
void EffectSetPS::updateEffect(uint8_t effectNum){
    if(effectArr[effectNum]){ //don't try to update an effect that doesn't exist (its pointer is nullptr)
        effectArr[effectNum]->update();
    }
};

//Returns the pointer to the specified effect in the effect array
//Note that the return type is EffectBasePS, so you'll only be able to access the vars listed in EffectBasePS.h
//Ie if you effect has a var called "fillBG" you won't be able to access it from here b/c "fillBG" is not in EffectBasePS
//Instead you'll need a pointer of the same type as your effect:
//Ie if your effect is FireworksPS, you'll need a pointer like FireworksPS *yourPointer
EffectBasePS* EffectSetPS::getEffectPtr(uint8_t effectNum){
    //Check that the effect exists, if not, we'll return a nullptr
    if(effectArr[effectNum]){ 
        return effectArr[effectNum];
    } else {
        return nullptr;
    }
};

//Calls the destructor for ALL the effects in the effect array (for freeing memory) 
//Note that the destructor for EffectBasePS is virtual.
//Every effect is an inherited class from EffectBasePS, so calling the virtual EffectBase destructor
//calls the individual effect constructor
//This allows you to clear all current effects without knowing their names/types individually
void EffectSetPS::destructAllEffects(void){
    for (uint8_t i = 0; i < numEffects; i++) {
        destructEffect(i);
    }
};

//Calls the destructor for all effects in the effect array AT AND AFTER the current effectDestLimit
//ie if the effectDestLimit is 2 and the number of effects in the array is 5 -> [0, 1, 2, 3, 4]
//then effects at indexes 2, 3, and 4 will be destructed, while those at 0 and 1 will not.
//see destructAllEffects() for more info on destructors
void EffectSetPS::destructEffsAftLim(void){
    destructEffsAftLim(effectDestLimit);
}

//Calls the destructor for all effects in the effect array AT AND AFTER the passed in limit
//ie if the passed in limit is 3 and the number of effects in the array is 5 -> [0, 1, 2, 3, 4]
//then effects at indexes 3 and 4 will be destructed, while those at 0, 1, and 2 will not.
//see destructAllEffects() for more info on destructors
void EffectSetPS::destructEffsAftLim(uint8_t limit){
    for (uint8_t i = limit; i < numEffects; i++) {
        destructEffect(i);
    }
}

//deletes the effect at the specified index in the effect array (for freeing memory)
//(see notes on destructAllEffects() above)
void EffectSetPS::destructEffect(uint8_t effectNum){
    if(effectArr[effectNum]){ //don't try to delete an effect that doesn't exist (its pointer is nullptr)
        effectArr[effectNum]->~EffectBasePS();
    }
};

//Updates all the effects in the effect array while also tracking the overall run time
//If the effects have been run for runTime (ms), then the effects are "done" and will no longer be updated
//If the "infinite" flag is set, the the runTime will be infinite.
void EffectSetPS::update(void){
    if(!done){
        currentTime = millis();
        //if this is the first time we've updated, set the started flag, and record the start time
        if(!started){ 
            startTime = currentTime;
            started = true;
        }

        //The elapsed time since the start time
        //(we calculate this here so if can be referenced by the fader utils)
        timeElapsed = currentTime - startTime;

        //If we've not reached the time limit (or we're running indefinitely), call all the effects' update functions
        if( infinite || timeElapsed <= runTime ) {
            for(uint8_t i = 0; i < numEffects; i++){
                updateEffect(i);
            }
        } else {
            //Reached the run time, time to stop updating
            done = true;
        }
    }
}

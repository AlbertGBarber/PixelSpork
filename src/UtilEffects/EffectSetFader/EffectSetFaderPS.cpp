#include "EffectSetFaderPS.h"

//Constructor
EffectSetFaderPS::EffectSetFaderPS(EffectSetPS &EffectSet, uint16_t FadeRunTime)
    {    
        reset(EffectSet, FadeRunTime);
        effectFader->rate = rate;
	}

EffectSetFaderPS::~EffectSetFaderPS(){
    effectFader->~EffectFaderPS();
}

//Resets the fader to use a new effect set and fade time
void EffectSetFaderPS::reset(EffectSetPS &newEffectSet, uint16_t newFadeRunTime){
    effectSet = &newEffectSet;
    //Get various timing vars from the EffectSet
    infinite = &effectSet->infinite;
    runTime = &effectSet->runTime;
    setTimeElap = &effectSet->timeElapsed;

    //Setup an EffectFader, and tie it's update rate to the EffectFaderSet's
    setupFader(newFadeRunTime);
}

//Creates and sets the run time for the EffectFader
void EffectSetFaderPS::setupFader(uint16_t newFadeRunTime){
    fadeRunTime = newFadeRunTime;
    if(!effectFader){ //If the effect fader doesn't exist, create one
        //effectFader->~EffectFaderPS();
        effectFader = new EffectFaderPS(*effectSet, false, fadeRunTime);
    } else { //set the run time for the fader
        effectFader->runTime = fadeRunTime;
    }
    reset();
}

//Resets the EffectSetFader, starting a new fade in/out sequence. 
//You should do this wherever you reset your EffectSet. 
//(but do so before destructing any effects, since the fader needs access to their segment sets)
void EffectSetFaderPS::reset(){
    firstUpdate = true;
    resetNextUpdate = false;
    fadeOutStarted = false;
    fadeInStarted = false;

    //If the effectFader has been created,
    //we need to reset the segments to the original brightness setting.
    if(effectFader){
        resetFader();
    }
}

//Resets the EffectFader, doing this will restart the current fade
//You shouldn't need to call this
void EffectSetFaderPS::resetFader(){
    effectFader->reset();
}

//Resets the segments to the original brightness setting
//useful if you want to prematurely end the fade
void EffectSetFaderPS::resetBrightness(){
    effectFader->resetBrightness();
}

//Updates the fader
//Basically we're directing the EffectFader instance to either fade the effects in the effect set in or out based on the time
//We don't calculate anything to actually do the fading here, we just manage the EffectFader
//The fader directly manipulates the brightness setting of each effect's segment set
//For more info on how the fader works, see EffectFaderPS.h
//The EffectSetFader does on fade in/out cycle before being "done", which fades the effect set in and out.
//Once a fade in/out cycle is finished, the EffectSetFader must be reset manually by calling reset()
//You should do this wherever you reset your EffectSet. 
//(but do so before destructing any effects, since the fader needs access to their segment sets)
void EffectSetFaderPS::update(void){
    currentTime = millis(); 
    
    //Check if it's time to update, ultimately the update rate isn't super important, as long as it's fast enough to not look choppy
    //We always update on the first cycle, so that we start the fading before any effects are updated in the effect set.
    //(otherwise you'd get a flash of the default brightness before the fading kicks in)
    if(firstUpdate || ( currentTime - prevTime ) >= *rate) {
        prevTime = currentTime;
        firstUpdate = false;

        //If we've not started a fade in, do so by reseting the effectFader, and setting it to fade in
        if(!fadeInStarted){
            if(!fadeIn){ //If we don't want to fade in, just the end the fader before it can start
                effectFader->done = true;
            } else {
                effectFader->direct = true;
                fadeInStarted = true;
                effectFader->reset(); 
            }
        }
        
        //We want to start a fade out whenever the remaining effect set run time is less than the fade run time.
        //(if the set is infinite, then we never want to fade out)
        if( !(*infinite) && !fadeOutStarted && *setTimeElap >= (*runTime - fadeRunTime) ){
            if(!fadeOut){ //If we don't want to fade out, just the end the fader before it can start
                effectFader->done = true;
            } else { //reset the effectFader, and setting it to fade out
                effectFader->direct = false;
                fadeOutStarted = true;
                effectFader->reset();
            }
        }

        //Update the effect fader
        //Once a fade is finished, the fader will be "done" and must be reset to fade again.
        effectFader->update();

        //EffectSetFader is intended to be run as part of an effect set.
        //When switching to a new effect the fader needs to be reset, so start a new fade in/out cycle
        //However, the reset needs to be called once the set is "done", but when that happens, no more updates are called
        //So we're stuck with the needing an update to reset, but no update being called.
        //The easiest solution is just to manually call fader.reset() whenever you restart the effect set.
        //The code below tries to automate the reset by checking if the next update() will be the final update.
        //I've left it commented out because it may not be super consistent, since it relies on times syncing up.
        //if(resetNextUpdate){
            //reset();
        //}
        //if(!(*infinite) && (*rate + *setTimeElap >= *runTime) ){
           //resetNextUpdate = true;
        //}

    }
}
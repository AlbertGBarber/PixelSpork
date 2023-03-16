#include "EffectFaderPS.h"

EffectFaderPS::EffectFaderPS(EffectSetPS &EffectSet, bool Direct, uint16_t RunTime) :
    effectSet(&EffectSet), direct(Direct), runTime(RunTime) 
    {
        numEffects = &effectSet->numEffects;
        reset();
    }

EffectFaderPS::~EffectFaderPS(){
    free(origBrightness_arr);
}

// sets a new set of effects for fading, as well as a new direction
void EffectFaderPS::reset(EffectSetPS &EffectSet, bool newDirect) {
    effectSet = &EffectSet;
    direct = newDirect;
    reset();
}

// resets the core fade flags, doing this will start a new fade
//Will reset the segment sets brightnesses back to their original values if the fader has started but not finished
void EffectFaderPS::reset() {
    if(started && !done){
        resetBrightness();
    }
    done = false;
    started = false;
    totalBriChng = 0;
}

//returns a pointer to the a segmentSet from the effect group array
SegmentSet* EffectFaderPS::getSegPtr(uint8_t effectNum) { 
    if(effectSet->effectArr[effectNum]){
        return effectSet->effectArr[effectNum]->segmentSetPtr;
    } else {
        return nullptr;
    }
}

// resets fade brightness settings, and calculates the stepRate
// The original brightnesses of each SegmentSet will be stored in *origBrightness_arr
// The maximum brightness across all the segmentSets is then found
// Using the minimum and maximum brightnesses, the stepRate is calculated
// the stepRate ensures than the maximum/minimum brightness is reached by the time limit
// if direct is true, we're starting from the minimum brightness, set all the segments to that brightness
// we don't set the segment brightnesses if direct is false, b/c we assume they're already set to their normal brightnesses
// Note that since we only take one max brightness, segments that have a lower/higher brightness will reach it before the run time (their brightness is capped to their original value or the min)
// this means some effects may get to their final values before others, but the comparative brightness between effects will be preserved
void EffectFaderPS::resetSegVars() {
    // if run time is 0, the stepRate calculation will fail,
    // since 0 is an unreasonable value, we treat it as setting the fade off
    if (runTime == 0) {
        stepRate = 0;
        return;
    }

    maxBrightness = 0;
    // clear the current brightness array for memory
    // and make a new one
    free(origBrightness_arr);
    origBrightness_arr = (uint8_t*) malloc(*numEffects * sizeof(uint8_t));
    // for each effect, get it's segment pointer
    // and save it's current brightness setting
    // then find the maximum brightness across all segmentSets
    // and use it to determine the step rate
    for (uint8_t i = 0; i < *numEffects; i++) {
        segmentSet = getSegPtr(i);
        if (segmentSet) {
            uint8_t brightnessTemp = segmentSet->brightness;
            origBrightness_arr[i] = brightnessTemp;
            if (brightnessTemp > maxBrightness) {
                maxBrightness = brightnessTemp;
            }
            // if we're starting from the dim end, set all the segments
            //(we assume for the bright end that the segments are already at their max brightnesses)
            if (direct) {
                segmentSet->brightness = minBrightness;
            }
        } else{
            origBrightness_arr[i] = 255;
        }
    }
    //The step rate is the change in brightness needed per millisecond to cover
    //the brightness difference in the run time
    stepRate = (float)(maxBrightness - minBrightness) / runTime;
}

//resets the segments to their original brightness setting
//useful if you want to prematurely end the fade
void EffectFaderPS::resetBrightness() {
    for (uint8_t i = 0; i < *numEffects; i++) {
        segmentSet = getSegPtr(i);
        if (segmentSet) {
            segmentSet->brightness = origBrightness_arr[i];
        }
    }
}

//updates the fade
//If the fade is done, we jump out right away
//If the fade has not started yet:
//     Record the start time, and set the start flag
//     Call resetSegVars() to setup all the pre-fade variables, and record the initial segment set brightness
//If the fade has started but we're past the run time:
//     The fade should be done, set the done flag
//     Set all the segmentSets back to their original brightnesses
//If the fade has started, and we're not past run time, and it's time to update
//     We keep a running total of the amount of brightness change so far
//     This is added to with each update according to how much time has passed
//     ( totalBriChng += stepRate * (currentTime - prevTime) )
//     Then, we adjust the brightness of each segment set by the total brighness change so far
//     capping the change to the max or min value if needed
//     This means that if the effect set has multiple effects sharing the same segment set,
//     the segment set's brighness is always set to one value,
//     rather than being changed for every effect in the set.
void EffectFaderPS::update() {
    if (!done) {
        currentTime = millis();

        //For the first update cycle we need to get the start times and also grab the current brightness' for the segments
        if (!started) {
            startTime = currentTime;
            prevTime = currentTime - *rate; //Force an initial update, but stops a big step jump on the inital update call
            started = true;
            resetSegVars();
        }

        if ((currentTime - startTime) >= runTime) {
            done = true;
            resetBrightness();
        } else if ((currentTime - prevTime) >= *rate) {
            //Increment the total brightness change by the stepRate * (the amount of time passed)
            //totalBriChng += ceil((float)stepRate * (currentTime - prevTime));
            totalBriChng += stepRate * (currentTime - prevTime);
            //Update the brightness for all of the segmentSets, adjusting up/down based on the direction
            //capping the results at the max/min values
            for (uint8_t i = 0; i < *numEffects; i++) {
                segmentSet = getSegPtr(i);
                if (segmentSet) {
                    if (direct) {
                        //Increasing in brightness, so we start at the minBrightness, going up over time
                        briChng = minBrightness + totalBriChng;
                        if (briChng > origBrightness_arr[i]) {
                            briChng = origBrightness_arr[i];
                        }
                    } else {
                        //Decreasing in brightness, so we start at the segment set's original brightness, and go down
                        briChng = origBrightness_arr[i] - totalBriChng;
                        if (briChng < minBrightness) {
                            briChng = minBrightness;
                        }
                    }
                    segmentSet->brightness = briChng;
                }
            }
            prevTime = currentTime;
        }
    }
}

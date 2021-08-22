#include "EffectFaderPS.h"

EffectFaderPS::EffectFaderPS(EffectBasePS** effGroup, uint8_t newNumEffects, bool direction, uint16_t RunTime)
    :group(effGroup)
    ,numEffects(newNumEffects)
    ,direct(direction)
    ,runTime(RunTime) {
    reset();
}

// sets a new set of effects for fading, as well as a new direction
void EffectFaderPS::reset(EffectBasePS** newGroup, uint8_t newNumEffects, bool direction) {
    group = newGroup;
    numEffects = newNumEffects;
    direct = direction;
    reset();
}

// resets the core fade flags, doing this will start a new fade
void EffectFaderPS::reset() {
    done = false;
    started = false;
    if (runTime == 0) {
        active = false;
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
        active = false;
        stepRate = 0;
        return;
    }

    maxBrightness = 0;
    // clear the current brightness array for memory
    // and make a new one
    delete[] origBrightness_arr;
    origBrightness_arr = new uint8_t[numEffects];
    // for each effect, get it's segment pointer
    // and save it's current brightness setting
    // then find the maximum brightness across all segmentSets
    // and use it to determine the step rate
    for (int i = 0; i < numEffects; i++) {
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
        }
    }
    // the step rate is the change in brightness needed per second to cover
    // the brightness difference in the run time
    stepRate = ((uint16_t)(maxBrightness - minBrightness) * 1000) / runTime;
}

//resets the segments to the original brightness setting
//useful if you want to prematurely end the fade
void EffectFaderPS::resetBrightness() {
    for (int i = 0; i < numEffects; i++) {
        segmentSet = getSegPtr(i);
        if (segmentSet) {
            segmentSet->brightness = origBrightness_arr[i];
        }
    }
}

// updates the fade
// If the fade is not active, or is done, we jump out right away
// If the fade has not started yet:
// record the start time, and set the start flag
// call resetSegVars() to setup all the pre-fade variables
// If the fade has started but we're past the run time:
// the fade should be done, set the done flag
// set all the segmentSets back to their original brightnesses
// if the fade has started, and we're not past run time, and it's time to update
// determine how much we need to dim/brighten by based on the elapsed time (rounded up to closest int)
// for each segment, adjust the brightness by the previous result
// capping the result to the max or min value if needed
// then write the result out to the segmentSet
void EffectFaderPS::update() {
    if (active && !done) {
        currentTime = millis();

        if (!started) {
            startTime = currentTime;
            prevTime = currentTime; // stops a big step jump on the inital update call
            started = true;
            resetSegVars();
        }

        if ((currentTime - startTime) >= runTime) {
            done = true;
            resetBrightness();
        } else if ((currentTime - prevTime) >= rate) {
            // the result is the stepRate * (the amount of time passed)
            // note that the time passed is in ms, while the stepRate is set in per seconds
            // we ceil the value to make sure we don't miss a step
            result = ceil((float)stepRate * (currentTime - prevTime) / 1000);
            // update the brightness for all of the segmentSets, adjusting up/down based on the direction
            // capping the results at the max/min values
            for (int i = 0; i < numEffects; i++) {
                segmentSet = getSegPtr(i);
                if (segmentSet) {
                    if (direct) {
                        result = segmentSet->brightness + result;
                        if (result > origBrightness_arr[i]) {
                            result = origBrightness_arr[i];
                        }
                    } else {
                        result = segmentSet->brightness - result;
                        if (result < minBrightness) {
                            result = minBrightness;
                        }
                    }
                    segmentSet->brightness = result;
                }
            }
            prevTime = currentTime;
        }
    }
}

//returns a pointer to the a segmentSet from the effect group array
SegmentSet* EffectFaderPS::getSegPtr(uint8_t effectNum) { 
    return (group[effectNum])->segmentSetPtr; 
}
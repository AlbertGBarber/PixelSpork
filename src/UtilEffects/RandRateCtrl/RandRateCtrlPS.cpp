#include "RandRateCtrlPS.h"

RandRateCtrlPS::RandRateCtrlPS(uint16_t BaseRate, int32_t RateRangeMin, int32_t RateRangeMax, uint16_t PauseTime,
                               uint16_t Rate)
    : rateRangeMin(RateRangeMin), rateRangeMax(RateRangeMax), pauseTime(PauseTime)  //
{
    //Bind the update rate for the class, like with other effects
    bindClassRatesPS();

    //bind the input rate
    baseRateOrig = BaseRate;
    baseRate = &baseRateOrig;

    //pick a new random rate
    //(we assume the base rate is the starting rate)
    outputRate = *baseRate;
    pickNewRate();
}

//Picks a new random update rate and removes any pauses
void RandRateCtrlPS::reset() {
    paused = false;
    pickNewRate();
}

//Resets the outputRate to the base rate.
//Also turns the utility off by setting "active" to false.
void RandRateCtrlPS::resetToBaseRate(){
    outputRate = *baseRate;
    active = false;
}

//updates the effect
//Each update we increment/decrement (depending on the direct) outputRate by 1
//If the outputRate has reached the end rate, we trigger a pause
//After the pause, we pick a new end rate and begin again
void RandRateCtrlPS::update() {
    currentTime = millis();

    if( active && (currentTime - prevTime) >= *rate ) {
        prevTime = currentTime;

        //if we haven't reached the end rate, we just need to increment the outputRate
        //otherwise we need to handle pausing
        if( !rateReached ) {
            updateRate();
        } else {
            //if we've reached the endRate but haven't started a pause, start one!
            if( !paused ) {
                paused = true;
                pauseStartTime = currentTime;
            }

            //If we've passed the pause time, end the pause (set the pause flag)
            //and choose a new endRate for the next transition
            if( (currentTime - pauseStartTime) >= pauseTimeFinal ) {
                paused = false;
                pickNewRate();
            }
        }
    }
}

//Increments or decrements the outputRate by 1
//If the outputRate has reached the endRate, then the current 
//transition is finished, and we set the rateReached flag.
void RandRateCtrlPS::updateRate() {

    //increment the outputRate one step
    outputRate += stepDirect;

    //if we've reached the end rate based on the direction
    //we need to end the transition
    if( (direct && outputRate >= endRate) || (!direct && outputRate <= endRate) ) {
        rateReached = true;
    }
}

//Sets a new endRate to transition to, also calculates the pause time (see notes in intro in .h file)
//Note that our starting rate is the current outputRate
void RandRateCtrlPS::pickNewRate() {
    //Since we're picking a new endRate, we need to trigger the transition to start on the next update()
    rateReached = false;

    //choose a new endRate
    //Note that we need to bound it to the uin16_t range
    temp = *baseRate + random(rateRangeMin, rateRangeMax);
    if( temp < 0 ) {
        temp = 0;
    } else if( temp > 65536 ) {
        temp = 65536;
    }
    endRate = temp;

    //Set if we need to increment/decrement the current rate (outputRate)
    //to reach the new endRate
    if( outputRate <= endRate ) {
        direct = true;
    } else {
        direct = false;
    }
    //get the increment step value using the direction
    //will be -1 if direct is false, 1 if direct is true
    stepDirect = direct - !direct;

    if( usePauseRatio ) {
        //If usePauseRatio is true, then we want to set the pause time in
        //proportion to the difference between the current and end rates and the maximum possible difference
        //So that if we're only changing the rate a small amount, we don't 
        //spend a lot of time paused (see notes in intro for more)
        rateRangeDiff = rateRangeMax - rateRangeMin; //maximum difference between the current and new rate
        startEndDiff = abs(int32_t(outputRate) - endRate); //Actual difference between the current and new rate
        //Adjust the pause time by  the ratio between the actual and maximum rate differences
        pauseTimeFinal = uint32_t(pauseTime * startEndDiff) / rateRangeDiff; 
    } else {
        //if usePauseRatio is false, then we're not scaling the pause time
        pauseTimeFinal = pauseTime;
    }
}
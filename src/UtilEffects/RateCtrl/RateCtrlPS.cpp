#include "RateCtrlPS.h"

RateCtrlPS::RateCtrlPS(uint16_t StartRate, uint16_t EndRate, bool Easing, bool Cycle, uint16_t Rate):
startRate(StartRate), endRate(EndRate), easing(Easing), cycle(Cycle)
{
    //bind the rate vars since they are inherited from BaseEffectPS
    bindClassRatesPS();
    reset();
}

//resets the utility to restart at the current start rate
void RateCtrlPS::reset(){
    rateReached = false;
    outputRate = startRate;
    prevTime = 0;
    if(startRate < endRate){
        direct = true;
    } else {
        direct = false;
    }
}

//sets new vars for the start and end rates and the update rate
void RateCtrlPS::reset(uint16_t StartRate, uint16_t EndRate, uint16_t Rate ){
    startRate = StartRate;
    endRate = EndRate;
    bindClassRatesPS();
    reset();
}

//Updates the output rate
//Each time we update we increment/decrement the outputRate by +/- 1 (based on the direction)
//To do the easing, we actually modify the update rate of the outputRate (ie the whole utility) using ease8InOutApprox()
//This slows/speeds up the rate of change of the output rate as needed
//so even if the difference between the start/end rates is small, we still get an easing effect
//Once the outputRate is reached, we set the rateReached flag, which will prevent changing the output
//If we're cycling, then we'll swap the start/end rates, wait through the pauseTime, then reset
void RateCtrlPS::update(){
    currentTime = millis();
    if( rateReached && cycle && ( currentTime - prevTime ) >= pauseTime ){ 
        reset(); 
    }

    //if it's time to update the effect, do so
    //for rateTemp info see below
    //once rateReached is set, we're done
    if( !rateReached && ( currentTime - prevTime ) >= rateTemp ) {
        prevTime = currentTime;

        //get the step amount based on the direction
        //will be -1 if direct is false, 1 if direct is true
        //we do this live so you can change the direct directly
        stepDirect = direct - !direct;

        //increment the outputRate one step
        outputRate += stepDirect;

        //if we're easing, we check how close our end rate is to our current rate
        //ie outputRate/endRate
        //to produce a ratio for ease8InOutApprox
        //The ratio must be between 0-255 
        //so we swap the denominator and numerator based on if the end rate is larger than the output rate
        //ie if the start rate is larger than the end rate
        //if we're not easing, then the ratio is always 1
        if(easing){
            if(direct){
                ratio = (outputRate * 255)/endRate;
            } else {
                ratio = (endRate * 255)/outputRate;
            }
        } else {
            ratio = 1;
        }

        //we need to modify the update rate of the utility for easing
        //but we still want to be able to tie rate to an exernal pointer
        //so we use a caculated rateTemp for the update rate
        rateTemp = *rate * ease8InOutApprox(ratio)/255;

        //if we've reached the end rate based on the direction
        //we need end the transition, or reverse it if cycle is on
        if( (direct && outputRate >= endRate)  || (!direct && outputRate <= endRate) ){
            rateReached = true;
            if(cycle == true){
                uint16_t tempRate = startRate;
                startRate = endRate;
                endRate = tempRate;
            }
        }
    }

}


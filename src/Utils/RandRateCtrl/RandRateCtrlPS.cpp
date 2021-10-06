#include "RandRateCtrlPS.h"

RandRateCtrlPS::RandRateCtrlPS(uint16_t BaseRate, int16_t RateRangeMin, int16_t RateRangeMax, bool Easing, uint16_t Rate)
    {    
        //Bind the update rate for the class, like with other effects
        bindClassRatesPS();
        //create an instance of RateRandomizer with the specified base and rate range vars
        //Since whenever we call this, we'll need a new rate, we'll set the update rate to 1ms
        RR = new RateRandomizerPS(BaseRate, RateRangeMin, RateRangeMax, 1);
        RR->update(); //get the first random rate
        //Create an instance of RateCtrl, starting from the base rate and transitioning to the RateRandomizer's random rate
        //cycle is turned off, since we will choose new end rates using the RR after each transtion
        RC = new RateCtrlPS(BaseRate, RR->outputRate, Easing, false, Rate);
        //point the RC rate to the same rate as the RandRateCtrl instance, so they stay in sync
        RC->rate = rate;
        //bind the output rate of the RandRateCtrl to the output rate of the RC to get the transition rates
        outputRate = &RC->outputRate;
	}

//destructor, cleans up the RateRandomizer and RateCtrl instances
RandRateCtrlPS::~RandRateCtrlPS(){
    delete RR;
    delete RC;
}

//Changes the rateRangeMin used in the RateRandomizer instance 
void RandRateCtrlPS::setRangeMin(int16_t newRangeMin){
    RR->rateRangeMin = newRangeMin;
}

//Changes the rateRangeMax used in the RateRandomizer instance 
void RandRateCtrlPS::setRangeMax(int16_t newRangeMax){
    RR->rateRangeMax = newRangeMax;
}

//Changes the baseRate used in the RateRandomizer instance 
void RandRateCtrlPS::setBaseRate(uint16_t newBaseRate){
    RR->baseRateOrig = newBaseRate;
    RR->baseRate = &RR->baseRateOrig;
}

//Changes the easing setting of the RateCtrl instance
void RandRateCtrlPS::setEasing(bool newEasing){
    RC->easing = newEasing;
}

//Changes the base rate, and min/max ranges of the RateRandomizer all at once
void RandRateCtrlPS::setRandomizer(uint16_t newBaseRate, int16_t newRangeMin, int16_t newRangeMax){
    setRangeMin(newRangeMin);
    setRangeMax(newRangeMax);
    setBaseRate(newBaseRate);
}

//End the current transition, effectively reseting the effect
void RandRateCtrlPS::reset(){
    RC->rateReached = true;
}

//updates the effect
//each update, we call RC->update to get the new output rate from the RateCtrl instance
//If that instance has reached the end rate, we pick a new end rate using the RateRandomizer instance
//While picking the new end rate, we also trigger the holding time, to pause the new transtion
void RandRateCtrlPS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;
        RC->update();
        
        //if the end rate has been reached, we need to choose a new one
        //and pause for the holdTime
        if(RC->rateReached){
            //if we haven't started holding, set the flag
            //and record the start time
            if(!holdActive){
                holdActive = true;
                holdStartTime = currentTime;
            }

            //If we've passed the hold time, cancel the hold flag
            //It's time to choose a new end rate to transition to
            if( ( currentTime - holdStartTime ) >= holdTime){
                holdActive = false;
                //Since we're starting a new transtion, 
                //we set the starting rate of the RateCtrl instance
                //as whatever we ended the current one at
                RC->startRate = RC->outputRate;
                //choose a new rate, and set it to the end rate of the RateCtrl instance 
                RR->update(); 
                RC->endRate = RR->outputRate;
                //reset the RateCtrl instance to start the new transtion
                //(this sets rateReached to false)
                RC->reset();
            }
        }
    }
}
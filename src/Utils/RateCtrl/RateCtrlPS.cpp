#include "RateCtrlPS.h"

RateCtrlPS::RateCtrlPS(uint16_t StartRate, uint16_t EndRate, bool Easing, bool Cycle, uint16_t Rate):
startRate(StartRate), endRate(EndRate), easing(Easing), cycle(Cycle)
{
    //bind the rate vars since they are inherited from BaseEffectPS
    bindClassRatesPS();
    reset();
}

void RateCtrlPS::reset(){
    rateReached = false;
    outputRate = startRate;
    prevTime = 0;
    if(startRate < endRate){
        setDirect(true);
    } else {
        setDirect(false);
    }
}

void RateCtrlPS::reset(uint16_t StartRate, uint16_t EndRate, uint16_t Rate ){
    startRate = StartRate;
    endRate = EndRate;
    bindClassRatesPS();
    reset();
}

bool RateCtrlPS::direct(void){
    return _direct;
}

void RateCtrlPS::setDirect(bool newDirect){
    _direct = newDirect;
    if(_direct){
        stepMulti = 1;
    } else {
        stepMulti = -1;
    }
}

void RateCtrlPS::update(){
    currentTime = millis();
    if( rateReached && cycle && ( currentTime - prevTime ) >= changeHold ){ reset(); }

    //if it's time to update the effect, do so
    if( !rateReached && ( currentTime - prevTime )  >= *rate ) {
        prevTime = currentTime;
        outputRate += stepMulti;
        if(easing){
            uint8_t ratio;
            if(_direct){
                ratio = (outputRate * 255)/endRate;
            } else {
                ratio = (endRate * 255)/outputRate;
            }
            rateTemp = rateOrig * ease8InOutApprox(ratio) / 255;
            rate = &rateTemp;
        } else {
            rate = &rateOrig;
        }

        if( (_direct && outputRate >= endRate)  || (!_direct && outputRate <= endRate) ){
            rateReached = true;
            if(cycle == true){
                uint16_t tempRate = startRate;
                startRate = endRate;
                endRate = tempRate;
            }
        }
    }

}


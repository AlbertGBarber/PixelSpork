#include "RateCtrlPS.h"

RateCtrlPS::RateCtrlPS(uint16_t StartRate, uint16_t EndRate, bool Easing, bool Cycle, uint16_t ChangeRate):
startRate(StartRate), endRate(EndRate), easing(Easing), cycle(Cycle), changeRate(ChangeRate), changeRateTemp(ChangeRate)
{
    reset();
}

void RateCtrlPS::reset(){
    rateReached = false;
    rate = startRate;
    prevTime = 0;
    if(startRate < endRate){
        setDirect(true);
    } else {
        setDirect(false);
    }
}

void RateCtrlPS::reset(uint16_t StartRate, uint16_t EndRate, uint16_t ChangeRate ){
    startRate = StartRate;
    endRate = EndRate;
    changeRate = ChangeRate;
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
    if( !rateReached && ( currentTime - prevTime )  >= changeRateTemp ) {
        prevTime = currentTime;
        rate += stepMulti;
        if(easing){
            uint8_t ratio;
            if(_direct){
                ratio = (rate * 255)/endRate;
            } else {
                ratio = (endRate * 255)/rate;
            }
            changeRate = changeRate * ease8InOutApprox(ratio) / 255; 
        } else {
            changeRateTemp = changeRate;
        }

        if( (_direct && rate >= endRate)  || (!_direct && rate <= endRate) ){
            rateReached = true;
            if(cycle == true){
                uint16_t tempRate = startRate;
                startRate = endRate;
                endRate = tempRate;
            }
        }
    }

}


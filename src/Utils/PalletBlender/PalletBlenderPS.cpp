#include "PalletBlenderPS.h"

PalletBlenderPS::PalletBlenderPS(palletPS *StartPallet, palletPS *EndPallet, bool Looped, uint8_t TotalSteps, uint16_t Rate):
endPallet(EndPallet), startPallet(StartPallet), looped(Looped), totalSteps(TotalSteps)
{   
    //bind the rate vars since they are inherited from BaseEffectPS
    bindClassRatesPS();
    reset( startPallet, endPallet);
}

//updates the blended pallet at the passed in rate
void PalletBlenderPS::update(){
    currentTime = millis();
    globalRateCheckPS();
    //if the blend is active, and enougth time has passed, update the pallet
    if( !blendEnd && ( ( currentTime - prevTime ) >= rate ) ){
        prevTime = currentTime;
        //for each color in the blend pallet, blend it towards a color in the end pallet
        //using the getCrossFadeColor function (see segDrawUtils.h)
        //it doesn't matter if one pallet is shorter than the other, b/c pallets wrap automatically
        for(int i = 0; i < blendPallet.length; i++){
            CRGB startColor = palletUtilsPS::getPalletColor(startPallet, i);
            CRGB endColor = palletUtilsPS::getPalletColor(endPallet, i);
            CRGB newColor = segDrawUtils::getCrossFadeColor(startColor, endColor, step, totalSteps);
            palletUtilsPS::setColor(&blendPallet, newColor, i);
            step++;
        }
        //if we have reached the totalSteps,
        //the blend pallet has reached the end pallet
        //set flags, and loop if needed
        if(step == totalSteps){
            blendEnd = true;
            if(looped){
                reset(endPallet, startPallet);
            }
        }
    }
}

//resets the core class variables, allowing you to reuse class instances
void PalletBlenderPS::reset(palletPS *StartPallet, palletPS *EndPallet, uint8_t TotalSteps, uint16_t Rate){
    reset();
    reset(StartPallet, EndPallet);
    totalSteps = TotalSteps;
    rate = Rate;
}

//resets just the start and end pallets
void PalletBlenderPS::reset(palletPS *StartPallet, palletPS *EndPallet){
    reset();
    //if we are randomizing, choose a randomized end pallet
    if(randomize){
        palletUtilsPS::randomize(EndPallet);
    }
    //setup the blendPallet pallet
    //it must be as long as the longest passed in pallet
    //this way we can always pair up a start and end color (even if the colors repeat)
    //otherwise you miss colors because you wouldn't have space for them
    endPallet = EndPallet;
    startPallet = StartPallet;
    uint8_t blendPalletLength = max(startPallet->length, endPallet->length);
    //delete the current (if there is one) blendPallet array of colors to free up memory
    //then create a new one, and pass that to a pallet
    //this has to be done this way so that the blendPallet array doesn't vanish after this reset() is finished
    delete[] blendPallet_arr;
    blendPallet_arr = new CRGB[blendPalletLength];
    blendPallet = {blendPallet_arr, blendPalletLength};
}

//reset the loop variables, basically starting the loop from scratch
void PalletBlenderPS::reset(){
    step = 0;
    prevTime = 0;
    blendEnd = false;
}

#include "PalletBlenderPS.h"

PalletBlenderPS::PalletBlenderPS(palletPS *StartPallet, palletPS *EndPallet, bool Looped, uint8_t TotalSteps, uint16_t Rate):
    endPallet(EndPallet), startPallet(StartPallet), looped(Looped), totalSteps(TotalSteps)
    {   
        //bind the rate vars since they are inherited from BaseEffectPS
        bindClassRatesPS();
        reset( startPallet, endPallet );
    }

PalletBlenderPS::~PalletBlenderPS(){
    delete[] blendPallet_arr;
}

//resets the core class variables, allowing you to reuse class instances
void PalletBlenderPS::reset(palletPS *StartPallet, palletPS *EndPallet, uint8_t TotalSteps, uint16_t Rate){
    reset();
    reset(StartPallet, EndPallet);
    totalSteps = TotalSteps;
    bindClassRatesPS();
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
    uint8_t blendPalletLengthTemp = max(startPallet->length, endPallet->length);
    //if the new blend pallet will be the same length as the current blend pallet
    //we don't need to make a new one
    if(blendPalletLengthTemp != blendPalletLength){
        setupBlendPallet(blendPalletLength);
    }
}

//reset the loop variables, basically starting the loop from scratch
void PalletBlenderPS::reset(){
    step = 0;
    prevTime = 0;
    blendEnd = false;
    holdActive = false;
}

//creates the blend pallet and the color storage arrays
void PalletBlenderPS::setupBlendPallet( uint8_t newBlendPalletLength){
    //delete the current (if there is one) blendPallet array of colors to free up memory
    //then create a new one, and pass that to a pallet
    //this has to be done this way so that the blendPallet array doesn't vanish after this function is finished
    blendPalletLength = newBlendPalletLength;
    delete[] blendPallet_arr;
    blendPallet_arr = new CRGB[blendPalletLength];
    blendPallet = {blendPallet_arr, blendPalletLength};
    update(); //update once to fill in the blendPallet
}

//updates the blended pallet at the passed in rate
void PalletBlenderPS::update(){
    currentTime = millis();

    //if the blend is active, and enougth time has passed, update the pallet
    if( ( currentTime - prevTime ) >= *rate ){
        if(!holdActive && !blendEnd){
            prevTime = currentTime;
            //for each color in the blend pallet, blend it towards a color in the end pallet
            //using the getCrossFadeColor function (see segDrawUtils.h)
            //it doesn't matter if one pallet is shorter than the other, b/c pallets wrap automatically
            for(uint8_t i = 0; i < blendPallet.length; i++){
                CRGB startColor = palletUtilsPS::getPalletColor(startPallet, i);
                CRGB endColor = palletUtilsPS::getPalletColor(endPallet, i);
                CRGB newColor = segDrawUtils::getCrossFadeColor(startColor, endColor, step, totalSteps);
                palletUtilsPS::setColor(&blendPallet, newColor, i);
                step++;
            }
            //if we have reached the totalSteps,
            //the blend pallet has reached the end pallet
            //set flags, and loop if needed
            if(step >= totalSteps){
                blendEnd = true;
                holdActive = true;
                holdStartTime = currentTime;
            }
        //if we're in a hold, check if the hold time has passed
        } else if( ( currentTime - holdStartTime ) >= holdTime) {
            holdActive = false;
            if(looped){
                reset(endPallet, startPallet);
            }
        }
    }
}

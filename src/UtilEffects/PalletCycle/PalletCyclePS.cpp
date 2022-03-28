#include "PalletCyclePS.h"

PalletCyclePS::PalletCyclePS(palletSetPS *PalletSet, bool Looped, uint8_t TotalSteps, uint16_t Rate):
    palletSet(PalletSet), looped(Looped)
    {    
        bindClassRatesPS();
        //create an instance of PalletBlenderPS to do the blends
        //the initial start/end pallets are set just to setup the PalletBlenderPS instance
        //we'll change the initial start and end pallets officially below
        currentIndex = 0;
        nextIndex = addmod8( currentIndex, 1, palletSet->length ); //(currentIndex + 1) % palletSet->length;
        PB = new PalletBlenderPS(palletSet->getPallet(currentIndex), palletSet->getPallet(nextIndex), false, TotalSteps, Rate);
        //point the PB update rate to the same rate as the PalletCyclePS instance, so they stay in sync
        PB->rate = rate;
        //setup the initial pallet blend
        reset(palletSet);
        //bind the output pallet of PB to the output of the PalletCycle 
        cyclePallet = &PB->blendPallet;
	}

PalletCyclePS::~PalletCyclePS(){
    PB->~PalletBlenderPS();
}

//restarts the blend cycle
void PalletCyclePS::reset(){
    cycleEnd = false;
    currentIndex = 0;
    nextIndex = addmod8( currentIndex, 1, palletSet->length ); //(currentIndex + 1) % palletSet->length;
    PB->startPallet = palletSet->getPallet(currentIndex);
    PB->endPallet = palletSet->getPallet(nextIndex);
    PB->reset();
}

//resets the blendCycle with a new pallet set
//To make sure the blend works well with various effects, we manually set the length of the blendPallet to be the same each cycle
//by pre-determining the longest pallet in the pallet set and setting the blendPallet's length to match
//(the length usually would change depending on the lengths of the start and end pallets. which might change each cycle)
//This should be ok because pallets wrap. So the blendPallet might have some repeated colors, but will still
//be the correct blend overall. (see Notes for more on this)
void PalletCyclePS::reset(palletSetPS *newPalletSet){
    palletSet = newPalletSet;
    //choose a new maximum pallet length based on the pallets in the pallet set
    maxPalletLength = 0;
    for(uint8_t i = 0; i < palletSet->length; i++){
        uint8_t palletLength = ( palletSet->getPallet(currentIndex) )->length;
        if( palletLength > maxPalletLength){
            maxPalletLength = palletLength;
        }
    }
    //manually setup the PalletBlendPS's blendPallet using the maximum pallet length
    PB->setupBlendPallet( maxPalletLength );
    reset();
}

//sets the total steps used in the blends (see PalletBlendPS)
void PalletCyclePS::setTotalSteps(uint8_t newTotalSteps){
    PB->totalSteps = newTotalSteps;
}

//sets the hold time between each pallet blend (see PalletBlendPS)
void PalletCyclePS::setHoldTime(uint16_t newHoldTime){
    PB->holdTime = newHoldTime;
}

//returns the current totalSteps in the PalletBlendPS instance
uint8_t PalletCyclePS::getTotalSteps(){
    return PB->totalSteps;
}

//updates the blend
//each time, we update the PalletBlendPS instance
//if the current blend has ended, and the hold has finished
//we check if we're reached the last pallet in the set, if we have (and we're not looping)
//we flag cycleEnd, and stop blending
//otherwise we move on to the next pallet and start the blend again
void PalletCyclePS::update(){
    currentTime = millis();
    
    if( (!cycleEnd || looped) && ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;
        PB->update();

        //if we've finished the current blend (and hold time), we need to move onto the next one 
        if( PB->blendEnd && !PB->holdActive){
            palletLength = palletSet->length;
            //if we're not looping, and the end pallet was the last on in the set, we need to set the end flag
            //otherwise move onto the next pallet
            if(!looped && ( nextIndex >= (palletLength - 1) ) ){
                cycleEnd = true;
            } else {
                //setup the next pallet to blend to
                //the starting pallet is the one we're just blended to (ie at nextIndex)
                currentIndex = nextIndex;
                nextIndex = mod8( (currentIndex + 1), palletLength );
                //if we're randomizing, we randomize the next pallet
                if(randomize){
                    palletUtilsPS::randomize( palletSet->getPallet(nextIndex) );
                }
                //set the pallets in the PalletBlendPS instance manually
                //(we're keeping close control of the length of the blendPallet (see note))
                //so we don't trigger any pallet re-sizing
                //then reset the PB to start the next blend
                PB->startPallet = palletSet->getPallet(currentIndex);
                PB->endPallet = palletSet->getPallet(nextIndex);
                PB->reset();
            }
        }
    }
    
}
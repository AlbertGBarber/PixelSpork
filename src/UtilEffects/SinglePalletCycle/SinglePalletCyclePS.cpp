#include "SinglePalletCyclePS.h"


SinglePalletCyclePS::SinglePalletCyclePS(palletPS *Pallet, uint8_t BlendMode, uint8_t TotalSteps, uint16_t Rate):
    palletOrig(Pallet), blendMode(BlendMode)
    {     
        bindClassRatesPS();
        //make sure all the pallet variables set so that new current/next pallets will be created
        cycleNum = 0;
        palletLength = 0;
        prevMode = 0;
        //create the initial current/next pallets based on the mode
        switchPallet();
        //create a PalletBlenderPS instance 
        PB = new PalletBlenderPS( &currentPallet, &nextPallet, false, TotalSteps, Rate);
        //point the PB update rate to the same rate as the SinglePalletCyclePS instance, so they stay in sync
        PB->rate = rate;
        //bind the output pallet of PB to the output of the SinglePalletCycle 
        cyclePallet = PB->blendPallet;
	}

SinglePalletCyclePS::~SinglePalletCyclePS(){
    delete[] palletColorArr1;
    delete[] palletColorArr2;
    PB->~PalletBlenderPS();
}

//restarts the pallet blend
void SinglePalletCyclePS::reset(){
    cycleNum = 0;
    PB->reset();
}

//sets the total steps used in the PalletBlenderPS instance
void SinglePalletCyclePS::setTotalSteps(uint8_t newTotalSteps){
    PB->totalSteps = newTotalSteps;
}

//returns the total steps used in the PalletBlenderPS instance
uint8_t SinglePalletCyclePS::getTotalSteps(){
    return PB->totalSteps;
}

//sets the hold time used in the PalletBlenderPS instance
void SinglePalletCyclePS::setHoldTime(uint16_t newHoldTime){
    PB->holdTime = newHoldTime;
}

//changes the pallet used as the basis for the blends
//also restarts the cycle
//this may change the cyclePallet's length
void SinglePalletCyclePS::setPallet(palletPS *pallet){
    palletOrig = pallet;
    switchPallet();
    PB->reset( &currentPallet, &nextPallet );
}

//sets the currentPallet and nextPallet depending on the cycleNum and the blendMode
//the cycle num varies from 0 to the original pallet's length -1
void SinglePalletCyclePS::switchPallet(){
    //create and allocate memory for the next/current pallets to be used in the blends
    //we only need to do this if the basis pallet (palletOrig) has changed and if the it's length is different than the previous pallet
    //otherwise we can keep using the existing next/current pallets
    //if the pallet's are created, set them both to match the original pallet
    uint8_t palletLengthTemp = palletOrig->length;
    if(palletLength != palletLengthTemp){
        palletLength = palletLengthTemp;
        delete[] palletColorArr1;
        delete[] palletColorArr2;
        palletColorArr1 = new CRGB[palletLength];
        palletColorArr2 = new CRGB[palletLength];
        currentPallet = { palletColorArr1, palletLength };
        nextPallet = { palletColorArr2, palletLength };
        for(uint8_t i = 0; i < palletLength; i++ ){
            palletColorArr1[i] = palletUtilsPS::getPalletColor(palletOrig, i);
            palletColorArr1[i] = palletUtilsPS::getPalletColor(palletOrig, i);
        }
    }

    //set the current/next pallet lengths (if not already set)
    //some blend modes need different pallet lengths
    //we can adjust the current/next pallet's length without actually changing their color array size
    //this keeps things simple, and makes it easier to switch modes
    if(prevMode != blendMode){
        prevMode = blendMode;
        if(blendMode <= 2){ //for blend modes 0-2
            currentPallet.length = palletLength;
            nextPallet.length = palletLength;
        } else { //for blend modes 3 and up
            currentPallet.length = 1;
            nextPallet.length = 1;
        }
    }

    //some blend modes allow for a direction setting
    //Note that for true, we want the colors in the pallet to move "forward"
    //this means we actually have to look back as we set them, hence the -1
    if(direct) {
        stepDirect = -1;
    } else {
        stepDirect = 1;
    }

    //Sets up the pallets according to the mode 
    //Modes:
    //0 -- Cycles the entire pallet by one step each cycle. 
    //     ie for direct = true, {blue, red, green} => {green, blue, red} => {red, green, blue} , etc
    //1 -- Like mode 0, but a random color is inserted at the beginning (end if direct is false) each cycle, (initially starts as pallet)
    //     ie for direct = true, {rand1, rand2, rand3} => {rand4, rand1, rand2} => {rand5, rand4, rand1}, etc
    //2 -- shuffles the pallet each cycle
    //     ie {blue, red, green} could go to {red, blue, green}, or {blue, green, red}, etc 
    //     direct has no effect
    //3 -- Makes the pallet length 1, cycles through each color of the pallet
    //     ie for direct = true and pallet {blue, red, green} it will be {blue} => {red} => {green} => {blue}, etc
    //4 -- Same as previous mode, but chooses the next color randomly from the pallet (will not be the current color)
    //5 -- Same as mode 3, but the next color is choosen completely randomly (not from the pallet)
    switch (blendMode){
        case 0: {
            //copy the next pallet into the current pallet
            //then copy the original pallet into the next pallet shifited forward/backwards by 1
            for(uint8_t i = 0; i < palletLength; i++ ){
                currentIndex = (i + stepDirect * (cycleNum + 1) ) + palletLength; //we add palletLength to prevent this from going negative
                palletColorArr1[i] = palletUtilsPS::getPalletColor(&nextPallet, i);
                palletColorArr2[i] = palletUtilsPS::getPalletColor(palletOrig, mod8(currentIndex, palletLength) );
            }
            break;
        }
        case 1: {
            //similar to case 0, but we are setting either the first or last color randomly, depending on direction
            //so we need to read from the nextPallet itself, rather than the original, to get the next pallet
            //this means we need to be careful what direction we read in
            //we do not want to read from an index that has already been set, or we'll copy the same color down the 
            //whole pallet
            uint8_t loopStart = 0;
            uint8_t loopEnd = palletLength - 1;
            if(direct){
                loopStart = loopEnd;
                loopEnd = 0;
            }

            for(uint8_t i = loopStart; i != loopEnd; i += stepDirect ){
                currentIndex = (i + stepDirect * (cycleNum + 1) ) + palletLength;
                palletColorArr1[i] = palletUtilsPS::getPalletColor(&nextPallet, i);
                palletColorArr2[i] = palletUtilsPS::getPalletColor(&nextPallet, mod8(currentIndex, palletLength) );
            }

            //randomize the start or end color
            if(direct){
                palletUtilsPS::randomize(&nextPallet, 0);
            } else {
                palletUtilsPS::randomize(&nextPallet, palletLength - 1);
            }
            break;
        }
        case 2: {
            //copy the next pallet into the current pallet
            //then copy the original pallet into the next pallet (to account for any pallet changes)
            for(uint8_t i = 0; i < palletLength; i++ ){
                palletColorArr1[i] = palletUtilsPS::getPalletColor(&nextPallet, i);
                palletColorArr2[i] = palletUtilsPS::getPalletColor(palletOrig, i);
            }
            //since the next pallet always ends up as the original pallet again,
            //when we shuffle we may end up as the current pallet again, 
            //to reduce this chance we'll shuffle twice
            //(An absolute solution would be to shuffle the original pallet, then copy it in
            //but we want to avoid modifying the original)
            palletUtilsPS::shuffle(&nextPallet); 
            palletUtilsPS::shuffle(&nextPallet);
            break;
        }
        case 3: {
            //cycles through each color in the original pallet
            //the output is a pallet with a single color of length 1
            currentIndex = ( stepDirect * (cycleNum + 1) ) + palletLength;
            palletColorArr1[0] = palletUtilsPS::getPalletColor(&nextPallet, 0);
            palletColorArr2[0] = palletUtilsPS::getPalletColor(palletOrig, mod8(currentIndex, palletLength) );
            break;
        }
        case 4: {
            //like case 3, but the next color is choosen randomly from the original pallet
            //(if the choosen color is the same as the current one, we'll just choose the next one along)
            uint8_t nextIndex = random(palletLength); 
            if(nextIndex == currentIndex){
                nextIndex = (currentIndex + 1) % palletLength;
            }
            palletColorArr1[0] = palletUtilsPS::getPalletColor(palletOrig, currentIndex);
            palletColorArr2[0] = palletUtilsPS::getPalletColor(palletOrig, nextIndex);
            currentIndex = nextIndex;
            break;
        }
        default: //mode 5 and up
            //a single color pallet where the color is choosen randomly,
            //doesn't use the original pallet at all
            palletColorArr1[0] = palletUtilsPS::getPalletColor(&nextPallet, 0);
            palletUtilsPS::randomize(&nextPallet, 0);
            break;
    }

}

//updates the blend
void SinglePalletCyclePS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;
        //update the PalletBlendPS instance
        PB->update();

        //if we've finished the current blend (and hold time), we need to move onto the next one
        if(PB->blendEnd && !PB->holdActive){
            cycleNum = addmod8( cycleNum, 1, palletOrig->length );
            switchPallet();
            PB->reset( &currentPallet, &nextPallet );
        }
    }
}
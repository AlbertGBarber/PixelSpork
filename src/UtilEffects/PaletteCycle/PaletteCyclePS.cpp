#include "PaletteCyclePS.h"

PaletteCyclePS::PaletteCyclePS(paletteSetPS *PaletteSet, bool Looped, uint8_t TotalSteps, uint16_t Rate):
    paletteSet(PaletteSet), looped(Looped)
    {    
        bindClassRatesPS();
        //create an instance of PaletteBlenderPS to do the blends
        //the initial start/end palettes are set just to setup the PaletteBlenderPS instance
        //we'll change the initial start and end palettes officially below
        currentIndex = 0;
        nextIndex = addmod8( currentIndex, 1, paletteSet->length ); //(currentIndex + 1) % paletteSet->length;
        PB = new PaletteBlenderPS(paletteSet->getPalette(currentIndex), paletteSet->getPalette(nextIndex), false, TotalSteps, Rate);
        //point the PB update rate to the same rate as the PaletteCyclePS instance, so they stay in sync
        PB->rate = rate;
        //setup the initial palette blend
        reset(paletteSet);
        //bind the output palette of PB to the output of the PaletteCycle 
        cyclePalette = &PB->blendPalette;
	}

PaletteCyclePS::~PaletteCyclePS(){
    PB->~PaletteBlenderPS();
}

//restarts the blend cycle
void PaletteCyclePS::reset(){
    cycleEnd = false;
    currentIndex = 0;
    nextIndex = addmod8( currentIndex, 1, paletteSet->length ); //(currentIndex + 1) % paletteSet->length;
    PB->startPalette = paletteSet->getPalette(currentIndex);
    PB->endPalette = paletteSet->getPalette(nextIndex);
    PB->reset();
}

//resets the blendCycle with a new palette set
//To make sure the blend works well with various effects, we manually set the length of the blendPalette to be the same each cycle
//by pre-determining the longest palette in the palette set and setting the blendPalette's length to match
//(the length usually would change depending on the lengths of the start and end palettes. which might change each cycle)
//This should be ok because palettes wrap. So the blendPalette might have some repeated colors, but will still
//be the correct blend overall. (see Notes for more on this)
void PaletteCyclePS::reset(paletteSetPS *newPaletteSet){
    paletteSet = newPaletteSet;
    //choose a new maximum palette length based on the palettes in the palette set
    maxPaletteLength = 0;
    for(uint8_t i = 0; i < paletteSet->length; i++){
        uint8_t paletteLength = ( paletteSet->getPalette(currentIndex) )->length;
        if( paletteLength > maxPaletteLength){
            maxPaletteLength = paletteLength;
        }
    }
    //manually setup the PaletteBlendPS's blendPalette using the maximum palette length
    PB->setupBlendPalette( maxPaletteLength );
    reset();
}

//sets the total steps used in the blends (see PaletteBlendPS)
void PaletteCyclePS::setTotalSteps(uint8_t newTotalSteps){
    PB->totalSteps = newTotalSteps;
}

//sets the hold time between each palette blend (see PaletteBlendPS)
void PaletteCyclePS::setHoldTime(uint16_t newHoldTime){
    PB->holdTime = newHoldTime;
}

//returns the current totalSteps in the PaletteBlendPS instance
uint8_t PaletteCyclePS::getTotalSteps(){
    return PB->totalSteps;
}

//updates the blend
//each time, we update the PaletteBlendPS instance
//if the current blend has ended, and the hold has finished
//we check if we're reached the last palette in the set, if we have (and we're not looping)
//we flag cycleEnd, and stop blending
//otherwise we move on to the next palette and start the blend again
void PaletteCyclePS::update(){
    currentTime = millis();
    
    if( (!cycleEnd || looped) && ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;
        PB->update();

        //if we've finished the current blend (and hold time), we need to move onto the next one 
        if( PB->blendEnd && !PB->holdActive){
            paletteLength = paletteSet->length;
            //if we're not looping, and the end palette was the last on in the set, we need to set the end flag
            //otherwise move onto the next palette
            if(!looped && ( nextIndex >= (paletteLength - 1) ) ){
                cycleEnd = true;
            } else {
                //setup the next palette to blend to
                //the starting palette is the one we're just blended to (ie at nextIndex)
                currentIndex = nextIndex;
                nextIndex = mod8( (currentIndex + 1), paletteLength );
                //if we're randomizing, we randomize the next palette
                if(randomize){
                    paletteUtilsPS::randomize( paletteSet->getPalette(nextIndex) );
                }
                //set the palettes in the PaletteBlendPS instance manually
                //(we're keeping close control of the length of the blendPalette (see note))
                //so we don't trigger any palette re-sizing
                //then reset the PB to start the next blend
                PB->startPalette = paletteSet->getPalette(currentIndex);
                PB->endPalette = paletteSet->getPalette(nextIndex);
                PB->reset();
            }
        }
    }
    
}
#include "PaletteCyclePS.h"

PaletteCyclePS::PaletteCyclePS(paletteSetPS &PaletteSet, bool Looped, bool Randomize, bool Shuffle,
                               bool StartPaused, uint16_t PauseTime, uint8_t TotalSteps, uint16_t Rate)
    : paletteSet(&PaletteSet), looped(Looped), randomize(Randomize), shuffle(Shuffle)  //
{
    bindClassRatesPS();
    //create an instance of PaletteBlenderPS to do the blends
    //the initial start/end palettes are set just to setup the PaletteBlenderPS instance
    //we'll change the initial start and end palettes officially below
    currentIndex = 0;
    nextIndex = addmod8(currentIndex, 1, paletteSet->length);
    PB = new PaletteBlenderPS(*paletteSet->getPalette(currentIndex), *paletteSet->getPalette(nextIndex), false, TotalSteps, Rate);
    setStartPaused(StartPaused);
    setPauseTime(PauseTime);
    //point the PB update rate to the same rate as the PaletteCyclePS instance, so they stay in sync
    PB->rate = rate;
    //setup the initial palette blend
    reset(*paletteSet);
    //bind the output palette of PB to the output of the PaletteCycle
    cyclePalette = &PB->blendPalette;
}

PaletteCyclePS::~PaletteCyclePS() {
    PB->~PaletteBlenderPS();
}

//restarts the blend cycle
void PaletteCyclePS::reset() {
    cycleNum = 0;
    done = false;
    currentIndex = 0;
    nextIndex = addmod8(currentIndex, 1, paletteSet->length);
    PB->startPalette = paletteSet->getPalette(currentIndex);
    PB->endPalette = paletteSet->getPalette(nextIndex);
    PB->reset();
}

/* resets the blendCycle with a new palette set
To make sure the blend works well with various effects, we manually set the length of the blendPalette to be the same each cycle
by pre-determining the longest palette in the palette set and setting the blendPalette's length to match
(the length usually would change depending on the lengths of the start and end palettes. which might change each cycle)
This should be ok because palettes wrap. So the blendPalette might have some repeated colors, but will still
be the correct blend overall. (see Notes for more on this) */
void PaletteCyclePS::reset(paletteSetPS &newPaletteSet) {
    paletteSet = &newPaletteSet;
    //choose a new maximum palette length based on the palettes in the palette set
    maxPaletteLength = 0;
    for( uint8_t i = 0; i < paletteSet->length; i++ ) {
        uint8_t paletteLength = (paletteSet->getPalette(i))->length;
        if( paletteLength > maxPaletteLength ) {
            maxPaletteLength = paletteLength;
        }
    }
    //manually setup the PaletteBlendPS's blendPalette using the maximum palette length
    PB->setupBlendPalette(maxPaletteLength);
    reset();
}

//sets the total steps used in the blends (see PaletteBlenderPS)
void PaletteCyclePS::setTotalSteps(uint8_t newTotalSteps) {
    PB->totalSteps = newTotalSteps;
}

//sets the pause time between each palette blend (see PaletteBlenderPS)
void PaletteCyclePS::setPauseTime(uint16_t newPauseTime) {
    PB->pauseTime = newPauseTime;
}

//sets the "startPaused" property of the palette blender (see PaletteBlenderPS)
void PaletteCyclePS::setStartPaused(bool newStartPaused) {
    PB->startPaused = newStartPaused;
}

//updates the blend
//each time, we update the PaletteBlendPS instance
//if the current blend has ended, and the pause has finished
//we check if we're reached the last palette in the set, if we have (and we're not looping)
//we flag done, and stop blending
//otherwise we move on to the next palette and start the blend again
void PaletteCyclePS::update() {
    currentTime = millis();

    if( active && (!done || looped) && (currentTime - prevTime) >= *rate ) {
        prevTime = currentTime;
        PB->update();

        //if we've finished the current blend (and pause time), we need to move onto the next one
        if( PB->blendEnd && !PB->paused ) {
            paletteSetLen = paletteSet->length;
            //if we're not looping, and the end palette was the last on in the set, we need to set the end flag
            //otherwise move onto the next palette
            if( !looped && (cycleNum >= (paletteSetLen - 1)) ) {
                done = true;
            } else {
                //Track how many blend's we've done
                cycleNum++;
                //setup the next palette to blend to
                //the starting palette is the one we're just blended to (ie at nextIndex)
                currentIndex = nextIndex;
                nextIndex = mod8((currentIndex + 1), paletteSetLen);

                //If we're shuffling, ie choosing the next palette at random, but making sure it's not the current palette
                //Then we pick a random palette. If it's not the same as the current palette, we keep it.
                //Otherwise we just go with the current nextIndex (the nest palette in line)
                if( shuffle ) {
                    indexGuess = random8(paletteSetLen);
                    if( indexGuess != currentIndex ) {
                        nextIndex = indexGuess;
                    }
                }

                //If we're randomizing
                if( randomize ) {
                    paletteUtilsPS::randomize(*paletteSet->getPalette(nextIndex), compliment);
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
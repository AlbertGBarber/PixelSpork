#include "PaletteBlenderPS.h"

PaletteBlenderPS::PaletteBlenderPS(palettePS *StartPalette, palettePS *EndPalette, bool Looped, uint8_t TotalSteps, uint16_t Rate):
    endPalette(EndPalette), startPalette(StartPalette), looped(Looped), totalSteps(TotalSteps)
    {   
        //bind the rate vars since they are inherited from BaseEffectPS
        bindClassRatesPS();
        reset( startPalette, endPalette );
    }

PaletteBlenderPS::~PaletteBlenderPS(){
    delete[] blendPalette_arr;
}

//resets the core class variables, allowing you to reuse class instances
void PaletteBlenderPS::reset(palettePS *StartPalette, palettePS *EndPalette, uint8_t TotalSteps, uint16_t Rate){
    reset();
    reset(StartPalette, EndPalette);
    totalSteps = TotalSteps;
    bindClassRatesPS();
}

//resets just the start and end palettes
void PaletteBlenderPS::reset(palettePS *StartPalette, palettePS *EndPalette){
    reset();
    //if we are randomizing, choose a randomized end palette
    if(randomize){
        paletteUtilsPS::randomize(EndPalette);
    }
    //setup the blendPalette palette
    //it must be as long as the longest passed in palette
    //this way we can always pair up a start and end color (even if the colors repeat)
    //otherwise you miss colors because you wouldn't have space for them
    endPalette = EndPalette;
    startPalette = StartPalette;
    uint8_t blendPaletteLengthTemp = max(startPalette->length, endPalette->length);
    //if the new blend palette will be the same length as the current blend palette
    //we don't need to make a new one
    if(blendPaletteLengthTemp != blendPaletteLength){
        setupBlendPalette(blendPaletteLength);
    }
}

//reset the loop variables, basically starting the loop from scratch
void PaletteBlenderPS::reset(){
    step = 0;
    prevTime = 0;
    blendEnd = false;
    holdActive = false;
}

//creates the blend palette and the color storage arrays
void PaletteBlenderPS::setupBlendPalette( uint8_t newBlendPaletteLength){
    //delete the current (if there is one) blendPalette array of colors to free up memory
    //then create a new one, and pass that to a palette
    //this has to be done this way so that the blendPalette array doesn't vanish after this function is finished
    blendPaletteLength = newBlendPaletteLength;
    delete[] blendPalette_arr;
    blendPalette_arr = new CRGB[blendPaletteLength];
    blendPalette = {blendPalette_arr, blendPaletteLength};
    update(); //update once to fill in the blendPalette
}

//updates the blended palette at the passed in rate
void PaletteBlenderPS::update(){
    currentTime = millis();

    //if the blend is active, and enougth time has passed, update the palette
    if( ( currentTime - prevTime ) >= *rate ){
        if(!holdActive && !blendEnd){
            prevTime = currentTime;
            //for each color in the blend palette, blend it towards a color in the end palette
            //using the getCrossFadeColor function (see colorUtilsPS.h)
            //it doesn't matter if one palette is shorter than the other, b/c palettes wrap automatically
            for(uint8_t i = 0; i < blendPalette.length; i++){
                startColor = paletteUtilsPS::getPaletteColor(startPalette, i);
                endColor = paletteUtilsPS::getPaletteColor(endPalette, i);
                newColor = colorUtilsPS::getCrossFadeColor(startColor, endColor, step, totalSteps);
                paletteUtilsPS::setColor(&blendPalette, newColor, i);
                step++;
            }
            //if we have reached the totalSteps,
            //the blend palette has reached the end palette
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
                reset(endPalette, startPalette);
            }
        }
    }
}

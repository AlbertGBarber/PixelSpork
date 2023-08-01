#include "PaletteSingleCyclePS.h"

//Constructor with custom direction setting
PaletteSingleCyclePS::PaletteSingleCyclePS(palettePS &InputPalette, uint8_t BlendMode, uint8_t TotalSteps, bool Direct, uint16_t Rate):
    inputPalette(&InputPalette), blendMode(BlendMode), direct(Direct)
    {
        bindClassRatesPS();
        //make sure all the palette variables set so that new current/next palettes will be created
        cycleNum = 0;
        paletteLength = 0;
        prevMode = 0;
        //create the initial current/next palettes based on the mode
        switchPalette();
        //create a PaletteBlenderPS instance 
        PB = new PaletteBlenderPS( currentPalette, nextPalette, false, TotalSteps, Rate );
        //point the PB update rate to the same rate as the PaletteSingleCyclePS instance, so they stay in sync
        PB->rate = rate;
        //bind the output palette of PB to the output of the SinglePaletteCycle 
        cyclePalette = PB->blendPalette;
    }

PaletteSingleCyclePS::~PaletteSingleCyclePS(){
    free(paletteColorArr1);
    free(paletteColorArr2);
    PB->~PaletteBlenderPS();
}

//restarts the palette blend
void PaletteSingleCyclePS::reset(){
    cycleNum = 0;
    switchPalette();
    PB->reset(currentPalette, nextPalette);
}

//sets the total steps used in the PaletteBlenderPS instance
void PaletteSingleCyclePS::setTotalSteps(uint8_t newTotalSteps){
    PB->totalSteps = newTotalSteps;
}

//returns the total steps used in the PaletteBlenderPS instance
uint8_t PaletteSingleCyclePS::getTotalSteps(){
    return PB->totalSteps;
}

//sets the pause time used in the PaletteBlenderPS instance
void PaletteSingleCyclePS::setPauseTime(uint16_t newPauseTime){
    PB->pauseTime = newPauseTime;
}

//sets the currentPalette and nextPalette depending on the cycleNum and the blendMode
//the cycle num varies from 0 to the input palette's length -1
void PaletteSingleCyclePS::switchPalette(){
    //create and allocate memory for the next/current palettes to be used in the blends
    //We only need to make a new palette pair if the current ones aren't large enough
    //This helps prevent memory fragmentation by limiting the number of heap allocations
    //but this may use up more memory overall.
    //If new palette's are created, set them both to match the input palette
    paletteLength = inputPalette->length;
    if( alwaysResizeObjPS || (paletteLength > paletteLenMax) ){
        paletteLenMax = paletteLength;

        //Create two new palettes the same length as inputPalette
        free(paletteColorArr2);
        paletteColorArr1 = (CRGB*) malloc(paletteLength * sizeof(CRGB));
        paletteColorArr2 = (CRGB*) malloc(paletteLength * sizeof(CRGB));
        currentPalette = { paletteColorArr1, paletteLength };
        nextPalette = { paletteColorArr2, paletteLength };

        //Copy the colors from inputPalette into the new palettes
        for(uint8_t i = 0; i < paletteLength; i++ ){
            paletteColorArr1[i] = paletteUtilsPS::getPaletteColor(*inputPalette, i);
            paletteColorArr2[i] = paletteUtilsPS::getPaletteColor(*inputPalette, i);
        }
    }

    //set the current/next palette lengths
    //some blend modes need different palette lengths
    //we can adjust the current/next palette's length without actually changing their color array size
    //this keeps things simple, and makes it easier to switch modes
    if(blendMode <= 3){ //for blend modes 0-3
        currentPalette.length = paletteLength;
        nextPalette.length = paletteLength;
    } else { //for blend modes 4 and up
        currentPalette.length = 1;
        nextPalette.length = 1;
    }

    //some blend modes allow for a direction setting
    //We need to set a direction step: 1 for forward (direct = true) and -1 for backwards (direct = false)
    //The boolean addition below spits out 1 or -1
    stepDirect = direct - !direct;
   
    //Sets up the palettes according to the mode 
    //Modes:
    //0 -- Cycles the entire palette by one step each cycle. 
    //     ie for direct = true, {blue, red, green} => {green, blue, red} => {red, green, blue} , etc
    //1 -- Like mode 0, but a random color is inserted at the beginning (end if direct is false) each cycle, (initially starts as palette)
    //     ie for direct = true, {rand1, rand2, rand3} => {rand4, rand1, rand2} => {rand5, rand4, rand1}, etc
    //2 -- Randomizes the whole palette each cycle for a palette with 3 random colors: {rand1, rand2, rand3} -> {rand4, rand5, rand6}, etc
    //3 -- Shuffles the palette each cycle
    //     ie {blue, red, green} could go to {red, blue, green}, or {blue, green, red}, etc 
    //     direct has no effect
    //4 -- Makes the palette length 1, cycles through each color of the palette
    //     ie for direct = true and palette {blue, red, green} it will be {blue} => {red} => {green} => {blue}, etc
    //5 -- Same as previous mode, but chooses the next color randomly from the palette (will not be the current color)
    //6 -- Same as mode 3, but the next color is chosen completely randomly (not from the palette)
    switch (blendMode){
        case 0: default: {
            //copy the next palette into the current palette
            //then copy the input palette into the next palette shifted forward/backwards by 1
            for(uint8_t i = 0; i < paletteLength; i++ ){
                currentIndex = i + (stepDirect * (cycleNum + 1) + paletteLength); //we add paletteLength to prevent this from going negative
                paletteColorArr1[i] = paletteUtilsPS::getPaletteColor(nextPalette, i);
                paletteColorArr2[i] = paletteUtilsPS::getPaletteColor(*inputPalette, mod8(currentIndex, paletteLength) );
            }
            break;
        }
        case 1: {
            //similar to case 0, but we are setting either the first or last color randomly, depending on direction
            //So we need to copy the nextPalette into the current palette, 
            //Then, shift the nextPalette forward/backwards by 1 depending on the direction,
            //and insert a new random color at either the start/end of the palette

            //Copy the nextPalette into the currentPalette
            for(uint8_t i = 0; i < paletteLength; i++ ){
                paletteColorArr1[i] = paletteUtilsPS::getPaletteColor(nextPalette, i);
            }

            //We need to shift the colors according to the direct, so we change out loop direction to match
            //This prevents us from reading from an index that has already been set, and copying the same color down the whole palette
            uint8_t loopStart = paletteLength - 1;
            uint8_t loopEnd = 0;

            if(direct){
                loopStart = 0;
                loopEnd = paletteLength - 1;
            }

            //Shift the array colors forward/backwards by 1
            for(uint8_t i = loopStart; i != loopEnd; i += stepDirect ){
                currentIndex = i + (stepDirect + paletteLength);
                paletteColorArr2[i] = paletteUtilsPS::getPaletteColor(nextPalette, mod8(currentIndex, paletteLength) );
            }

            //randomize the start or end color
            paletteUtilsPS::randomize(nextPalette, loopEnd);
           
            break;
        }
        case 2: {
            //Randomize the whole palette for each cycle

            //Copy the nextPalette into the currentPalette
            for(uint8_t i = 0; i < paletteLength; i++ ){
                paletteColorArr1[i] = paletteUtilsPS::getPaletteColor(nextPalette, i);
            }

            //Randomize the next palette
            paletteUtilsPS::randomize(nextPalette);
           
            break;
        }
        case 3: {
            //copy the next palette into the current palette
            //then copy the input palette into the next palette (to account for any palette changes)
            for(uint8_t i = 0; i < paletteLength; i++ ){
                paletteColorArr1[i] = paletteUtilsPS::getPaletteColor(nextPalette, i);
                paletteColorArr2[i] = paletteUtilsPS::getPaletteColor(*inputPalette, i);
            }
            //since the next palette always ends up as the input palette again,
            //when we shuffle we may end up as the current palette again, 
            //to reduce this chance we'll shuffle twice
            //(An absolute solution would be to always copy and shuffle the input palette, 
            //but we want to avoid modifying the input)
            paletteUtilsPS::shuffle(nextPalette); 
            paletteUtilsPS::shuffle(nextPalette);
            break;
        }
        case 4: {
            //cycles through each color in the input palette
            //the output is a palette with a single color of length 1
            currentIndex = ( stepDirect * (cycleNum + 1) ) + paletteLength;
            paletteColorArr1[0] = paletteUtilsPS::getPaletteColor(nextPalette, 0);
            paletteColorArr2[0] = paletteUtilsPS::getPaletteColor(*inputPalette, mod8(currentIndex, paletteLength) );
            break;
        }
        case 5: {
            //like case 4, but the next color is chosen randomly from the input palette
            //(if the chosen color is the same as the current one, we'll just choose the next one along)
            uint8_t nextIndex = random(paletteLength); 
            if(nextIndex == currentIndex){
                nextIndex = addmod8(currentIndex, 1, paletteLength);
            }
            paletteColorArr1[0] = paletteUtilsPS::getPaletteColor(*inputPalette, currentIndex);
            paletteColorArr2[0] = paletteUtilsPS::getPaletteColor(*inputPalette, nextIndex);
            currentIndex = nextIndex;
            break;
        }
        case 6:
            //a single color palette where the color is chosen randomly,
            //doesn't use the input palette at all
            paletteColorArr1[0] = paletteUtilsPS::getPaletteColor(nextPalette, 0);
            paletteUtilsPS::randomize(nextPalette, 0);
            break;
    }

}

//updates the blend
void PaletteSingleCyclePS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;
        //update the PaletteBlendPS instance
        PB->update();

        //if we've finished the current blend (and pause time), we need to move onto the next one
        if(PB->blendEnd && !PB->paused){
            cycleNum = addmod8( cycleNum, 1, inputPalette->length );
            switchPalette();
            //We use the next palette pair reset of the palette blender
            //This captures if the either of the cycle palettes has changed in length (because the inputPalette or mode was changed)
            //And will create a new blendPalette
            //If they're the same length, paletteBlender will skip creating a new palette.
            PB->reset(currentPalette, nextPalette);
        }
    }
}
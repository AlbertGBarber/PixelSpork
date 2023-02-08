#include "SinglePaletteCyclePS.h"


SinglePaletteCyclePS::SinglePaletteCyclePS(palettePS *Palette, uint8_t BlendMode, uint8_t TotalSteps, uint16_t Rate):
    paletteOrig(Palette), blendMode(BlendMode)
    {     
        bindClassRatesPS();
        //make sure all the palette variables set so that new current/next palettes will be created
        cycleNum = 0;
        paletteLength = 0;
        prevMode = 0;
        //create the initial current/next palettes based on the mode
        switchPalette();
        //create a PaletteBlenderPS instance 
        PB = new PaletteBlenderPS( &currentPalette, &nextPalette, false, TotalSteps, Rate);
        //point the PB update rate to the same rate as the SinglePaletteCyclePS instance, so they stay in sync
        PB->rate = rate;
        //bind the output palette of PB to the output of the SinglePaletteCycle 
        cyclePalette = PB->blendPalette;
	}

SinglePaletteCyclePS::~SinglePaletteCyclePS(){
    free(paletteColorArr1);
    free(paletteColorArr2);
    PB->~PaletteBlenderPS();
}

//restarts the palette blend
void SinglePaletteCyclePS::reset(){
    cycleNum = 0;
    PB->reset();
}

//sets the total steps used in the PaletteBlenderPS instance
void SinglePaletteCyclePS::setTotalSteps(uint8_t newTotalSteps){
    PB->totalSteps = newTotalSteps;
}

//returns the total steps used in the PaletteBlenderPS instance
uint8_t SinglePaletteCyclePS::getTotalSteps(){
    return PB->totalSteps;
}

//sets the pause time used in the PaletteBlenderPS instance
void SinglePaletteCyclePS::setPauseTime(uint16_t newPauseTime){
    PB->pauseTime = newPauseTime;
}

//changes the palette used as the basis for the blends
//also restarts the cycle
//this may change the cyclePalette's length
void SinglePaletteCyclePS::setPalette(palettePS *palette){
    paletteOrig = palette;
    switchPalette();
    PB->reset( &currentPalette, &nextPalette );
}

//sets the currentPalette and nextPalette depending on the cycleNum and the blendMode
//the cycle num varies from 0 to the original palette's length -1
void SinglePaletteCyclePS::switchPalette(){
    //create and allocate memory for the next/current palettes to be used in the blends
    //we only need to do this if the basis palette (paletteOrig) has changed and if the it's length is different than the previous palette
    //otherwise we can keep using the existing next/current palettes
    //if the palette's are created, set them both to match the original palette
    uint8_t paletteLengthTemp = paletteOrig->length;
    if(paletteLength != paletteLengthTemp){
        paletteLength = paletteLengthTemp;
        free(paletteColorArr1);
        free(paletteColorArr2);
        paletteColorArr1 = (CRGB*) malloc(paletteLength * sizeof(CRGB));
        paletteColorArr2 = (CRGB*) malloc(paletteLength * sizeof(CRGB));
        currentPalette = { paletteColorArr1, paletteLength };
        nextPalette = { paletteColorArr2, paletteLength };
        for(uint8_t i = 0; i < paletteLength; i++ ){
            paletteColorArr1[i] = paletteUtilsPS::getPaletteColor(paletteOrig, i);
            paletteColorArr1[i] = paletteUtilsPS::getPaletteColor(paletteOrig, i);
        }
    }

    //set the current/next palette lengths (if not already set)
    //some blend modes need different palette lengths
    //we can adjust the current/next palette's length without actually changing their color array size
    //this keeps things simple, and makes it easier to switch modes
    if(prevMode != blendMode){
        prevMode = blendMode;
        if(blendMode <= 2){ //for blend modes 0-2
            currentPalette.length = paletteLength;
            nextPalette.length = paletteLength;
        } else { //for blend modes 3 and up
            currentPalette.length = 1;
            nextPalette.length = 1;
        }
    }

    //some blend modes allow for a direction setting
    //Note that for true, we want the colors in the palette to move "forward"
    //this means we actually have to look back as we set them, hence the -1
    if(direct) {
        stepDirect = -1;
    } else {
        stepDirect = 1;
    }

    //Sets up the palettes according to the mode 
    //Modes:
    //0 -- Cycles the entire palette by one step each cycle. 
    //     ie for direct = true, {blue, red, green} => {green, blue, red} => {red, green, blue} , etc
    //1 -- Like mode 0, but a random color is inserted at the beginning (end if direct is false) each cycle, (initially starts as palette)
    //     ie for direct = true, {rand1, rand2, rand3} => {rand4, rand1, rand2} => {rand5, rand4, rand1}, etc
    //2 -- shuffles the palette each cycle
    //     ie {blue, red, green} could go to {red, blue, green}, or {blue, green, red}, etc 
    //     direct has no effect
    //3 -- Makes the palette length 1, cycles through each color of the palette
    //     ie for direct = true and palette {blue, red, green} it will be {blue} => {red} => {green} => {blue}, etc
    //4 -- Same as previous mode, but chooses the next color randomly from the palette (will not be the current color)
    //5 -- Same as mode 3, but the next color is choosen completely randomly (not from the palette)
    switch (blendMode){
        case 0: {
            //copy the next palette into the current palette
            //then copy the original palette into the next palette shifited forward/backwards by 1
            for(uint8_t i = 0; i < paletteLength; i++ ){
                currentIndex = (i + stepDirect * (cycleNum + 1) ) + paletteLength; //we add paletteLength to prevent this from going negative
                paletteColorArr1[i] = paletteUtilsPS::getPaletteColor(&nextPalette, i);
                paletteColorArr2[i] = paletteUtilsPS::getPaletteColor(paletteOrig, mod8(currentIndex, paletteLength) );
            }
            break;
        }
        case 1: {
            //similar to case 0, but we are setting either the first or last color randomly, depending on direction
            //so we need to read from the nextPalette itself, rather than the original, to get the next palette
            //this means we need to be careful what direction we read in
            //we do not want to read from an index that has already been set, or we'll copy the same color down the 
            //whole palette
            uint8_t loopStart = 0;
            uint8_t loopEnd = paletteLength - 1;
            if(direct){
                loopStart = loopEnd;
                loopEnd = 0;
            }

            for(uint8_t i = loopStart; i != loopEnd; i += stepDirect ){
                currentIndex = (i + stepDirect * (cycleNum + 1) ) + paletteLength;
                paletteColorArr1[i] = paletteUtilsPS::getPaletteColor(&nextPalette, i);
                paletteColorArr2[i] = paletteUtilsPS::getPaletteColor(&nextPalette, mod8(currentIndex, paletteLength) );
            }

            //randomize the start or end color
            if(direct){
                paletteUtilsPS::randomize(&nextPalette, 0);
            } else {
                paletteUtilsPS::randomize(&nextPalette, paletteLength - 1);
            }
            break;
        }
        case 2: {
            //copy the next palette into the current palette
            //then copy the original palette into the next palette (to account for any palette changes)
            for(uint8_t i = 0; i < paletteLength; i++ ){
                paletteColorArr1[i] = paletteUtilsPS::getPaletteColor(&nextPalette, i);
                paletteColorArr2[i] = paletteUtilsPS::getPaletteColor(paletteOrig, i);
            }
            //since the next palette always ends up as the original palette again,
            //when we shuffle we may end up as the current palette again, 
            //to reduce this chance we'll shuffle twice
            //(An absolute solution would be to shuffle the original palette, then copy it in
            //but we want to avoid modifying the original)
            paletteUtilsPS::shuffle(&nextPalette); 
            paletteUtilsPS::shuffle(&nextPalette);
            break;
        }
        case 3: {
            //cycles through each color in the original palette
            //the output is a palette with a single color of length 1
            currentIndex = ( stepDirect * (cycleNum + 1) ) + paletteLength;
            paletteColorArr1[0] = paletteUtilsPS::getPaletteColor(&nextPalette, 0);
            paletteColorArr2[0] = paletteUtilsPS::getPaletteColor(paletteOrig, mod8(currentIndex, paletteLength) );
            break;
        }
        case 4: {
            //like case 3, but the next color is choosen randomly from the original palette
            //(if the choosen color is the same as the current one, we'll just choose the next one along)
            uint8_t nextIndex = random(paletteLength); 
            if(nextIndex == currentIndex){
                nextIndex = (currentIndex + 1) % paletteLength;
            }
            paletteColorArr1[0] = paletteUtilsPS::getPaletteColor(paletteOrig, currentIndex);
            paletteColorArr2[0] = paletteUtilsPS::getPaletteColor(paletteOrig, nextIndex);
            currentIndex = nextIndex;
            break;
        }
        default: //mode 5 and up
            //a single color palette where the color is choosen randomly,
            //doesn't use the original palette at all
            paletteColorArr1[0] = paletteUtilsPS::getPaletteColor(&nextPalette, 0);
            paletteUtilsPS::randomize(&nextPalette, 0);
            break;
    }

}

//updates the blend
void SinglePaletteCyclePS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;
        //update the PaletteBlendPS instance
        PB->update();

        //if we've finished the current blend (and pause time), we need to move onto the next one
        if(PB->blendEnd && !PB->paused){
            cycleNum = addmod8( cycleNum, 1, paletteOrig->length );
            switchPalette();
            PB->reset( &currentPalette, &nextPalette );
        }
    }
}
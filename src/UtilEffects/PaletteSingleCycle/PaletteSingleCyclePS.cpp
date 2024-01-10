#include "PaletteSingleCyclePS.h"

//Constructor with custom direction setting
PaletteSingleCyclePS::PaletteSingleCyclePS(palettePS &InputPalette, uint8_t BlendMode, bool Direct, 
                                           bool StartPaused, uint8_t TotalSteps, uint16_t Rate)
    : inputPalette(&InputPalette), blendMode(BlendMode), direct(Direct)  //
{
    bindClassRatesPS();
    //create the initial current/next palettes based on the mode
    switchPalette();
    //create a PaletteBlenderPS instance
    PB = new PaletteBlenderPS(currentPalette, nextPalette, false, TotalSteps, Rate);
    PB->startPaused = StartPaused;
    //point the PB update rate to the same rate as the PaletteSingleCyclePS instance, so they stay in sync
    PB->rate = rate;
    //bind the output palette of PB to the output of the SinglePaletteCycle
    cyclePalette = PB->blendPalette;
}

PaletteSingleCyclePS::~PaletteSingleCyclePS() {
    free(paletteColorArr1);
    free(paletteColorArr2);
    free(indexOrder);
    PB->~PaletteBlenderPS();
}

//restarts the palette blend
void PaletteSingleCyclePS::reset() {
    cycleNum = 0;
    currentIndex = 0;
    initPaletteColors(); //Resets the current and next palettes to a starting configuration
    switchPalette(); //Sets up the next palette state
    PB->reset(currentPalette, nextPalette);
}

//sets the total steps used in the PaletteBlenderPS instance
void PaletteSingleCyclePS::setTotalSteps(uint8_t newTotalSteps) {
    PB->totalSteps = newTotalSteps;
}

//sets the pause time used in the PaletteBlenderPS instance
void PaletteSingleCyclePS::setPauseTime(uint16_t newPauseTime) {
    PB->pauseTime = newPauseTime;
}

//sets the "startPaused" property of the palette blender (see PaletteBlenderPS)
void PaletteSingleCyclePS::setStartPaused(bool newStartPaused) {
    PB->startPaused = newStartPaused;
}

//Resets the current and next blend palettes to an inital configuration.
//For this, we just copy the input palette into both current and next palettes,
//which acts as the starting point for any blend.
//After the palettes are setup, switchPalette() can be called to advance to the next blend state (based on blend mode).
void PaletteSingleCyclePS::initPaletteColors(){
    paletteLength = inputPalette->length;
    //Note that we can only setup the palettes if they exist
    if( currentPalette.paletteArr && (paletteLength <= paletteLenMax) ) {
        //Copy the colors from inputPalette into the new palettes
        for( uint8_t i = 0; i < paletteLength; i++ ) {
            paletteColorArr1[i] = paletteUtilsPS::getPaletteColor(*inputPalette, i);
            paletteColorArr2[i] = paletteUtilsPS::getPaletteColor(*inputPalette, i);
            indexOrder[i] = i; //set the inital palette order to match the palette
        }
    }
}

//sets the currentPalette and nextPalette depending on the cycleNum and the blendMode
//the cycle num varies from 0 to the input palette's length -1
void PaletteSingleCyclePS::switchPalette() {
    //create and allocate memory for the next/current palettes to be used in the blends
    //We only need to make a new palette pair if the current ones aren't large enough
    //This helps prevent memory fragmentation by limiting the number of heap allocations
    //but this may use up more memory overall.
    //If new palette's are created, set them both to match the input palette
    paletteLength = inputPalette->length;
    if( alwaysResizeObj_PS || (paletteLength > paletteLenMax) ) {
        paletteLenMax = paletteLength;

        //Create two new palettes the same length as inputPalette
        free(paletteColorArr2);
        paletteColorArr1 = (CRGB *)malloc(paletteLength * sizeof(CRGB));
        paletteColorArr2 = (CRGB *)malloc(paletteLength * sizeof(CRGB));
        currentPalette = {paletteColorArr1, paletteLength};
        nextPalette = {paletteColorArr2, paletteLength};

        //Create any array to track the palette index order when shuffling (see blend modes)
        free(indexOrder);
        indexOrder = (uint8_t *)malloc(paletteLength * sizeof(uint8_t));
        
        initPaletteColors();
    }

    //set the current/next palette lengths
    //some blend modes need different palette lengths
    //we can adjust the current/next palette's length without actually changing their color array size
    //this keeps things simple, and makes it easier to switch modes
    if( blendMode <= 3 ) {  //for blend modes 0-3
        currentPalette.length = paletteLength;
        nextPalette.length = paletteLength;
    } else {  //for blend modes 4 and up
        currentPalette.length = 1;
        nextPalette.length = 1;
    }

    //some blend modes allow for a direction setting
    //We need to set a direction step: 1 for forward (direct = true) and -1 for backwards (direct = false)
    //The boolean addition below spits out 1 or -1
    stepDirect = direct - !direct;

    //Sets up the palettes according to the mode
    //Modes:
    //  0 -- Cycles the entire palette by one step each cycle.
    //       ie for direct = true, {blue, red, green} => {green, blue, red} => {red, green, blue} , etc
    //  1 -- Like mode 0, but a random color is inserted at the beginning (end if direct is false) each cycle, (initially starts as palette)
    //       ie for direct = true, {rand1, rand2, rand3} => {rand4, rand1, rand2} => {rand5, rand4, rand1}, etc
    //  2 -- Randomizes the whole palette each cycle for a palette with 3 random colors: {rand1, rand2, rand3} -> {rand4, rand5, rand6}, etc
    //  3 -- Shuffles the palette each cycle
    //       ie {blue, red, green} could go to {red, blue, green}, or {blue, green, red}, etc
    //       Note that the same palette order can repeat, the likely-hood depends on the palette length.
    //       DO not use this for a palette with only 2 colors!
    //       direct has no effect
    //  4 -- Makes the palette length 1, cycles through each color of the palette
    //       ie for direct = true and palette {blue, red, green} it will be {blue} => {red} => {green} => {blue}, etc
    //  5 -- Same as previous mode, but chooses the next color randomly from the palette (will not be the current color)
    //  6 -- Same as mode 4, but the next color is chosen completely randomly (not from the palette)
    switch( blendMode ) {
        case 0:
        default: {
            //copy the next palette into the current palette
            //then copy the input palette into the next palette shifted forward/backwards by 1
            for( uint8_t i = 0; i < paletteLength; i++ ) {
                currentIndex = i + (stepDirect * (cycleNum + 1) + paletteLength);  //we add paletteLength to prevent this from going negative
                paletteColorArr1[i] = paletteUtilsPS::getPaletteColor(nextPalette, i);
                paletteColorArr2[i] = paletteUtilsPS::getPaletteColor(*inputPalette, mod8(currentIndex, paletteLength));
            }
            break;
        }
        case 1: {
            //similar to case 0, but we are setting either the first or last color randomly, depending on direction
            //So we need to copy the nextPalette into the current palette,
            //Then, shift the nextPalette forward/backwards by 1 depending on the direction,
            //and insert a new random color at either the start/end of the palette

            //Copy the nextPalette into the currentPalette
            for( uint8_t i = 0; i < paletteLength; i++ ) {
                paletteColorArr1[i] = paletteUtilsPS::getPaletteColor(nextPalette, i);
            }

            //We need to shift the colors according to the direct, so we change out loop direction to match
            //This prevents us from reading from an index that has already been set, and copying the same color down the whole palette
            //When direct is true, we loop from the end of the palette to the start, shifting each color forward by one,
            //and inserting a random color at the start of the palette.
            //If direct is false, we do the same, but reversed. 

            //Set the loop direction variables
            uint8_t loopStart = paletteLength - 1;
            uint8_t loopEnd = 0;
            if( !direct ) {
                loopStart = 0;
                loopEnd = paletteLength - 1;
            }

            //Shift the array colors forward/backwards by 1
            //Note that we shift in the opposite of stepDirect, ie backwards if direct is true, and forwards if direct is false
            //because we are copying the colors one at a time, ending with the final new random color
            for( uint8_t i = loopStart; i != loopEnd; i -= stepDirect ) {
                //get (i + stepDirect * -1), paletteLength is added to keep the value positive (doesn't effect the mod below)
                currentIndex = i + (paletteLength - stepDirect);
                paletteColorArr2[i] = paletteUtilsPS::getPaletteColor(nextPalette, mod8(currentIndex, paletteLength));
            }

            //randomize the start or end color
            paletteUtilsPS::randomizeCol(nextPalette, loopEnd);
            break;
        }
        case 2: {
            //Randomize the whole palette for each cycle

            //Copy the nextPalette into the currentPalette
            for( uint8_t i = 0; i < paletteLength; i++ ) {
                paletteColorArr1[i] = paletteUtilsPS::getPaletteColor(nextPalette, i);
            }

            //Randomize the next palette
            paletteUtilsPS::randomize(nextPalette);
            break;
        }
        case 3: {
            //copy the next palette into the current palette
            for( uint8_t i = 0; i < paletteLength; i++ ) {
                paletteColorArr1[i] = paletteUtilsPS::getPaletteColor(nextPalette, i);
            }

            //shuffle the next palette to get a new palette order
            //We track the new order of the palette indexes using the indexOrder array
            //(note that shuffle() does allow repeats, so you may get the same palette order back,
            //with the likely-hood decreasing with longer palettes)
            paletteUtilsPS::shuffle(nextPalette, indexOrder);
            
            //To account for any changes in the inputPalette, we copy it into the nextPalette
            //using the indexOrder array to match the shuffled color indexes. 
            for( uint8_t i = 0; i < paletteLength; i++ ) {
                paletteColorArr2[i] = paletteUtilsPS::getPaletteColor(*inputPalette, indexOrder[i]);
            }
            break;
        }
        case 4: {
            //cycles through each color in the input palette
            //the output is a palette with a single color of length 1
            currentIndex = (stepDirect * (cycleNum + 1)) + paletteLength;
            paletteColorArr1[0] = paletteUtilsPS::getPaletteColor(nextPalette, 0);
            paletteColorArr2[0] = paletteUtilsPS::getPaletteColor(*inputPalette, mod8(currentIndex, paletteLength));
            break;
        }
        case 5: {
            //like case 4, but the next color is chosen randomly from the input palette
            //(if the chosen color is the same as the current one, we'll just choose the next one along)
            uint8_t nextIndex = random(paletteLength);
            if( nextIndex == currentIndex ) {
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
            paletteUtilsPS::randomizeCol(nextPalette, 0);
            break;
    }
}

//updates the blend
void PaletteSingleCyclePS::update() {
    currentTime = millis();

    if( active && (currentTime - prevTime) >= *rate ) {
        prevTime = currentTime;
        //update the PaletteBlendPS instance
        PB->update();

        //if we've finished the current blend (and pause time), we need to move onto the next one
        if( PB->blendEnd && !PB->paused ) {
            cycleNum = addmod8(cycleNum, 1, inputPalette->length);
            switchPalette();
            //We use the next palette pair reset of the palette blender
            //This captures if the either of the cycle palettes has changed in length (because the inputPalette or mode was changed)
            //And will create a new blendPalette
            //If they're the same length, paletteBlender will skip creating a new palette.
            PB->reset(currentPalette, nextPalette);
        }
    }
}
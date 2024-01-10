#include "PaletteBlenderPS.h"

//Base constructor
PaletteBlenderPS::PaletteBlenderPS(palettePS &StartPalette, palettePS &EndPalette, bool Looped, uint8_t TotalSteps,
                                   uint16_t Rate)
    : endPalette(&EndPalette), startPalette(&StartPalette), looped(Looped), totalSteps(TotalSteps)  //
{
    //bind the rate vars since they are inherited from BaseEffectPS
    bindClassRatesPS();
    reset(*startPalette, *endPalette);
}

//Constructor including a randomize option
PaletteBlenderPS::PaletteBlenderPS(palettePS &StartPalette, palettePS &EndPalette, bool Looped, bool Randomize,
                                   uint8_t TotalSteps, uint16_t Rate)
    : endPalette(&EndPalette), startPalette(&StartPalette), looped(Looped), randomize(Randomize), totalSteps(TotalSteps)  //
{
    //bind the rate vars since they are inherited from BaseEffectPS
    bindClassRatesPS();
    reset(*startPalette, *endPalette);
}

PaletteBlenderPS::~PaletteBlenderPS() {
    free(blendPalette_arr);
}

//resets the core class variables, allowing you to reuse class instances
void PaletteBlenderPS::reset(palettePS &StartPalette, palettePS &EndPalette, uint8_t TotalSteps, uint16_t Rate)  //
{
    reset(StartPalette, EndPalette);
    totalSteps = TotalSteps;
    bindClassRatesPS();
}

//resets just the start and end palettes
void PaletteBlenderPS::reset(palettePS &StartPalette, palettePS &EndPalette) {
    reset();

    //setup the blendPalette palette
    //it must be as long as the longest passed in palette
    //this way we can always pair up a start and end color (even if the colors repeat)
    //otherwise you miss colors because you wouldn't have space for them
    endPalette = &EndPalette;
    startPalette = &StartPalette;
    uint8_t blendPaletteLengthTemp = maxPS(startPalette->length, endPalette->length);

    //create the blend palette
    setupBlendPalette(blendPaletteLengthTemp);
}

//reset the loop variables, basically starting the blend from scratch
void PaletteBlenderPS::reset() {
    step = 0;
    ratio = 0;
    blendEnd = false;
    paused = false;
    pauseDone = false;

    if( endPalette && startPalette ) {  //a small guard just incase our palettes don't exist yet
        //if we are randomizing, choose a randomized end palette
        if( randomize ) {
            paletteUtilsPS::randomize(*endPalette, compliment);
        }

        //Set the inital palette colors, which ensures the blend palette matches the starting palette.
        blendPaletteColors();
    }
}

//creates the blend palette and the color storage arrays
void PaletteBlenderPS::setupBlendPalette(uint8_t newBlendPaletteLength) {
    //We only need to make a new blend palette if the current one isn't large enough
    //This helps prevent memory fragmentation by limiting the number of heap allocations
    //but this may use up more memory overall.
    if( alwaysResizeObj_PS || (newBlendPaletteLength > blendPaletteMaxLen) ) {
        blendPaletteMaxLen = newBlendPaletteLength;
        //delete the current blendPalette array of colors to free up memory
        //then create a new one, and pass that to a palette
        free(blendPalette_arr);
        blendPalette_arr = (CRGB *)malloc(blendPaletteMaxLen * sizeof(CRGB));
        blendPalette = {blendPalette_arr, blendPaletteMaxLen};
    } else {
        //if the new blend palette length is less than the current length,
        //we can adjust the length of the palette to "hide" the extra colors
        blendPalette.length = newBlendPaletteLength;
    }
    blendPaletteColors();  //update once to fill in the blendPalette
}

//updates the blended palette at the passed in rate
//Note that when looping, we cycle between blending and pausing.
//This cycle needs to be able to run in both directions depending on "startPaused" (starting with a pause or not)
//If startPaused is true, then our cycle goes pause-blend-pause-blend, etc
//If it's false, then we go blend-pause-blend-pause
//So the function is configured to begin with either a pause or blend depending on startPaused.
//This requires quite a few flags, since the cycle's flow is essentially reversed, see the comments in the function for more
void PaletteBlenderPS::update() {
    currentTime = millis();

    //if the blend is active, and enough time has passed, update the palette
    if( active && (currentTime - prevTime) >= *rate ) {
        prevTime = currentTime;

        //If we're starting with a pause, we need to setup the pause on the first cycle ONLY
        //So we only start a pause if one hasn't happened yet
        //(ie we're not paused and a pause hasn't finished)
        if( startPaused && !pauseDone && !paused ) {
            //Start a pause at the current time
            paused = true;
            pauseStartTime = currentTime;
        }

        //if we're paused, we check if the pause time has passed,
        //if not, then we just jump out the update() using a return
        if( paused ) {
            if( (currentTime - pauseStartTime) >= pauseTime ) {
                //If it's time to continue blending the colors we deactivate the pause
                paused = false;
                pauseDone = true;

                //if we didn't start with a pause, and we're looping, it's time to
                //reset the loop and set the new palettes.
                if( !startPaused && looped ) {
                    //swap the palettes for looping
                    loopPalettes();
                }
            } else {
                return;
            }
        }

        //if we're not paused and the blend hasn't finished, we need to blend the next step
        step++;
        //All the colors blend at the same rate so the blend ratio is the same for all of them
        ratio = step * 255 / totalSteps;

        //Set the next blend palette colors
        blendPaletteColors();

        //if we have reached the totalSteps,
        //the blend palette has reached the end palette
        //So we need to stop the blend and decide what to do next
        if( step >= totalSteps ) {
            blendEnd = true;

            //If we didn't pause before blending, we need to start our pause now
            //But if we did already do a pause, and we're looping, our pause-blend cycle needs to be restarted
            if( !startPaused && !pauseDone) {
                paused = true;
                pauseStartTime = currentTime;
            } else if( looped ) {
                //swap the palettes for looping
                loopPalettes();
            }
        }
    }
}

//Blends the palette colors according to the current "step"
void PaletteBlenderPS::blendPaletteColors() {
    //for each color in the blend palette, blend it towards a color in the end palette
    //using the getCrossFadeColor function (see colorUtilsPS.h)
    //it doesn't matter if one palette is shorter than the other, b/c palettes wrap automatically
    for( uint8_t i = 0; i < blendPalette.length; i++ ) {
        startColor = paletteUtilsPS::getPaletteColor(*startPalette, i);
        endColor = paletteUtilsPS::getPaletteColor(*endPalette, i);
        newColor = colorUtilsPS::getCrossFadeColor(startColor, endColor, ratio);
        paletteUtilsPS::setColor(blendPalette, newColor, i);
    }
}

//Once a loop has ended, our blend palette is now the same as our end palette
//So to loop back to our start palette, we swap the end and start palettes.
void PaletteBlenderPS::loopPalettes() {
    palTempPtr = startPalette;
    startPalette = endPalette;
    endPalette = palTempPtr;
    //reset the class variables to start a new blend
    reset();
}
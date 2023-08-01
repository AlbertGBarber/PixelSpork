#include "PaletteSliderPS.h"

//Constructor for slider palette using a pattern
PaletteSliderPS::PaletteSliderPS(palettePS &PaletteTarget, patternPS &Pattern, uint16_t SliderPalLen, uint16_t BlendSteps, bool SingleShift, uint16_t Rate): 
    paletteTarget(&PaletteTarget), pattern(&Pattern), sliderPalLen(SliderPalLen), blendSteps(BlendSteps), singleShift(SingleShift)
    {    
        init(Rate);
	}

//Constructor for slider palette using the target palette as the pattern
PaletteSliderPS::PaletteSliderPS(palettePS &PaletteTarget, uint16_t SliderPalLen, uint16_t BlendSteps, bool SingleShift, uint16_t Rate): 
    paletteTarget(&PaletteTarget), sliderPalLen(SliderPalLen), blendSteps(BlendSteps), singleShift(SingleShift)
    {    
        setPaletteAsPattern();
        init(Rate);
	}

PaletteSliderPS::~PaletteSliderPS(){
    free(sliderPalColArr);
}

//setup core vars
void PaletteSliderPS::init(uint16_t Rate){
    //bind the rate pointer vars since they are inherited from BaseEffectPS
    bindClassRatesPS();
    makeSliderPalette(sliderPalLen);
    reset();
}

//resets the slider palette back to it's original state
void PaletteSliderPS::reset(){
    blendStep = 0;
    patternIndex = 0;
    paused = false;
    prevTime = 0;
}

//sets the pattern to match follow the colors in the target palette
//ie for a palette of length 5, the pattern will be 0, 1, 2, 3, 4
//(all the palette indexes in order)
void PaletteSliderPS::setPaletteAsPattern(){
    free(patternTemp.patternArr);
    generalUtilsPS::setPaletteAsPattern(patternTemp, *paletteTarget);
    pattern = &patternTemp;
}

//Creates the ouput slider palette according to the passed in length
//Note that a new palette will be created if the new length is greater than the memory length of the current palette
//It also resets the effect and calls update() once to fill in the initial palette colors
void PaletteSliderPS::makeSliderPalette(uint16_t paletteLength){
    //We only need to make a new slider palette if the current one isn't large enough
    //This helps prevent memory fragmentation by limiting the number of heap allocations
    //but this may use up more memory  overall.
    if( alwaysResizeObjPS || (paletteLength > sliderPalLenMax) ){
        sliderPalLenMax = paletteLength;
        free(sliderPalColArr);
        sliderPalColArr = (CRGB*) malloc(sliderPalLenMax * sizeof(CRGB));
        sliderPalette = {sliderPalColArr, sliderPalLenMax};
    } else {
        //if the new slider palette length is less than the current length,
        //we can adjust the length of the palette to "hide" the extra colors
        sliderPalette.length = paletteLength;
    }
    //The new palette will be blank, so we need to call update() once to fill it with colors
    reset();
    update();
}

//updates the slider palette
//Each update cycle the colors in the slider palette are advanced towards their target colors by one step (blendStep)
//The target colors are fetched for each palette color using the pattern and the paletteTarget
//We use patternIndex to track how many color cycles we've been through
//Since each color follows the same pattern, we use the patternIndex to offset what pattern each slider palette color is on
//Once blendStep == blendSteps (the total blend length) then a color cycle is finished
//We then advance the patternIndex, and also trigger a hold
//The hold continues for pauseTime, and then the color cycle starts again
//(while holding the colors are fixed --  a pause before continuing the cycle)
//Note that there are two modes of switching colors: using a single shift, and shifting the whole palette length
//  When single shifting:
//      The palette colors all follow the same pattern, with each one being offset from the others
//      For example, if I had a palette of {red, green, blue, orange} and a slider palette of length 2, 
//      and the pattern is just the palette in order
//      The first set of slider palette colors would be {red, green} transitioning to {green, blue} 
//      then to {blue, orange}, etc. So each color follows the one after it.
//  When shifting the whole palette length:
//      The palette colors transition as a full group, with each color targeting the color that is a palette length away from it
//      Using the same example palettes from the single shifting:
//      The slider palette would transition as {red, green} goes to {blue, orange} goes back to {red, green}
//      The colors transition in pairs
//      If the slider palette were length 3, it would go {red, green, blue} -> {orange, red, green} -> {blue, orange, red}
//      The target color is 3 away from the starting color
void PaletteSliderPS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        if(paused){
            if( (currentTime - pauseStartTime) >= pauseTime) {
                //If it's time to continue blending the colors we deactivate the pause 
                paused = false;
            } else{
                return;
            }
        }
        
        //We need to advance slider palette colors to their next steps
        sliderPalLen = sliderPalette.length;
        //If we're shifting the colors forward by 1 for each blend then
        //the patternStep is only one. So that each color follows the one ahead of it
        //Otherwise we doing whole pattern chunks, where all the colors need to jump forward by the 
        //length of the slider palette
        if(singleShift){
            patternStep = 1;
        } else {
            patternStep = sliderPalLen;
        }
        //advance the blend step 
        //we do this here rather than at the end so that we catch hold times correctly
        blendStep++;
        //All the colors blend at the same rate so the blend ratio is the same for all of them
        ratio = blendStep * 255 / blendSteps;
        
        //Loop over the sliderPalette and set it's blended colors
        for(uint16_t i = 0; i < sliderPalLen; i++){
            
            //Get the starting color offset by the patternIndex
            startIndex = patternUtilsPS::getPatternVal(*pattern, i + patternIndex);
            startColor = paletteUtilsPS::getPaletteColor(*paletteTarget, startIndex);
            //get the target color. We offset it by the patternStep.
            endIndex = patternUtilsPS::getPatternVal(*pattern, i + patternIndex + patternStep);
            endColor = paletteUtilsPS::getPaletteColor(*paletteTarget, endIndex);
            //get the blended color between the start and end colors
            sliderPalColArr[i] = colorUtilsPS::getCrossFadeColor(startColor, endColor, ratio);
        }

        //If we're at the last blend step, then we need start a new blend with new colors
        //(and also trigger a hold)
        if(blendStep == blendSteps){
            blendStep = 0;
            //Advance the patternIndex so the palette colors will advance
            //We advance by patternStep, which is either 1 or the sliderPalette length
            patternIndex = addMod16PS(patternIndex, patternStep, pattern->length);
            //Start a hold at the current time
            paused = true;
            pauseStartTime = currentTime;
        }
    
    }
}
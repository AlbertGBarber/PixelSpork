#include "RollingWavesSL.h"

//constructor with pattern
RollingWavesSL::RollingWavesSL(SegmentSet &SegSet, patternPS &Pattern, palettePS &Palette, CRGB BGColor, uint8_t GradLength, uint8_t TrailMode, uint8_t Spacing, uint16_t Rate):
    SegSet(SegSet), pattern(&Pattern), palette(&Palette), gradLength(GradLength), spacing(Spacing), trailMode(TrailMode)
    {    
        init(BGColor,Rate);
	}

//constructor with palette as pattern
RollingWavesSL::RollingWavesSL(SegmentSet &SegSet, palettePS &Palette, CRGB BGColor, uint8_t GradLength, uint8_t TrailMode, uint8_t Spacing, uint16_t Rate):
    SegSet(SegSet), palette(&Palette), gradLength(GradLength), spacing(Spacing), trailMode(TrailMode)
    {    
        setPaletteAsPattern();
        init(BGColor, Rate);
	}

//constructor with random colors
RollingWavesSL::RollingWavesSL(SegmentSet &SegSet, uint8_t NumColors, CRGB BGColor, uint8_t GradLength, uint8_t TrailMode, uint8_t Spacing, uint16_t Rate):
    SegSet(SegSet), gradLength(GradLength), spacing(Spacing), trailMode(TrailMode)
    {    
        paletteTemp = paletteUtilsPS::makeRandomPalette(NumColors);
        palette = &paletteTemp;
        setPaletteAsPattern();
        init(BGColor, Rate);
	}

RollingWavesSL::~RollingWavesSL(){
    free(paletteTemp.paletteArr);
    free(patternTemp.patternArr);
}

//inits core variables for the effect
void RollingWavesSL::init(CRGB BgColor, uint16_t Rate){
    //bind the rate and SegSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    bindBGColorPS();
    cycleNum = 0;
    setTrailMode(trailMode);
    setTotalEffectLength();
}

//sets the gradLength
//we need to change the totalCycleLength to match
//and recalculate the trail settings
void RollingWavesSL::setGradLength(uint8_t newGradLength){
    gradLength = newGradLength;
    setTrailMode(trailMode);
    setTotalEffectLength();
}

//sets the spacing between the waves
//and recalculates the totalCycleLength, since it includes the spacing
void RollingWavesSL::setSpacing(uint8_t newSpacing){
    spacing = newSpacing;
    setTotalEffectLength();
}

//sets a new pattern for the effect
//we need to change the totalCycleLength to match
void RollingWavesSL::setPattern(patternPS &newPattern){
    pattern = &newPattern;
    setTotalEffectLength();
}

//sets the pattern to match the current palette
//ie for a palette length 5, the pattern would be 
//{0, 1, 2, 3, 4}
void RollingWavesSL::setPaletteAsPattern(){
    patternTemp = generalUtilsPS::setPaletteAsPattern(*palette);
    pattern = &patternTemp;
    setTotalEffectLength();
}

//calculates the totalCycleLength, which represents the total number of possible colors a pixel can have
//ie the total length of all the waves (of each color in the pattern) combined
//includes the spacing after each wave
//The blend limit is the length of a wave plus its spacing
void RollingWavesSL::setTotalEffectLength(){
    blendLimit = gradLength + spacing;
    totalCycleLength = pattern->length * blendLimit;
}

//sets various limits for drawing different types of trails
//Note that when the trails are drawn we draw the leading trail first, then the center pixel
//and then the ending trail
//So we adjust the limits to set the lengths of each trail depending on the mode
//Length vars:
    //firstHalfGrad => the length of the leading trail (fades in from 0) 
    //                 anything after this will be the ending trail
    //halfGrad => The length of half a wave (or full wave if not mode 1)
    //blendStepAdjust => offsets the blend step if needed
    //midPoint => The location of the pixel with the highest brightness in the wave
    //            ie the wave "head"
//The Modes:
//  0: Only ending trail will be drawn
//  1: Both ending and leading trails will be drawn (at half gradLength)
//  2: Only the leading trail will be drawn
void RollingWavesSL::setTrailMode(uint8_t newTrailMode){
    trailMode = newTrailMode;
    switch (trailMode){
        case 0: //end trail only
            //Since we only draw the end trail firstHalfGrad = 0
            //The first pixel is the "head", so the midPoint is 0
            halfGrad = gradLength;
            firstHalfGrad = 0;
            blendStepAdjust = 0;
            midPoint = 0;
            break;
        case 2: //leading trail only
            //Since we only draw the leading trail firstHalfGrad = gradLength
            //The last pixel is the "head", so the midPoint is gradLength - 1
            halfGrad = gradLength;
            firstHalfGrad = gradLength;
            blendStepAdjust = 0;
            midPoint = gradLength - 1;
            break;
        default: //case 1, both trails
            //Since we're drawing both trails, halfGrad = (gradLength)/2 <- rounds down
            //and firstHalfGrad = halfGrad, so both trails are equal length
            //blendStepAdjust = 0 or 1 to adjust the center for odd length waves 
            //The "head" is the wave center which is at halfGrad
            halfGrad = (gradLength)/2;
            firstHalfGrad = halfGrad;
            blendStepAdjust = mod16PS(gradLength, 2);
            //if( (gradLength % 2) != 0){
                //blendStepAdjust = 1;
            //}
            midPoint = halfGrad;
            break;
    }
}

//Updates the effect
//Each update we run across all the LEDs and draw the waves and spacing based on the cycleNum and the pattern
//We increment the blendStep after each led, which tracks what step we're on with each wave
//The blendStep cycles between 0 and the blendLimit (gradLength + spacing)
//If the blendStep is 0, then a wave and it's spacing have finished and we choose the next color for the next wave
//To draw the waves we use the variables set by setTrailMode()
//We first draw the leading trail up to firstHalfGrad
//Then we draw the ending trail up to gradLength
//After the gradLength we draw any spacing pixels
//At some point during the wave we will hit the midPoint, this is the "head" of the wave
//and is drawn at full brightness (so that the wave dimming can be non-linear, but the "head" is always full color)
//Once all the leds have been filled a cycle is complete and cycleNum is incremented
void RollingWavesSL::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        //fetch some core vars
        //we re-fetch these in case the segment set or palette has changed
        numSegs = SegSet.numSegs;
        numLines = SegSet.numLines;
        
        //we need to set the current color for the initial loop step
        //because it will not automatically be set in the loop unless the first blendStep is 0
        setNextColors(0);
       
        for (uint16_t i = 0; i < numLines; i++) {

            blendStep = addmod8(cycleNum, i, blendLimit); // what step of the wave we're on (incl spacing)
            //If the blendStep is 0, then a wave has finished, and we need to choose the next color
            if( blendStep == 0 ){
                //the color we're at based on the current index
                setNextColors(i);
            }
                
            //draw the various parts of the wave
            //the limit vars are set by setTrailMode
            //We first draw the leading trail up to firstHalfGrad
            //Then we draw the ending trail up to gradLength
            //After the gradLength we draw any spacing pixels
            if(blendStep < firstHalfGrad ){
                //For full dimming we want stepTemp to be halfGrad
                //since we're doing the leading trail, we start at halfGrad
                stepTemp = halfGrad - blendStep;
                setBg = false;
            } else if(blendStep < gradLength) {
                //For the ending trail we do the same as the first trail, but 
                //we need to start at 0 (no dimming), and we need to adjust for the inital blendStep value
                stepTemp = halfGrad - (gradLength - blendStep) + blendStepAdjust;
                setBg = false;
            } else {
                //if we're past the gradLength, we setting spacing pixels
                setBg = true;
            }

            //reverse the line number so that the effect moves positively along the strip
            lineNum = numLines - i - 1;

            for(uint16_t j = 0; j < numSegs; j++){
                //get the physical pixel location based on the line and seg numbers
                pixelNum = segDrawUtils::getPixelNumFromLineNum(SegSet, numLines, j, lineNum);
                //We get the pixel info for the current led, supplying the background or wave color info
                //depending on if the current led is a spacing pixel
                if(setBg){
                    colorOut = segDrawUtils::getPixelColor(SegSet, pixelNum, *bgColor, bgColorMode, j, lineNum);
                } else{
                    colorOut = segDrawUtils::getPixelColor(SegSet, pixelNum, currentColor, colorMode, j, lineNum);

                    //Dim the color
                    //If the blendStep is at the "head" led, we don't dim it
                    if(blendStep != midPoint){
                        colorOut = desaturate(colorOut, stepTemp, halfGrad);
                    }
                }
                segDrawUtils::setPixelColor(SegSet, pixelNum, colorOut, 0, j, lineNum);
            }
        }

        cycleNum = addMod16PS( cycleNum, 1, totalCycleLength );
        showCheckPS();
    }
}

//returns a dimmed color (towards 0) based on the input steps and totalSteps
//step == totalSteps is fully dimmed
//Note that we offset totalSteps by 1, so we never reach full dim (since it would produce blank pixels)
//the maximum brightness is scaled by dimPow
//dimPow 255 will produce a normal linear gradient, but for more shimmery waves we can dial the brightness down
//The "head" wave pixel will still be drawn at full brightness since it's drawn separately 
CRGB RollingWavesSL::desaturate(CRGB &color, uint8_t step, uint8_t totalSteps) {

    dimRatio = (dimPow - (uint16_t)step * dimPow / (totalSteps + 1));

    //ratio = dim8_video(ratio); 
    //uint8_t ratio = 255 - triwave8( 128 * (uint16_t)step / (totalSteps + 1) );

    //quickly scales each of the color components by dimRatio
    nscale8x3(color.r, color.g, color.b, dimRatio);

    return color;
}

//sets the color for the passed in pixel number based on the cycleNum
//note that the pixel number is local to the segment set, not the physical led number
void RollingWavesSL::setNextColors(uint16_t segPixelNum){
    //(segPixelNum + cycleNum) % totalCycleLength
    currentColorIndex = addMod16PS( segPixelNum, cycleNum, totalCycleLength ) / blendLimit; // what color we've started from (integers always round down)
    //the color we're at based on the current index
    currentPattern = patternUtilsPS::getPatternVal(*pattern, currentColorIndex);
    currentColor = paletteUtilsPS::getPaletteColor(*palette, currentPattern);
}
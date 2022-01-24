#include "RollingWavesFastPS.h"

//constructor with pattern
RollingWavesFastPS::RollingWavesFastPS(SegmentSet &SegmentSet, patternPS *Pattern, palletPS *Pallet, CRGB BGColor, uint8_t GradLength, uint8_t TrailMode, uint8_t Spacing, uint16_t Rate):
    segmentSet(SegmentSet), pattern(Pattern), pallet(Pallet), gradLength(GradLength), spacing(Spacing), trailMode(TrailMode)
    {    
        init(BGColor,Rate);
	}

//constuctor with pallet as pattern
RollingWavesFastPS::RollingWavesFastPS(SegmentSet &SegmentSet, palletPS *Pallet, CRGB BGColor, uint8_t GradLength, uint8_t TrailMode, uint8_t Spacing, uint16_t Rate):
    segmentSet(SegmentSet), pallet(Pallet), gradLength(GradLength), spacing(Spacing), trailMode(TrailMode)
    {    
        setPalletAsPattern();
        init(BGColor, Rate);
	}

//constructor with random colors
RollingWavesFastPS::RollingWavesFastPS(SegmentSet &SegmentSet, uint8_t NumColors, CRGB BGColor, uint8_t GradLength, uint8_t TrailMode, uint8_t Spacing, uint16_t Rate):
    segmentSet(SegmentSet), gradLength(GradLength), spacing(Spacing), trailMode(TrailMode)
    {    
        palletTemp = EffectUtilsPS::makeRandomPallet(NumColors);
        pallet = &palletTemp;
        setPalletAsPattern();
        init(BGColor, Rate);
	}

RollingWavesFastPS::~RollingWavesFastPS(){
    delete[] palletTemp.palletArr;
    delete[] patternTemp.patternArr;
}

//inits core variables for the effect
void RollingWavesFastPS::init(CRGB BgColor, uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    bindBGColorPS();
    cycleNum = 0;
    setTrailMode(trailMode);
    setTotalEffectLength();
}

//sets the gradLength
//we need to change the totalCycleLength to match
void RollingWavesFastPS::setGradLength(uint8_t newGradLength){
    gradLength = newGradLength;
    setTrailMode(trailMode);
    setTotalEffectLength();
}

//sets the spacing between the waves
//and recalculates the totalCycleLength, since it includes the spacing
void RollingWavesFastPS::setSpacing(uint8_t newSpacing){
    spacing = newSpacing;
    setTotalEffectLength();
}

//sets a new pattern for the effect
//we need to change the totalCycleLength to match
void RollingWavesFastPS::setPattern(patternPS *newPattern){
    pattern = newPattern;
    setTotalEffectLength();
}

//binds the pallet to a new one
//don't need to change the totalCycleLength since the pallet is referenced
//only indirectly though the pattern
void RollingWavesFastPS::setPallet(palletPS* newPallet){
    pallet = newPallet;
}

//sets the pattern to match the current pallet
//ie for a pallet length 5, the pattern would be 
//{0, 1, 2, 3, 4}
void RollingWavesFastPS::setPalletAsPattern(){
    patternTemp = EffectUtilsPS::setPalletAsPattern(pallet);
    pattern = &patternTemp;
    setTotalEffectLength();
}

//caculates the totalCycleLength, which represents the total number of possible colors a pixel can have
//ie the total length of all the waves (of each color in the pattern) combined
//includes the spacing after each wave
//The blend limit is the length of a wave plus its spacing
void RollingWavesFastPS::setTotalEffectLength(){
    // the number of steps in a full cycle (fading through all the colors)
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
void RollingWavesFastPS::setTrailMode(uint8_t newTrailMode){
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
            blendStepAdjust = 0;
            if( (gradLength % 2) != 0){
                blendStepAdjust = 1;
            }
            midPoint = halfGrad;
            break;
    }
}

//We need to pre-fill the strip with the first set of waves
//in order for the led colors to be copied properly in the main update cycle
void RollingWavesFastPS::initalFill(){
    cycleNum = 0;

    //numPixels is the loop limit below, so we subtract 1
    numPixels = segmentSet.numActiveSegLeds - 1;

    //we need to draw the initial waves on the strip
    //to pre-fill it for the main update cycle
    //to do this we run across all the leds
    //every gradsteps number of leds, we increment the wave colors,
    //we loop forwards to match the direction of the loop in update()
    //so that where this initial setup ends, 
    //the cycleNum var will be correct
    for (uint16_t i = 0; i <= numPixels; i++) {

        // if we've gone through gradLength cycles
        // a wave is finished and we need to move to the next color
        if (cycleNum == 0) {
            setNextColors(i);
        }
        pixelNumber = segDrawUtils::getSegmentPixel(segmentSet, i);
        colorOut = getWaveColor(cycleNum);
        segDrawUtils::setPixelColor(segmentSet, pixelNumber, colorOut, 0, 0, 0);

        cycleNum = (cycleNum + 1) % blendLimit; //track what step we're on in the wave
    }
    initFillDone = true;
}

//Updates the effect
//For all leds up to the final on we copy the color of the led in front of it
//For the final led we caculate the next wave blendStep based on the cycleNum and the pattern
//The blendStep after cycle, which tracks what step of the wave we're on
//The blendStep cycles between 0 and the blendLimit (gradLength + spacing)
//If the blendStep is 0, then a wave and it's spacing have finished and we choose the next color for the next wave
//To draw the waves we use the variables set by setTrailMode()
//We first draw the leading trail up to firstHalfGrad
//Then we draw the ending trail up to gradLength
//After the gradLength we draw any spacing pixels
//At some point during the wave we will hit the midPoint, this is the "head" of the wave
//and is drawn at full bightness (so that the wave dimming can be non-linear, but the "head" is always full color)
//Once all the leds have been filled a cycle is complete and cycleNum is incremented
void RollingWavesFastPS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        //We need to pre-fill the strip with a full cycle the first time the update is called
        //so that the colors are copied down the strip correctly on subsequent cycles
        if(!initFillDone){
            initalFill();
        }

        //numPixels is the loop limit below, so we subtract 1
        numPixels = segmentSet.numActiveSegLeds - 1;

        //prep for the loop below.
        //The first pixel we need to copy into is the last pixel in the strip
        nextPixelNumber = segDrawUtils::getSegmentPixel(segmentSet, 0);
       
        for (uint16_t i = 0; i <= numPixels; i++) {
            //The nextPixelNumber from the previous loop iteration is now
            //the pixelNumber for this iteration
            pixelNumber = nextPixelNumber;
            nextPixelNumber = segDrawUtils::getSegmentPixel(segmentSet, i + 1);
            if(i == numPixels){
                blendStep = (cycleNum + i) % blendLimit; // what step we're on between the current and next color
                //If the blendStep is 0, then a wave has finished, and we need to choose the next color
                if( blendStep == 0 ){
                    //the color we're at based on the current index
                    setNextColors(i);
                }

                //get the next wave step color and set it
                colorOut = getWaveColor(blendStep);
                segDrawUtils::setPixelColor(segmentSet, pixelNumber, colorOut, 0, 0, 0);
                
            } else{
                //copy the color of the next pixel in line into the current pixel
                segmentSet.leds[pixelNumber] = segmentSet.leds[nextPixelNumber];
            }
            
        }

        cycleNum = (cycleNum + 1) % totalCycleLength;
        showCheckPS();
    }
}

//Returns a wave color blended towards 0 based on the trailMode and the step
//For each wave we first draw the leading trail up to firstHalfGrad
//Then we draw the ending trail up to gradLength
//After the gradLength we draw any spacing pixels
//At some point during the wave we will hit the midPoint, this is the "head" of the wave
//and is drawn at full bightness (so that the wave dimming can be non-linear, but the "head" is always full color)
//Once all the leds have been filled a cycle is complete and cycleNum is incremented
CRGB RollingWavesFastPS::getWaveColor(uint8_t step){

    //draw the various parts of the wave
    //the limit vars are set by setTrailMode
    //We first draw the leading trail up to firstHalfGrad
    //Then we draw the ending trail up to gradLength
    //After the gradLength we draw any spacing pixels
    if(step < firstHalfGrad ){
        //For full dimming we want stepTemp to be halfGrad
        //since we're doing the leading trail, we start at halfGrad
        stepTemp = halfGrad - step;
        setBg = false;
    } else if(blendStep < gradLength) {
        //For the ending trail we do the same as the first trail, but 
        //we need to start at 0 (no dimming), and we need to adjust for the inital blendStep value
        stepTemp = halfGrad - (gradLength - step) + blendStepAdjust;
        setBg = false;
    } else {
        setBg = true;
    }
    
    
    //return the wave color depending on if the current led is a spacing pixel
    if(setBg){
        colorOut = *bgColor;
    } else{
        if(step == midPoint){
            //if the blendStep is at the "head" led, we don't dim it
            colorOut = currentColor;
        } else {
            colorOut = desaturate(currentColor, stepTemp, halfGrad);
        }
    }

    return colorOut;
}

//returns a dimmed color (towards 0) based on the input steps and totalSteps
//step == totalSteps is fully dimmed
//Note that we offset totalSteps by 1, so we never reach full dim (since it would produce blank pixels)
//the maximum brightness is scaled by dimPow
//dimPow 255 will produce a normal linear gradient, but for more shimmery waves we can dial the bightness down
//The "head" wave pixel will still be drawn at full brightness since it's drawn seperatly 
CRGB RollingWavesFastPS::desaturate(CRGB color, uint8_t step, uint8_t totalSteps) {

    dimRatio = (dimPow - (uint16_t)step * dimPow / (totalSteps + 1));

    //ratio = dim8_video(ratio); 
    //uint8_t ratio = 255 - triwave8( 128 * (uint16_t)step / (totalSteps + 1) );

    //quickly scales each of the color components by dimRatio
    nscale8x3(color.r, color.g, color.b, dimRatio);

    return color;
}

//For calling whenever a wave has finished
//Chooses a new color for the next wave depending on the the options for random colors
void RollingWavesFastPS::setNextColors(uint16_t segPixelNum){
    if(!randColors){
        currentColorIndex = ( ( segPixelNum + cycleNum ) % totalCycleLength ) / blendLimit; // what color we've started from (integers always round down)
        //the color we're at based on the current index
        currentPattern = patternUtilsPS::getPatternVal(pattern, currentColorIndex);
        currentColor = palletUtilsPS::getPalletColor(pallet, currentPattern);
    } else {
        //choose the next color based on the random mode
        if(randMode == 0){
            //choose a completely random color
            currentColor = segDrawUtils::randColor();
        } else {
            //choose a color randomly from the pattern (making sure it's not the same as the current color)
            currentPattern = EffectUtilsPS::shuffleIndex(pattern, currentPattern);
            currentColor = palletUtilsPS::getPalletColor( pallet, currentPattern );
        }
    }
   
}

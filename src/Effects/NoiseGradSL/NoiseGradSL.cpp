#include "NoiseGradSL.h"

//Constructor with palette
NoiseGradSL::NoiseGradSL(SegmentSet &SegmentSet, palettePS *Palette, CRGB BgColor, uint16_t BlendStepsBase, 
                                       uint16_t BlendStepsRange, uint8_t PhaseScale, uint8_t FreqScale, uint8_t BriScale, 
                                       uint16_t BlendRate, uint16_t Rate):
    segmentSet(SegmentSet), palette(Palette), blendStepsBase(BlendStepsBase), blendStepsRange(BlendStepsRange), phaseScale(PhaseScale), freqScale(FreqScale), briScale(BriScale)
    {    
        init(BgColor, BlendRate, Rate);
	}

//Constructor with randomly generated palette
NoiseGradSL::NoiseGradSL(SegmentSet &SegmentSet, uint8_t numColors, CRGB BgColor, uint16_t BlendStepsBase, 
                                       uint16_t BlendStepsRange, uint8_t PhaseScale, uint8_t FreqScale, uint8_t BriScale,
                                       uint16_t BlendRate, uint16_t Rate):
    segmentSet(SegmentSet), blendStepsBase(BlendStepsBase), blendStepsRange(BlendStepsRange), phaseScale(PhaseScale), freqScale(FreqScale), briScale(BriScale)
    {    
        paletteTemp = paletteUtilsPS::makeRandomPalette(numColors);
        palette = &paletteTemp;
        init(BgColor, BlendRate, Rate);
	}

NoiseGradSL::~NoiseGradSL(){
    delete[] paletteTemp.paletteArr;
}

//Initializes core common varaibles
void NoiseGradSL::init(CRGB BgColor, uint16_t BlendRate, uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    bindBGColorPS();

    //bind the blendRate pointer
    //we store the passed in BlendRate in blendRateOrig
    blendRateOrig = BlendRate;
    blendRate = &blendRateOrig;

    //Set the initial blend steps
    //We want to make sure we start a blend shift so we set blendStepsTarget = blendSteps
    //(See shiftBlendSteps())
    if(blendStepsBase <= 0){
        blendStepsBase = 1;
    }
    blendSteps = blendStepsBase;
    blendStepsTarget = blendSteps;
    shiftBlendSteps();

    //Catch is any noise var is == 0
    if( phaseScale <= 0 ){
       phaseScale = 1; 
    }

    if( freqScale <= 0 ){
       freqScale = 1; 
    }

    //as a shorthand, passing briScale as 0 will turn off the brightness modulation
    //we also set the briScale to 20, so that if it's turned on, the scale is a reasonable value
    if(briScale == 0){
        doBrightness = false;
        briScale = 20;
    }

}

//Updates the effect
//The effect consists of two parts: a palette gradient wave and brightness zones, both controlled by noise
//The gradient waves shift across the segment lines using a cos() function where the phase uses noise
//This produces a gradient that shifts in random bursts along the segments
//Meanwhile we shift the number of gradient blend steps to a new value every so often. 
//This changes how the long the gradient is.
//We only pick a new shift value at most once every blendRate (ms), and then quickly blend to it, which helps keep the effect smooth
//To add some more interest, the brightness of each segment line is modulated by a noise function
//Which produces random blobs of dimmed pixels (similar to the lava effect)
//Both of these extra steps can be turned off by setting blendStepsRange = 0 and dobrightness = false
void NoiseGradSL::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;
        
        //Shift the number of steps taken for the palette gradient
        //We only want to do this at most once every blendRate ms, but then do so quickly
        //This lets the effect stay calm for a while, and then quickly shift to a new look
        //We only want to shift if blendStepsRange > 0, since otherwise we'd just be shifting to the same value
        blendShiftTime = currentTime - prevBlendShiftTime;
        if( (blendStepsRange > 0) && (blendShiftTime >= *blendRate) ){
            //If we've reached the current blend target, we can reset the clock
            //but we always call shiftBlendSteps(), so that we always have a new target set
            if(blendSteps == blendStepsTarget){
                prevBlendShiftTime = currentTime;
            } 
                shiftBlendSteps();    
        }
        //re-fetch some core variables
        numSegs = segmentSet.numSegs;
        numLines = segmentSet.maxSegLength;
        totBlendLength = blendSteps * palette->length;

        //Get a phase value for our waves using noise
        //The offset keeps the waves moving across the strip
        noisePhase = inoise8( currentTime/phaseScale, currentTime/20 ) + phaseOffset;

        //Get the output of the cos() wave, using the noise as the phase to keep it shifting along
        //and convert it to somewhere in the palette gradient
        index = cos8( currentTime/freqScale + noisePhase );
        colorIndex = scale16by8( totBlendLength, index );

        //To shift the colors across the segments we change the offset value in getPaletteGradColor()
        //To get the offset we use colorOffset + colorIndex where colorOffset cycles from 0 to totBlendLength
        //This ensures that the whole palette is seen.
        //Unfortunatly just adding the offsets togther shifts the colors in reverse across the segments
        //To shift them forward, we need to subtract the offsets, but we need to keep everything positive
        //so we use the mod formula below. Adding 2 * totBlendLength ensures that we stay positive, but doesn't
        //effect the final result
        colorOffsetTot = mod16PS( 2 * totBlendLength - colorOffset - colorIndex, totBlendLength);

        //run over each of the leds in the segment set and set a color/brightness value
        for (uint16_t i = 0; i < numLines; i++) {
            
            //if we modulating the brightness, calculate a new brightness for each line
            //otherwise leave it alone
            if(doBrightness){
                bri = inoise8( i * briScale, currentTime/briFreq );
                // The range of the inoise8 function is roughly 16-238.
                // These two operations expand those values out to roughly 0..255
                // You can comment them out if you want the raw noise data.
                bri = qsub8( bri, 16);
                bri = qadd8( bri, scale8( bri, 39 ) );

                //brighten up, as the color palette itself often contains the 
                //light/dark dynamic range desired
                if( bri > 127 ) {
                    bri = 255;
                } else {
                    bri = dim8_raw( bri * 2 );
                }
            } else {
                bri = 255;
            }

            //get the blended palette color of each line based on the line number and the offset we caluclated for this cycle
            colorOut = paletteUtilsPS::getPaletteGradColor(palette, i, colorOffsetTot, totBlendLength, blendSteps);

            //color each segment pixel in the blended palette color
            for(uint16_t j = 0; j < numSegs; j++){
                //get the current pixel's location in the segment set
                pixelNum = segDrawUtils::getPixelNumFromLineNum(segmentSet, numLines, j, i);

                //get background color info for the current pixel
                colorTarget = segDrawUtils::getPixelColor(segmentSet, pixelNum, *bgColor, bgColorMode, j, i);

                //get the color blended towards the background and output it
                colorTarget = colorUtilsPS::getCrossFadeColor(colorOut, colorTarget, 255 - bri);
                //colorFinal = colorOut;
                //nscale8x3( colorFinal.r, colorFinal.g, colorFinal.b, bri); //scaling for if you want to switch to a blank bg
                segDrawUtils::setPixelColor(segmentSet, pixelNum, colorTarget, 0, 0, 0);   

            }
        }
        showCheckPS();
        //update the offsets
        phaseOffset++;
        colorOffset = addMod16PS(colorOffset, 1, totBlendLength);
    }
}

//Shifts the blendSteps towards the blendStepstarget by one step (this keeps things smooth)
//If it's reached the target, pick a new target to shift to
//!!Do NOT set the blendSteps directly after turning on shiftBlendSteps()
//if you do, be sure to adjust blendSteps = blendStepsTarget
void NoiseGradSL::shiftBlendSteps(){
    if(blendSteps == blendStepsTarget){
        //get a new target scale
        blendStepsTarget = blendStepsBase + random16(blendStepsRange);

        //set the scale step (+1 or -1)
        //if we happen to have re-rolled the current scale, then 
        //we want to re-roll. The easist way to do this is to set scaleStep to 0
        //so scaleTarget will stay equal to scale, triggering a re-roll when setShiftScale is called again
        if(blendStepsTarget == blendSteps){
            blendStepsStep = 0;
        } else if (blendStepsTarget > blendSteps){
            blendStepsStep = 1; 
        } else {
            blendStepsStep = -1;
        }
    }
    blendSteps += blendStepsStep;  
}


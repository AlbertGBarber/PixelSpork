#include "NoiseWavesSL.h"

//Constructor with palette
NoiseWavesSL::NoiseWavesSL(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, uint16_t BlendScale,
                           uint8_t PhaseScale, uint8_t FreqScale, uint16_t Rate)
    : palette(&Palette), blendScale(BlendScale), phaseScale(PhaseScale), freqScale(FreqScale)  //
{
    init(BgColor, SegSet, Rate);
}

//Constructor with randomly generated palette
NoiseWavesSL::NoiseWavesSL(SegmentSetPS &SegSet, uint8_t numColors, CRGB BgColor, uint16_t BlendScale,
                           uint8_t PhaseScale, uint8_t FreqScale, uint16_t Rate)
    : blendScale(BlendScale), phaseScale(PhaseScale), freqScale(FreqScale)  //
{
    paletteTemp = paletteUtilsPS::makeRandomPalette(numColors);
    palette = &paletteTemp;
    init(BgColor, SegSet, Rate);
}

NoiseWavesSL::~NoiseWavesSL() {
    free(paletteTemp.paletteArr);
}

//Initializes core common variables
void NoiseWavesSL::init(CRGB BgColor, SegmentSetPS &SegSet, uint16_t Rate) {
    //bind the rate and segSet pointer vars since they are inherited from BaseEffectPS
    bindSegSetPtrPS();
    bindClassRatesPS();

    //bind the bgColor pointer
    bindBGColorPS();

    //Catch is any noise var is == 0
    if( phaseScale <= 0 ) {
        phaseScale = 1;
    }

    if( freqScale <= 0 ) {
        freqScale = 1;
    }

    if( blendSpeed <= 0 ) {
        blendSpeed = 1;
    }
}

/* Updates the effect
The core of the effect consists of a brightness wave whose frequency and phase are changed over time using noise values
The phase is calculated once every update and added to a cos wave that is calculated at each pixel
The cos wave uses a noise value for its frequency. Normally changing the frequency causes sudden jumps in the effect,
but because the noise varies smoothly over time, jumps are avoided
Meanwhile the colors also vary with noise based on time, but are not tied to a wave, so the effect is more like random patches
Overall these components produce an effect of varying colors with waves of
brightness that both move and grow/shrink across the strip with time
The waves are drawn across segment lines (so each line has the same color)
There are various settings for the number of waves, speed, etc which can be found in the .h file into
Note that the background is fully compatible with background color modes, so it can vary with time */
void NoiseWavesSL::update() {
    currentTime = millis();

    if( (currentTime - prevTime) >= *rate ) {
        prevTime = currentTime;

        freqCounter = 0;
        //re-fetch some core variables incase the palette has changed
        numSegs = segSet->numSegs;
        numLines = segSet->numLines;
        totBlendLength = blendSteps * palette->length;

        //Get a phase value for our waves using noise
        //The hue offset keeps the waves moving across the strip
        noisePhase = inoise8(currentTime / phaseScale) + hue;

        //run over each segment line and set color and brightness values for the whole line
        for( uint16_t i = 0; i < numLines; i++ ) {
            //Get a changing frequency multiplier, which helps produce the waves
            //We need this to change as we loop or all the pixels will have the same wave value
            //we cap this to keep the frequency from getting too high
            freqCounter = addmod8(freqCounter, 1, 60);

            //Get a color based on some noise
            //For the color index we add an hue offset the increments each cycle
            //Noise values tend to fall around the center of the 255 range, so by adding
            //a moving hue offset we make sure that we see all the palette colors and keeps the effect varying
            index = inoise8(i * blendScale, currentTime / blendSpeed) + hue;

            /* Get a brightness based on a cos wave with noisy inputs
            The brightness is set by a wave who's frequency and phase vary as functions of noise
            The noise in turn varies with time
            This produces a set of brightness waves that both move and grow/shrink across the segment lines
            Since noise vary's smoothly, the frequency change is not noticeable as long at the noise is "zoomed - in" enough
            In practice this means keeping the time devisor around 10. Setting it higher makes the waves choppy, and lower makes them to fast
            Also we add 10 to the freqCounter to make sure we have a noticeable frequency at all times */
            bri = cos8((freqCounter + 10) * inoise8(currentTime / 10) / freqScale + noisePhase);  //remove freqCounter for neat blinking

            //get the blended color from the palette
            //scale the index to the length of all the color blends combined
            colorIndex = scale16by8(totBlendLength, index);
            colorOut = paletteUtilsPS::getPaletteGradColor(*palette, colorIndex, 0, totBlendLength, blendSteps);

            //take the line color and brightness and apply it to all the seg pixels on the line
            for( uint16_t j = 0; j < numSegs; j++ ) {
                //get the current pixel's location in the segment set
                pixelNum = segDrawUtils::getPixelNumFromLineNum(*segSet, j, i);

                //get background color info for the current pixel
                colorTarget = segDrawUtils::getPixelColor(*segSet, pixelNum, *bgColor, bgColorMode, j, i);

                //get the color blended towards the background and output it
                colorTarget = colorUtilsPS::getCrossFadeColor(colorOut, colorTarget, 255 - bri);
                segDrawUtils::setPixelColor(*segSet, pixelNum, colorTarget, 0, 0, 0);
            }
        }
        showCheckPS();

        //increment the noise center hue offset if desired
        //hue offset is a uint8_t so it wraps at 256
        if(hueCycle){
           hue++; 
        }
    }
}

//For NoiseWaves, not seg line based
/*
//Updates the effect
//The core of the effect consists of a brightness wave whose frequency and phase are changed over time using noise values
//The phase is calculated once every update and added to a cos wave that is calculated at each pixel
//The cos wave uses a noise value for its frequency. Normally changing the frequency causes sudden jumps in the effect,
//but because the noise varies smoothly over time, jumps are avoided
//Meanwhile the colors also vary with noise based on time, but are not tied to a wave, so the effect is more like random patches
//Overall these components produce an effect of varying colors with waves of 
//brightness that both move and grow/shrink across the strip with time
//There are various settings for the number of waves, speed, etc which can be found in the .h file into
//Note that the background is fully compatible with background color modes, so it can vary with time
void NoiseWavesPS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        freqCounter = 0;
        pixelCount = 0;
        //re-fetch some core variables incase the palette has changed
        numSegs = segSet->numSegs;
        totBlendLength = blendSteps * palette->length;

        //Get a phase value for our waves using noise
        //The hue offset keeps the waves moving across the strip
        noisePhase = inoise8( currentTime/phaseScale ) + hue offset;

        //run over each of the leds in the segment set and set a color/brightness value
        for (uint16_t i = 0; i < numSegs; i++) {
            totSegLen = segSet->getTotalSegLength(i);
            for(uint16_t j = 0; j < totSegLen; j++){
                //Get a changing frequency multiplier, which helps produce the waves
                //We need this to change as we loop or all the pixels will have the same wave value
                //we cap this to keep the frequency from getting too high
                freqCounter = addmod8(freqCounter, 1, 60);

                //get the current pixel's location in the segment set
                pixelNum = segDrawUtils::getSegmentPixel(*segSet, i, j);

                //Get a color based on some noise
                //For the color index we add an hue offset the increments each cycle
                //Noise values tend to fall around the center of the 255 range, so by adding
                //a moving hue offset we make sure that we see all the palette colors and keeps the effect varying
                index = inoise8( pixelCount * blendScale, currentTime/blendSpeed ) + hue offset;// beatsin16(4, 0, totBlendLength /2);
                
                //Get a brightness based on a cos wave with noisy inputs
                //The brightness is set by a wave who's frequency and phase vary as functions of noise
                //The noise in turn varies with time
                //This produces a set of brightness waves that both move and grow/shrink across the strip with time
                //Since noise vary's smoothly, the frequency change is not noticeable as long at the noise is "zoomed - in" enough
                //In practice this means keeping the time devisor around 10. Setting it higher makes the waves choppy, and lower makes them to fast
                //Also we add 10 to the freqCounter to make sure we have a noticeable frequency at all times
                bri = cos8( (freqCounter + 10) * inoise8( pixelCount, currentTime/10 ) / freqScale + noisePhase );

                //cos8(pixelCount * 30 + bandStart );
                
                //get the blended color from the palette
                colorIndex = scale16by8( totBlendLength, index ); //scale the index to the length of all the color blends combined
                colorOut = paletteUtilsPS::getPaletteGradColor(palette, colorIndex, 0, totBlendLength, blendSteps);

                //nscale8x3( colorOut.r, colorOut.g, colorOut.b, bri); //scaling for if you want to switch to a blank bg

                //get background color info for the current pixel
                lineNum = segDrawUtils::getLineNumFromPixelNum(*segSet, j, i);
                colorTarget = segDrawUtils::getPixelColor(*segSet, pixelNum, *bgColor, bgColorMode, i, lineNum);

                //get the color blended towards the background and output it
                colorOut = colorUtilsPS::getCrossFadeColor(colorOut, colorTarget, 255 - bri);
                segDrawUtils::setPixelColor(*segSet, pixelNum, colorOut, 0, 0, 0);   

                pixelCount++;
            }
        }
        showCheckPS();
        hue offset++;
    }
}
*/

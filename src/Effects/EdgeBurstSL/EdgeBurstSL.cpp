#include "EdgeBurstSL.h"

//Constructor for rainbow mode
EdgeBurstSL::EdgeBurstSL(SegmentSet &SegmentSet, uint8_t BurstFreq, uint16_t Rate):
    segmentSet(SegmentSet), burstFreq(BurstFreq)
    {    
        init(Rate);
        rainbowMode = true;

        //We create a random palette in case rainbow mode is turned off without setting a palette
        //This won't be used while rainbowMode is true
        paletteTemp = paletteUtilsPS::makeRandomPalette(3);
        palette = &paletteTemp; 
	}

//Constructor for colors from palette
EdgeBurstSL::EdgeBurstSL(SegmentSet &SegmentSet, palettePS &Palette, uint8_t BurstFreq, uint16_t Rate):
    segmentSet(SegmentSet), burstFreq(BurstFreq), palette(&Palette)
    {    
        init(Rate);
	}

//Constructor for a randomly created palette
//RandomizePal = true will randomize the palette for each wave
EdgeBurstSL::EdgeBurstSL(SegmentSet &SegmentSet, uint8_t numColors, bool RandomizePal, uint8_t BurstFreq, uint16_t Rate):
    segmentSet(SegmentSet), burstFreq(BurstFreq), randomizePal(RandomizePal)
    {    
        init(Rate);

        paletteTemp = paletteUtilsPS::makeRandomPalette(numColors);
        palette = &paletteTemp; 
	}

EdgeBurstSL::~EdgeBurstSL(){
    free(paletteTemp.paletteArr);
}

//initialize core vars
void EdgeBurstSL::init(uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
        
    //minimum burst freq is 1
    if(burstFreq < 1){
        burstFreq = 1;
    }
}

/*
    Original code:
    export function beforeRender(delta) {
        t1 = triangle(time(.1))
    }
    export function render(index) {
        f = index/pixelCount
        edge = clamp(triangle(f) + t1 * 4 - 2, 0, 1)
        v = triangle(edge)
        h = edge * edge - .2
        s = 1
        hsv(h, s, v)
    }
 */

//Updates the effect
//To be honest I don't really know how the waves work
//But the main driver is t1
void EdgeBurstSL::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        //fetch some core vars
        //we re-fetch these in case the segment set or palette has changed
        numSegs = segmentSet.numSegs;
        numLines = segmentSet.numLines;

        //Get the blend length for each color in the palette
        //(using 255 steps across the whole palette)
        //We do this so we only need to do it once per cycle
        blendLength = 255 / palette->length;
        
        beatVal = beat8(burstFreq);
        t1 = triwave8(beatVal);

        //We want to change the wave spawn point after a wave has finished
        //A wave ends/starts every half cycle of the triwave8 above
        //So we want to set the offset every time the wave passes through the mid-point (128)
        //Unfortunatly, the wave doesn't usually hit 128 exactly due to the frequency
        //So we need to catch it after is passes through, but then only set the offset once
        //hence the flipFlop boolean, which stops us from setting the offset more than once each half cycle
        if(beatVal > 128 && !offsetFlipFlop){
            pickRandStart();
        } else if(beatVal < 128 && offsetFlipFlop){
            pickRandStart();
        }

        //set a color for each line and then color in all the pixels on the line
        for (uint16_t i = 0; i < numLines; i++) {

            f = (uint32_t)(i * 255) / numLines;
            //note really sure how burstPause works, but it seems to easily adjust the time between waves
            edge = triwave8(f) + t1 * burstPause - (burstPause / 2 * 255);
            edge = clamp8PS(edge, 0, 255);
            v = triwave8( uint8_t(edge) );
            h = edge * edge / 255 - 51;

            //If we're in rainbow mode, pick a color using th HSV color wheel
            //Otherwise pick a color from the palette. Note that we use 255 blend steps for the whole palette.
            //We also need to dim the color by v.
            if(rainbowMode){
                colorOut = CHSV(h, 255, v);
            } else {
                colorOut = paletteUtilsPS::getPaletteGradColor(*palette, h, 0, 255, blendLength);
                nscale8x3(colorOut.r, colorOut.g, colorOut.b, v);
            }

            for (uint16_t j = 0; j < numSegs; j++) {
                //get the physical pixel location based on the line and seg numbers
                //and then write out the color
                //Note that the actual line written to is offset and wraps
                pixelNum = segDrawUtils::getPixelNumFromLineNum(segmentSet, numLines, j, addMod16PS(i, offset, numLines) );
                segDrawUtils::setPixelColor(segmentSet, pixelNum, colorOut, 0, 0, 0);
            }

        }
        showCheckPS();
    }
}

//randomizes the wave start point (the offset)
//sets the flipFlop so we know not to set the offset more than once per wave cycle
//Also increments the burstCount var to track how many bursts we've done
//If randomizePal is true, the colors in paletteTemp will be set randomly
void EdgeBurstSL::pickRandStart(){
    offsetFlipFlop = !offsetFlipFlop;
    burstCount++;
    offset = random16(numLines);
    if(randomizePal){
        paletteUtilsPS::randomize(paletteTemp);
    }
}
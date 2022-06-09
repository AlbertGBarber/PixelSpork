#include "ColorMeltSLPS.h"

//Constructor for rainbow mode
ColorMeltSLPS::ColorMeltSLPS(SegmentSet &SegmentSet, uint8_t MeltFreq, uint8_t PhaseFreq, uint16_t Rate):
    segmentSet(SegmentSet), meltFreq(MeltFreq), phaseFreq(PhaseFreq)
    {    
        rainbowMode = true;
        init(Rate);

        //We create a random pallet in case rainbow mode is turned off without setting a pallet
        //This won't be used while rainbowMode is true
        palletTemp = palletUtilsPS::makeRandomPallet(3);
        pallet = &palletTemp; 
	}

//Constructor for colors from pallet
ColorMeltSLPS::ColorMeltSLPS(SegmentSet &SegmentSet, palletPS *Pallet, uint8_t MeltFreq, uint8_t PhaseFreq, uint16_t Rate):
    segmentSet(SegmentSet), pallet(Pallet), meltFreq(MeltFreq), phaseFreq(PhaseFreq)
    {    
        init(Rate);
	}

//Constructor for a randomly created pallet
ColorMeltSLPS::ColorMeltSLPS(SegmentSet &SegmentSet, uint8_t numColors, uint8_t MeltFreq, uint8_t PhaseFreq, uint16_t Rate):
    segmentSet(SegmentSet), meltFreq(MeltFreq), phaseFreq(PhaseFreq)
    {    
        init(Rate);
        palletTemp = palletUtilsPS::makeRandomPallet(numColors);
        pallet = &palletTemp; 
	}

ColorMeltSLPS::~ColorMeltSLPS(){
    delete[] palletTemp.palletArr;
}

void ColorMeltSLPS::init(uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    if(phaseFreq == 0){
        phaseEnable = false;
    }

    //minimum melt freq is 1
    if(meltFreq < 1){
        meltFreq = 1;
    }
}

/*
    Original code:
    hl = pixelCount/2
    export function beforeRender(delta) {
        t1 =  time(.1)
        t2 = time(0.13)
    }
    export function render(index) {
        c1 = 1-abs(index - hl)/hl
        c2 = wave(c1)
        c3 = wave(c2 + t1)
        v = wave(c3 + t1)
        v = v*v
        hsv(c1 + t2,1,v)
     }
 */

//updates the effect
//I don't fully understand how the effect works, it's mainly combining a bunch of waves
void ColorMeltSLPS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;
        
        //fetch some core vars
        //we re-fetch these in case the segment set or pallet has changed
        numSegs = segmentSet.numSegs;
        numLines = segmentSet.maxSegLength;

        hl = numLines/2;
        t1 = beat8(meltFreq); 
        t2 = beat8(7); 

        //increment the phase
        if(phaseEnable){
            phase = beat8(phaseFreq);
        }

        //Get the blend length for each color in the pallet
        //(using 255 steps across the whole pallet)
        //We do this so we only need to do it once per cycle
        blendLength = 255 / pallet->length;

        //set a color for each line and then color in all the pixels on the line
        for (uint16_t i = 0; i < numLines; i++) {

            c1 = 255 - abs(i - hl) * 255 / hl;
            c2 = sin8(c1 + phase); //adding the phase here seems to work best
            c3 = sin8(c2 + c1);

            v = sin8(c3 + t1);
            v = v * v / 255;

            //If we're in rainbow mode, pick a color using th HSV color wheel
            //Otherwise pick a color from the pallet. Note that we use 255 blend steps for the whole pallet.
            //We also need to dim the color by v.
            if(rainbowMode){
                colorOut = CHSV(c1 + t2, 255, v);
            } else {
                colorOut = palletUtilsPS::getPalletGradColor(pallet, c1 + t2, 0, 255, blendLength);
                nscale8x3(colorOut.r, colorOut.g, colorOut.b, v);
            }

            for (uint8_t j = 0; j < numSegs; j++) {
                //get the physical pixel location based on the line and seg numbers
                //and then write out the color
                pixelNum = segDrawUtils::getPixelNumFromLineNum(segmentSet, numLines, j,  numLines - i - 1);
                segDrawUtils::setPixelColor(segmentSet, pixelNum, colorOut, 0, 0, 0);
            }
        }
        showCheckPS();
    }
}
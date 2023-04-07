#include "RainbowFontsSL.h"

RainbowFontsSL::RainbowFontsSL(SegmentSet &SegSet, uint8_t WaveFreq, uint16_t Rate):
    SegSet(SegSet), waveFreq(WaveFreq)
    {    
        //bind the rate and SegSet pointer vars since they are inherited from BaseEffectPS
        bindSegPtrPS();
        bindClassRatesPS();
        
        //minimum wave freq is 1
        if(waveFreq < 1){
            waveFreq = 1;
        }
	}

//Updates the effect
//To be honest I don't really know how the waves work
//But the main driver is t1
void RainbowFontsSL::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        //fetch some core vars
        //we re-fetch these in case the segment set or palette has changed
        numSegs = SegSet.numSegs;
        numLines = SegSet.numLines;
        
        hl = numLines/2;

        t1 = beat8(waveFreq);

        //set a color for each line and then color in all the pixels on the line
        for (uint16_t i = 0; i < numLines; i++) {
            
            c = 255 - abs(i - hl) * 255 / hl;
            c = sin8(c);
            c = sin8(c + t1);

            colorOut = CHSV(c, 255, 255);

            //reverse the line number so that the effect moves positively along the strip
            lineNum = numLines - i - 1;

            for (uint16_t j = 0; j < numSegs; j++) {
                //get the physical pixel location based on the line and seg numbers
                //and then write out the color
                //Note that the actual line written to is offset and wraps
                pixelNum = segDrawUtils::getPixelNumFromLineNum(SegSet, numLines, j,  lineNum);
                segDrawUtils::setPixelColor(SegSet, pixelNum, colorOut, 0, 0, 0);
            }

        }
        showCheckPS();
    }
}
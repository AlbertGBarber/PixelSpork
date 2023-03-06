#include "PacificaPS.h"

PacificaPS::PacificaPS(SegmentSet &SegmentSet, uint16_t Rate):
    segmentSet(SegmentSet)
    {    
        //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
        bindSegPtrPS();
        bindClassRatesPS();
        //We can pre-allocate the number of gradient steps between the palette colors
        //This speeds up execution
        totBlendLength = numSteps * pacificaPal1PS.length;
	}

void PacificaPS::update(){
    currentTime = millis();

    //if it's time to update the effect, do so
    deltams = currentTime - prevTime;
    if( ( deltams ) >= *rate ) {
        prevTime = currentTime;

        numSegs = segmentSet.numSegs;

        // Increment the four "color index start" counters, one for each wave layer.
        // Each is incremented at a different speed, and the speeds vary over time.
        speedfactor1 = beatsin16(3, 179, 269);
        speedfactor2 = beatsin16(4, 179, 269);
        deltams1 = (deltams * speedfactor1) / 256;
        deltams2 = (deltams * speedfactor2) / 256;
        deltams21 = (deltams1 + deltams2) / 2;
        sCIStart1 += (deltams1 * beatsin88(1011,10,13));
        sCIStart2 -= (deltams21 * beatsin88(777,8,11));
        sCIStart3 -= (deltams1 * beatsin88(501,5,7));
        sCIStart4 -= (deltams2 * beatsin88(257,4,6));

        //Clear out the LED array to a dim background blue-green
        segDrawUtils::fillSegSetColor(segmentSet, *bgColor, 0);

        //Render each of four layers, with different scales and speeds, that vary over time
        doOneLayer( &pacificaPal1PS, sCIStart1, beatsin16( 3, 11 * 256, 14 * 256 ), beatsin8( 10, 70, 130 ), 0-beat16(301) ); //10
        doOneLayer( &pacificaPal2PS, sCIStart2, beatsin16( 4,  6 * 256,  9 * 256) , beatsin8( 17, 40, 80 ), beat16(401) ); //17
        doOneLayer( &pacificaPal3PS, sCIStart3, 6 * 256, beatsin8( 9, 10, 38 ), 0-beat16(503)); //9
        doOneLayer( &pacificaPal3PS, sCIStart4, 5 * 256, beatsin8( 8, 10, 28 ), beat16(601)); //8

        //Add brighter 'whitecaps' where the waves lines up more
        addWhitecaps();

        //Deepen the blues and greens a bit
        deepenColors();

        showCheckPS();
    }
}

//Add one layer of waves into the led array
void PacificaPS::doOneLayer(palettePS *palette, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff){
    ci = cistart;
    waveangle = ioff;
    wavescale_half = (wavescale / 2) + 20;
    //Run over each of the leds and set the color
    for (uint16_t i = 0; i < numSegs; i++) {
        totSegLen = segmentSet.getTotalSegLength(i);
        for(uint16_t j = 0; j < totSegLen; j++){
            waveangle += 250;
            s16 = sin16( waveangle ) + 32768;
            cs = scale16( s16 , wavescale_half ) + wavescale_half;
            ci += cs;
            sindex16 = sin16( ci ) + 32768;
            index = scale16( sindex16, totBlendLength );

            //returns the blended color from the palette mapped into numSteps
            colorOut = paletteUtilsPS::getPaletteGradColor(*palette, index, 0, totBlendLength, numSteps);
            nscale8x3(colorOut.r, colorOut.g, colorOut.b, bri);
            pixelNum = segDrawUtils::getSegmentPixel(segmentSet, i, j);

            segmentSet.leds[pixelNum] += colorOut;
        }
    }
}

//Add extra 'white' to areas where the four layers of light have lined up brightly
void PacificaPS::addWhitecaps(){
    basethreshold = beatsin8( 9, 55, 65);
    wave = beat8( 7 );
        
    for (uint16_t i = 0; i < numSegs; i++) {
        totSegLen = segmentSet.getTotalSegLength(i);
        for(uint16_t j = 0; j < totSegLen; j++){
            threshold = scale8( sin8( wave ), 20) + basethreshold;
            wave += 7;
            pixelNum = segDrawUtils::getSegmentPixel(segmentSet, i, j);
            lightLvl = segmentSet.leds[pixelNum].getAverageLight();
            if( lightLvl > threshold) {
                overage = lightLvl - threshold;
                overage2 = qadd8( overage, overage );
                segmentSet.leds[pixelNum] += CRGB( overage, overage2, qadd8( overage2, overage2) );
            }
        }
    }
}

 // Deepen the blues and greens
void PacificaPS::deepenColors(){
    for (uint16_t i = 0; i < numSegs; i++) {
        totSegLen = segmentSet.getTotalSegLength(i);
        for(uint16_t j = 0; j < totSegLen; j++){
            pixelNum = segDrawUtils::getSegmentPixel(segmentSet, i, j);
            segmentSet.leds[pixelNum].blue  = scale8( segmentSet.leds[pixelNum].blue, 145); 
            segmentSet.leds[pixelNum].green = scale8( segmentSet.leds[pixelNum].green, 200); 
            //segmentSet.leds[pixelNum].red = scale8( segmentSet.leds[pixelNum].red, 200); //for lava colors
            segmentSet.leds[pixelNum] |= CRGB( 2, 5, 7);
            //segmentSet.leds[pixelNum] |= CRGB( 8, 0, 0); //for lava colors

            //Need to check to dim the pixel color manually
            //b/c we're not calling setPixelColor directly
            //we do this here b/c deepenColors is the last function in setting the colors
            segDrawUtils::handleBri(segmentSet, pixelNum);
        }
    }
}
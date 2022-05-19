#include "PacificaSegLinePS.h"

//Normal constructor
PacificaSegLinePS::PacificaSegLinePS(SegmentSet &SegmentSet, uint16_t Rate):
    segmentSet(SegmentSet)
    {    
        init(Rate);
	}

//constructor with addWhiteCaps setting
PacificaSegLinePS::PacificaSegLinePS(SegmentSet &SegmentSet, bool AddWhiteCaps, uint16_t Rate):
    segmentSet(SegmentSet), addWhiteCaps(AddWhiteCaps)
    {    
        init(Rate);
	}

void PacificaSegLinePS::init(uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    CRGB BgColor = CRGB( 2, 6, 10 );
    //CRGB BgColor = CRGB(10, 0, 0); //for lava colors
    bindBGColorPS();
    //We can pre-allocate the number of gradient steps between the pallet colors
    //This speeds up execution
    totBlendLength = numSteps * pacificaPal1PS.length;
}

//Updates the effect
//This version of Pacifica differs from the normal because we draw the colors along seg lines
//This produces uniform waves that shift across the whole segment set
//I have had to make some adjustments to the code, including eliminating the deepenColors() function and
//making the addWhiteCaps() function optional
//This is because the colors are added to each pixel's current color when the doOneLayer() is called
//But the same pixel can exist in multiple seg lines, so it will be added to multiple times
//This caused the deepenColors() to consistently ruin the colors.
//Likewise, addWhiteCaps()'s purpose was to brighten the colors where multiple waves met,
//But we are now basically doing this by default with common line pixels
//so addWhiteCaps() is now only needed for exact matrixes
//Overall, the end result is very similar to the orignal Pacifica, and still very pretty
void PacificaSegLinePS::update(){
    currentTime = millis();

    deltams = currentTime - prevTime;
    //if it's time to update the effect, do so
    if( ( deltams ) >= *rate ) {
        prevTime = currentTime;

        //fetch some core vars
        //we re-fetch these in case the segment set has changed
        numSegs = segmentSet.numSegs;
        numLines = segmentSet.maxSegLength;

        //Increment the four "color index start" counters, one for each wave layer.
        //Each is incremented at a different speed, and the speeds vary over time.
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
        doOneLayer( &pacificaPal1PS, sCIStart1, beatsin16( 3, 11 * 256, 14 * 256), beatsin8( 10, 70, 130), 0-beat16( 301) ); //10
        doOneLayer( &pacificaPal2PS, sCIStart2, beatsin16( 4,  6 * 256,  9 * 256), beatsin8( 17, 40, 80), beat16( 401) ); //17
        doOneLayer( &pacificaPal3PS, sCIStart3, 6 * 256, beatsin8( 9, 10, 38 ), 0-beat16(503)); //9
        doOneLayer( &pacificaPal3PS, sCIStart4, 5 * 256, beatsin8( 8, 10, 28 ), beat16(601)); //8

        //Add brighter 'whitecaps' where the waves lines up more
        //only needed for exact matrixes
        //(for onther shapes was already add the waves together in doOneLayer when pixels are common to multiple lines)
        if(addWhiteCaps){
            addWhitecaps();
        }

        showCheckPS();
    }
}

//Add one layer of waves into the led array
void PacificaSegLinePS::doOneLayer(palletPS *pallet, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff){
    ci = cistart;
    waveangle = ioff;
    wavescale_half = (wavescale / 2) + 20;
    //Run over each of the lines and set a color
    //We get one color for each segment line and then output it to all the segments
    for (uint16_t i = 0; i < numLines; i++) {
        waveangle += 250;
        s16 = sin16( waveangle ) + 32768;
        cs = scale16( s16 , wavescale_half ) + wavescale_half;
        ci += cs;
        sindex16 = sin16( ci ) + 32768;
        index = scale16( sindex16, totBlendLength );
        //returns the blended color from the pallet mapped into numSteps
        colorOut = palletUtilsPS::getPalletGradColor(pallet, index, 0, totBlendLength, numSteps);
        nscale8x3(colorOut.r, colorOut.g, colorOut.b, bri);

        //output the color to all the line segment pixels
        //Since the colors are additive, pixels in multiple lines will be brighter than those in single lines
        //This is more or less eqivalent to doing addWhitecaps()
        for(uint8_t j = 0; j < numSegs; j++){
            pixelNum = segDrawUtils::getPixelNumFromLineNum(segmentSet, numLines, j, i);
            segmentSet.leds[pixelNum] += colorOut;
        }
    }
}

// Add extra 'white' to areas where the four layers of light have lined up brightly
void PacificaSegLinePS::addWhitecaps(){
    basethreshold = beatsin8( 9, 55, 65);
    wave = beat8( 7 );

    //each line has on color, so we only set the threshold once
    //We still check the lightLvl of each pixel, because pixels can be common to multiple lines
    //so their colors from doOneLayer() can differ
    for (uint16_t i = 0; i < numLines; i++) {
        threshold = scale8( sin8( wave ), 20) + basethreshold;
        wave += 7;
        for(uint8_t j = 0; j < numSegs; j++){
            pixelNum = segDrawUtils::getPixelNumFromLineNum(segmentSet, numLines, j, i);
            lightLvl = segmentSet.leds[pixelNum].getAverageLight();
            if( lightLvl > threshold && lightLvl < thresholdMax) {
                overage = lightLvl - threshold;
                overage2 = qadd8( overage, overage );
                segmentSet.leds[pixelNum] += CRGB( overage, overage2, qadd8( overage2, overage2) );
            }
        }
    }
}

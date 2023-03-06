#include "PacificaHueSL.h"

//Normal constructor
PacificaHueSL::PacificaHueSL(SegmentSet &SegmentSet, uint16_t Rate):
    segmentSet(SegmentSet)
    {    
        //use 0 for the hueRate b/c we don't want the hue to change
        init(0, Rate);
	}

//Normal constructor addWhiteCaps and hue setting
//The default hue is 130 (found in PacificaHuePal.h)
PacificaHueSL::PacificaHueSL(SegmentSet &SegmentSet, bool AddWhiteCaps, uint8_t Hue, uint16_t Rate):
    segmentSet(SegmentSet), addWhiteCaps(AddWhiteCaps)
    {    
        //use 0 for the hueRate b/c we don't want the hue to change
        init(0, Rate);
        setHue(Hue);
	}

//constructor with addWhiteCaps and hue rate settings
//The hue will be the initial hue of the palette
PacificaHueSL::PacificaHueSL(SegmentSet &SegmentSet, bool AddWhiteCaps, uint8_t Hue, uint16_t HueRate, uint16_t Rate):
    segmentSet(SegmentSet), addWhiteCaps(AddWhiteCaps)
    {    
        init(HueRate, Rate);
        setHue(Hue);
	}

PacificaHueSL::~PacificaHueSL(){
    PacificaPalette->~PacificaHuePalPS();
}

//sets up core effect variables
void PacificaHueSL::init(uint16_t HueRate, uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    PacificaPalette = new PacificaHuePalPS();
    hue = PacificaPalette->pfHue;

    totBlendLength = 240;
    //Produces a total blend length of 240 for the whole palette, matches the original code
    numSteps = totBlendLength / PacificaPalette->pacificaPal1PS.length;

    //Point the bgColor to the bgColor in the pacifica palette, since it needs to change if the hue changes
    bgColor = &PacificaPalette->bgColor;

    //bind the hue rate to it's pointer
    hueRateOrig = HueRate;
    hueRate = &hueRateOrig;
}

//Sets the pacifica palette hue
//Note that this is actually set in the PacificaPalette instance,
//but we mirror the value here for ease of access
void PacificaHueSL::setHue(uint8_t newHue){
    hue = newHue;
    PacificaPalette->setHue(newHue);
}

//Updates the effect
//This version of Pacifica differs from the normal because we draw the colors along seg lines
//This produces uniform waves that shift across the whole segment set
//We also allow for different hues (see PacificaHuePal.h)
//I have had to make some adjustments to the code, including eliminating the deepenColors() function and
//making the addWhiteCaps() function optional
//This is because the colors are added to each pixel's current color when the doOneLayer() is called
//But the same pixel can exist in multiple seg lines, so it will be added to multiple times
//This caused the deepenColors() to consistently ruin the colors (and it also doesn't work well with multiple hues).
//Likewise, addWhiteCaps()'s purpose was to brighten the colors where multiple waves met,
//But we are now basically doing this by default with common line pixels
//so addWhiteCaps() is now only needed for exact matrixes
//Overall, the end result is very similar to the orignal Pacifica, and still very pretty
void PacificaHueSL::update(){
    currentTime = millis();

    deltams = currentTime - prevTime;
    //if it's time to update the effect, do so
    if( ( deltams ) >= *rate ) {
        prevTime = currentTime;
        
        //increment the hue if needed
        if(*hueRate > 0 && ( currentTime - prevHueTime ) >= *hueRate ){
            prevHueTime = currentTime; 
            PacificaPalette->incHue();
            hue = PacificaPalette->pfHue;
        }

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

        //Clear out the LED array to a dim background
        segDrawUtils::fillSegSetColor(segmentSet, *bgColor, 0);

        //Render each of four layers, with different scales and speeds, that vary over time
        doOneLayer( &PacificaPalette->pacificaPal1PS, sCIStart1, beatsin16( 3, 11 * 256, 14 * 256), beatsin8( 10, 70, 130), 0-beat16(301) ); 
        doOneLayer( &PacificaPalette->pacificaPal2PS, sCIStart2, beatsin16( 4,  6 * 256,  9 * 256), beatsin8( 17, 40, 80), beat16(401) ); 
        doOneLayer( &PacificaPalette->pacificaPal3PS, sCIStart3, 6 * 256, beatsin8( 9, 10, 38 ), 0-beat16(503)); 
        doOneLayer( &PacificaPalette->pacificaPal3PS, sCIStart4, 5 * 256, beatsin8( 8, 10, 28 ), beat16(601)); 

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
void PacificaHueSL::doOneLayer(palettePS *palette, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff){
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
        //returns the blended color from the palette mapped into numSteps
        colorOut = paletteUtilsPS::getPaletteGradColor(*palette, index, 0, totBlendLength, numSteps);
        nscale8x3(colorOut.r, colorOut.g, colorOut.b, bri);

        //output the color to all the line segment pixels
        //Since the colors are additive, pixels in multiple lines will be brighter than those in single lines
        //This is more or less eqivalent to doing addWhitecaps()
        for(uint16_t j = 0; j < numSegs; j++){
            pixelNum = segDrawUtils::getPixelNumFromLineNum(segmentSet, numLines, j, i);
            segmentSet.leds[pixelNum] += colorOut;

            //Need to check to dim the pixel color manually
            //b/c we're not calling setPixelColor directly
            //We only want to do this once per pixel, so we only do it if we're not setting whitecaps
            if(!addWhiteCaps){
                segDrawUtils::handleBri(segmentSet, pixelNum);
            }
        }
    }
}

// Add extra 'white' to areas where the four layers of light have lined up brightly
void PacificaHueSL::addWhitecaps(){
    //I've set the base threshold to 0, seems to work best with the hue based palette 
    basethreshold = 0;// beatsin8( 9, 35, 45); //modified limits from original values: 55, 65
    wave = beat8( 7 );

    //each line has one color, so we only set the threshold once
    //We still check the lightLvl of each pixel, because pixels can be common to multiple lines
    //so their colors from doOneLayer() can differ
    //We also set a cap on the brightness, so pixel's don't become fully white
    for (uint16_t i = 0; i < numLines; i++) {
        threshold = scale8( sin8( wave ), 20) + basethreshold;
        wave += 7;
        for(uint16_t j = 0; j < numSegs; j++){
            pixelNum = segDrawUtils::getPixelNumFromLineNum(segmentSet, numLines, j, i);
            lightLvl = segmentSet.leds[pixelNum].getAverageLight();
            if( lightLvl > threshold && lightLvl < thresholdMax) {
                overage = lightLvl - threshold;
                overage2 = qadd8( overage, overage );
                //modified the white cap addition to better match the current hue
                //CRGB( overage, overage2, qadd8( overage2, overage2) ); 
                segmentSet.leds[pixelNum] += CHSV(PacificaPalette->pfHue, overage2, qadd8( overage2, overage2));
            }
            //Need to check to dim the pixel color manually
            //b/c we're not calling setPixelColor directly
            //we do this here b/c addWhitecaps is the last function in setting the colors
            segDrawUtils::handleBri(segmentSet, pixelNum);
        }
    }
}

/* 
//Deepen the blues and greens
//Kept for reference, but it doesn't really work with multiple hues
//You just end up dimming all the colors
void PacificaHueSL::deepenColors(){
    for (uint16_t i = 0; i < numSegs; i++) {
        totSegLen = segmentSet.getTotalSegLength(i);
        for(uint16_t j = 0; j < totSegLen; j++){
            pixelNum = segDrawUtils::getSegmentPixel(segmentSet, i, j);
            segmentSet.leds[pixelNum].blue  = scale8( segmentSet.leds[pixelNum].blue, 145); //173?
            segmentSet.leds[pixelNum].green = scale8( segmentSet.leds[pixelNum].green, 200); //173?
            segmentSet.leds[pixelNum].red = scale8( segmentSet.leds[pixelNum].red, 200); //173?
            segmentSet.leds[pixelNum] |= CHSV((PacificaPalette->pfHue + 6), 255, 8); //CRGB( 2, 5, 7);
        }
    }
} 
*/

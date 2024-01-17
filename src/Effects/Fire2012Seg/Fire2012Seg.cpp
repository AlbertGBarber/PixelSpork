#include "Fire2012Seg.h"

Fire2012Seg::Fire2012Seg(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, uint8_t Cooling,
                         uint8_t Sparking, bool Blend, uint16_t Rate)
    : palette(&Palette), cooling(Cooling), sparking(Sparking), blend(Blend)  //
{
    //bind the rate and segSet pointer vars since they are inherited from BaseEffectPS
    bindSegSetPtrPS();
    bindClassRatesPS();
    //bind background color pointer (if needed)
    bindBGColorPS();
    reset();
}

Fire2012Seg::~Fire2012Seg() {
    free(heat);
    free(heatSegStarts);
}

//resets the effect and creates new heat arrays
//call this if you change segment sets or sections
void Fire2012Seg::reset() {
    numSegs = segSet->numSegs;
    numLeds = segSet->numLeds;

    //create the heat array to store temperatures of each line point
    //We only need to make a new heat array if the current one isn't large enough
    //This helps prevent memory fragmentation by limiting the number of heap allocations
    //but this may use up more memory overall.
    if( alwaysResizeObj_PS || (numLeds > maxNumLeds) ) {
        maxNumLeds = numLeds;
        free(heat);
        heat = (uint8_t *)malloc(numLeds * sizeof(uint8_t));

        //For storing the local starting points for each segment's heat
        //(see comments below)
        free(heatSegStarts);
        heatSegStarts = (uint16_t *)malloc(numSegs * sizeof(uint16_t));
    }

    //The heat array works by storing heat values at points on the strip
    //For speed, the heat values for the whole segSet are stored in one array,
    //using offsets to ensure we only work on the section that corresponds to the current segment
    //the offsets are stored in the heatSegStarts in order of the segments
    uint16_t segmentSetLength = 0;
    // get the total segSet length, and set the starting offsets
    for( uint16_t i = 0; i < numSegs; i++ ) {
        heatSegStarts[i] = segmentSetLength;  // the start offsets are just the lengths of each segment
        segmentSetLength += segSet->getTotalSegLength(i);
    }

    //Reset the heats in the heat array
    for( uint16_t i = 0; i < numLeds; i++ ) {
        heat[i] = 0;
    }
}

/* Updates the effect
This code has been adapted from https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/#fire
to work with segments and palettes
Overall the effect manages a heat array of uint8_t's, with each pixel in the segment set being having it's own
heat (temperature) value
These heat values are heated and cooled with each update cycle, producing the flame effect
Each segment has it's own separate fire simulation
We use a single array heat to manage multiple segments by splitting it into sections of lengths equal to the segment lengths
We store the start indexes of the segments in the heatSegStarts array (created in reset())
Within a fire colors taken from a palette and blended together to create a smooth flame (see setPixelHeatColorPalette() for info) */
void Fire2012Seg::update() {
    currentTime = millis();

    //Before We loop
    //work out some palette vars to be used in getPixelHeatColorPalette
    //setting them here is more efficient since they only need to be set once per update cycle
    //(we check them each update incase the palette has changed)
    paletteLength = palette->length;
    //paletteLimit = paletteLength - 1;
    //For color blending, we need to divide the temperature range (0 - 255) into sections of the palette
    //we divide by paletteLength + 1 because we need to include a section for the background
    paletteSecLen = 255 / (paletteLength + 1);

    if( (currentTime - prevTime) >= *rate ) {
        prevTime = currentTime;

        //For each segment do the following:
        for( uint16_t i = 0; i < numSegs; i++ ) {
            //get the length of the segment we're working on
            segLength = segSet->getTotalSegLength(i);

            heatSecStart = heatSegStarts[i];  // current segment's start index in the heat array

            //Step 1. Cool down every cell a little
            for( uint16_t j = 0; j < segLength; j++ ) {

                heatIndex = j + heatSecStart;  // adjusted index for heat array
                //subtract a random cooling factor from the current heat (qsub8 prevents running past 0)
                heat[heatIndex] = qsub8(heat[heatIndex], random8(0, ((cooling * 10) / segLength) + 2));
                //cooldown = random(0, ((cooling * 10) / segLength) + 2) ;

                // if cooldown would turn off a pixel, turn if off,
                // otherwise just cool it down
                //if (cooldown > heat[heatIndex]) {
                //heat[heatIndex] = 0;
                //} else {
                //heat[heatIndex] = heat[heatIndex] - cooldown;
                //}
            }

            //Step 2. Heat from each cell drifts 'up' and diffuses a little
            for( uint16_t k = (segLength - 1); k >= 2; k-- ) {
                heatIndex = k + heatSecStart;  // adjusted index for heat array
                heat[heatIndex] = (heat[heatIndex - 1] + heat[heatIndex - 2] + heat[heatIndex - 2]) / 3;
            }

            //Step 3. Randomly ignite new 'sparks' near the bottom
            if( random8(255) < sparking ) {
                // pick a random pixel near the start of the strip
                //default is 7, but we'll lower this for shorter segments
                sparkPoint = 7;
                if( segLength < sparkPoint ) {
                    sparkPoint = 2;
                }
                heatIndex = random8(sparkPoint) + heatSecStart;  // adjusted index for heat array
                // add a random bit of heat (qadd8 keeps within 255)
                heat[heatIndex] = qadd8(heat[heatIndex], random8(160, 255));
                //heat[heatIndex] = heat[heatIndex] + random8(160, 255);
            }

            //Step 4. For each flame, convert heat to palette colors colors and output
            for( uint16_t k = 0; k < segLength; k++ ) {
                ledLoc = segDrawUtils::getSegmentPixel(*segSet, i, k);  //the physical location of the led
                colorOut = fire2012SegUtilsPS::getPixelHeatColorPalette(palette, paletteLength, paletteSecLen,
                                                                        bgColor, heat[k + heatSecStart], blend);

                segDrawUtils::setPixelColor(*segSet, ledLoc, colorOut, 0, 0, 0);
            }
        }
        showCheckPS();
    }
}
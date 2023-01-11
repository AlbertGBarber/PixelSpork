#include "Fire2012SL.h"

Fire2012SL::Fire2012SL(SegmentSet &SegmentSet, palettePS *Palette, CRGB BgColor, uint8_t Cooling, uint8_t Sparking, bool Blend, uint16_t Rate, bool Direct):
    segmentSet(SegmentSet), palette(Palette), cooling(Cooling), sparking(Sparking), blend(Blend), direct(Direct)
    {    
        //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
        bindSegPtrPS();
        bindClassRatesPS();
        //bind background color pointer (if needed)
        bindBGColorPS();
        reset();
	}

Fire2012SL::~Fire2012SL(){
    free(heat);
}

//resets the effect and creates new heat arrays
//call this if you change segment sets or sections
void Fire2012SL::reset(){
    free(heat);

    //fetch some core vars
    numSegs = segmentSet.numSegs;
    numLines = segmentSet.maxSegLength;
    uint16_t numPoints = numLines * numSegs;
    
    //create the heat array to store temperatures of each line point
    heat = (uint8_t*) malloc(numPoints * sizeof(uint8_t));
    for(uint16_t i = 0; i < numPoints; i++){
        heat[i] = 0;
    }
}

//Updates the effect
//This code has been adapted from https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/#fire
//to work with segments and palettes
//Overall the effect manages a heat array of uint8_t's, with each pixel in the segment set being having it's own 
//heat (temperature) value
//These heat values are heated and cooled with each update cycle, producing the flame effect
//Each segment line has it's own seperate fire simulation
//We use a single array heat to manage multiple segments by splitting it into sections of lengths equal to the number of segments
//Within a fire, colors taken from a palette and blended together to create a smooth flame (see setPixelHeatColorPalette() for info)
void Fire2012SL::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        //Before We loop
        //work out some palette vars to be used in getPixelHeatColorPalette
        //setting them here is more efficent since they only need to be set once per update cycle
        //(we check them each update incase the palette has changed)
        paletteLength = palette->length;
        //paletteLimit = paletteLength - 1;
        //For color blending, we need to divide the temperature range (0 - 255) into sections of the palette
        //we divide by paletteLength + 1 because we need to include a section for the background
        paletteSecLen = 255 / (paletteLength + 1); 

        //For each segment line do the following:
        for (uint8_t i = 0; i < numLines; i++) {

            heatSecStart = i * numSegs; // current segment line's start index in the heat array

            //Step 1. Cool down every cell a little
            for (uint16_t j = 0; j < numSegs; j++) {

                heatIndex = j + heatSecStart; // adjusted index for heat array
                //subtract a random cooling factor from the current heat (qsub8 prevents running past 0)
                heat[heatIndex] = qsub8( heat[heatIndex], random8(0, ((cooling * 10) / numSegs) + 2) );
            }

            //Step 2. Heat from each cell drifts 'up' and diffuses a little
            for (uint16_t j = (numSegs - 1); j >= 2; j--) {
                heatIndex = j + heatSecStart; // adjusted index for heat array
                heat[heatIndex] = (heat[heatIndex - 1] + heat[heatIndex - 2] + heat[heatIndex - 2]) / 3;
            }

            //Step 3. Randomly ignite new 'sparks' near the bottom
            if (random8(255) < sparking) {
                //pick a random pixel near the start of the strip
                //default is 7, but we'll lower this for shorter segments
                sparkPoint = 7;
                if(numSegs < sparkPoint){
                    sparkPoint = 2;
                }
                heatIndex = random8(sparkPoint) + heatSecStart; // adjusted index for heat array
                //add a random bit of heat (qadd8 keeps within 255)
                heat[heatIndex] = qadd8( heat[heatIndex], random8(160, 255) );
                //heat[heatIndex] = heat[heatIndex] + random8(160, 255);
            }

            //Step 4. For each flame, convert heat to palette colors colors and output
            //Also adjust for the direction of the flame
            for (uint16_t j = 0; j < numSegs; j++) {

                //if the flames are reversed, then we reverse which segment we're writing the temperature out to
                if(!direct){
                    segNum = numSegs - j - 1;
                } else {
                    segNum = j;
                }

                //get the physical pixel location based on the line and seg numbers
                ledLoc = segDrawUtils::getPixelNumFromLineNum(segmentSet, numLines, segNum, i);
                //write out the temeprature color
                colorOut = Fire2012SegUtilsPS::getPixelHeatColorPalette(palette, paletteLength, paletteSecLen, 
                                                                       bgColor, heat[j + heatSecStart], blend);

                segDrawUtils::setPixelColor(segmentSet, ledLoc, colorOut, 0, 0, 0);    
            }
        }
        showCheckPS();
    }
}

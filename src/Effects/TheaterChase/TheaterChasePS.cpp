#include "TheaterChasePS.h"

TheaterChasePS::TheaterChasePS(SegmentSet &SegmentSet, CRGB Color, CRGB BgColor, uint8_t LitLength, uint8_t Spacing, uint16_t Rate):
    segmentSet(SegmentSet)
    {    
        //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
        bindSegPtrPS();
        bindClassRatesPS();
        bindBGColorPS();
        //bind the color varaible since it's a pointer
        colorOrig = Color;
        color = &colorOrig;
        //set the litLength and spacing,
        //we set them using thier functions since they have a min value of 1
        setLitLength(LitLength);
        setSpacing(Spacing);
        setDrawLength(); //redundant, but left as failsafe
	}

//sets the litLength (min value 1)
void TheaterChasePS::setLitLength(uint8_t newLitlength){
    litLength = newLitlength;
    if(litLength < 1) {
        litLength = 1;
    }
    setDrawLength();
}

//sets the spacing (min value 1)
void TheaterChasePS::setSpacing(uint8_t newSpacing){
    spacing = newSpacing;
    if(spacing < 1) {
        spacing = 1;
    }
    setDrawLength();
}

//recaculates the totalDrawLength based on the spacing and litLength
//needs to be re-done each time one of them changes
void TheaterChasePS::setDrawLength(){
    totalDrawLength = spacing + litLength;
}

//updates the effect
//Each time we update, we first re-draw the background color 
//Then we run across all the pixels, skipping forward in blocks of totalDrawLength (the sum of the spot the litLength and the spacing)
//For each block we fill the colored spot in up to litLength
//The position of the spot is based in the cycleCount, with cycles from 0 to totalDrawLength - 1, and increments once per update
//Basically, the strip is broken up into lengths of totalDrawLength size
//each length has its own spot that moves back and forth based on what cycle number we're on
//since the spots are all the same, they line up when the end/start, so it looks like they move down the strip
void TheaterChasePS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        numPixels = segmentSet.numActiveSegLeds;

        //fill the whole strip with bgColor
        segDrawUtils::fillSegSetColor(segmentSet, *bgColor, bgColorMode);

        //Re-draw the spots, working with regions of totalDrawLength (sum of litLength and spacing)
        //Each spot loops around it's region, rather than actually moving down the strip
        for (uint16_t i = 0; i < numPixels; i += totalDrawLength ) {   
            for(uint8_t j = 0; j < litLength; j++){
                //when we draw the spot, we start with the first pixel of the spot,
                //if the spot size is greater than 1 (litLength), then we will run into the next 
                //spot region. To prevent this, loop the spot round by modding by totalDrawLength
                pixelLoc = i + (j + cycleCount) % totalDrawLength;
                segDrawUtils::setPixelColor(segmentSet, pixelLoc, *color, colorMode);
            }
        }
        
        cycleCount = (cycleCount + 1) % (totalDrawLength);
        showCheckPS();
    }
}
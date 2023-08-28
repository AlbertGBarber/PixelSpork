#include "TheaterChaseSL.h"

TheaterChaseSL::TheaterChaseSL(SegmentSetPS &SegSet, CRGB Color, CRGB BgColor, uint8_t LitLength,
                               uint8_t Spacing, uint16_t Rate)  //
{
    //bind the rate and segSet pointer vars since they are inherited from BaseEffectPS
    bindSegSetPtrPS();
    bindClassRatesPS();
    //bind the bgColor pointer
    bindBGColorPS();
    //bind the color variable since it's a pointer
    colorOrig = Color;
    color = &colorOrig;
    //set the litLength and spacing,
    //we set them using their functions since they have a min value of 1
    setLitLength(LitLength);
    setSpacing(Spacing);
}

//sets the litLength (min value 1)
void TheaterChaseSL::setLitLength(uint8_t newLitLength) {
    litLength = newLitLength;
    if( litLength < 1 ) {
        litLength = 1;
    }
}

//sets the spacing (min value 1)
void TheaterChaseSL::setSpacing(uint8_t newSpacing) {
    spacing = newSpacing;
    if( spacing < 1 ) {
        spacing = 1;
    }
}

/* updates the effect
Each time we update, we first re-draw the background color 
Then we run across all the pixels, skipping forward in blocks of totalDrawLength (the sum of the spot the litLength and the spacing)
For each block we fill the colored spot in up to litLength
The position of the spot is based in the cycleNum, with cycles from 0 to totalDrawLength - 1, and increments once per update
Basically, the strip is broken up into lengths of totalDrawLength size
each length has its own spot that moves back and forth based on what cycle number we're on
since the spots are all the same, they line up when the end/start, so it looks like they move down the strip */
void TheaterChaseSL::update() {
    currentTime = millis();

    if( (currentTime - prevTime) >= *rate ) {
        prevTime = currentTime;

        numLines = segSet->numLines;
        //recalculate the draw length so you can change spacing or litLength on the fly
        totalDrawLength = spacing + litLength;

        //fill the whole strip with bgColor
        segDrawUtils::fillSegSetColor(*segSet, *bgColor, bgColorMode);

        //Re-draw the spots, working with regions of totalDrawLength (sum of litLength and spacing)
        //Each spot loops around it's region, rather than actually moving down the strip
        for( uint16_t i = 0; i < numLines; i += totalDrawLength ) {
            for( uint8_t j = 0; j < litLength; j++ ) {
                //when we draw the spot, we start with the first pixel of the spot,
                //if the spot size is greater than 1 (litLength), then we will run into the next
                //spot region. To prevent this, loop the spot round by modding by totalDrawLength
                lineNum = i + addMod16PS(j, cycleNum, totalDrawLength);  //i + (j + cycleNum) % totalDrawLength;
                segDrawUtils::drawSegLine(*segSet, lineNum, *color, colorMode);
            }
        }

        cycleNum = addMod16PS(cycleNum, 1, totalDrawLength);  //(cycleNum + 1) % (totalDrawLength);
        showCheckPS();
    }
}
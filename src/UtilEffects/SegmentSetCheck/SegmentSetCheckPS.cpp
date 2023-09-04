#include "SegmentSetCheckPS.h"

SegmentSetCheckPS::SegmentSetCheckPS(SegmentSetPS &SegSet, uint8_t TestMode)
    : testMode(TestMode)  //
{
    uint16_t Rate = 500;  //set a default rate of 500ms
    init(SegSet, Rate);
}

SegmentSetCheckPS::SegmentSetCheckPS(SegmentSetPS &SegSet, uint8_t TestMode, uint16_t Rate)
    : testMode(TestMode)  //
{
    init(SegSet, Rate);
}

void SegmentSetCheckPS::init(SegmentSetPS &SegSet, uint16_t Rate) {
    //bind the rate and segSet pointer vars since they are inherited from BaseEffectPS
    bindSegSetPtrPS();
    bindClassRatesPS();

    //Set the initial testing mode using nextTestMode()
    //nextTestMode() either sets the current testing mode (mode) to testMode (if we only want to run one test)
    //or advances to the next testing mode, 0 or 1, if we're doing both tests
    //We want to start at test 0, so we set mode to 1 so that nextTestMode() wraps it back to 0
    mode = 1;
    nextTestMode();
}

/* A testing function used to check segment configuration
It has two modes:
    0: Goes through each segment in the segment set
        Sets the first and last pixels to red and blue respectively for each segment
        While the rest of the segment is lit up in green, one pixel at a time
    1: Colors the first and last segment line in red and blue respectively
        While the other lines are flashed in green, one by one
What modes are run is based on the testMode value:
    0: Only testing mode 0, repeating
    1: Only testing mode 1, repeating
    2+: Testing both modes, one after another, repeating */
void SegmentSetCheckPS::update() {
    currentTime = millis();

    if( (currentTime - prevTime) >= *rate ) {
        prevTime = currentTime;

        //If we're starting a new test we need to clear the segment set
        if( testStart ) {
            segDrawUtils::turnSegSetOff(*segSet);
            testStart = false;
        }

        if( mode == 0 ) {
            numSegs = segSet->numSegs;
            totSegLen = segSet->getTotalSegLength(segNum);
            //for each segment we set the first pixel to red, the last to blue,
            //and then fill in the rest with green one by one from the start of the segment to the end
            segDrawUtils::setPixelColor(*segSet, pixelCount, CRGB::Green, 0, segNum);
            segDrawUtils::setPixelColor(*segSet, 0, CRGB::Red, 0, segNum);
            segDrawUtils::setPixelColor(*segSet, totSegLen - 1, CRGB::Blue, 0, segNum);

            pixelCount++;
            //if the we're at the end of the segment, we need to switch to the next segment
            if( pixelCount >= totSegLen ) {
                //reset the pixelCount and advance to the next segment
                pixelCount = 1;
                segNum++;
                //if the next segment is inactive, we set the pixelCount to its length,
                //so that we'll switch to the next segment on the next cycle
                //if(segNum != numSegs && !segSet->getSegActive(segNum)){
                //pixelCount = segSet->getTotalSegLength(segNum);
                //}
                //clear the segment set
                segDrawUtils::turnSegSetOff(*segSet);
            }

            //if we've done all the segments we need to advance to the next testing mode
            //(segment lines)
            if( segNum >= numSegs ) {
                nextTestMode();
            }
        } else if( mode == 1 ) {
            //check the segment lines
            //The first and last segment line are colored red and blue respectively
            //while each other segment line is lit up in green one at a time
            numLines = segSet->numLines;
            //turn off the previous segment line
            segDrawUtils::drawSegLine(*segSet, pixelCount - 1, 0, 0);
            //draw the segment line in green
            segDrawUtils::drawSegLine(*segSet, pixelCount, CRGB::Green, 0);

            //set the first and last lines to red and green
            segDrawUtils::drawSegLine(*segSet, 0, CRGB::Red, 0);
            segDrawUtils::drawSegLine(*segSet, numLines - 1, CRGB::Blue, 0);

            pixelCount++;
            //if we've drawn all the lines we need to advance to the next testing mode
            if( pixelCount >= numLines ) {
                nextTestMode();
            }
        } else {
            //If we're here, then something has bugged out, try to go to a new test mode
            nextTestMode();
        }

        showCheckPS();
    }
}

//advances to the next testing mode based on the current mode
//ie moving between mode 0 and 1, or just repeating the current mode
//depending on the check mode
void SegmentSetCheckPS::nextTestMode() {
    if( testMode < 2 ) {
        //if the testMode is 0 or 1, then we're only doing that mode
        //so we set the mode to match, so we repeat the mode
        mode = testMode;
    } else {
        //if we're doing both modes we need to advance to the next mode
        //wrapping back to 0 as we advance
        mode = addmod8(mode, 1, 2);
    }
    //Reset test variables to begin a new test stage
    pixelCount = 1;
    segNum = 0;
    testStart = true;
}
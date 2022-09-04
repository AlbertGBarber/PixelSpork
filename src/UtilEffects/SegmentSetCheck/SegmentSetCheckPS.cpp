#include "SegmentSetCheckPS.h"

SegmentSetCheckPS::SegmentSetCheckPS(SegmentSet &SegmentSet, uint8_t TestMode):
    segmentSet(SegmentSet), testMode(TestMode)
    {    
        //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
        bindSegPtrPS();
        uint16_t Rate = 500; //set a default rate of 500ms
        bindClassRatesPS();
	}

SegmentSetCheckPS::SegmentSetCheckPS(SegmentSet &SegmentSet, uint8_t TestMode, uint16_t Rate):
    segmentSet(SegmentSet), testMode(TestMode)
    {  
        //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
        bindSegPtrPS();
        bindClassRatesPS();
    }

//A testing function used to check segment configuration
//It has two modes:
//  0: Goes through each segment in the segment set
//     Sets the first and last pixels to red and blue respectivly for each segment
//     While the rest of the segment is lit up in green, one pixel at a time
//  1: Colors the first and last segment line in red and blue respectivly
//     While the other lines are flashed in green, one by one
//What modes are run is based on the testMode value:
//  0: Only testing mode 0, repeating
//  1: Only testing mode 1, repeating
//  2+: Testing both modes, one after another, repeating
void SegmentSetCheckPS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;
        
        if(mode == 0 && testMode != 1){
            numSegs = segmentSet.numSegs;
            totSegLen = segmentSet.getTotalSegLength(segNum);
            //for each segment we set the first pixel to red, the last to blue,
            //and then fill in the rest with green one by one from the start of the segment to the end
            segDrawUtils::setPixelColor(segmentSet, pixelCount, segNum, CRGB::Green, 0);
            segDrawUtils::setPixelColor(segmentSet, 0, segNum, CRGB::Red, 0);
            segDrawUtils::setPixelColor(segmentSet, totSegLen - 1, segNum, CRGB::Blue, 0);

            pixelCount++;
            //if the we're at the end of the segment, we need to switch to the next segment
            if(pixelCount >= totSegLen){
                //reset the pixelCount and advance to the next segment
                pixelCount = 1;
                segNum++;
                //if the next segment is inactive, we set the pixelCount to its length,
                //so that we'll switch to the next segment on the next cycle
                //if(segNum != numSegs && !segmentSet.getSegActive(segNum)){
                    //pixelCount = segmentSet.getTotalSegLength(segNum);
                //}
                //clear the segment set
                segDrawUtils::fillSegSetColor(segmentSet, 0, 0);
            }

            //if we've done all the segments we need to advance to the next testing mode
            //(segment lines)
            if(segNum >= numSegs){
                nextStage();
            }
        } else if(testMode != 0) {
            //check the segment lines
            //The first and last segment line are colored red and blue respectivly
            //while each other segment line is lit up in green one at a time
            numLines = segmentSet.maxSegLength;
            //turn off the previous segment line
            segDrawUtils::drawSegLineSimple(segmentSet, pixelCount - 1, 0, 0);
            //draw the segment line in green
            segDrawUtils::drawSegLineSimple(segmentSet, pixelCount, CRGB::Green, 0);

            //set the first and last lines to red and green
            segDrawUtils::drawSegLineSimple(segmentSet, 0, CRGB::Red, 0);
            segDrawUtils::drawSegLineSimple(segmentSet, numLines - 1, CRGB::Blue, 0);

            pixelCount++;
            //if we've drawn all the lines we need to advance to the next testing mode
            if(pixelCount >= numLines){
                nextStage();
            }
        }

        showCheckPS();
    }
}

//advances to the next testing mode based on the current mode
//ie moving between mode 0 and 1, or just repeating the current mode
//depending on the check mode
void SegmentSetCheckPS::nextStage(){
    if(testMode <= 1){
        //if the testMode is 0 or 1, then we're only doing that mode
        //so we set the mode to match, so we repeat the mode
        mode = testMode;
    } else {
        //if we're doing both modes we need to advance to the next mode
        //wraping back to 0 as we advance
        mode = addmod8(mode, 1, 2);
    }
    pixelCount = 1;
    segNum = 0;
    segDrawUtils::fillSegSetColor(segmentSet, 0, 0);
}
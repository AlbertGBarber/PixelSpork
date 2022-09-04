#include "SoftTwinkleSL.h"

SoftTwinkleSL::SoftTwinkleSL(SegmentSet& SegmentSet, uint8_t Density, uint16_t Rate): 
    segmentSet(SegmentSet), density(Density)
    {
        // bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
        bindSegPtrPS();
        bindClassRatesPS();
        reset();
    }

//turns off all the leds in the segment set to reset the effect
void SoftTwinkleSL::reset(){
    segDrawUtils::fillSegSetColor(segmentSet, 0, 0);
}

void SoftTwinkleSL::update() {
    currentTime = millis();

    if ((currentTime - prevTime) >= *rate) {
        prevTime = currentTime;

        numLines = segmentSet.maxSegLength;
        longestSeg = segmentSet.segNumMaxSegLength;

        for (uint16_t i = 0; i < numLines; i++) {
            //pixelNum = segDrawUtils::getSegmentPixel(segmentSet, i);
            //Get the color for the line
            //To do this we get the color of the pixel on the longest seg in the line
            //b/c that pixel will not be shared with another line, and so should not have had its color modified 
            //since the last update
            pixelNum = segDrawUtils::getPixelNumFromLineNum(segmentSet, numLines, longestSeg, i);
            color = segmentSet.leds[pixelNum];
            if (!color)
                continue; // skip black pixels
            if (color.r & 1) { // is red odd?
                color -= lightcolor; // darken if red is odd
            } else {
                color += lightcolor; // brighten if red is even
            }
            //color the segment line
            segDrawUtils::drawSegLineSimple(segmentSet, i, color, 0);
        }

        // Randomly choose a segment line, and if it's black, 'bump' it up a little.
        // Since it will now have an EVEN red component, it will start getting
        // brighter over time.
        if (random8() < density) {
            //like before, we always use the line pixel on the longest segment
            //since it's not shared with any other lines, so it's color won't have been overwritten
            lineNum = random16(numLines);
            pixelNum = segDrawUtils::getPixelNumFromLineNum(segmentSet, numLines, longestSeg, lineNum);
            color = segmentSet.leds[pixelNum];
            if (!color) {
                segDrawUtils::drawSegLineSimple(segmentSet, lineNum, lightcolor, 0);
            }
        }
        showCheckPS();
    }
}
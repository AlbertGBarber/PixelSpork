#include "SoftTwinklePS.h"

SoftTwinklePS::SoftTwinklePS(SegmentSet& SegmentSet, uint8_t Density, uint16_t Rate): 
    segmentSet(SegmentSet), density(Density)
    {
        // bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
        bindSegPtrPS();
        bindClassRatesPS();
        reset();
    }

//turns off all the leds in the segment set to reset the effect
void SoftTwinklePS::reset(){
    segDrawUtils::fillSegSetColor(segmentSet, 0, 0);
}

void SoftTwinklePS::update() {
    currentTime = millis();

    if ((currentTime - prevTime) >= *rate) {
        prevTime = currentTime;

        numPixels = segmentSet.numActiveSegLeds;
        for (int i = 0; i < numPixels; i++) {
            pixelNum = segDrawUtils::getSegmentPixel(segmentSet, i);
            color = segmentSet.leds[pixelNum];
            if (!color)
                continue; // skip black pixels
            if (color.r & 1) { // is red odd?
                color -= lightcolor; // darken if red is odd
            } else {
                color += lightcolor; // brighten if red is even
            }
            segmentSet.leds[pixelNum] = color;
        }

        // Randomly choose a pixel, and if it's black, 'bump' it up a little.
        // Since it will now have an EVEN red component, it will start getting
        // brighter over time.
        if (random8() < density) {
            pixelNum = segDrawUtils::getSegmentPixel(segmentSet, random16(numPixels));
            color = segmentSet.leds[pixelNum];
            if (!color) {
                segmentSet.leds[pixelNum] = lightcolor;
            }
        }
        showCheckPS();
    }
}
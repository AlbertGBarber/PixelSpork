#ifndef segDrawUtils_h
#define segDrawUtils_h

#include "FastLED.h"
#include "segmentSection.h"
#include "segment.h"
#include "segmentSet.h"
#include "pixelInfoPS.h"

#define dLed 65536

//utility functions for finding the physical pixel number(i.e. it's strip location) of segment pixels
//and for coloring pixels
//!!YOU SHOULD ALWAYS USE THESE FUNCTIONS TO DRAW ON SEGMENTS
//!!DO NOT TRY DRAWING DIRECTLY UNLESS YOU 100% KNOW WHAT YOU ARE DOING
namespace segDrawUtils{
    void 
        turnSegSetOff(SegmentSet segmentSet),
        getSegLocationFromPixel(SegmentSet segmentSet, uint16_t segPixelNum, uint16_t locData[2] ),
        fillSegSetColor(SegmentSet segmentSet, CRGB color, uint8_t colorMode),
        fillSegColor(SegmentSet segmentSet, byte segNum, CRGB color, uint8_t colorMode),
        fillSegSecColor(SegmentSet segmentSet, uint8_t segNum, uint16_t secNum, CRGB color, uint8_t colorMode ),
        fillSegLengthColor(SegmentSet segmentSet, CRGB color, uint8_t colorMode, uint16_t segNum, uint16_t startSegPixel, uint16_t endPixel),
        fillSegSetlengthColor(SegmentSet segmentSet, CRGB color, uint8_t colorMode, uint16_t startSegPixel, uint16_t endPixel),
        drawSegLine(SegmentSet segmentSet, byte lineNum, byte Pattern[], CRGB pallet[], uint8_t colorMode, uint8_t bgColorMode, boolean brReplace),
        drawSegLineSection(SegmentSet segmentSet, uint8_t startSeg, uint8_t endseg, byte lineNum, byte Pattern[], CRGB pallet[], uint8_t colorMode, uint8_t bgColorMode, boolean brReplace),
        drawSegLineSimple(SegmentSet segmentSet, byte lineNum, CRGB color, uint8_t colorMode),
        drawSegLineSimpleSection(SegmentSet segmentSet, uint8_t startSeg, uint8_t endSeg, byte lineNum, CRGB color, uint8_t colorMode),
        setPixelColor(SegmentSet segmentSet, uint16_t segPixelNum, CRGB color, uint8_t colorMode),
        setPixelColor(SegmentSet segmentSet, uint16_t segPixelNum, uint8_t segNum, CRGB color, uint8_t colorMode),
        setPixelColor(SegmentSet segmentSet, uint16_t pixelNum, CRGB color, uint8_t colorMode, uint8_t segNum, uint8_t lineNum),
        getPixelColor(SegmentSet segmentSet, pixelInfoPS *pixelInfo, CRGB color, uint8_t colorMode, uint16_t segPixelNum),
        fadeSegSetToBlackBy(SegmentSet segmentSet, uint8_t val),
        fadeSegToBlackBy(SegmentSet segmentSet, uint8_t segNum, uint8_t val),
        fadeSegSecToBlackBy(SegmentSet segmentSet, uint8_t segNum, uint16_t secNum, uint8_t val);
            
            //fillSegBgGradientRGB(SegmentSet segmentSet),
            //setBgPallet(RGB pallet[], uint8_t palletLength),
    
    uint8_t 
        getLineNumFromPixelNum(SegmentSet segmentSet, uint16_t segPixelNum),
        getLineNumFromPixelNum(SegmentSet segmentSet, uint16_t segPixelNum, uint8_t segNum),
        getCrossFadeColorComp(uint8_t startColor, uint8_t endColor, uint8_t ratio);

    static uint16_t
        fixedRBRate = 50; //default setting for color mode 7 of setPixelColor in ms

    uint16_t
        getSegmentPixel(SegmentSet segmentSet, uint16_t segPixelNum),
        getSegmentPixel(SegmentSet segmentSet, uint8_t segNum, uint16_t num),
        getPixelNumFromLineNum(SegmentSet segmentSet, uint16_t maxSegLength, uint8_t segNum, uint8_t lineNum);
        
    CRGB
        wheel( byte wheelPos, uint8_t rainbowOffset ),
        wheel( byte wheelPos, uint8_t offset, uint8_t satur, uint8_t value ),
        getPixelColor(SegmentSet segmentSet, uint16_t pixelNum, CRGB color, uint8_t colorMode, uint8_t segNum, uint8_t lineNum),
        getCrossFadeColor(CRGB startColor, CRGB endColor, uint8_t step, uint8_t totalSteps),
        getCrossFadeColor(CRGB startColor, CRGB endColor, uint8_t ratio),
        randColor();

};

#endif
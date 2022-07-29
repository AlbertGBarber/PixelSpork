#ifndef segDrawUtils_h
#define segDrawUtils_h

//TODO:
//-- Consolidate vars
//-- Add shortcuts for situations with one segment in a set

#include "FastLED.h"
#include "segmentSection.h"
#include "segment.h"
#include "segmentSet.h"
#include "pixelInfoPS.h"
#include "ColorUtils/colorUtilsPS.h"
#include "PaletteFiles.h"
#include "MathUtils/mathUtilsPS.h"

//The value to indicate a dummy led (max of uint16_t)
//Pixels with this value will be ignored by set color setting functions
#define dLed 65535

//utility functions for finding the physical pixel number(i.e. it's strip location) of segment pixels
//and for coloring pixels
//!!YOU SHOULD ALWAYS USE THESE FUNCTIONS TO DRAW ON SEGMENTS
//!!DO NOT TRY DRAWING DIRECTLY UNLESS YOU 100% KNOW WHAT YOU ARE DOING
namespace segDrawUtils{
    //Function Definitions
    void 
        turnSegSetOff(SegmentSet &segmentSet),
        getSegLocationFromPixel(SegmentSet &segmentSet, uint16_t segPixelNum, uint16_t locData[2] ),
        fillSegSetColor(SegmentSet &segmentSet, CRGB color, uint8_t colorMode),
        fillSegColor(SegmentSet &segmentSet, uint8_t segNum, CRGB color, uint8_t colorMode),
        fillSegSecColor(SegmentSet &segmentSet, uint8_t segNum, uint16_t secNum, CRGB color, uint8_t colorMode ),
        fillSegLengthColor(SegmentSet &segmentSet, CRGB color, uint8_t colorMode, uint16_t segNum, uint16_t startSegPixel, uint16_t endPixel),
        fillSegSetlengthColor(SegmentSet &segmentSet, CRGB color, uint8_t colorMode, uint16_t startSegPixel, uint16_t endPixel),
        drawSegLine(SegmentSet &segmentSet, uint16_t lineNum, uint8_t Pattern[], CRGB palette[], uint8_t colorMode, uint8_t bgColorMode, bool brReplace),
        drawSegLineSection(SegmentSet &segmentSet, uint8_t startSeg, uint8_t endseg, uint16_t lineNum, uint8_t Pattern[], CRGB palette[], uint8_t colorMode, uint8_t bgColorMode, bool brReplace),
        drawSegLineSimple(SegmentSet &segmentSet, uint16_t lineNum, CRGB color, uint8_t colorMode),
        drawSegLineSimpleSection(SegmentSet &segmentSet, uint8_t startSeg, uint8_t endSeg, uint16_t lineNum, CRGB color, uint8_t colorMode),
        setPixelColor(SegmentSet &segmentSet, uint16_t segPixelNum, CRGB color, uint8_t colorMode),
        setPixelColor(SegmentSet &segmentSet, uint16_t segPixelNum, uint8_t segNum, CRGB color, uint8_t colorMode),
        setPixelColor(SegmentSet &segmentSet, uint16_t pixelNum, CRGB color, uint8_t colorMode, uint8_t segNum, uint16_t lineNum),
        getPixelColor(SegmentSet &segmentSet, pixelInfoPS *pixelInfo, CRGB color, uint8_t colorMode, uint16_t segPixelNum),
        setGradOffset(SegmentSet &segmentSet, uint16_t offsetMax),
        fadeSegSetToBlackBy(SegmentSet &segmentSet, uint8_t val),
        fadeSegToBlackBy(SegmentSet &segmentSet, uint8_t segNum, uint8_t val),
        fadeSegSecToBlackBy(SegmentSet &segmentSet, uint8_t segNum, uint16_t secNum, uint8_t val);
    
    uint8_t 
        getLineNumFromPixelNum(SegmentSet &segmentSet, uint16_t segPixelNum),
        getLineNumFromPixelNum(SegmentSet &segmentSet, uint16_t segPixelNum, uint8_t segNum);

    uint16_t
        getSegmentPixel(SegmentSet &segmentSet, uint16_t segPixelNum),
        getSegmentPixel(SegmentSet &segmentSet, uint8_t segNum, uint16_t num),
        getPixelNumFromLineNum(SegmentSet &segmentSet, uint16_t maxSegLength, uint8_t segNum, uint16_t lineNum);
        
    CRGB
        getPixelColor(SegmentSet &segmentSet, uint16_t pixelNum, CRGB color, uint8_t colorMode, uint8_t segNum, uint16_t lineNum);
    
    //pre-allocated space for function variables
    //Since these functions are all called a lot, it reduce call times
    //While the memory cost is small
    static unsigned long
        currentTime;

    static uint8_t
        ratio,
        numSegs,
        numSec;

    static uint16_t
        locData[2],
        startData[2],
        endData[2],
        lineNum,
        count,
        prevCount,
        lengthSoFar,
        maxSegLength,
        pixelNum,
        secStartPixel,
        colorModeDom,
        colorModeNum,
        offsetMax,
        absSecLength,
        startLimit;
    
    static int8_t
        step,
        secLengthSign;

    static int16_t
        endLimit,
        secLength;
    
    static bool
        segDirection;
    
    static CRGB
        colorFinal;

};

#endif
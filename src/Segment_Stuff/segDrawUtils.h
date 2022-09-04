#ifndef segDrawUtils_h
#define segDrawUtils_h

//TODO:
//-- Consolidate vars (sorta done, unless you want to make things hard to read)
//-- Add shortcuts for situations with one segment in a set?
//-- Add an "add glitter" function to show(), would be based on setting in segment set
//   Could add it as its own effect instead?

#include "FastLED.h"
#include "segmentSection.h"
#include "segment.h"
#include "segmentSet.h"
#include "pixelInfoPS.h"
#include "ColorUtils/colorUtilsPS.h"
#include "PaletteFiles.h"
#include "MathUtils/mathUtilsPS.h"

//dLed is used to indicate a dummy led (max of uint16_t)
//Pixels with this value will be ignored by color setting functions
//This is critical for handling cases where you try to find the address of a pixel
//that doesn't exist in the segment set
//ex for a segment set with 300 pixels, trying to get the address of the 301st
//We must be able to return a location that is impossible for all segment sets
//(we can't just return 301 because that could be an actual address in the segment set)
//Hence we use the constant dLed as a global impossible address marker
//This limits the maximum segment set length to 65434, but that's still waaay more pixels than any current MC can handle
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
        fillSegColor(SegmentSet &segmentSet, uint16_t segNum, CRGB color, uint8_t colorMode),
        fillSegSecColor(SegmentSet &segmentSet, uint16_t segNum, uint16_t secNum, uint16_t pixelCount, CRGB color, uint8_t colorMode ),
        fillSegLengthColor(SegmentSet &segmentSet, CRGB color, uint8_t colorMode, uint16_t segNum, uint16_t startSegPixel, uint16_t endPixel),
        fillSegSetlengthColor(SegmentSet &segmentSet, CRGB color, uint8_t colorMode, uint16_t startSegPixel, uint16_t endPixel),
        drawSegLine(SegmentSet &segmentSet, uint16_t lineNum, uint8_t Pattern[], CRGB palette[], uint8_t colorMode, uint8_t bgColorMode, bool brReplace),
        drawSegLineSection(SegmentSet &segmentSet, uint16_t startSeg, uint16_t endseg, uint16_t lineNum, uint8_t Pattern[], CRGB palette[], uint8_t colorMode, uint8_t bgColorMode, bool brReplace),
        drawSegLineSimple(SegmentSet &segmentSet, uint16_t lineNum, CRGB color, uint8_t colorMode),
        drawSegLineSimpleSection(SegmentSet &segmentSet, uint16_t startSeg, uint16_t endSeg, uint16_t lineNum, CRGB color, uint8_t colorMode),
        setPixelColor(SegmentSet &segmentSet, uint16_t segPixelNum, CRGB color, uint8_t colorMode),
        setPixelColor(SegmentSet &segmentSet, uint16_t segPixelNum, uint16_t segNum, CRGB color, uint8_t colorMode),
        setPixelColor(SegmentSet &segmentSet, uint16_t pixelNum, CRGB color, uint8_t colorMode, uint16_t segNum, uint16_t lineNum),
        handleBri(SegmentSet &segmentSet, uint16_t pixelNum),
        show(SegmentSet &segmentSet, bool showNow),
        getPixelColor(SegmentSet &segmentSet, pixelInfoPS *pixelInfo, CRGB color, uint8_t colorMode, uint16_t segPixelNum),
        setGradOffset(SegmentSet &segmentSet, uint16_t offsetMax),
        fadeSegSetToBlackBy(SegmentSet &segmentSet, uint8_t val),
        fadeSegToBlackBy(SegmentSet &segmentSet, uint16_t segNum, uint8_t val),
        fadeSegSecToBlackBy(SegmentSet &segmentSet, uint16_t segNum, uint16_t secNum, uint8_t val);
    
    uint8_t 
        getLineNumFromPixelNum(SegmentSet &segmentSet, uint16_t segPixelNum),
        getLineNumFromPixelNum(SegmentSet &segmentSet, uint16_t segPixelNum, uint16_t segNum);

    uint16_t
        getSegmentPixel(SegmentSet &segmentSet, uint16_t segPixelNum),
        getSegmentPixel(SegmentSet &segmentSet, uint16_t segNum, uint16_t num),
        getPixelNumFromLineNum(SegmentSet &segmentSet, uint16_t maxSegLength, uint16_t segNum, uint16_t lineNum);
        
    CRGB
        getPixelColor(SegmentSet &segmentSet, uint16_t pixelNum, CRGB color, uint8_t colorMode, uint16_t segNum, uint16_t lineNum);
    
    //pre-allocated space for function variables
    //Since these functions are all called a lot, it reduce call times
    //While the memory cost is small
    static unsigned long
        currentTime;

    static uint8_t
        numSec;

    static uint16_t
        locData1[2],
        locData2[2],
        lineNum,
        lengthSoFar,
        pixelNum,
        secStartPixel,
        colorModeDom,
        colorModeNum,
        offsetMax,
        startLimit;
    
    //Don't change these! You need all three to prevent overwrites.
    static int8_t
        step,
        stepDir,
        secLengthSign;

    static int16_t
        endLimit,
        secLength;
    
    static bool
        segDirection,
        hasContSec;
    
    static CRGB
        colorFinal;

};

#endif
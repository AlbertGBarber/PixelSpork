#ifndef segDrawUtils_h
#define segDrawUtils_h

//TODO:
//-- Consolidate vars (sorta done, unless you want to make things hard to read)
//-- Add shortcuts for situations with one segment in a set?

#include "FastLED.h"
#include "./Include_Lists/GlobalVars/GlobalVars.h"
#include "segmentSections.h"
#include "Segment.h"
#include "SegmentSet.h"
#include "pixelInfoPS.h"
#include "ColorUtils/colorUtilsPS.h"
#include "Include_Lists/PaletteFiles.h"
#include "MathUtils/mathUtilsPS.h"

//utility functions for finding the physical pixel number(i.e. it's strip location) of segment pixels and for coloring pixels
//!!YOU SHOULD ALWAYS USE THESE FUNCTIONS TO DRAW ON SEGMENTS
//!!DO NOT TRY DRAWING DIRECTLY UNLESS YOU 100% KNOW WHAT YOU ARE DOING
//The functions are split into groups based on their purpose below
//Check the function comments in the .cpp file for info for each function
namespace segDrawUtils{

    void //Functions for filling segments or parts of segments
        turnSegSetOff(SegmentSet &SegSet),
        fillSegSetColor(SegmentSet &SegSet, const CRGB &color, uint8_t colorMode),
        fillSegColor(SegmentSet &SegSet, uint16_t segNum, const CRGB &color, uint8_t colorMode),
        fillSegSecColor(SegmentSet &SegSet, uint16_t segNum, uint16_t secNum, uint16_t pixelCount, const CRGB &color, uint8_t colorMode ),
        fillSegLengthColor(SegmentSet &SegSet, uint16_t segNum, uint16_t startSegPixel, uint16_t endPixel, const CRGB &color, uint8_t colorMode ),
        fillSegSetLengthColor(SegmentSet &SegSet, uint16_t startSegPixel, uint16_t endPixel, const CRGB &color, uint8_t colorMode );
   
    void //Functions for drawing segment lines
        drawSegLine(SegmentSet &SegSet, uint16_t lineNum, const CRGB &color, uint8_t colorMode),
        drawSegLineSection(SegmentSet &SegSet, uint16_t startSeg, uint16_t endSeg, uint16_t lineNum, const CRGB &color, uint8_t colorMode);
        
    void //Functions for setting the color of single pixels, for different levels of knowledge about where your pixel is
        setPixelColor(SegmentSet &SegSet, uint16_t segPixelNum, const CRGB &color, uint8_t colorMode),
        setPixelColor(SegmentSet &SegSet, uint16_t segPixelNum, const CRGB &color, uint8_t colorMode, uint16_t segNum),
        setPixelColor(SegmentSet &SegSet, uint16_t pixelNum, const CRGB &color, uint8_t colorMode, uint16_t segNum, uint16_t lineNum),
        setGradOffset(SegmentSet &SegSet, uint16_t offsetMax); //Handles updating the rainbow offset vals in the segment set.

    void //Functions for getting the color that a pixel will be based on the color mode.
        getPixelColor(SegmentSet &SegSet, uint16_t segPixelNum, pixelInfoPS *pixelInfo, const CRGB &color, uint8_t colorMode);
    CRGB
        getPixelColor(SegmentSet &SegSet, uint16_t pixelNum, const CRGB &color, uint8_t colorMode, uint16_t segNum, uint16_t lineNum);
    
    void //Functions to do with displaying the pixels
        handleBri(SegmentSet &SegSet, uint16_t pixelNum),
        show(SegmentSet &SegSet, bool showNow);

    void //Functions for dimming the segment set, or parts of it
        fadeSegSetToBlackBy(SegmentSet &SegSet, uint8_t val),
        fadeSegToBlackBy(SegmentSet &SegSet, uint16_t segNum, uint8_t val),
        fadeSegSecToBlackBy(SegmentSet &SegSet, uint16_t segNum, uint16_t secNum, uint8_t val);

    
    void //Functions for finding what segment or line number your pixel is on the segment set 
        getSegLocationFromPixel(SegmentSet &SegSet, uint16_t segPixelNum, uint16_t locData[2] );
    uint16_t 
        getLineNumFromPixelNum(SegmentSet &SegSet, uint16_t segPixelNum),
        getLineNumFromPixelNum(SegmentSet &SegSet, uint16_t segPixelNum, uint16_t segNum);

    //Functions for finding where the pixel is on the segment set, for different levels of knowledge about where your pixel is.
    uint16_t
        getSegmentPixel(SegmentSet &SegSet, uint16_t segPixelNum),
        getSegmentPixel(SegmentSet &SegSet, uint16_t segNum, uint16_t num),
        getPixelNumFromLineNum(SegmentSet &SegSet, uint16_t numLines, uint16_t segNum, uint16_t lineNum);
    
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
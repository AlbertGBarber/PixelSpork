#ifndef PatternShifterSeg_h
#define PatternShifterSeg_h

//TODO -- Make an effect that moves the pattern both along segment and segment lines at the same time
//        Shouldn't be too much work, you'll need two cycleNums, one for lines and segments, and then to take the 
//        line shifting code from PatternShifterSL.
//        Will also need to turn off the last segment and line, and have separate update rates for segments and lines
//        (maybe make it so the rates are additions to the base rate ie lineRate = rate + x)

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "Effects/PatternShifterSL/shiftPatternPS.h"
#include "MathUtils/mathUtilsPS.h"

/*
An effect used to move shiftPatterns along segments
shiftPatterns are manually created, and are used to represent custom display patterns
like arrows, text?, flags, etc
For a full explanation, see shiftPatternPS.h

This effect moves shiftPatterns from one segment to another, wrapping at the segment set end
Ie if you had a pattern where and single segment was lit up, 
it would be shifted from one segment to the next as the effect runs.
(This doesn't change the pattern, all the movement is local to the effect)

You can set the direction of the motion (first to last segment) or visa versa

This effect is really only useful for segment set with multiple segments
Ie 2D shapes

The shiftPattern can be set to repeat both along segment lines and segments,
which will draw the pattern as many times as possible along the segment set
For ie, for a segment set with 10 lines and 8 segments:
    If you repeat along segment lines, a pattern that is 5 lines (number of "rows") 
       long will be draw twice (10/5) (offset so the patterns don't overlap)
    If you repeat along segments, a pattern that draws on 2 segments will be drawn four times (8/2)
The effect tries to fit as many patterns as possible into the segment set, 
so repeating works best if you can fit an even number of patterns onto the segment set

Patterns can be longer (have more segments) than the segment set, the pattern will cycle across as the effect runs
However because the line values of the pattern don't shift, the pattern must fit onto the segment's lines,
anything extra won't be drawn.

Note that shiftPatterns generally only work with a specific segment set
!!Make sure you use the correct segment set when setting up the effect!!

The bgColor will be draw on any pattern indexes with the value 255.
The bgColor is a pointer, so you can bind it to an external color variable.

This effect is fully compatible with colorModes.

!!If you want to adjust the shiftPattern while the effect is running, be very careful not
to change how many segments the pattern takes up (numSegs in the shiftPattern struct)
Changing the color index's is fine, but it's probably easier to change the palette colors instead.

Example calls: 
    Note that I cannot provide a universal shiftPattern for all segmentSets
    See shiftPatternPS.h for how to create one.

    A super basic pattern for a segment set with a single segment (one strip)
    Two dots which will blink on and off (they are cycling on and off the segment)
    This isn't a great display of the effect, since it really doesn't work with just a single segment

    uint16_t patternSegs = mainSegments.numSegs;

    uint16_t basicPattern_arr[(2 + 2) * 2] = { <--(<<how long each "row"is>>) * <<number of rows>>
        0, 1,   0, 255,  //segment line 0, dot 1, then space
        1, 2,   255, 0, //segment line 1, space, then dot 2
    };
    shiftPatternPS basicPattern(basicPattern_arr, SIZE(basicPattern_arr), patternSegs);

    PatternShifterSeg(mainSegments, basicPattern, palette1, 0, false, false, true, 100);
    Will shift the "basicPattern" across the segment set using colors from palette1
    The background is blank
    The pattern is not repeated across segment lines or segments
    The segment shift direction is true (from first to last segment)
    The effect updates at 100ms

Constructor inputs: 
    shiftPattern -- The input shiftPattern (see shiftPatternPS.h for info), 
                    make sure the pattern's segment set matches the effect's!!!
                    You can change the shiftPattern later using setShiftPattern()
    palette -- The colors used to color the pattern
    bgColor -- The color of the background, specified by 255 in the shiftPattern
    repeatLine -- Sets if the pattern "rows" are to be repeated along the segment lines
    repeatSeg -- Sets if the segment part of the pattern "rows" are to be repeated along the segments
    direct -- Sets the direction the pattern moves along the segments, true is from the first to last segment
    Rate -- The update rate (ms) note that this is synced with all the particles.

Functions:
    setShiftPattern(shiftPatternPS *newShiftPattern) -- Sets the effect shiftPattern, will restart the effect
    setRepeat(newRepeatSeg, newRepeatLine) -- Sets if the shiftPattern will be repeated along the segment set lines or/and segments
    reset() -- Restarts the effect
    update() -- updates the effect 

Other Settings:
    colorMode (default 0) -- sets the color mode for the pattern pixels (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the background pixels (see segDrawUtils::setPixelColor)

Reference Vars:
    cycleNum -- How many times the pattern has be shifted,
                resets every time the pattern has been cycled across the whole segment set
    repeatLine -- (see notes above), set using setRepeat()
    repeatSeg -- (see notes above), set using setRepeat()

*/
class PatternShifterSeg : public EffectBasePS {
    public:
        PatternShifterSeg(SegmentSet &SegSet, shiftPatternPS &ShiftPattern, palettePS &Palette, CRGB BgColor, 
                          bool RepeatLine, bool RepeatSeg, bool Direct, uint16_t Rate);  

        SegmentSet 
            &SegSet; 
        
        uint8_t
            colorMode = 0,
            bgColorMode = 0;
        
        uint16_t
            cycleNum = 0; //for reference only
        
        bool
            repeatLine, //for reference only, set using setRepeat()
            repeatSeg, //for reference only, set using setRepeat()
            direct; //true is first to last segment
        
        CRGB 
            bgColorOrig,
            *bgColor = nullptr; //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)
        
        shiftPatternPS
            *shiftPattern = nullptr;
        
        palettePS
            *palette = nullptr;

        void 
            setShiftPattern(shiftPatternPS &newShiftPattern),
            setRepeat(bool newRepeatSeg, bool newRepeatLine),
            reset(),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;

        int8_t
            directStep; //1 or -1
        
        uint8_t
            modeOut,
            colorIndex;

        uint16_t 
            numPatSegs,
            numLines,
            numSegs,
            pixelNum,
            numPatRows,
            patternRow,
            patLineLength,
            repeatLineNum,
            modVal,
            rowStartIndex,
            startLine,
            endLine,
            prevSeg = 65535,
            segNum,
            repeatLineCount = 1,
            repeatSegCount = 1;
        
        CRGB 
            colorOut;
};

#endif
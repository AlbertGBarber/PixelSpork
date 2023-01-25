#ifndef PatternShifterSL_h
#define PatternShifterSL_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "shiftPatternPS.h"
#include "MathUtils/mathUtilsPS.h"

//TODO -- add "bounce" ability? Could just reverse direction of cycleNum at each end
//        If you want to flip the pattern, you'll need to flip the start/end lines -> add/subtract the patternLineLen?

/*
An effect used to move shiftPatterns along segment lines
shiftPatterns are manually created, and are used to represent custom display patterns
like arrows, text?, flags, etc
For a full explanation, see shiftPatternPS.h

This effect moves shiftPatterns from one segment line to another, wrapping at the segment line end
Ie if you had a pattern where and single segment line was lit up, 
it would be shifted from one segment line to the next as the effect runs.
(This doesn't change the pattern, all the movement is local to the effect)

This effect is really only usefull for segment set with multiple segments
Ie 2D shapes

The shiftPattern can be set to repeat, which will draw the pattern as many times as possible along the segment set
For ie, for a segment set with 10 lines:
    If you repeat along segment lines, a pattern that is 5 lines long will be draw twice (10/5) (offset so the patterns don't overlap)
Repeating works best if you can fit an even number of patterns onto the segment set

Patterns can be longer (have more segment lines) than the segment set, the pattern will cycle across as the effect runs
Your pattern color "rows" must be the same length as the number of segments in the segment set
ie for a set with 5 segments, your pattern "rows" should have a color for each segment.

Note that shiftPatterns generally only work with a specific segment set
!!Make sure you use the correct segment set when setting up the effect!!

The bgColor will be draw on any pattern indexes with the value 255.
The bgColor is a pointer, so you can bind it to an external color variable.

This effect is fully compatible with colorModes.

!!If you want to adjust the shiftPattern while the effect is running, be very careful not
to change how many segment lines the pattern takes up (patLineLength in the shiftPattern struct)
Changing the color index's is fine, but it's probably easier to change the palette colors instead.

Example calls: 
    Note that I cannot provide a universal shiftPattern for all segmentSets
    See shiftPatternPS.h for how to create one.

    A super basic pattern for a segment set with a single segment (one strip)
    This pattern is just two dots seperated by a space.

    uint16_t patternSegs = mainSegments.numSegs;

    uint16_t basicPattern_arr[(1 + 2) * 3] = { <--(<<how long each "row" is>>) * <<number of rows>>
        0, 1,   0,   //segment line 0, dot 1
        1, 2,   255, //segment line 1, space
        2, 3,   0,   //segment line 2, dot 2
    };
    shiftPatternPS basicPattern(basicPattern_arr, SIZE(basicPattern_arr), patternSegs);

    PatternShifterSL(mainSegments, &basicPattern, &palette1, 0, false, 100);
    Will shift the "basicPattern" across the segment set using colors from palette1
    The background is blank
    The pattern is not repeated
    The effect updates at 100ms

Constructor inputs: 
    shiftPattern -- The input shiftPattern (see shiftPatternPS.h for info), 
                    make sure the pattern's segment set matches the effect's!!!
                    You can change the shiftPattern later using setShiftPattern()
    palette -- The colors used to color the pattern
    BgColor -- The color of the background, specified by 255 in the shiftPattern
    repeat -- If the pattern is to be repeated along the segment set
    Rate -- The update rate (ms) note that this is synced with all the particles.

Functions:
    setShiftPattern(shiftPatternPS *newShiftPattern) -- Sets the effect shiftPattern, will restart the effect
    setRepeat(bool newRepeat) -- Sets if the shiftPattern will be repeated along the segment set
    reset() -- Restarts the effect
    update() -- updates the effect 

Reference Vars:
    cycleNum -- How many times the pattern has be shifted,
                resets every time the pattern has been cycled across the whole segment set
    repeat -- (see notes above), set using setRepeat(bool newRepeat)

Other Settings:
    colorMode (default 0) -- sets the color mode for the pattern pixels (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the background pixels (see segDrawUtils::setPixelColor)

*/

class PatternShifterSL : public EffectBasePS {
    public:
        PatternShifterSL(SegmentSet &SegmentSet, shiftPatternPS *ShiftPattern, palettePS *Palette, CRGB BgColor, bool Repeat, uint16_t Rate);  

        SegmentSet 
            &segmentSet; 
        
        uint8_t
            colorMode = 0,
            bgColorMode = 0;
        
        uint16_t
            cycleNum = 0; //for reference only
        
        bool
            repeat; //for reference only, set using setRepeat()
        
        CRGB 
            bgColorOrig,
            *bgColor = nullptr; //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)
        
        shiftPatternPS
            *shiftPattern = nullptr;
        
        palettePS
            *palette = nullptr;

        void 
            setShiftPattern(shiftPatternPS *newShiftPattern),
            setRepeat(bool newRepeat),
            reset(),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        uint8_t
            modeOut,
            colorIndex;

        uint16_t 
            numPatSegs,
            numLines,
            pixelNum,
            numPatRows,
            patternRow,
            patLineLength,
            modVal,
            rowStartIndex,
            startLine,
            endLine,
            crntRepeatNum,
            prevLine = 65535,
            repeatCount = 1;
        
        CRGB 
            colorOut;
};

#endif
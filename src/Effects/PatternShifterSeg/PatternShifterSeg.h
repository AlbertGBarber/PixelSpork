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
NOTE: You must understand ShiftPatterns to use this effect (see shiftPatternPS.h or the "Shift Pattern''s wiki page).
      This effect is really only useful for segment set with multiple segments (2D).

An effect that moves a shiftPatternPS along SEGMENTS. 
Shift patterns are manually created, and are used to draw more complex patterns like arrows, text?, flags, etc. 
Also see PatternShifterSL.h for an alternate, orthogonal, version of this effect. 
For a full explanation of shift patterns, see shiftPatternPS.h.

This effect moves a shift pattern from one SEGMENT to another, 
wrapping at the segment set end. Ie if you had a pattern where a single segment was lit up,
it would be shifted from one segment to the next as the effect runs. 
(This doesn't change the pattern, all the movement is local to the effect). 
You can set the direction of the motion (first to last segment) or visa versa. 
For this effect, patterns can have more segments than the segment set 
(ie the shift pattern rows can be longer than `numSegs`). 
The pattern will cycle across the segments as the effect runs. 
However, because the pattern is not shifted across segment lines,
 the pattern cannot be longer than `numLines`, anything extra won't be drawn.

For example, if I have an 4x4 matrix, arranged into 4 segment rows, and use the following pattern:

uint16_t basicPattern_arr[(6 + 2) * 3] = { //<--(<<how long each "row"is>>) * <<number of rows>>
    0, 2,    0, 255, 0, 255, 0, 0
    2, 5,   255, 0, 255, 0,  0, 0
    5, 5,    0,  0,  0,  0,  0, 0 // <-- This row WON't be drawn b/c we only have 4 segment lines, but it wants to be drawn on the 5th line.
//                          [-^^-]   <-- These columns will be drawn despite our matrix only having 4 segments because 
//                                        the effect shifts the pattern across segments.
};

The line number of the final row is line 5, but our segment set only has 4 lines (4 matrix columns). 
The extra 5th line will never be draw because the effect doesn't shift the pattern across segment lines. 
However, note that the pattern in each row (excluding the first 2 line markers) is length 6, 
with an alternating pattern. Although this is longer than our 4 matrix segments, 
the full row will be drawn eventually because the effect shifts the pattern across segments. 
So the final two columns will be cycled on/off as the pattern moves. 

The bgColor will be draw on any pattern indexes with the value 255.
The bgColor is a pointer, so you can bind it to an external color variable.

This effect is fully compatible with colorModes.

!!While you can change the values of a pattern during runtime, you should avoid changing the row/column dimensions.
You can change the pattern using the `setShiftPattern()` function. 

    Repeating Patterns:
        The shift pattern can be set to repeat, segment lines, segments, or both,
        which will draw the pattern as many times as possible along the segment set,
        so that the pattern seamlessly loops. 
        For example, for a segment set with 10 lines and 8 segments:
            If you repeat along segment lines, a pattern that is 5 lines (number of "rows") 
            long will be draw twice (10/5) (offset so the patterns don't overlap)
            If you repeat along segments, a pattern that draws on 2 segments will be drawn four times (8/2)
        
        You can change the repeating using the `setRepeat( bool newRepeatSeg, bool newRepeatLine )` function.

        Note that when repeating across segment lines, the pattern must fill evenly into the lines, 
        otherwise part of the final repeat will be partially cut-off. 
        For example, if we have a segment set with 10 lines, but our pattern is 3 lines long, 
        the pattern will be repeated 4 times (10 / 3, rounding up), so that the entire segment set is filled. 
        However, the total number of lines taken up by our repeated pattern is 12 (3 * 4), 
        so the last 2 lines of the final repeat will be cut-off (12 -10). 
        For the pattern to fit evenly, it would have to be either 2 or 5 lines long. 
        This is not a problem when repeating across segments, because the pattern is shifted across them, 
        so any cut-off parts will be cycled on/off.

    The Effect's `segSet` pointer:
        Note that the effect has a `segSet` pointer like other effects, 
        but the effect doesn't take a segment set as an input. 
        Instead it gets its `segSet` from the shift pattern (see shiftPatternPS.h for more detail). 
        In other words, the effect's `segSet` pointer is bound to the shift pattern's `segSet` pointer. 
        (Usually a shift pattern is intended to work with a specific segment set, 
        so it made sense to include the segment set in the pattern itself). 
        This info is only relevant if you plan on changing the segment set. 
        If you do so, you should change the shift pattern's `segSet` rather than the effect's.

Example calls: 
    Note that I cannot provide a universal shiftPattern for all segmentSets
    See shiftPatternPS.h for how to create one.

    A super basic pattern for a segment set with a single segment (one strip)
    Two dots which will blink on and off (they are cycling on and off the segment)
    This isn't a great display of the effect, since it really doesn't work with just a single segment

    uint16_t patternSegs = mainSegments.numSegs;

    uint16_t basicPattern_arr[(2 + 2) * 2] = { //<--(<<how long each "row"is>>) * <<number of rows>>
        0, 1,   0, 255,  //segment line 0, dot 1, then space
        1, 2,   255, 0, //segment line 1, space, then dot 2
    };
    shiftPatternPS basicPattern(mainSegments, patternSegs, basicPattern_arr, SIZE(basicPattern_arr));

    PatternShifterSeg patternShifterSeg(basicPattern, cybPnkPal_PS, 0, false, false, true, 100);
    Will shift the "basicPattern" across its segment set using colors from the cybPnkPal_PS palette
    The background is blank
    The pattern is not repeated across segment lines or segments
    The segment shift direction is true (from first to last segment)
    The effect updates at 100ms

Constructor inputs: 
    shiftPattern -- The input shiftPattern (see shiftPatternPS.h for info), 
                    You can change the shiftPattern later using setShiftPattern()
    palette -- The colors used to color the pattern
    bgColor -- The color of the background, specified by 255 in the shiftPattern
    repeatLine -- Sets if the pattern "rows" are to be repeated along the segment lines
    repeatSeg -- Sets if the segment part of the pattern "rows" are to be repeated along the segments
    direct -- Sets the direction the pattern moves along the segments, true is from the first to last segment
    Rate -- The update rate (ms) note that this is synced with all the particles.

Other Settings:
    colorMode (default 0) -- sets the color mode for the pattern pixels (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the background pixels (see segDrawUtils::setPixelColor)

Functions:
    setShiftPattern(shiftPatternPS *newShiftPattern) -- Sets the effect's shiftPattern.
    setRepeat(newRepeatSeg, newRepeatLine) -- Sets if the shiftPattern will be repeated along the segment set lines or/and segments
    reset() -- Restarts the effect
    update() -- updates the effect 
    
Reference Vars:
    cycleNum -- How many times the pattern has be shifted,
                resets every time the pattern has been cycled across the whole segment set
    repeatLine -- (see notes above), set using setRepeat()
    repeatSeg -- (see notes above), set using setRepeat()
    shiftPattern (sorta) -- While you can change the values of a pattern, you should avoid changing the row/column dimensions. 
                            Use setShiftPattern() to change to a new pattern.

*/
class PatternShifterSeg : public EffectBasePS {
    public:
        PatternShifterSeg(shiftPatternPS &ShiftPattern, palettePS &Palette, CRGB BgColor,
                          bool RepeatLine, bool RepeatSeg, bool Direct, uint16_t Rate);

        uint8_t
            colorMode = 0,
            bgColorMode = 0;

        uint16_t
            cycleNum = 0;  //for reference only

        bool
            repeatLine,  //for reference only, set using setRepeat()
            repeatSeg,   //for reference only, set using setRepeat()
            direct;      //true is first to last segment

        CRGB
            bgColorOrig,
            *bgColor = nullptr;  //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)

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
            directStep;  //1 or -1

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
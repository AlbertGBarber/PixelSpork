#include "PatternShifterSeg.h"

PatternShifterSeg::PatternShifterSeg(shiftPatternPS &ShiftPattern, palettePS &Palette, CRGB BgColor,
                                     bool RepeatLine, bool RepeatSeg, bool Direct, uint16_t Rate)
    : palette(&Palette), repeatLine(RepeatLine), repeatSeg(RepeatSeg), direct(Direct)  //
{
    //bind the rate and segSet pointer vars since they are inherited from BaseEffectPS
    //(we bind the segSet pointer in setShiftPattern b/c its part of shiftPatterns)
    bindClassRatesPS();
    //bind background color pointer
    bindBGColorPS();

    setShiftPattern(ShiftPattern);
    reset();
}

//resets the effect to restart it
void PatternShifterSeg::reset() {
    cycleNum = 0;
}

//Sets the pattern to be the passed in pattern, also sets up various effect variables
//If you change the pattern you must do it via this function
void PatternShifterSeg::setShiftPattern(shiftPatternPS &newShiftPattern) {

    shiftPattern = &newShiftPattern;

    //bind the shift pattern's segment set pointer to the effect's segSet pointer
    //(using the effect's segSet pointer is important because it makes sure the effect behaves like other effects)
    segSet = shiftPattern->segSet;

    //number of lines and segs in the shiftPattern's segment set
    numLines = segSet->numLines;
    numSegs = segSet->numSegs;

    //number of "rows" in the pattern
    numPatRows = shiftPattern->numRows;

    //how many segment lines the pattern takes up
    patLineLength = shiftPattern->patLineLength;

    //If the pattern is longer than the number of lines, we don't to try to draw any lines that are off the segment set
    //So we cap the number of lines
    //(it's fine if the pattern is shorter than the number of lines)
    if( patLineLength > numLines ) {
        patLineLength = numLines;
    }

    //Set the wrapping point for the pattern
    //If the pattern is longer than the number of segments we need to increase the mod amount
    //so that the whole pattern is cycled across the segments
    numPatSegs = shiftPattern->patRowLength;
    if( numPatSegs > numSegs ) {
        modVal = numPatSegs;
    } else {
        modVal = numSegs;
    }

    setRepeat(repeatSeg, repeatLine);
}

//Sets the repeat value of the effect and calculates how many repeats will fit on the segment set (rounded up)
void PatternShifterSeg::setRepeat(bool newRepeatSeg, bool newRepeatLine) {
    repeatSeg = newRepeatSeg;
    repeatLine = newRepeatLine;

    numLines = segSet->numLines;
    numSegs = segSet->numSegs;

    //how many segment lines the pattern takes up
    patLineLength = shiftPattern->patLineLength;

    //Calculate how many pattern line repeats will fit on the segment set !!lines!!
    if( repeatLine ) {
        repeatLineCount = ceil((float)numLines / patLineLength);
    } else {
        repeatLineCount = 1;  //One pattern will always be drawn, even with 0 repeats
    }

    //Calculate how many pattern segment repeats will fit on the segment set !!segments!!
    if( repeatSeg ) {
        repeatSegCount = ceil((float)numSegs / numPatSegs);
    } else {
        repeatSegCount = 1;  //One pattern will always be drawn, even with 0 repeats
    }
}

/* Updates the effect
For info on shiftPatterns, see shiftPatternPS.h
The goal of the effect is to shift the segment section of the pattern by one every update cycle
Ie so the pattern shifts along the segments. 
To do this we draw the pattern segment line by segment line, segment by segment pixel
To move the pattern, we offset the segments we're drawing to by a cycleNum (which increments each cycle)
So the pattern for segment 0 may actually be drawn on segment 1 etc.
The pattern is allowed to repeat across segment lines and segments, to cover the whole segment set
(the number of repeats is worked out in setRepeat())
If we're repeating, then we draw the pattern "rows" repeatLineCount number of times,
and the segment pattern repeatSegCount number of times, 
with each new pattern being offset by the patLineLength (how many segment lines the pattern covers),
or the numPatSegs (how many segments the pattern takes up)
so that the patterns are placed directly one after another, filling the whole segment set.
Note that since we never move the pattern's segment lines, the pattern must fit into the segment set, anything extra is cut off
On the other hand, the length of the segment pattern can be longer than the number of segments in the segment set,
with any extra parts being cycled on as the pattern moves. */
void PatternShifterSeg::update() {
    currentTime = millis();

    if( (currentTime - prevTime) >= *rate ) {
        prevTime = currentTime;

        numSegs = segSet->numSegs;

        //We use this to shift the segment pattern forward or backward based on the direction
        //It resolves to either 1 or -1 depending on direct.
        directStep = direct - !direct;

        //If we're not repeating, we need to turn off the last segment
        //that the pattern was on for the previous cycle (as long as it's in the segment set)
        if( !repeatSeg && prevSeg < numSegs ) {
            segDrawUtils::fillSegColor(*segSet, prevSeg, *bgColor, bgColorMode);
        }

        //To draw the pattern, we run over each pattern "row", getting the start and end lines,
        //and then filling in each segment according to the "row" in shiftPattern array
        //We repeat this process repeatLineCount number of times (min of 1), offsetting where the lines are each time
        for( uint16_t i = 0; i < numPatRows * repeatLineCount; i++ ) {

            //The current pattern row, adjusted for repeat number
            patternRow = mod16PS(i, numPatRows);

            //The number of the repeat we're on, ie the "second repeat out of four"
            repeatLineNum = i / numPatRows;

            //Get the starting index for the current pattern "row"
            rowStartIndex = shiftPattern->getPatRowStartIndex(patternRow);
            //Get the pattern "row's" starting line, and then offset it by the cycleNum and which repeat we're on, wrapping as needed
            //This moves the pattern over time
            //We also do the same to get the end line
            startLine = shiftPattern->getLineStartOrEnd(patternRow, false);
            startLine = startLine + patLineLength * repeatLineNum;

            endLine = shiftPattern->getLineStartOrEnd(patternRow, true);
            endLine = endLine + patLineLength * repeatLineNum;

            //Draw the pattern row between the start and end lines
            //We draw each line segment by segment, getting each segment pixel color individually
            //Note that because the pattern may be wrapping, the endLine may actually be before the start line
            //But because shiftPatterns are continuous, if we also wrap as we increment j, we know that
            //we will eventually reach the end line
            for( uint16_t j = startLine; j != endLine; j++ ) {
                //If the current line falls outside of the segment set, we need to avoid drawing it
                //so we skip its loop iteration
                if( j >= numLines ) {
                    continue;
                }

                //Color in the pixels for each segment in the current pattern row
                //For each segment we get the colorIndex from the pattern, the output the color
                //Note that the output segments are shifted according to the cycleNum and direction
                //We repeat this process repeatSegCount number of times
                for( uint16_t k = 0; k < numPatSegs * repeatSegCount; k++ ) {

                    //Get the actual output segment, which is shifted by the cycleNum either backwards or forwards depending on the direction
                    //Note that to keep cycleNum * directStep positive, we add modVal to it.
                    //(This doesn't change the cycle motion, )
                    segNum = addMod16PS(k, cycleNum * directStep + modVal, modVal);

                    //For longer shiftPatterns, parts may fall outside the segment set
                    //we want to avoid drawing these, so we skip them
                    if( segNum >= numSegs ) {
                        continue;
                    }

                    colorIndex = mod16PS(k, numPatSegs);
                    //Get the color index of the segment pixel in the pattern
                    colorIndex = shiftPattern->getLineColorIndexQuick(rowStartIndex, colorIndex);
                    //In shiftPatterns, 255 indicates a background
                    //so we need to check for this before outputting
                    if( colorIndex == 255 ) {
                        colorOut = *bgColor;
                        modeOut = bgColorMode;
                    } else {
                        colorOut = paletteUtilsPS::getPaletteColor(*palette, colorIndex);
                        modeOut = colorMode;
                    }
                    //get the physical pixel location and output the color and colorMode
                    pixelNum = segDrawUtils::getPixelNumFromLineNum(*segSet, numLines, segNum, j);
                    segDrawUtils::setPixelColor(*segSet, pixelNum, colorOut, modeOut, segNum, j);
                }
            }
        }

        //Set prevSeg, which is the segment that must be turned off if we're not repeating the shiftPattern
        //The prevSeg is just the current location of the first/last segment in the shiftPattern,
        //(first seg if direct is true, last seg if direct is false)
        //So we just need to work out where the first or last segment is based on the cycleNum
        if( direct ) {
            //When moving forward, the segment we need to turn off is always the first segment, shifted by the cycleNum
            //(Since shiftPattern always have the segments in order in each pattern "row", so we always have a colored first segment)
            //This just happens to be cycleNum, since all the segments in the pattern are all shifted by cycleNum
            prevSeg = cycleNum;
        } else {
            //When moving backwards, the segment we need to turn off is always the last segment in the pattern "row",
            //but we need to shift the segment by cycleNum, including wrapping
            //Note that to keep cycleNum * directStep positive, we add modVal to it.
            prevSeg = addMod16PS(numPatSegs - 1, cycleNum * directStep + modVal, modVal);
        }

        //Increment the cycleNum to shift the pattern forward in the next update cycle
        cycleNum = addMod16PS(cycleNum, 1, modVal);
        showCheckPS();
    }
}
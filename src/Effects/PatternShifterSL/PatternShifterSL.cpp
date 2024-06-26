#include "PatternShifterSL.h"

PatternShifterSL::PatternShifterSL(shiftPatternPS &ShiftPattern, palettePS &Palette, CRGB BgColor, bool Repeat, uint16_t Rate)
    : palette(&Palette), repeat(Repeat)  //
{
    //bind the rate vars since they are inherited from BaseEffectPS
    //(we bind the segSet pointer in setShiftPattern b/c its part of shiftPatterns)
    bindClassRatesPS();
    //bind background color pointer
    bindBGColorPS();

    setShiftPattern(ShiftPattern);
    reset();
}

//resets the effect to restart it
void PatternShifterSL::reset() {
    cycleNum = 0;
}

//Sets the pattern to be the passed in pattern, also sets up various effect variables
//If you change the pattern you must do it via this function
void PatternShifterSL::setShiftPattern(shiftPatternPS &newShiftPattern) {
    shiftPattern = &newShiftPattern;

    //bind the shift pattern's segment set pointer to the effect's segSet pointer
    //(using the effect's segSet pointer is important because it makes sure the effect behaves like other effects)
    segSet = shiftPattern->segSet;

    //number of lines in the shiftPattern's segment set
    numLines = segSet->numLines;

    //number of "rows" in the pattern
    numPatRows = shiftPattern->numRows;

    //how many segment lines the pattern takes up
    patLineLength = shiftPattern->patLineLength;

    //get the number of segments from the pattern
    numPatSegs = shiftPattern->patRowLength;

    //We need to cap the number of pattern segments if it's greater than the number of segments in the segment set
    //Otherwise we'd try to draw onto segments that didn't exist
    //(it's fine if the pattern has fewer segments than numSegs, nothing will be draw on the missing segments)
    uint16_t numSegs = segSet->numSegs;
    if( numPatSegs > numSegs ) {
        numPatSegs = numSegs;
    }

    setRepeat(repeat);
}

//Sets the repeat value of the effect and calculates how many repeats will fit on the segment set (rounded up)
void PatternShifterSL::setRepeat(bool newRepeat) {
    repeat = newRepeat;
    numLines = segSet->numLines;

    //how many segment lines the pattern takes up
    patLineLength = shiftPattern->patLineLength;

    //Calculate how many pattern repeats will fit on the segment set
    if( repeat ) {
        repeatCount = ceil((float)numLines / patLineLength);
    } else {
        repeatCount = 1;  //One pattern will always be drawn, even with 0 repeats
    }

    //Set the wrapping point for the pattern
    //To account for repeats, we treat the actual pattern size as the "line length of pattern" * "number of repeats".
    //So that we treat the output pattern as the whole repeated pattern. 
    //Then, when we draw, we slide the segment line "window" across the pattern, only drawing what fits in the "window".
    //So, if the overall pattern is longer than the number of lines we need to increase our wrapping point (mod amount)
    //so that the whole repeated pattern is cycled across the segment set.
    //However, if the pattern is shorter that the number of segment lines, we just need to wrap at the end line. 
    if( (patLineLength * repeatCount) > numLines ) {
        modVal = patLineLength * repeatCount;
    } else {
        modVal = numLines;
    }
}

/* Updates the effect
For info on shiftPatterns, see shiftPatternPS.h
The goal of the effect is to move the shiftPattern across the segment set lines.
For each update we draw the pattern, offsetting the start and end points by the cycleNum (which is incremented after each update)
So the pattern is shifted across the segment set one line at a time.
If we're repeating, then we draw the pattern up to repeatCount number of times, 
with each new pattern being offset by the patLineLength (how many segment lines the pattern covers)
so that the patterns are placed directly one after another, filling the whole segment set.
We draw each pattern segment line by line, seg by seg, filling in each pixel as we go according to the shiftPattern
Note that since we move the pattern's segment lines, the pattern can be longer than the segment lines,
with any extra parts being cycled on as the pattern moves.
On the other hand, the length of the segment pattern cannot be greater than the number of segments in the segment set
since any extra will not be drawn. */
void PatternShifterSL::update() {
    currentTime = millis();

    if( (currentTime - prevTime) >= *rate ) {
        prevTime = currentTime;

        //If we're not repeating, we need to turn off the last segment line
        //that the pattern was on for the previous cycle (as long as it's in the segment set)
        if( !repeat && prevLine < numLines ) {
            //fill in background if not repeating
            segDrawUtils::drawSegLine(*segSet, prevLine, *bgColor, bgColorMode);
        }

        //To draw the pattern, we run over each pattern "row", getting the start and end lines,
        //and then filling in each line and segment according to the "row" in shiftPattern array
        //We repeat this process repeatLineCount number of times (min of 1), offsetting where the lines are each time
        for( uint16_t i = 0; i < numPatRows * repeatCount; i++ ) {

            //The current pattern row, adjusted for repeat number
            patternRow = mod16PS(i, numPatRows);

            //The number of the repeat we're on, ie the "second repeat out of four"
            curRepeatNum = i / numPatRows;

            //Get the starting index for the current pattern "row"
            rowStartIndex = shiftPattern->getPatRowStartIndex(patternRow);

            //Get the pattern "row's" starting line, and then offset it by the cycleNum and which repeat we're on, wrapping as needed
            //This moves the pattern over time
            //We also do the same to get the end line
            startLine = shiftPattern->getLineStartOrEnd(patternRow, false);
            startLine = addMod16PS(startLine, cycleNum + patLineLength * curRepeatNum, modVal);

            endLine = shiftPattern->getLineStartOrEnd(patternRow, true);
            endLine = addMod16PS(endLine, cycleNum + patLineLength * curRepeatNum, modVal);

            //Draw the pattern row between the start and end lines
            //We draw each line segment by segment, getting each segment pixel color individually
            //Note that because the pattern may be wrapping, the endLine may actually be before the start line
            //But because shiftPatterns are continuous, if we also wrap as we increment j, we know that
            //we will eventually reach the end line
            for( uint16_t j = startLine; j != endLine; j = addMod16PS(j, 1, modVal) ) {
                //If the pattern is longer than the number of segment lines, some
                //of the pattern will always be off the segment set,
                //We don't want to try to draw these, so we skip them
                if( j >= numLines ) {
                    continue;
                }

                //For each segment we get the colorIndex from the pattern, the output the color
                for( uint16_t k = 0; k < numPatSegs; k++ ) {
                    //Get the color index of the segment pixel in the pattern
                    colorIndex = shiftPattern->getLineColorIndexQuick(rowStartIndex, k);
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
                    pixelNum = segDrawUtils::getPixelNumFromLineNum(*segSet, k, j);
                    segDrawUtils::setPixelColor(*segSet, pixelNum, colorOut, modeOut, k, j);
                }
            }
        }

        //Set prevLine, which is the line that must be turned off if we're not repeating the shiftPattern
        //The prevLine is just the current location of the first line in the shiftPattern, since shiftPatterns always move forwards
        //So we just need to work out where the first line is based on the cycleNum
        prevLine = shiftPattern->getLineStartOrEnd(0, false);
        prevLine = addMod16PS(prevLine, cycleNum, modVal);

        //Increment the cycleNum to shift the pattern forward in the next update cycle
        cycleNum = addMod16PS(cycleNum, 1, modVal);
        showCheckPS();
    }
}
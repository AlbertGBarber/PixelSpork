#ifndef shiftPatternPS_h
#define shiftPatternPS_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WConstants.h"
#include "WProgram.h"
#include "pins_arduino.h"
#endif

/* 
This is a struct used to define a shiftPattern
A shift pattern is a pattern of colors that is shifted either along segment lines or segments (depending on the effect)
They are meant to be used with either the PatternShiftSL or PatternShiftSeg effects

For example, you might use one of these effects to draw a flag and move it across you segment set.
The flag would be stored as a shiftPattern (which would then be passed into one of the effects)

The main part of a shift pattern is the patternArr, which is a 1D array of both color indexes and line numbers
The array has a very specific structure, and is usually created to work with a single segment set.
This is most easily shown with an example:

Lets say I have an 8x8 matrix and I want to draw an arrow shifting horizontally across it
I've already defined a SegmentSet with 8 segments, each representing a horizontal line on the matrix.
This means that the segment lines will be the vertical lines on the matrix:
(see SegmentSet.h for more info on segments)
    0  *  *  *  *  *  *  * <- Seg 0
    *  0  *  *  *  *  *  * <- Seg 1
    *  *  0  *  *  *  *  * <- Seg 2
    *  *  *  0  *  *  *  * <- Seg 3
    *  *  *  0  *  *  *  * <- Seg 4
    *  *  0  *  *  *  *  * <- Seg 5
    *  0  *  *  *  *  *  * <- Seg 6
    0  *  *  *  *  *  *  * <- Seg 7
    |  |  |  |  |  |  |  |
    0  1  2  3  4  5  6  7 <- Segment Lines

I have called the SegmentSet horzLines.

The 0's in the above diagram represent the arrow we want to draw (with all of the pixels being part of the background)
To create the arrow as a shiftPattern I would represent it as:

//***********************************************************
    //255 in the pattern indicates the background, but to keep the matrix easy to read, I define 255 as I
    uint8_t I = 255; 

    //Each line color pattern is numSegs long, it's easier to define an numSegs to plug in here
    const uint16_t numSegs = horzLines.numSegs;

    //The pattern array to store the arrow shape 
    uint16_t arrowPattern_arr[(numSegs + 2) * 4] = {
        0, 1,   0, I, I, I, I, I, I, 0,
        1, 2,   I, 0, I, I, I, I, 0, I,
        2, 3,   I, I, 0, I, I, 0, I, I,
        3, 4,   I, I, I, 0, 0, I, I, I,
    };
    //  Part 1  ------ Part 2 --------
    //Seg Lines --Line Color Pattern--

    //Final part of setting up the shiftPattern:
    //Note that we include the segment set we intend to use the pattern with in the pattern definition
    shiftPatternPS arrowPattern(horzLines, numSegs, arrowPattern_arr, SIZE(arrowPattern_arr);

//******************************************************************************

The array above may look strange at first, but it is actually quite simple.
While shiftPattern arrays are one continuous array, they actually have distinct sections,
which each represent a pattern for a segment line (or lines).
To help show this (and make it much easier to see what a pattern is), I've split the array across multiple text lines.
Each line represents a segment line in the matrix and consists of parts 1 and 2.
Part 1 is ALWAYS two digits, representing the start and end segment lines for the line color pattern (part 2).
This means that for patterns with repeating parts, you don't need to keep re-defining same pattern for each segment line,
you can just increase how many lines each part of the pattern is draw over.

Part 2 is the line color pattern.
It is almost ALWAYS numSegs long (unless the pattern is for use in PatternShifterSeg, then it can be longer)
with each entry representing a palette color index (or 255 if it's to be background) for each seg.

Lets take the first the first line in the array:
    0, 1,   0, I, I, I, I, I, I, 0,
// Part 1   ------ Part 2 --------
//Seg Lines --Line Color Pattern--   

Part 1 is the start and end segment lines for the line color pattern represented by part 2.
So the first pattern line starts at segment line 0, and ends at line 1,
meaning that the pattern will be draw on one segment line: line 0.
(!!!!Note that the pattern is draw up to the last line, but does not include it)
Lets say that we wanted to the color line pattern over two lines;
we'd simply change the end line to 2, so the pattern would be draw on lines 0 and 1.

Part 2 is the line color pattern. 
It 8 has entries (the same as the number of segments in the segment set, ie rows in the matrix)
With each entry representing a palette color index or background for each the 0'th line pixel in each segment
(since the pattern ony spans 1 line, each index corresponds directly to an individual pixel in the matrix)
The background pixels are indicated by 255, but in order to make the pattern easier to read I've set 
a variable "I" = 255 and then used it in the pattern instead.
So the line 0, I, I, I, I, I, I, 0, will be output as: 
segments 0 and 7 (the first and last segments) will be set to palette color 0, 
while the other segments will be set to the background color (which is set in the effects, not here).

The remaining lines in the pattern follow a similar setup, with each being 1 line long,
with all of them combining to create the arrow pattern we want. 

The overall length of the array is (numSegs + 2) * 4. 
This should be similar for most other patterns:
(numSegs + 2) is the length of each pattern "row", the +2 spanning part 1 and numSegs spanning part 2
*4 is the number of "rows" in the pattern

With all this info, we can create the shiftPattern using its constructor:

shiftPatternPS(SegmentSet &SegSet, uint16_t PatRowLength, uint16_t *PatternArr, uint16_t Length)

Where:
    * SegSet is the segment set we want to draw the pattern on: horzLines
    * PatRowLength is how long each "row" of the pattern is (with out the +2).
      In this case this is the numSegs in the horzLines: 8
    * PatternArr and Length are the patter array itself and the array's length
      (which we use SIZE() to set automatically)

To summarize:
shiftPatterns are usually specific to a SegmentSet
For the pattern array:
    Part 1 is how many segment lines the pattern line spans (up to, but not including the final line).
    Part 2 is the color pattern on the segments. 
    255 is used to indicate a background color.
The length of the array is usually (<numSegs> + 2) * number of "rows" in the pattern,
"rows" can vary in length, but are usually numSegs long (see the PatternShifter effects for more info)

Final gotchas:
*The segment lines (part 1) don't have to be in sequential order, ie you can have a a pattern line from 3 to 4, then a line from 0 to 3
 (this can be useful when working with odd shaped segment sets, where lines overlap)
*Gaps between segment lines may cause weird effects (ie you have a pattern line from 0 to 2 and from 5 to 6, but nothing in between)
 Gaps won't be filed with anything, so the lines patterns will just repeat as the pattern moves
*Make sure you use the correct array length
*Patterns require a segment set as a input. This is tied to the pattern's segSet pointer, like in effects. 
 You can change the segSet just as you would in effects, this will also change the segSet for the PatternShifter effects. 
*/

//See above for explanation of shiftPatterns
struct shiftPatternPS {
    SegmentSet *segSet; //pointer to the segment set for the shift pattern
    uint16_t *patternArr = nullptr; //pointer to the pattern array
    uint16_t length; //length of the pattern array
    uint16_t patRowLength; //how many long each line color pattern "row" is (not including the +2 for the lines span)
  
    uint16_t numRows; //how many "rows" are in the pattern array
    uint16_t patLineLength; //How many total lines in the segment set the pattern spans, ie "from segment line 0 to 8"

    //Constructor
    shiftPatternPS(SegmentSet &SegSet, uint16_t PatRowLength, uint16_t *PatternArr, uint16_t Length){
        //bind the SegSet address to the segSet pointer
        bindSegSetPtrPS();

        patternArr = PatternArr;
        length = Length;
        patRowLength = PatRowLength;

        numRows = length / (2 + patRowLength);

        //Temp vars used to find the total number of lines spanned by the shiftPattern
        uint16_t startLine = 65535; //We want to find the lowest startLine so we need to start it at uint16_t max
        uint16_t endLine = 0;//We want to find the highest endLine so we need to start it at 0
        uint16_t startLineTemp, endLineTemp;

        //We want to find the total number of lines spanned by the shiftPattern
        //To do this we want to find the pattern's minimum and maximum start and end lines
        //So we search the shift pattern and record the lines in startLine and endLine
        //(this search assumes that end lines are always greater than start lines, which is required as part of the shiftPattern setup)
        for(uint16_t i = 0; i < numRows; i++){

            startLineTemp = getLineStartOrEnd(i, false);
            endLineTemp = getLineStartOrEnd(i, true);

            if(startLineTemp < startLine){
                startLine = startLineTemp ;
            }

            if(endLineTemp > endLine){
                endLine = endLineTemp;
            }
        }

        //The total line length of the pattern
        patLineLength = endLine - startLine; 
    };

    //-----------------------------------------------------------
    //Helper functions

    //Returns the first index of the specified "row" in the patternArr
    uint16_t getPatRowStartIndex(uint16_t patternRow){
        return patternRow * (2 + patRowLength);
    };

    //Returns either the start or end segment line for the specified "row" in the patternArr
    //isLineEnd = true will return the end line for the row
    uint16_t getLineStartOrEnd(uint16_t patternRow, bool isLineEnd){
        uint16_t patRowStartIndex = getPatRowStartIndex(patternRow);
        //The starting line is always the first index in each "row"
        //while the end line is always the second index
        //so we can use the isLineEnd bool to shift to the correct index (0 for the start line, 1 for the end line)
        //by casing the bool to an int, making it 0 or 1
        return patternArr[ patRowStartIndex + (uint8_t)isLineEnd ];
    };

    //returns the color index for the specified "row" in the patternArr at the specified segNum
    uint16_t getLineColorIndex(uint16_t patternRow, uint16_t segNum){
        uint16_t patRowStartIndex = getPatRowStartIndex(patternRow);
        return patternArr[ patRowStartIndex + 2 + segNum ];
    };

    //returns the color index based on the "row" start index and the specified segNum
    //Is faster than getLineColorIndex(), but you need to already know the "row" starting index
    //( found using getPatRowStartIndex() )
    uint16_t getLineColorIndexQuick(uint16_t patRowStartIndex, uint16_t segNum){
        return patternArr[ patRowStartIndex + 2 + segNum ];
    }; 

} ;

#endif

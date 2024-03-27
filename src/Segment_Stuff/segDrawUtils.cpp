#include "segDrawUtils.h"

using namespace segDrawUtils;
/*
Function inputs distinguish between pixel locations local to the segment set
and the pixel's physical address on the strip by using "segPixelNum" and "pixelNum" vars.
Note that counting starts from 0 for both cases!!
    * segPixelNum is the location local to the segment set, not the led's physical address.
       ex: The 25th pixel in the segment set, it's segPixelNum is 25.
    * pixelNum is the phyiscal address of the led
    ex: The 25th pixel in the segment set has a physical address of 10 so it's pixelNum is 10
The input vars of functions tell you what type of pixel location it wants
*/

//returns the physical number of the nth pixel in the segment set
//ie I want the 10th pixel as measured from the start of the segment set, this is actually the 25th pixel on the strip
uint16_t segDrawUtils::getSegmentPixel(SegmentSetPS &SegSet, uint16_t segPixelNum) {
    getSegLocationFromPixel(SegSet, segPixelNum, locData1);
    return getSegmentPixel(SegSet, locData1[0], locData1[1]);
}

//Returns the segment number and the pixel number of that segment for the nth pixel
//The data is stored in a uint16_t "locData" array of length 2, which you have to supply.
//A pointer to the array is passed to the function, which the function then fills in.
//locData[0] is the segment number
//locData[1] is the pixel number in the segment
//ex we want the 12th pixel in the segment set. This is the 8th pixel of the second segment in the set
//so locData[0] = 2 and locData[1] = 8.
//If the passed in pixel number isn't on the segment (it's greater than the total segment set length)
//Then the locData[1] pixel number will be passed back as D_LED_PS, which will prevent it being drawn
void segDrawUtils::getSegLocationFromPixel(SegmentSetPS &SegSet, uint16_t segPixelNum, uint16_t *locData) {
    lengthSoFar = 0;
    //Before we search for the segment pixel we set default values for locData array
    //These act as a safeguard incase the segPixelNum is off the end of the segment set
    //In which case we want to avoid trying to draw to it
    //so we set the led location to D_LED_PS, which will be ignored when trying to write out any colors
    locData[0] = 0;
    locData[1] = D_LED_PS;
    for( uint16_t i = 0; i < SegSet.numSegs; i++ ) {
        lengthSoFar += SegSet.getTotalSegLength(i);
        if( (lengthSoFar - 1) >= segPixelNum ) {
            locData[0] = i;
            locData[1] = segPixelNum - (lengthSoFar - SegSet.getTotalSegLength(i));
            break;
        }
    }
}

//IF YOU'RE GONNA OPTIMIZE ANYTHING OPTIMIZE THIS FUNCTION
//finds the address of the pixel at a given position in a segment
//ie we want to find the address of the 5th pixel in the second segment.
//note, if the segment is in the reverse direction we want the pixel from the end of the segment
uint16_t segDrawUtils::getSegmentPixel(SegmentSetPS &SegSet, uint16_t segNum, uint16_t segPixelNum) {
    if( segPixelNum >= SegSet.getTotalSegLength(segNum) ) {
        return D_LED_PS;
    }
    numSec = SegSet.getTotalNumSec(segNum);
    lengthSoFar = 0;
    //num is the index of the pixel in the segment and is 0 based
    //segmentNum index of the segment in the segment array
    segDirection = SegSet.getSegDirection(segNum);

    //if the segment is descending, we want to count backwards, so we change the loop variables
    if( !segDirection ) {
        step = -1;
        endLimit = -1;
        startLimit = numSec - 1;
    } else {
        //counting loop setup variables, the default is a ascending segment, so we count forward
        step = 1;           //int8_t
        endLimit = numSec;  //int16_t
        startLimit = 0;     //uint8_t
    }
    //run through each segment section, summing the lengths of the sections,
    //starting at the end or beginning of the segment depending on direction
    //if the sum is larger than the number we want, then the pixel is in the current section
    //use the section to get the physical pixel number
    for( int16_t i = startLimit; i != endLimit; i += step ) {
        secLength = SegSet.getSecLength(segNum, i);         //sec length can be negative
        secLengthSign = (secLength > 0) - (secLength < 0);  //either 1 or -1
        secLength = secLength * secLengthSign;              //get the positive version of secLength
        lengthSoFar += secLength;                           //always add a positive sec length, we want to know the physical length of each section
        //if the count is greater than the number we want (num always starts at 0, so secLength will always be one longer than the max num in the section)
        //the num'th pixel is in the current segment.
        //for ascending segments:
        //we add the difference between how many pixel's we've counted
        //and how many we want (ie num - prevCount) to the section's starting pixel
        //for descending segments:
        //we add the section length and subtract the difference (num - prevCount) - 1
        //unless the secLength is 1, then it's just the start pixel
        if( lengthSoFar > segPixelNum ) {
            //We want to get the length up to the current section (so we find where the pixel is in this section)
            //But we already added the current secLength to our length count, so we need to subtract it off again
            lengthSoFar -= secLength;
            //Switch how we output to match the two possible segment section types
            //If the first if statement is true, then the segment has continuous sections, with starting pixels and lengths
            //Otherwise the segment will have a single mixed section, with an array of physical pixel locations and a length
            //( SegSet.getSecContArrPtr(segNum) returns false if the segment has a null section array pointer,
            //it should have a real mixed section pointer instead )
            if( SegSet.getSecContArrPtr(segNum) ) {
                //for continuous sections we get the starting pixel, and then count from it
                secStartPixel = SegSet.getSecStartPixel(segNum, i);
                if( secLength == 1 ) {
                    return secStartPixel;
                } else if( segDirection ) {
                    return (secStartPixel + secLengthSign * (segPixelNum - lengthSoFar));
                } else {
                    return (secStartPixel + secLengthSign * (secLength - (segPixelNum - lengthSoFar) - 1));
                }
            } else {
                //for mixed sections we grab the pixel value from the section array
                if( segDirection ) {
                    //if the segment has a mixed section, return the pixel value at the passed in pixel number
                    return SegSet.getSecMixPixel(segNum, i, segPixelNum - lengthSoFar);
                } else {
                    //if the segment is reversed, we grab the pixel counting from the end of the section
                    return SegSet.getSecMixPixel(segNum, i, secLength - (segPixelNum - lengthSoFar) - 1);
                }
            }
        }
    }

    //If we don't find the pixel, return a dummy led for safety
    return D_LED_PS;
}

//turns all pixel in a segment set off
void segDrawUtils::turnSegSetOff(SegmentSetPS &SegSet) {
    fillSegSetColor(SegSet, 0, 0);
}

//fills and entire segment set with a color
//ie all its segments and all their sections
void segDrawUtils::fillSegSetColor(SegmentSetPS &SegSet, const CRGB &color, uint8_t colorMode) {
    for( uint16_t i = 0; i < SegSet.numSegs; i++ ) {
        fillSegColor(SegSet, i, color, colorMode);
    }
}

//Fills a segment with a specific color
void segDrawUtils::fillSegColor(SegmentSetPS &SegSet, uint16_t segNum, const CRGB &color, uint8_t colorMode) {
    numSec = SegSet.getTotalNumSec(segNum);
    lengthSoFar = 0;
    //Color each segment section. We keep track of how many pixels we've colored using lengthSoFar
    //This is important for tracking the line number, see fillSegSecColor()
    for( uint8_t i = 0; i < numSec; i++ ) {
        fillSegSecColor(SegSet, segNum, i, lengthSoFar, color, colorMode);
        lengthSoFar += SegSet.getSecLength(segNum, i);
    }
}

/* Fills a segment section with a specific color
(ex: a segment section has three sub sections: {1, 4} , {8, 3}, {14, 8}) (so 4 pixels starting at 1, 3 starting at 8, and 8 starting at 14)
this function fills all sections with a color
To get the right line number, we need to know how far the sections pixels are in the overall segment
This is represented as the passed in pixelCount, which is the total length of all the sections up to the current section
Since you'll usually be using this function to color in all the sections of a segment, the pixelCount is easy to track
(see fillSegColor())
But if you need to track it for an arbitrary section you can use the code below:
Where secNum is the section you're coloring in
numSec = SegSet.getTotalNumSec(segNum);
lengthSoFar = 0;
for(uint8_t i = 0; i < secNum; i++){
 lengthSoFar += SegSet.getSecLength(segNum, i);
} */
void segDrawUtils::fillSegSecColor(SegmentSetPS &SegSet, uint16_t segNum, uint16_t secNum, uint16_t pixelCount, const CRGB &color, uint8_t colorMode) {
    secLength = SegSet.getSecLength(segNum, secNum);

    //Switch how we output to match the two possible segment section types
    //If the first if statement is true, then the segment has default sections, with starting pixels and lengths
    //Otherwise the segment will have a single mixed section, with an array of physical pixel locations and a length
    //(SegSet.getSecContArrPtr(segNum) returns false if the segment has a null section array pointer,
    //it should have a real mixed section pointer instead )
    if( SegSet.getSecContArrPtr(segNum) ) {
        secStartPixel = SegSet.getSecStartPixel(segNum, secNum);
        step = (secLength > 0) - (secLength < 0);  // account for negative lengths
        for( uint16_t i = secStartPixel; i != (secStartPixel + secLength); i += step ) {
            //The line num uses pixelCount because we need the location of the pixel relative to the overall length of the segment.
            //Ie pixel number 5 in the section could be the 20th pixel in the overall segment
            //We increment pixelCount each loop as we set each new pixel
            setPixelColor(SegSet, i, color, colorMode, segNum, getLineNumFromPixelNum(SegSet, pixelCount, segNum));
            pixelCount++;
        }
    } else {
        //For mixed sections we just have to run across the section array and set every pixel in it
        for( uint16_t i = 0; i < secLength; i++ ) {
            pixelNum = SegSet.getSecMixPixel(segNum, secNum, i);
            //for the line number, we do the same as above for continuous sections by counting the number of pixels
            setPixelColor(SegSet, pixelNum, color, colorMode, segNum, getLineNumFromPixelNum(SegSet, pixelCount, segNum));
            pixelCount++;
        }
    }
}

//Fills in a length of a segment with a color, using a start and end pixel
//pixel numbers are local to the segment, not global. ie 1-8th pixel in the segment
void segDrawUtils::fillSegLengthColor(SegmentSetPS &SegSet, uint16_t segNum, uint16_t startSegPixel, uint16_t endPixel, const CRGB &color, uint8_t colorMode) {
    //below is the fastest way to do this
    //there's no point in trying to split the length into partially and completely filled segment sections
    //because in the end you need to call getSegmentPixel() for each pixel anyway
    for( uint16_t i = startSegPixel; i <= endPixel; i++ ) {
        setPixelColor(SegSet, i, color, colorMode, segNum);
    }
}

//Fills in a length of a segment set in a color, using a start and end pixel
//pixel numbers are local to the segment set, not the global pixel numbers. Ie 5th through 8th pixel in the segment set
//(starting from 0)
void segDrawUtils::fillSegSetLengthColor(SegmentSetPS &SegSet, uint16_t startSegPixel, uint16_t endPixel, const CRGB &color, uint8_t colorMode) {
    //to fill the section in we split it into three parts:
    //*the segment containing the start pixel
    //*the segment containing the end pixel
    //*all segments in between the above pair
    //we then:
    //fill in the start pixel's segment after the start pixel
    //fill in all the middle segments
    //fill in the end pixel's segment up to the end pixel
    //gotchas:
    //if we only have one segment in the set, we just fill it directly
    //if the start and end pixel are in the same segment, we fill it directly
    if( SegSet.numSegs == 1 ) {  //only one segment
        fillSegLengthColor(SegSet, 0, startSegPixel, endPixel, color, colorMode);
    } else {
        //locData1 is the data for the starting pixel
        //locData2 is the data for the ending pixel
        getSegLocationFromPixel(SegSet, startSegPixel, locData1);
        getSegLocationFromPixel(SegSet, endPixel, locData2);
        if( locData1[0] == locData2[0] ) {  //start and end pixel in same segment
            fillSegLengthColor(SegSet, locData1[0], locData1[1], locData2[1], color, colorMode);
        } else {  //general case
            fillSegLengthColor(SegSet, locData1[0], locData1[1], SegSet.getTotalSegLength(locData1[0]) - 1, color, colorMode);
            for( uint16_t i = locData1[0] + 1; i < locData2[0]; i++ ) {
                fillSegColor(SegSet, i, color, colorMode);
            }
            fillSegLengthColor(SegSet, locData2[0], 0, locData2[1], color, colorMode);
        }
    }
}

//Draws a segment line of one color
void segDrawUtils::drawSegLine(SegmentSetPS &SegSet, uint16_t lineNum, const CRGB &color, uint8_t colorMode) {
    drawSegLineSection(SegSet, 0, SegSet.numSegs - 1, lineNum, color, colorMode);
}

//Draws a segment line of one color between startSeg and endSeg (including endSeg)
void segDrawUtils::drawSegLineSection(SegmentSetPS &SegSet, uint16_t startSeg, uint16_t endSeg, uint16_t lineNum, const CRGB &color, uint8_t colorMode) {
    for( uint16_t i = startSeg; i <= endSeg; i++ ) {  // for each segment, set the color, if we're in rainbow mode, set the rainbow color
        pixelNum = getPixelNumFromLineNum(SegSet, i, lineNum);
        setPixelColor(SegSet, pixelNum, color, colorMode, i, lineNum);
    }
}

//This function is the basis for 2D segment sets.
//Will return a pixel such that you can draw a "straight" line across all segments, using the longest segment as the basis.
//The pixels are mapped to the closest line, so some pixels may exist in multiple lines. 
//Retuns the pixel number located on segment "segNum" located along line "lineNum" where the total number of lines is the length of the longest segment
//Note that it returns the physical address of the Pixel
uint16_t segDrawUtils::getPixelNumFromLineNum(SegmentSetPS &SegSet, uint16_t segNum, uint16_t lineNum) {
    //This formula dedicated to my father, who saved me from many hours of head-scratching in an instant
    return getSegmentPixel(SegSet, segNum, ((lineNum * SegSet.getTotalSegLength(segNum)) / SegSet.numLines));
}

//returns the line number (based on the max segment length) of a pixel in a segment set
//(segPixelNum is local to the segment set)
uint16_t segDrawUtils::getLineNumFromPixelNum(SegmentSetPS &SegSet, uint16_t segPixelNum) {
    //get the segment number and led number of where the pixel is located in the strip
    getSegLocationFromPixel(SegSet, segPixelNum, locData1);
    return getLineNumFromPixelNum(SegSet, locData1[1], locData1[0]);
}

//returns the line number (based on the max segment length) of a pixel in a segment
//(segPixelNum is local to the segment)
uint16_t segDrawUtils::getLineNumFromPixelNum(SegmentSetPS &SegSet, uint16_t segPixelNum, uint16_t segNum) {
    //the formula below is the same as the one in getPixelNumFromLineNum
    //but solving for lineNum instead of pixelNum
    return (uint16_t)(segPixelNum * SegSet.numLines) / SegSet.getTotalSegLength(segNum);
}

//Sets the pixel color at (lineNum, segNum), 
//treating the segment set as matrix with dimensions numLines x numSegs.
//So your "x" input is a line number ("lineNum"), and your "y" input is a segment number ("segNum").
//This function should be particularly handy in adapting existing matrix-based effects to Pixel Spork
void segDrawUtils::setPixelColor_XY(SegmentSetPS &SegSet, uint16_t lineNum, uint16_t segNum, const CRGB &color, uint8_t colorMode){
    pixelNum = getPixelNumFromLineNum(SegSet, segNum, lineNum);
    setPixelColor(SegSet, pixelNum, color, colorMode, segNum, lineNum);
}

//sets pixel colors (same as other setPixelColor funct)
//doesn't need lineNum as argument. If lineNum is needed, it will be determined
//note segPixelNum is local to the segment set (ie 5th pixel in the whole set)
void segDrawUtils::setPixelColor(SegmentSetPS &SegSet, uint16_t segPixelNum, const CRGB &color, uint8_t colorMode) {
    getSegLocationFromPixel(SegSet, segPixelNum, locData1);
    setPixelColor(SegSet, locData1[1], color, colorMode, locData1[0]);
}

//note segPixelNum is local to the segment (ie 5th pixel in the segment)
void segDrawUtils::setPixelColor(SegmentSetPS &SegSet, uint16_t segPixelNum, const CRGB &color, uint8_t colorMode, uint16_t segNum) {
    pixelNum = getSegmentPixel(SegSet, segNum, segPixelNum);
    lineNum = 0;
    if( colorMode == 3 || colorMode == 8 ) {
        lineNum = getLineNumFromPixelNum(SegSet, segPixelNum, segNum);
    }
    setPixelColor(SegSet, pixelNum, color, colorMode, segNum, lineNum);
}

//Sets color of target pixel (actual address of the led, not local to segment)
//also adjusts the output color to the brightness of the SegSet
//see getPixelColor() for explanation of colorMode and other inputs
void segDrawUtils::setPixelColor(SegmentSetPS &SegSet, uint16_t pixelNum, const CRGB &color, uint8_t colorMode, uint16_t segNum, uint16_t lineNum) {
    if( pixelNum == D_LED_PS ) {
        return;  //if we are given a dummy pixel don't try to color it
    }
    SegSet.leds[pixelNum] = getPixelColor(SegSet, pixelNum, color, colorMode, segNum, lineNum);

    handleBri(SegSet, pixelNum);
}

/* !!!This must be called whenever a pixel color is set!!! 
(it is called automatically by other color setting functions, but you 
must call it manually if you set a color manually, ie segSet.led[3] = CRGB::Red)
Fades an individual pixel to match the brightness of the segment set
Note that FastLED doesn't track each pixel's brightness directly,
so calling this twice on the same pixel will fade it twice
Therefore make sure you only call this once after you set a pixel's color */
void segDrawUtils::handleBri(SegmentSetPS &SegSet, uint16_t pixelNum) {
    //if the segment set has a brightness different than overall strip
    //adjust the outgoing color to match the brightness
    //it's (255 - SegSet.brightness) because
    //fadeToBlackBy takes a percent of 255, where 255 is the maximum fade
    //while for the SegSet, 255 is actually fully bright
    if( SegSet.brightness != 255 ) {
        SegSet.leds[pixelNum].fadeToBlackBy(255 - SegSet.brightness);
    }
}

//Returns the pixel color at (lineNum, segNum) (accounting for Color Modes), 
//treating the segment set as matrix with dimensions numLines x numSegs.
//So your "x" input is a line number ("lineNum"), and your "y" input is a segment number ("segNum").
//This function should be particularly handy in adapting existing matrix-based effects to Pixel Spork
//The input color will be returned unchanged if the Color Mode is 0.
CRGB segDrawUtils::getPixelColor_XY(SegmentSetPS &SegSet, uint16_t lineNum, uint16_t segNum, const CRGB &color, uint8_t colorMode){
    pixelNum = getPixelNumFromLineNum(SegSet, segNum, lineNum);
    getPixelColor(SegSet, pixelNum, color, colorMode, segNum, lineNum);
}

/* Fills in a pixelInfoPS struct with data (the pixel's actual address, what segment it's in, it's line number, and what color it should be)
(the passed in struct is empty and will be filled in, but you must provide it,
the segPixelNum is the number of the pixel you want the info for)
you're probably calling this to account for different color modes before manipulating or storing a color
this gives you all the data to call the full setPixelColor() directly
the passed in color will be set to struct color if color mode is zero
segPixelNum is local to the segment set (ie 10th pixel in the whole set) */
void segDrawUtils::getPixelColor(SegmentSetPS &SegSet, uint16_t segPixelNum, pixelInfoPS *pixelInfo, const CRGB &color, uint8_t colorMode) {
    getSegLocationFromPixel(SegSet, segPixelNum, locData1);
    pixelInfo->segNum = locData1[0];
    pixelInfo->pixelLoc = getSegmentPixel(SegSet, locData1[0], locData1[1]);
    pixelInfo->lineNum = getLineNumFromPixelNum(SegSet, locData1[1], locData1[0]);
    if( colorMode == 0 ) {
        pixelInfo->color = color;
    } else {
        pixelInfo->color = getPixelColor(SegSet, pixelInfo->pixelLoc, color, colorMode, locData1[0], pixelInfo->lineNum);
    }
}

/* IF YOU CHANGE THE COLOR MODE ORDER BE SURE TO CHANGE IT IN THE SegOffsetCycler Util and setPixelColor() above!!
Returns the color of a pixel based on the color mode and other variables
For color mode 0, the input color will be returned as is
For other modes, the color will either be set to part of a rainbow,
or as part of the gradient based on the SegSet's gradPalette (see SegSet.h for info)
Color modes are as follows (higher vals => using the gradient palette):
     mode 0: use passed in color
     mode 1 & 6: colors each pixel according to a rainbow or gradient spread across the total number of leds in the SegSet
     mode 2 & 7: colors each segment according to a rainbow or gradient spread across all segments
     mode 3 & 8: colors each segment line according to a rainbow or gradient mapped to the longest segment
     mode 4 & 9: Produces a single color that cycles through the rainbow or gradient at the SegSet's offsetRate
                 Used to color a whole effect as a single color that cycles through the rainbow or gradient
                 Note that 256 steps are used for the gradient/rainbow
     mode 5 & 10:  Same as mode 4, but the direction of the cycle is reversed
The total number of gradient steps are based on the SegSet values:
Each mode uses a different value so you can switch between modes more easily
     mode 1 & 6: SegSet.gradLenVal (defaulted to the number of leds in the segment set)
     mode 2 & 7: SegSet.gradSegVal (defaulted to the number of segments in the SegSet)
     mode 3 & 9: SegSet.gradLineVal (defaulted to the length of the longest segment (numLines) )
     mode 4 & 9 and 5 & 10: 256 (not user chargeable)
By changing these values you can create shorter or longer rainbows/gradients
Note that rainbows are limited to 255 steps, any longer and they will stretch out
Custom gradients can have any number of steps (default 255), set with gradOffsetMax.
If gradOffsetMax / (num colors in grad palette) > 255, the colors will be stretched.
You can the use SegSet's gradOffset to shift the gradient across the pixels */
CRGB segDrawUtils::getPixelColor(SegmentSetPS &SegSet, uint16_t pixelNum, const CRGB &color, uint8_t colorMode, uint16_t segNum, uint16_t lineNum) {
    //if( pixelNum == D_LED_PS ){
    //  return color; //if we're passed in a dummy led, just return the current color b/c it won't be output
    //}
    //colorFinal = 0;
    switch( colorMode ) {
        case 0:
        default:
            colorFinal = color;
            return colorFinal;
            break;
        case 1:  //colors each pixel according to a rainbow or gradient spread across the total number of leds in the SegSet
        case 6:
            colorModeDom = SegSet.gradLenVal;  //the total number of gradient steps
            colorModeNum = pixelNum;           //the current step
            break;
        case 2:  //colors each segment according to a rainbow or gradient spread across all segments
        case 7:
            colorModeDom = SegSet.gradSegVal;  //the total number of gradient steps
            colorModeNum = segNum;             //the current step
            break;
        case 3:  //colors each segment line according to a rainbow or gradient mapped to the longest segment
        case 8:
            colorModeDom = SegSet.gradLineVal;  //the total number of gradient steps
            colorModeNum = lineNum;             //the current step
            break;
        case 4:                                                            //produces a single color that cycles through the rainbow or gradient at the SegSet's offsetRate
        case 9:                                                            //used to color a whole effect as a single color that cycles through the rainbow or gradient
            colorModeDom = 256;                                            //The number of gradient steps are capped at 256
            colorModeNum = mod16PS(millis() / (*SegSet.offsetRate), 256);  //gets the step we're on
            //colorFinal = colorUtilsPS::wheel( colorModeNum & 255, 0 );
            break;
        case 5:   //Same as case 4 & 9, but the cycle direction is reversed
        case 10:  //(useful for effects where the main pixels are case 4 or 9, while the background is case 5 or 10)
            colorModeDom = 256;
            colorModeNum = 255 - mod16PS(millis() / (*SegSet.offsetRate), 256);
            //colorFinal = colorUtilsPS::wheel( 255 - (colorModeNum & 255), 0 );
            break;
    }

    //Color mode 0 returns within its case statement
    //for all other modes we need to do the following:

    //get either a rainbow or gradient color
    //(see colorUtils::wheel() and paletteUtilsPS::getPaletteGradColor() for info)
    //we also set offsetMax for the offset cycle here
    //because for all rainbow gradients it needs to be 256,
    //Custom gradient lengths are capped at the segment set's gradOffsetMax
    if( colorMode < 6 ) {
        offsetMax = 256;
        colorFinal = colorUtilsPS::wheel((colorModeNum * offsetMax) / colorModeDom, SegSet.gradOffset, SegSet.sat, SegSet.val);
    } else {
        offsetMax = SegSet.gradOffsetMax;
        colorFinal = paletteUtilsPS::getPaletteGradColor(*SegSet.gradPalette, (colorModeNum * offsetMax) / colorModeDom, SegSet.gradOffset, offsetMax);
        //Old code, uses the segment set grad lengths for gradOffsetMax
        //offsetMax = colorModeDom;
        //colorFinal = paletteUtilsPS::getPaletteGradColor(*SegSet.gradPalette, colorModeNum, SegSet.gradOffset, offsetMax);
    }

    //updates the gradient offset value
    setGradOffset(SegSet, offsetMax);

    return colorFinal;
}

/* Displays the current effect by calling FastLed.show()
The effect is only displayed if the effect's showNow var is true (it defaults to true)
This allow you to have multiple active effects, while only writing out to the leds once for all of them
ie if you have 3 effects, only one should have a true showNow var (usually the effect with shortest update rate)
Each effect is free to write to the FastLed Leds array (the color of each led), but they'll only be displayed
when the effect with the true showNow var is updated
This function also handles segments with single pixel sections by writing to all the leds in the single sections */
void segDrawUtils::show(SegmentSetPS &SegSet, bool showNow) {
    //Before we try to write out the led colors we check if any segments have single pixel sections
    for( uint16_t i = 0; i < SegSet.numSegs; i++ ) {

        //If a segment has one or more single sections, we need to handle them
        //by copying the color from the first led of each section
        if( SegSet.getSegHasSingle(i) ) {

            //Check the type of sections the segment has (continuous or mixed)
            hasContSec = SegSet.getSecContArrPtr(i);

            //For each segment section, if the section is single,
            //copy the color from the first section pixel into all the other section pixels
            for( uint8_t j = 0; j < numSec; j++ ) {
                if( SegSet.getSecIsSingle(i, j) ) {

                    //get the actual length of the section
                    secLength = SegSet.getSecTrueLength(i, j);

                    //Switch how we output to match the two possible segment section types
                    //If the first if statement is true, then the segment has default sections, with starting pixels and lengths
                    //Otherwise the segment will have a single mixed section, with an array of physical pixel locations and a length
                    //( SegSet.getSecContArrPtr(segNum) returns false if the segment has a null section array pointer,
                    //it should have a real mixed section pointer instead )
                    if( hasContSec ) {
                        secStartPixel = SegSet.getSecStartPixel(i, j);
                        colorFinal = SegSet.leds[secStartPixel];

                        step = (secLength > 0) - (secLength < 0);  // account for negative lengths
                        for( int16_t k = secStartPixel; k != (secStartPixel + secLength); k += step ) {
                            SegSet.leds[k] = colorFinal;
                        }
                    } else {
                        colorFinal = SegSet.leds[SegSet.getSecMixPixel(i, j, 0)];
                        //In this case the segment has a section of mixed pixel values
                        //We just have to run across the section array and set every pixel in it
                        for( uint16_t k = 0; k < secLength; k++ ) {
                            pixelNum = SegSet.getSecMixPixel(i, j, k);
                            //for the line number, since we only have on section, the pixel number is just i
                            SegSet.leds[pixelNum] = colorFinal;
                        }
                    }
                }
            }
        }
    }

    //if we're displaying the pixels for this effect, write them out
    if( showNow ) {
        FastLED.show();
    }
}

/* increments/decrements the gradOffset value of the passed in SegSet
wrapping the value at offsetMax
The offset is only changed if enough time has passed (set by SegSet.offsetRate)
and runOffset is true for the SegSet
The offset is incremented/decremented by the SegSet's offsetStep value each update
(this can be used to speed up offset cycles without needing a fast offsetRate)
Generally this function is called as part of getPixelColor or SegOffsetCycler
to automatically adjust the gradient or rainbow offset during an effect */
void segDrawUtils::setGradOffset(SegmentSetPS &SegSet, uint16_t offsetMax) {
    if( SegSet.runOffset ) {
        currentTime = millis();
        if( currentTime - SegSet.offsetUpdateTime > *SegSet.offsetRate ) {
            //either (1 or -1) * offsetStep
            stepDir = (SegSet.offsetDirect - !(SegSet.offsetDirect)) * SegSet.offsetStep;
            SegSet.offsetUpdateTime = currentTime;
            SegSet.gradOffset = addMod16PS(SegSet.gradOffset, offsetMax - stepDir, offsetMax);
        }
    }
}

//fades an entire segment set to black by a certain percentage (out of 255)
//uses FastLED's fadeToBlackBy function
void segDrawUtils::fadeSegSetToBlackBy(SegmentSetPS &SegSet, uint8_t val) {
    for( uint16_t i = 0; i < SegSet.numSegs; i++ ) {
        fadeSegToBlackBy(SegSet, i, val);
    }
}

//fades an entire segment to black by a certain percentage (out of 255)
//uses FastLED's fadeToBlackBy function
void segDrawUtils::fadeSegToBlackBy(SegmentSetPS &SegSet, uint16_t segNum, uint8_t val) {
    numSec = SegSet.getTotalNumSec(segNum);
    // run through segment's sections, fetch the startPixel and length of each, then color each pixel
    for( uint8_t i = 0; i < numSec; i++ ) {
        fadeSegSecToBlackBy(SegSet, segNum, i, val);
    }
}

//fades an entire segment section to black by a certain percentage (out of 255)
//(ex: a segment section has three sub sections: {1, 4} , {8, 3}, {14, 8}) (so 4 pixels starting at 1, 3 starting at 8, and 8 starting at 14)
//this function fades one section to black
//uses FastLED's fadeToBlackBy function
void segDrawUtils::fadeSegSecToBlackBy(SegmentSetPS &SegSet, uint16_t segNum, uint16_t secNum, uint8_t val) {
    secLength = SegSet.getSecLength(segNum, secNum);

    //Switch how we output to match the two possible segment section types
    //If the first if statement is true, then the segment has default sections, with starting pixels and lengths
    //Otherwise the segment will have a single mixed section, with an array of physical pixel locations and a length
    //( SegSet.getSecContArrPtr(segNum) returns false if the segment has a null section array pointer,
    //it should have a real mixed section pointer instead )
    if( SegSet.getSecContArrPtr(segNum) ) {
        secStartPixel = SegSet.getSecStartPixel(segNum, secNum);

        step = (secLength > 0) - (secLength < 0);  // account for negative lengths
        for( int16_t i = secStartPixel; i != (secStartPixel + secLength); i += step ) {
            SegSet.leds[i].fadeToBlackBy(val);
        }
    } else {
        //In this case the segment has a section of mixed pixel values
        //We just have to run across the section array and set every pixel in it
        for( uint16_t i = 0; i < secLength; i++ ) {
            pixelNum = SegSet.getSecMixPixel(segNum, secNum, i);
            SegSet.leds[i].fadeToBlackBy(val);
        }
    }
}

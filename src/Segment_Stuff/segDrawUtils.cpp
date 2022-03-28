#include "segDrawUtils.h"

using namespace segDrawUtils;

//!!!! Inputs distinguish between pixel num and seg pixel number (seg pixel number is local to the segment)

//returns the physical number of the nth pixel in the segment set
//ie I want the 10th pixel as measured from the start of the segment, this is acutally the 25th pixel on the strip
uint16_t segDrawUtils::getSegmentPixel(SegmentSet &segmentSet, uint16_t segPixelNum){
    getSegLocationFromPixel(segmentSet, segPixelNum, locData);
    return getSegmentPixel(segmentSet, locData[0], locData[1]);
}

//returns the segment number and the pixel number of that segment for the nth pixel
//data is package is passed into the passed in locData[2] array
//locData[0] is the segment number
//locData[1] is the pixel number in the segment
//ex we want the 12th pixel in the segment set. This is the 8th pixel of the second segment in the set 
//so locData[0] = 2 and locData[1] = 8.
void segDrawUtils::getSegLocationFromPixel(SegmentSet &segmentSet, uint16_t segPixelNum, uint16_t locData[2] ){
   lengthSoFar = 0;
   //set defaults for locData
   //we set the led location to the dLed b/c
   //if it's off the end of the segmentSet, we won't find it when we search
   //so we mark it as non existant
   locData[0] = 0;
   locData[1] = dLed; 
   for (uint8_t i = 0; i < segmentSet.numSegs; i++) {
        //if(segmentSet.getSegActive(i)){ //only count active segments
            lengthSoFar += segmentSet.getTotalSegLength(i);
            if( (lengthSoFar - 1) >= segPixelNum){
                locData[0] = i;
                locData[1] = segPixelNum - ( lengthSoFar - segmentSet.getTotalSegLength(i) );
                break;
            }
       //}
   }
}

// finds the pixel number of the pixel at a given position in a segment
// ie we want to find the pixel number of the 5th pixel in the second segment
// if the segment is in the reverse direction we want the pixel for the end of the segment
uint16_t segDrawUtils::getSegmentPixel(SegmentSet &segmentSet, uint8_t segNum, uint16_t segPixelNum) {
    // num is the index of the pixel in the segment and is 0 based
    // segmentNum index of the segement in the segment array
    segDirection = segmentSet.getSegDirection(segNum);
    numSec = segmentSet.getTotalNumSec(segNum);
    count = 0;
    prevCount = 0; 

    // if the segment is decending, we want to count backwards, so we change the loop variables
    if (!segDirection) {
        step = -1;
        endLimit = -1;
        startLimit = numSec - 1;
    } else {
        // counting loop setup variables, the default is a ascending segment, so we count forward
        step = 1; //int8_t
        endLimit = numSec;  //int16_t
        startLimit = 0; //uint8_t
    }
    
    // run through each segment section, summing the lengths of the sections,
    // starting at the end or beginning of the segment depending on direction
    // if the sum is larger than the number we want, then the pixel is in the current section
    // use the section to get the physical pixel number
    for (int16_t i = startLimit; i != endLimit; i += step) {
        secLength = segmentSet.getSecLength(segNum, i); // sec length can be negative
        secLengthSign = (secLength > 0) - (secLength < 0);
        absSecLength = abs(secLength);
        prevCount = count;
        count += absSecLength; // always add a positive sec length, we want to know the physical length of each section

        // if the count is greater than the number we want (num always starts at 0, so secLength will always be one longer than the max num in the section)
        // the num'th pixel is in the current segment.
        // for ascending segments:
        // we add the difference between how many pixel's we've counted
        // and how many we want (ie num - prevCount) to the section's starting pixel
        // for descending segments:
        // we add the section length and subtract the difference (num - prevCount) - 1

        // unless the secLength is 1, then it's just the start pixel
        if (count > segPixelNum) {
            secStartPixel = segmentSet.getSecStartPixel(segNum, i);
            if (secLength == 1 || secStartPixel == dLed ) {
                return secStartPixel;
            } else if (segDirection) {
                return (secStartPixel + secLengthSign * (segPixelNum - prevCount));
            } else {
                return (secStartPixel + secLengthSign * (absSecLength - (segPixelNum - prevCount) - 1));
            }
        }
    }
    //if we don't find the pixel across all the segments, we return the dLed value
    //when passed into setPixelColor, this value will be safely ignored
    return dLed;
}

//turns all pixel in a segment set off
void segDrawUtils::turnSegSetOff(SegmentSet &segmentSet){
    fillSegSetColor(segmentSet, 0, 0);
}

//fills and entire segment set with a color
//ie all its segments and all their sections
void segDrawUtils::fillSegSetColor(SegmentSet &segmentSet, CRGB color, uint8_t colorMode){
    for(uint8_t i = 0; i < segmentSet.numSegs; i++){
        fillSegColor(segmentSet, i, color, colorMode);
    }
}

// fills a segment with a specific color
void segDrawUtils::fillSegColor(SegmentSet &segmentSet, uint8_t segNum, CRGB color, uint8_t colorMode) {
    numSec = segmentSet.getTotalNumSec(segNum);
    // run through segment's sections, fetch the startPixel and length of each, then color each pixel
    for (int i = 0; i < numSec; i++) {
        fillSegSecColor(segmentSet, segNum, i, color, colorMode );
    }
}

//fills a segment section with a specific color 
//(ex: a segment section has three sub sections: {1, 4} , {8, 3}, {14, 8}) (so 4 pixels starting at 1, 3 starting at 8, and 8 starting at 14)
//this function fills all sections with a color
void segDrawUtils::fillSegSecColor(SegmentSet &segmentSet, uint8_t segNum, uint16_t secNum, CRGB color, uint8_t colorMode ){
    secStartPixel = segmentSet.getSecStartPixel(segNum, secNum);
    if(secStartPixel != dLed){
        secLength = segmentSet.getSecLength(segNum, secNum);
        step = (secLength > 0) - (secLength < 0); // account for negative lengths
        for (int i = secStartPixel; i != (secStartPixel + secLength); i += step) {
            setPixelColor(segmentSet, i, color, colorMode, segNum, getLineNumFromPixelNum(segmentSet, i - secStartPixel, segNum));
        }
    }
}

//fills in a length of a segment with a color, using a start and end pixel
//pixel numbers are local to the segment, not global. ie 1-8th pixel in the segment
void segDrawUtils::fillSegLengthColor(SegmentSet &segmentSet, CRGB color, uint8_t colorMode, uint16_t segNum, uint16_t startSegPixel, uint16_t endPixel){
    //below is the fastest way to do this
    //there's no point in trying to split the length into partially and completely filled segement sections
    //because in the end you need to call getSegmentPixel() for each pixel anyway
    for(int i = startSegPixel; i <= endPixel; i++){
        setPixelColor(segmentSet, i, segNum, color, colorMode);
    }
}

//fills in a length of a segement set in a color, using a start and end pixel
//pixel numbers are local to the segment set, not the global pixel numbers. Ie 5th through 8th pixel in the segment set
//(starting from 0)
void segDrawUtils::fillSegSetlengthColor(SegmentSet &segmentSet, CRGB color, uint8_t colorMode, uint16_t startSegPixel, uint16_t endPixel){
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
    if(segmentSet.numSegs == 1){ //only one segment
        fillSegLengthColor(segmentSet, color, colorMode, 0, startSegPixel, endPixel);
    } else {
        getSegLocationFromPixel(segmentSet, startSegPixel, startData );
        getSegLocationFromPixel(segmentSet, endPixel, endData );
        if( startData[0] == endData[0] ){ //start and end pixel in same segment
            fillSegLengthColor(segmentSet, color, colorMode, startData[0], startData[1], endData[1]);
        } else { //general case
            fillSegLengthColor(segmentSet, color, colorMode, startData[0], startData[1], segmentSet.getTotalSegLength(startData[0]) - 1 );
            for(int i = startData[0] + 1 ; i < endData[0]; i++){
                fillSegColor(segmentSet, i, color, colorMode);
            }
            fillSegLengthColor(segmentSet, color, colorMode, endData[0], 0, endData[1]);
        }
    }
}

// draws a line between segments, does its best to make a straight line
// the segment colors follow the provided pattern of pallet indecies
// the pattern length must match the number of segments
void segDrawUtils::drawSegLine(SegmentSet &segmentSet, uint16_t lineNum, uint8_t Pattern[], CRGB pallet[], uint8_t colorMode, uint8_t bgColorMode, bool brReplace) {
    numSegs = segmentSet.numSegs;
    drawSegLineSection(segmentSet, 0, numSegs - 1, lineNum, Pattern, pallet, colorMode, bgColorMode, brReplace);
}

// draws a line between segments, from startSeg to endSeg (including endSeg), does its best to make a straight line
// the segment colors follow the provided pattern of pallet indecies
// the pattern length must match the number of segments!
// if brReplace is true, any part of the pattern with a value of zero (ie first element in the pallet) will be treated as a background colored pixel
// and will use bgColorMode for it's color mode (same modes as colorMode)
// if you don't want this, set brReplace to false
void segDrawUtils::drawSegLineSection(SegmentSet &segmentSet, uint8_t startSeg, uint8_t endseg, uint16_t lineNum, uint8_t Pattern[], CRGB pallet[], uint8_t colorMode, uint8_t bgColorMode, bool brReplace) {
    maxSegLength = segmentSet.maxSegLength;
    pixelNum = 0;
    colorFinal = 0;
    uint8_t colorModeTemp = colorMode;
    for (uint8_t i = startSeg; i <= endseg; i++) {
        colorModeTemp = colorMode;
        pixelNum = getPixelNumFromLineNum(segmentSet, maxSegLength, i, lineNum);
        if (brReplace && Pattern[i] == 0) {
            colorModeTemp = bgColorMode;
        }
        if (colorModeTemp == 0) { //color mode is to use the color from the pattern
            colorFinal = pallet[Pattern[i]];
        }
        setPixelColor(segmentSet, pixelNum, colorFinal, colorMode, i, lineNum);
    }
}

// draws a segment line of one color, does not need a pallet or pattern, passing -1 as the color will do a rainbow based on the Wheel() function
void segDrawUtils::drawSegLineSimple(SegmentSet &segmentSet, uint16_t lineNum, CRGB color, uint8_t colorMode) {
    numSegs = segmentSet.numSegs;
    drawSegLineSimpleSection(segmentSet, 0, numSegs - 1, lineNum, color, colorMode);
}

// draws a segment line of one color betweeb startSeg and endSeg (including endSeg)
// does not need a pallet or pattern,
void segDrawUtils::drawSegLineSimpleSection(SegmentSet &segmentSet, uint8_t startSeg, uint8_t endSeg, uint16_t lineNum, CRGB color, uint8_t colorMode) {
    maxSegLength = segmentSet.maxSegLength;
    for (uint8_t i = startSeg; i <= endSeg; i++) { // for each segment, set the color, if we're in rainbow mode, set the rainbow color
        pixelNum = getPixelNumFromLineNum(segmentSet, maxSegLength, i, lineNum);
        setPixelColor(segmentSet, pixelNum, color, colorMode, i, lineNum);
    }
} 

// retuns the pixel number located on segment segNum located along line lineNum where the total number of lines matches the pixels in the longest segment
// in other words, will return a pixel such that you can draw a straight line accross all segments, using the longest segment as the basis
uint16_t segDrawUtils::getPixelNumFromLineNum(SegmentSet &segmentSet, uint16_t maxSegLength, uint8_t segNum, uint16_t lineNum) { 
    return getSegmentPixel(segmentSet, segNum, (lineNum * uint16_t(segmentSet.getTotalSegLength(segNum)) / maxSegLength)); 
}

//returns the line number (based on the max segment length) of a pixel in a segment set
//(pixel num is local to the segment set)
uint8_t segDrawUtils::getLineNumFromPixelNum(SegmentSet &segmentSet, uint16_t segPixelNum){
    //get the segment number and led number of where the pixel is located in the strip
    getSegLocationFromPixel(segmentSet, segPixelNum, locData );
    return getLineNumFromPixelNum(segmentSet, locData[1], locData[0]);
}

//returns the line number (based on the max segment length) of a pixel in a segment
//(pixel num is local to the segment number)
uint8_t segDrawUtils::getLineNumFromPixelNum(SegmentSet &segmentSet, uint16_t segPixelNum, uint8_t segNum){
    maxSegLength = segmentSet.maxSegLength;
    //the formula below is the same as the one in getPixelNumFromLineNum
    //but solving for lineNum instead of pixelNum
    return (uint16_t)(segPixelNum * maxSegLength) / segmentSet.getTotalSegLength(segNum) ;
}

//sets pixel colors (same as other setPixelColor funct)
//doesn't need lineNum as argument. If lineNum is needed, it will be determined
//note segPixelNum is local to the segement set (ie 5th pixel in the whole set)
void segDrawUtils::setPixelColor(SegmentSet &segmentSet, uint16_t segPixelNum, CRGB color, uint8_t colorMode){
    getSegLocationFromPixel(segmentSet, segPixelNum, locData);
    setPixelColor(segmentSet, locData[1], locData[0], color, colorMode);
}

//note segPixelNum is local to the segment (ie 5th pixel in the segment)
void segDrawUtils::setPixelColor(SegmentSet &segmentSet, uint16_t segPixelNum, uint8_t segNum, CRGB color, uint8_t colorMode){
    pixelNum = getSegmentPixel(segmentSet, segNum, segPixelNum);
    lineNum = 0;
    if(colorMode == 4 || colorMode == 10){
        lineNum = getLineNumFromPixelNum(segmentSet, segPixelNum, segNum);
    }
    setPixelColor(segmentSet, pixelNum, color, colorMode, segNum, lineNum);
}

//Sets color of target pixel (actual address of the led, not local to segment)
//also adjusts the output color to the brightness of the segmentSet
//see getPixelColor() for explanation of colorMode and other inputs
void segDrawUtils::setPixelColor(SegmentSet &segmentSet, uint16_t pixelNum, CRGB color, uint8_t colorMode, uint8_t segNum, uint16_t lineNum){
    if( pixelNum == dLed || !segmentSet.getSegActive(segNum) ){
        return; //if we are given a dummy pixel, don't try to color it
    }
    //colorFinal = 0;
    colorFinal = getPixelColor(segmentSet, pixelNum, color, colorMode, segNum, lineNum);
    segmentSet.leds[pixelNum] = colorFinal;
    //if the segment set has a brightness different than overall strip
    //adjust the outgoing color to match the brightness
    //it's (255 - segmentSet.brightness) because 
    //fadeToBlackBy takes a percent of 255, where 255 is the maxmimum fade
    //while for the segmentSet, 255 is acutally fully bright
    if(segmentSet.brightness != 255){
        segmentSet.leds[pixelNum].fadeToBlackBy(255 - segmentSet.brightness);
    }
}

//fills in a pixelInfoPS struct with data (the pixel's actual address, what segment it's in, it's line number, and what color it should be)
//(the passed in struct is empty and will be filled in, but you must provide it, 
//the segPixelNum is the number of the pixel you want the info for)
//you're probably calling this to account for different color modes before manipulating or storing a color
//this gives you all the data to call the full setPixelColor() directly
//the passed in color will be set to struct color if color mode is zero
//segPixelNum is local to the segment set (ie 10th pixel in the whole set)
void segDrawUtils::getPixelColor(SegmentSet &segmentSet, pixelInfoPS *pixelInfo, CRGB color, uint8_t colorMode, uint16_t segPixelNum){
    getSegLocationFromPixel(segmentSet, segPixelNum, locData);
    pixelInfo->segNum = locData[0];
    pixelInfo->pixelLoc = getSegmentPixel(segmentSet, locData[0], locData[1]);
    pixelInfo->lineNum = getLineNumFromPixelNum(segmentSet, locData[1], locData[0]);
    if(colorMode == 0){
        pixelInfo->color = color;
    } else {
        pixelInfo->color = getPixelColor(segmentSet, pixelInfo->pixelLoc, color, colorMode, locData[0], pixelInfo->lineNum);
    }
}

//Returns the color of a pixel based on the color mode and other variables
//For color mode 0, the input color will be returned as is
//For other modes, the color will either be set to part of a rainbow, 
//or as part of the gradient based on the segmentSet's gradPallet (see segmentSet.h for info)
//Also updates the segmentSet's gradOffset value, if active (see setGradOffset() and segmentSet.h )
//Color modes are as follows (higher vals => using the gradient pallet):
//      mode 0: use passed in color
//      mode 1 & 7: colors each pixel according to a rainbow or gradient spread across the total number of leds in the segmentSet
//      mode 2 & 8: same as case 1 & 7, but only counts the leds in active segments
//      mode 3 & 9: colors each segment according to a rainbow or gradient spread across all segments
//      mode 4 & 10: colors each segment line according to a rainbow or gradient mapped to the longest segment
//      mode 5 & 11: Produces a single color that cycles through the rainbow or gradient at the segmentSet's offsetRate
//               Used to color a whole effect as a single color that cycles through the rainbow or gradient
//               Note that 256 steps are used for the gradient/rainbow
//      mode 6 & 12:  Same as mode 7, but the direction of the cycle is reversed
//The total number of gradient steps are based on the segmentSet values:
//Each mode uses a different value so you can switch between modes more easily  
//      mode 1 & 7: segmentSet.gradLenVal (defaulted to the number of leds in the segment set)
//      mode 2 & 8: segmentSet.numActiveSegLeds ( total number of active leds in the segment set (don't change this manually) )
//      mode 3 & 9: segmentSet.gradSegVal (defaulted to the number of segments in the segmentSet)
//      mode 4 & 10: segmentSet.gradLineVal (defaulted to the length of the longest segment (maxSeglength) )
//      mode 5 & 11: 256 (not user changable)
//By changing these values you can create shorter or longer rainbows/gradients
//(note that rainbows will repeat every 255 steps, while gradients will be stretched over them)
//You can the use segmentSet's gradOffset to shift the gradient across the pixels
CRGB segDrawUtils::getPixelColor(SegmentSet &segmentSet, uint16_t pixelNum, CRGB color, uint8_t colorMode, uint8_t segNum, uint16_t lineNum){
    if( pixelNum == dLed ){
        return color; //if we're passed in a dummy led, just return the current color b/c it won't be output
    }
    //colorFinal = 0;
    switch(colorMode) { 
        case 0:
            colorFinal = color;
            return colorFinal;
            break;
        case 1: // colors each pixel according to a rainbow or gradient spread across the total number of leds in the segmentSet
        case 7:
            colorModeDom = segmentSet.gradLenVal; //the total number of gradient steps
            colorModeNum = pixelNum; //the current step
            break;  
        case 2: // same as case 1 & 7, but only counts the leds in active segments
        case 8:
            colorModeDom = segmentSet.numActiveSegLeds; //the total number of gradient steps
            colorModeNum = pixelNum; //the current step
            break;
        case 3: // colors each segment according to a rainbow or gradient spread across all segments
        case 9:
            colorModeDom = segmentSet.gradSegVal; //the total number of gradient steps
            colorModeNum = segNum; //the current step
            break;
        case 4: // colors each segment line according to a rainbow or gradient mapped to the longest segment
        case 10:
            colorModeDom = segmentSet.gradLineVal; //the total number of gradient steps
            colorModeNum = lineNum; //the current step
            break;
        case 5:  //produces a single color that cycles through the rainbow or gradient at the segmentSet's offsetRate
        case 11: //used to color a whole effect as a single color that cycles through the rainbow or gradient
            colorModeDom = 255; //The number of gradient steps are capped at 256
            colorModeNum = mod16PS( millis() / *segmentSet.offsetRate, 255 ); //gets the step we're on
            //colorFinal = colorUtilsPS::wheel( colorModeNum & 255, 0 );
            break;
        case 6:  //Same as case 5 & 11, but the cycle direction is reversed
        case 12: //(useful for effects where the main pixels are case 5 or 11, while the background is case 6 or 12)
            colorModeDom = 255;
            colorModeNum = 255 - mod16PS( millis() / *segmentSet.offsetRate, 255 );
            //colorFinal = colorUtilsPS::wheel( 255 - (colorModeNum & 255), 0 );
            break;
        default: 
            colorFinal = color;
            return colorFinal;
            break;
    }

    //Color mode 0 returns within its case statement
    //for all other modes we need to do the following:

    //get either a rainbow or gradient color
    //(see colorUtils::wheel() and palletUtilsPS::getPalletGradColor() for info)
    //we also set offsetMax for the offset cycle here
    //because for all rainbow gradients it needs to be 255, 
    //while for all pallet gradients it's whatever colorModeDom is
    if(colorMode < 7){
        offsetMax = 255;
        colorFinal = colorUtilsPS::wheel( (colorModeNum * 255) / colorModeDom, segmentSet.gradOffset,  segmentSet.rainbowSatur, segmentSet.rainbowVal );
    } else{
        offsetMax = colorModeDom;
        colorFinal = palletUtilsPS::getPalletGradColor(segmentSet.gradPallet, colorModeNum, segmentSet.gradOffset, colorModeDom);      
    }

    //updates the gradient offset value
    setGradOffset(segmentSet, offsetMax);

    return colorFinal;
}

//increments/decements the gradOffset value of the passed in segmentSet
//wrapping the value at offsetMax
//The offset is only changed if enough time has passed (set by segmentSet.offsetRate)
//and runOffset is true for the segmentSet
//Generally this function is called as part of getPixelColor or SegOffsetCycler
//to automatically adjust the graident or rainbow offset during an effect
void segDrawUtils::setGradOffset(SegmentSet &segmentSet, uint16_t offsetMax){
    if(segmentSet.runOffset){
        currentTime = millis();
        if(currentTime - segmentSet.offsetUpdateTime > *segmentSet.offsetRate){
            step = segmentSet.offsetDirect - !segmentSet.offsetDirect; //either 1 or -1
            segmentSet.offsetUpdateTime = currentTime;
            segmentSet.gradOffset = addMod16PS(segmentSet.gradOffset, offsetMax + step, offsetMax);
        }
    }
}

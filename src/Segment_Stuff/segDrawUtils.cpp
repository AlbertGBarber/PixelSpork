#include "segDrawUtils.h"

using namespace segDrawUtils;

//!!!! Inputs distinguish between pixel num and seg pixel number (seg pixel number is local to the segment)

// retuns the pixel number located on segment segNum located along line lineNum where the total number of lines matches the pixels in the longest segment
// in other words, will return a pixel such that you can draw a straight line accross all segments, using the longest segment as the basis
uint16_t segDrawUtils::getPixelNumFromLineNum(SegmentSet segmentSet, uint16_t maxSegLength, uint8_t segNum, uint8_t lineNum) { 
    return getSegmentPixel(segmentSet, segNum, (uint16_t(lineNum) * uint16_t(segmentSet.getTotalSegLength(segNum)) / maxSegLength)); 
}

//returns the line number (based on the max segment length) of a pixel in a segment set
//(pixel num is local to the segment set)
uint8_t segDrawUtils::getLineNumFromPixelNum(SegmentSet segmentSet, uint16_t segPixelNum){
    uint16_t locData[2];
    //get the segment number and led number of where the pixel is located in the strip
    getSegLocationFromPixel(segmentSet, segPixelNum, locData );
    return getLineNumFromPixelNum(segmentSet, locData[1], locData[0]);
}

//returns the line number (based on the max segment length) of a pixel in a segment
//(pixel num is local to the segment number)
uint8_t segDrawUtils::getLineNumFromPixelNum(SegmentSet segmentSet, uint16_t segPixelNum, uint8_t segNum){
    uint16_t maxSegLength = segmentSet.maxSegLength;
    //the formula below is the same as the one in getPixelNumFromLineNum
    //but solving for lineNum instead of pixelNum
    return (uint16_t)(segPixelNum * maxSegLength) / segmentSet.getTotalSegLength(segNum) ;
}

//returns the physical number of the nth pixel in the segment set
//ie I want the 10th pixel as measured from the start of the segment, this is acutally the 25th pixel on the strip
uint16_t segDrawUtils::getSegmentPixel(SegmentSet segmentSet, uint16_t segPixelNum){
    uint16_t locData[2];
    getSegLocationFromPixel(segmentSet, segPixelNum, locData);
    return getSegmentPixel(segmentSet, locData[0], locData[1]);
}

//returns the segment number and the pixel number of that segment for the nth pixel
//data is package is passed into the passed in locData[2] array
//locData[0] is the segment number
//locData[1] is the pixel number in the segment
//ex we want the 12th pixel in the segment set. This is the 8th pixel of the second segment in the set 
//so locData[0] = 2 and locData[1] = 8.
void segDrawUtils::getSegLocationFromPixel(SegmentSet segmentSet, uint16_t segPixelNum, uint16_t locData[2] ){
   uint16_t lengthSoFar = 0;
   locData[0] = 0;
   locData[1] = 0;
   for (int i = 0; i < segmentSet.numSegs; i++) {
       if(segmentSet.getSegActive(i)){ //only count active segments
            lengthSoFar += segmentSet.getTotalSegLength(i);
            if( (lengthSoFar - 1) >= segPixelNum){
                locData[0] = i;
                locData[1] = segPixelNum - ( lengthSoFar - segmentSet.getTotalSegLength(i) );
                break;
            }
       }
   }
}

// finds the pixel number of the pixel at a given position in a segment
// ie we want to find the pixel number of the 5th pixel in the second segment
// if the segment is in the reverse direction we want the pixel for the end of the segment
uint16_t segDrawUtils::getSegmentPixel(SegmentSet segmentSet, uint8_t segNum, uint16_t segPixelNum) {
    // num is the index of the pixel in the segment and is 0 based
    // segmentNum index of the segement in the segment array
    boolean segDirection = segmentSet.getSegDirection(segNum);
    uint16_t numSec = segmentSet.getTotalNumSec(segNum);
    uint16_t count = 0;
    uint16_t prevCount = 0;
    uint16_t secStartPixel;
    int16_t secLength;
    uint16_t AbsSecLength;
    int secLengthSign;

    // counting loop setup variables, the default is a ascending segment, so we count forward
    int16_t step = 1;
    int16_t endLimit = numSec;
    int16_t startLimit = 0;

    // if the segment is decending, we want to count backwards, so we change the loop variables
    if (!segDirection) {
        step = -1;
        endLimit = -1;
        startLimit = numSec - 1;
    }
    // run through each segment section, summing the lengths of the sections,
    // starting at the end or beginning of the segment depending on direction
    // if the sum is larger than the number we want, then the pixel is in the current section
    // use the section to get the physical pixel number
    for (int i = startLimit; i != endLimit; i += step) {
        secLength = segmentSet.getSecLength(segNum, i); // sec length can be negative
        secLengthSign = (secLength > 0) - (secLength < 0);
        AbsSecLength = abs(secLength);
        prevCount = count;
        count += AbsSecLength; // always add a positive sec length, we want to know the physical length of each section

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
                return (secStartPixel + secLengthSign * (AbsSecLength - (segPixelNum - prevCount) - 1));
            }
        }
    }
    //if we don't find the pixel across all the segments, we return the dLed value
    //when passed into setPixelColor, this value will be safely ignored
    return dLed;
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
//satur and value set saturation and value of the fastLed CHSV function
CRGB segDrawUtils::wheel( byte wheelPos, uint8_t offset, uint8_t satur, uint8_t value ){
    CRGB rgbColor;
    wheelPos = (uint16_t)(255 - wheelPos + offset) % 255;
    hsv2rgb_rainbow( CHSV(wheelPos, satur, value), rgbColor );
    return rgbColor;
}

CRGB segDrawUtils::wheel( byte wheelPos, uint8_t rainbowOffset) { 
    return wheel( wheelPos, rainbowOffset, 255, 255);
}

//turns all pixel in a segment set off
void segDrawUtils::turnSegSetOff(SegmentSet segmentSet){
    fillSegSetColor(segmentSet, 0, 0);
}

//fills and entire segment set with a color
//ie all its segments and all their sections
void segDrawUtils::fillSegSetColor(SegmentSet segmentSet, CRGB color, uint8_t colorMode){
    for(int i = 0; i < segmentSet.numSegs; i++){
        fillSegColor(segmentSet, i, color, colorMode);
    }
}

// fills a segment with a specific color
void segDrawUtils::fillSegColor(SegmentSet segmentSet, byte segNum, CRGB color, uint8_t colorMode) {
    byte numSecs = segmentSet.getTotalNumSec(segNum);
    // run through segment's sections, fetch the startPixel and length of each, then color each pixel
    for (int i = 0; i < numSecs; i++) {
        fillSegSecColor(segmentSet, segNum, i, color, colorMode );
    }
}

//fills a segment section with a specific color 
//(ex: a segment section has three sub sections: {1, 4} , {8, 3}, {14, 8}) (so 4 pixels starting at 1, 3 starting at 8, and 8 starting at 14)
//this function fills all sections with a color
void segDrawUtils::fillSegSecColor(SegmentSet segmentSet, uint8_t segNum, uint16_t secNum, CRGB color, uint8_t colorMode ){
    uint16_t startPixel = segmentSet.getSecStartPixel(segNum, secNum);
    if(startPixel != dLed){
        int16_t length = segmentSet.getSecLength(segNum, secNum);
        int8_t step = (length > 0) - (length < 0); // account for negative lengths
        for (int i = startPixel; i != (startPixel + length); i += step) {
            setPixelColor(segmentSet, i, color, colorMode, segNum, getLineNumFromPixelNum(segmentSet, i - startPixel, segNum));
        }
    }
}

//fills in a length of a segment with a color, using a start and end pixel
//pixel numbers are local to the segment, not global. ie 1-8th pixel in the segment
void segDrawUtils::fillSegLengthColor(SegmentSet segmentSet, CRGB color, uint8_t colorMode, uint16_t segNum, uint16_t startSegPixel, uint16_t endPixel){
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
void segDrawUtils::fillSegSetlengthColor(SegmentSet segmentSet, CRGB color, uint8_t colorMode, uint16_t startSegPixel, uint16_t endPixel){
    uint16_t startData[2], endData[2];
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
void segDrawUtils::drawSegLine(SegmentSet segmentSet, byte lineNum, byte Pattern[], CRGB pallet[], uint8_t colorMode, uint8_t bgColorMode, boolean brReplace) {
    byte numSegs = segmentSet.numSegs;
    drawSegLineSection(segmentSet, 0, numSegs - 1, lineNum, Pattern, pallet, colorMode, bgColorMode, brReplace);
}

// draws a line between segments, from startSeg to endSeg (including endSeg), does its best to make a straight line
// the segment colors follow the provided pattern of pallet indecies
// the pattern length must match the number of segments!
// if brReplace is true, any part of the pattern with a value of zero (ie first element in the pallet) will be treated as a background colored pixel
// and will use bgColorMode for it's color mode (same modes as colorMode)
// if you don't want this, set brReplace to false
void segDrawUtils::drawSegLineSection(SegmentSet segmentSet, uint8_t startSeg, uint8_t endseg, byte lineNum, byte Pattern[], CRGB pallet[], uint8_t colorMode, uint8_t bgColorMode, boolean brReplace) {
    uint16_t maxSegLength = segmentSet.maxSegLength;
    uint16_t pixelNum = 0;
    CRGB colorFinal = 0;
    uint8_t colorModeTemp = colorMode;
    for (int i = startSeg; i <= endseg; i++) {
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
void segDrawUtils::drawSegLineSimple(SegmentSet segmentSet, byte lineNum, CRGB color, uint8_t colorMode) {
    byte numSegs = segmentSet.numSegs;
    drawSegLineSimpleSection(segmentSet, 0, numSegs - 1, lineNum, color, colorMode);
}

// draws a segment line of one color betweeb startSeg and endSeg (including endSeg)
// does not need a pallet or pattern,
void segDrawUtils::drawSegLineSimpleSection(SegmentSet segmentSet, uint8_t startSeg, uint8_t endSeg, byte lineNum, CRGB color, uint8_t colorMode) {
    uint16_t maxSegLength = segmentSet.maxSegLength;
    uint16_t pixelNum = 0;
    for (int i = startSeg; i <= endSeg; i++) { // for each segment, set the color, if we're in rainbow mode, set the rainbow color
        pixelNum = getPixelNumFromLineNum(segmentSet, maxSegLength, i, lineNum);
        setPixelColor(segmentSet, pixelNum, color, colorMode, i, lineNum);
    }
} 

//fills in a pixelInfoPS struct with data (the pixel's actual address, what segment it's in, it's line number, and what color it should be)
//(the passed in struct is empty and will be filled in, but you must provide it, 
//the segPixelNum is the number of the pixel you want the info for)
//you're probably calling this to account for different color modes before manipulating or storing a color
//this gives you all the data to call the full setPixelColor() directly
//the passed in color will be set to struct color if color mode is zero
//segPixelNum is local to the segment set (ie 10th pixel in the whole set)
void segDrawUtils::getPixelColor(SegmentSet segmentSet, pixelInfoPS *pixelInfo, CRGB color, uint8_t colorMode, uint16_t segPixelNum){
    uint16_t locData[2];
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

//sets pixel colors (same as other setPixelColor funct)
//doesn't need lineNum as argument. If lineNum is needed, it will be determined
//note segPixelNum is local to the segement set (ie 5th pixel in the whole set)
void segDrawUtils::setPixelColor(SegmentSet segmentSet, uint16_t segPixelNum, CRGB color, uint8_t colorMode){
    uint16_t locData[2];
    getSegLocationFromPixel(segmentSet, segPixelNum, locData);
    setPixelColor(segmentSet, locData[1], locData[0], color, colorMode);
}

//note segPixelNum is local to the segment (ie 5th pixel in the segment)
void segDrawUtils::setPixelColor(SegmentSet segmentSet, uint16_t segPixelNum, uint8_t segNum, CRGB color, uint8_t colorMode){
    uint8_t lineNum = 0;
    uint16_t pixelNum = getSegmentPixel(segmentSet, segNum, segPixelNum);
    if(colorMode == 1){
        lineNum = getLineNumFromPixelNum(segmentSet, segPixelNum, segNum);
    }
    setPixelColor(segmentSet, pixelNum, color, colorMode, segNum, lineNum);
}

// Sets color of target pixel (actual address of the led, not local to segment)
// Color modes are as follows:
// mode 0: use passed in color
// mode 1: rainbow spread across the longest segment
// mode 2: use the bgSeg gradient
// mode 3: use the non segmented BgGradient
// mode 4: rainbow spread across all segments
// mode 5: rainbow spread across whole strip
// mode 6: rainbow spread across whole all active leds in the segment set
// mode 7: produces a single color that cycles through the rainbow over global time at fixedRBRate,
//         used to color a whole effect as a single color that cycles through the rainbow
void segDrawUtils::setPixelColor(SegmentSet segmentSet, uint16_t pixelNum, CRGB color, uint8_t colorMode, uint8_t segNum, uint8_t lineNum){
    if( pixelNum == dLed || !segmentSet.getSegActive(segNum) ) return; //if we are given a dummy pixel, don't try to color it
    CRGB colorFinal = 0;
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

// returns the color of a pixel based on the color mode and other variables
// you pass in a color incase the color is not changed
// Color modes are as follows:
// mode 0: use passed in color
// mode 1: rainbow spread across the longest segment
// mode 2: use the bgSeg gradient
// mode 3: use the non segmented BgGradient
// mode 4: rainbow spread across all segments
// mode 5: rainbow spread across whole strip
// mode 6: rainbow spread across whole all active leds in the segment set
// mode 7: produces a single color that cycles through the rainbow over global time at fixedRBRate,
//         used to color a whole effect as a single color that cycles through the rainbow
// mode 8: Same as mode 7, but the direction of the cycle is reversed
CRGB segDrawUtils::getPixelColor(SegmentSet segmentSet, uint16_t pixelNum, CRGB color, uint8_t colorMode, uint8_t segNum, uint8_t lineNum){
    CRGB colorFinal = 0;
    uint16_t containerVar;
    switch(colorMode) { 
        case 0: //shouldn't ever be calling this
            colorFinal = color;
            break;
        case 1:// colors each line according to a rainbow mapped to the longest segment
            containerVar = segmentSet.maxSegLength;
            colorFinal = wheel((uint16_t(lineNum) * 256 / containerVar) & 255, segmentSet.rainbowOffset, segmentSet.rainbowSatur, segmentSet.rainbowVal );
            break;
        case 2: // colors each line to match the BgGradient segmented
            colorFinal = color;
            //colorFinal = getSegBgGradientColor(segmentSet, segNum);
            break;
        case 3: // colors each line to match the BgGradient
            colorFinal = color;
            //colorFinal = getBgGradientColor(pixelNum);
            break;
        case 4:// colors each line according to a rainbow spread across all segments
            containerVar = segmentSet.numSegs;
            colorFinal = wheel((segNum * 256 / containerVar) & 255, segmentSet.rainbowOffset, segmentSet.rainbowSatur, segmentSet.rainbowVal );
            break;
        case 5: // colors each line according to a rainbow spread across whole strip
            containerVar = segmentSet.numLeds;
            colorFinal = wheel((pixelNum * 256 / containerVar) & 255, segmentSet.rainbowOffset,  segmentSet.rainbowSatur, segmentSet.rainbowVal );
            break;
        case 6: // colors each line according to a rainbow spread across the active segments
            containerVar = segmentSet.numActiveSegLeds;
            colorFinal = wheel((pixelNum * 256 / containerVar) & 255, segmentSet.rainbowOffset, segmentSet.rainbowSatur, segmentSet.rainbowVal );
            break;
        case 7: //produces a single color that cycles through the rainbow over global time at fixedRBRate 
                //used to color a whole effect as a single color that cycles through the rainbow
            containerVar = millis()/fixedRBRate;
            colorFinal = wheel( containerVar % 255, 0 );
            break;
        case 8: //produces a single color that cycles through the rainbow over global time at fixedRBRate 
                //but is reversed from mode 7 above
                //used to color a whole effect as a single color that cycles through the rainbow
            containerVar = millis()/fixedRBRate;
            colorFinal = wheel( 255 - (containerVar % 255), 0 );
            break;
        default: 
            colorFinal = color;
            break;
    }
    return colorFinal;
}

//returns a color that is blended/cross-faded between a start and end color according to the ratio of step/totalSteps
//maximum value of totalSteps is 255 (since the color components are 0-255 uint8_t's)
CRGB segDrawUtils::getCrossFadeColor(CRGB startColor, CRGB endColor, uint8_t step, uint8_t totalSteps){
    uint8_t ratio = (uint16_t)step * 255 / totalSteps;
    return getCrossFadeColor(startColor, endColor, ratio);
}

//returns a color that is blended/cross-faded between a start and end color according to the ratio
//the ratio is between 0 and 255, 255 being the total conversion to the end color
CRGB segDrawUtils::getCrossFadeColor(CRGB startColor, CRGB endColor, uint8_t ratio){
    //am using the built in fastLed function for blending
    //the commented code after is an alternative way to do it, but yeilds the same result in most cases
    return blend( startColor, endColor, ratio );

    // CRGB result;
    // result.r = startColor.r + ( (int16_t)( endColor.r - startColor.r ) * ratio) ) / 255;
    // result.g = startColor.g + ( (int16_t)( endColor.g - startColor.g ) * ratio) ) / 255;
    // result.b = startColor.b + ( (int16_t)( endColor.b - startColor.b ) * ratio) ) / 255;

    // return CRGB(result.r, result.g, result.b);
}

//returns an interpolated value between two 8 bit uint's according to the ratio
//the ratio is between 0 and 255, 255 being the total conversion to the end int
//useful for bledning of individual RGB color components
uint8_t segDrawUtils::getCrossFadeColorComp(uint8_t startColor, uint8_t endColor, uint8_t ratio){
    if(startColor == endColor) return startColor;

    if(startColor > endColor){
        return startColor - (int16_t)(startColor - endColor) * ratio / 255;
    } else{
        return startColor + (int16_t)(endColor - startColor) * ratio / 255;
    }
}

//returns a random color
CRGB segDrawUtils::randColor(){
  return CRGB( random8(), random8(), random8() );
}

//fades an entire segment set to black by a certain percentage (out of 255)
//uses FastLED's fadeToBlackBy function
void segDrawUtils::fadeSegSetToBlackBy(SegmentSet segmentSet, uint8_t val){
    for(int i = 0; i < segmentSet.numSegs; i++){
        fadeSegToBlackBy(segmentSet, i, val);
    }
}


//fades an entire segment to black by a certain percentage (out of 255)
//uses FastLED's fadeToBlackBy function
void segDrawUtils::fadeSegToBlackBy(SegmentSet segmentSet, uint8_t segNum, uint8_t val) {
    byte numSecs = segmentSet.getTotalNumSec(segNum);
    // run through segment's sections, fetch the startPixel and length of each, then color each pixel
    for (int i = 0; i < numSecs; i++) {
        fadeSegSecToBlackBy(segmentSet, segNum, i, val);
    }
}

//fades an entire segment section to black by a certain percentage (out of 255)
//(ex: a segment section has three sub sections: {1, 4} , {8, 3}, {14, 8}) (so 4 pixels starting at 1, 3 starting at 8, and 8 starting at 14)
//this function fades one section to black
//uses FastLED's fadeToBlackBy function
void segDrawUtils::fadeSegSecToBlackBy(SegmentSet segmentSet, uint8_t segNum, uint16_t secNum, uint8_t val){
    uint16_t startPixel = segmentSet.getSecStartPixel(segNum, secNum);
    if(startPixel != dLed){
        int16_t length = segmentSet.getSecLength(segNum, secNum);
        int8_t step = (length > 0) - (length < 0); // account for negative lengths
        for (int i = startPixel; i != (startPixel + length); i += step) {
            segmentSet.leds[i].fadeToBlackBy(val);
        }
    }
}
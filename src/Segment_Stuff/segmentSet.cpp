#include "segmentSet.h"

SegmentSet::SegmentSet(struct CRGB *segLeds, uint16_t segAllLeds, Segment **segmentsArr, uint8_t numSegments) :
    numSegs(numSegments), segArr(segmentsArr), leds(segLeds), numLeds(segAllLeds)	
	{
		maxSegLength = getMaxSegLength();	
		segNumMaxSegLength = getsegNumMaxSegLength();
		numActiveSegLeds = getNumActiveSegLeds();
		resetGradVals();
		
		//create a default gradient palette
		CRGB *newPalette_arr = new CRGB[2];
        newPalette_arr[0] = CRGB(230, 15, 230); //purple
        newPalette_arr[1] = CRGB(0, 200, 0); //green
        paletteTemp = {newPalette_arr, 2};
        gradPalette = &paletteTemp;

		offsetRate = &offsetRateOrig;
	}

//resets the gradient vars to their defaults
void SegmentSet::resetGradVals(){
	gradLenVal = numLeds;
	gradLineVal = maxSegLength;
	gradSegVal = numSegs;
}

//returns the maxium length across all active segments
uint16_t SegmentSet::getMaxSegLength(void){
  	uint16_t maxSegLength = 0;
  	uint16_t totalLength;
  	//walk across all the segments
  	//if the segment length is greater than the current maxSegLength, store it as the maxSegLength
  	for(uint8_t i = 0; i < numSegs; i++ ){
		if(getSegActive(i)){
			totalLength = getTotalSegLength(i);
			if( totalLength > maxSegLength ){
				maxSegLength = totalLength;
			}
		}
  	}
	//record the segment with the maximum segment length
	segNumMaxSegLength = getsegNumMaxSegLength();
  	return maxSegLength;
}

//returns the segment number that has the maxium length across all the segments
uint16_t SegmentSet::getsegNumMaxSegLength(void){
  	uint16_t totalLength;
  	//walk across all the segments
  	//if the segment length is equal to the maxSegLength, return it
  	for(int i = 0; i < numSegs; i++ ){
	  	totalLength = getTotalSegLength(i);
	  	//if we find the segment with the right length, break out of the loop and return it
	  	if( totalLength == maxSegLength ){
			segNumMaxSegLength = i;
			break; 
	  	}
  	}
  	return segNumMaxSegLength;
}

//returns the total number of leds in the segment set that in active segments
uint16_t SegmentSet::getNumActiveSegLeds(void){
  	uint16_t ledCount = 0;
   	for(int i = 0; i < numSegs; i++ ){
	   	if(getSegActive(i)){
	  		ledCount += getTotalSegLength(i);
 		}
   	}
 	return ledCount;
}

//returns a pointer to the specified segment instance
Segment* SegmentSet::getSegPtr(uint8_t segNum){
	return *(segArr + segNum);
}

//returns the specified segment's totalLength (segNum is the segment's index in the segments array passed to the constructor)
uint16_t  SegmentSet::getTotalSegLength(uint8_t segNum){
	return getSegPtr(segNum)->totalLength; //secNum = 0, it's not needed for the call
}

//returns the specified segment's number of sections
uint16_t  SegmentSet::getTotalNumSec(uint8_t segNum){
	return getSegPtr(segNum)->numSec;//secNum = 0, it's not needed for the call
}

// //returns the pointer to the specified segment
// uint16_t  SegmentSet::getSectionPtr(uint8_t segNum){
// 	getSegPtr(segNum);
// }

//!!Only works for segments with continuous sections (not mixed sections!)
//returns the start pixel of the specified section in the specified segment (secNum is the index of the section within the segment array)
//ie the start pixel of the 0th section in the 0th segment
uint16_t  SegmentSet::getSecStartPixel(uint8_t segNum, uint8_t secNum){
	return getSegPtr(segNum)->getSecStartPixel(secNum);
}

//!!Only works for segments with mixed sections (not continuous sections!)
//returns the physcial led location of the pixel in the passed in segment number and segment pixel number
//ie the 5th pixel in the 0th segment
uint16_t  SegmentSet::getSecMixPixel(uint8_t segNum,  uint16_t pixelNum ){
	return getSegPtr(segNum)->getSecMixPixel(pixelNum);
}

//returns the length of the specified section in the specified segment	
int16_t  SegmentSet::getSecLength(uint8_t segNum, uint8_t secNum){
	return getSegPtr(segNum)->getSecLength(secNum);
}

//returns the direction of the specified segment
bool SegmentSet::getSegDirection(uint8_t segNum){
	return getSegPtr(segNum)->dirct;
}

//returns the active var for the specified segment
bool SegmentSet::getSegActive(uint8_t segNum){
	return getSegPtr(segNum)->active;
}

//returns the pointer to the specified segment's section array
segmentSecCont* SegmentSet::getSecArrPtr(uint8_t segNum){
	return getSegPtr(segNum)->secPtr;
}

//returns the pointer to the specified segment's section mixed array
segmentSecMix* SegmentSet::getSecMixArrPtr(uint8_t segNum){
	return getSegPtr(segNum)->secMixPtr;
}

//sets the direction of the specified segment to the specified direction
void SegmentSet::setSegDirection(uint8_t segNum, bool direction){
	getSegPtr(segNum)->dirct = direction;
}

//sets all the segments in the segmentSet to the specified direction
void SegmentSet::setAllSegDirection(bool direction){
	for(int i = 0; i < numSegs; i++ ){
		setSegDirection(i, direction);
	}
}

//flips the direction of every freq segment, starting with the first segment according to startAtFirst
//ie if you have three segments and do flipSegDirectionEvery(2, true), the first and third segment will be flipped
//but if you do flipSegDirectionEvery(2, false), only the second segment will be flipped
void SegmentSet::flipSegDirectionEvery(uint8_t freq, bool startAtFirst){
	bool currentDirect;
	//run over the segments and check if they match the flip frequency,
	//if they do, flip the direction
	for(int i = 0; i < numSegs; i++ ){
		if( checkSegFreq(freq, i, startAtFirst) ){
			currentDirect = getSegDirection(i);
			setSegDirection(i, !currentDirect);
		}
	}
}

//sets the direction of every freq segment, starting with the first segment according to startAtFirst
//ie if you have three segments and do setsegDirectionEvery(2, true, true), the first and third segment will be be set to true
//but if you do flipSegDirectionEvery(2, true, false), only the second segment will be set to true
void SegmentSet::setsegDirectionEvery(uint8_t freq, bool direction, bool startAtFirst){
	//run over the segments and check if they match the flip frequency,
	//if they do, set the direction
	for(int i = 0; i < numSegs; i++ ){
		if( checkSegFreq(freq, i, startAtFirst) ){
			setSegDirection(i, direction);
		}
	}
}

//checks if a given segment number occurs in the segmentSet at the given freq, accouting for if the 
//count is started at the first segment or not
//ie checkDirectionFlip(2, 1, true) will return false, b/c we are checking for every other segment, and we started with the first
//but, checkDirectionFlip(2, 1, false)	will return true, b/c the we are not starting on the first segment
bool SegmentSet::checkSegFreq(uint8_t freq, uint8_t num, bool startAtFirst){
	uint8_t segNum;
	uint8_t testNum;
	//if we're checking the first segment, and startAtFirst is true, we automatically
	//return true
	if(startAtFirst && num == 0){
		return true;
	}
	
	//b/c 0 % (any num) == 0, if we want to start at first, we just use the passed in num
	//otherwise we need to skip ahead one, as we are offsetting from the first segment
	if( startAtFirst ){
		testNum = num;
	} else {
		testNum = num + 1;
	}
	
	//if the segment occurs at the freq, and it isn't off the end of the segmentSet
	//then return true
	//testNum % freq
	if( mod8(testNum, freq) == 0 && segNum < numSegs){
		return true;
	} else {
		return false;
	}
}

//attempts to adjust the current brightness to the passed in brightness
//FastLed only allows you to dim/brighten leds by a certain percent (out of 255) from their current color
//so to get to the new brightness level, we need to adjust the brightness relative to the current brightness
//since all brightness values are a value out of 255, the adjustment should be the difference between the 
//current and new brightness
//we then dim/brighten based on the sign of the difference
void SegmentSet::setBrightness(uint8_t newBrightness){
	if( newBrightness == brightness){
		return;
	}
	//abs((int16_t)(255 - (int16_t)(newBrightness * 255)/ brightness)) ?
	uint8_t percentDiff = abs(newBrightness - brightness);
	if(newBrightness < brightness){
		fadeToBlackBy(leds, numLeds, percentDiff);
	} else {
		fadeLightBy(leds, numLeds, percentDiff);
	}
	brightness = newBrightness;
}

void SegmentSet::setSegActive(uint8_t segNum, bool state){
	bool isActive = getSegActive(segNum);
	if(isActive != state){
		Segment *ptr = *(segArr + segNum);
		ptr->active = state;
		if(isActive){
			numActiveSegLeds += getTotalSegLength(segNum);
		} else {
			numActiveSegLeds -= getTotalSegLength(segNum);
		}
	}
}
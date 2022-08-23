#include "segment.h"

//Constructor for creating a segment with a normal section
Segment::Segment( uint8_t numSections, segmentSecCont *segSectionArr, bool direction, bool segActive) :
    numSec(numSections), secPtr(segSectionArr), dirct(direction), active(segActive)
	{
		totalLength = getSegTotLen();
	}

//Constructor for creating a segment with a mixed section
//note that the you are limited to one mixed section
//This is because a mixed section can contain any number of pixels in any order,
//so you don't need more than one section
Segment::Segment(segmentSecMix *segSecMix, bool direction, bool segActive) :
    secMixPtr(segSecMix), dirct(direction), active(segActive)
	{
    numSec = 1;
		totalLength = getSegTotLen();
	}

//returns the total length of the segment by summing the length of each segment section
uint16_t Segment::getSegTotLen(){
    uint16_t totalLength = 0;
    for(int i = 0; i < numSec; i++ ){
        totalLength += abs(getSecLength(i));
    }
    return totalLength;
}

//!!!!Only works for continuous segment sections, not mixed sections
//returns the start pixel of the specified section (secNum is the index of the section within the segmentSecCont array)
//b/c sections are stored in flash, we need to use pgm_read_word to fetch their properites
//the start pixel is a uint16_t, pgm_read_word reads 16bit words.
uint16_t Segment::getSecStartPixel( uint8_t secNum ){
	return pgm_read_word( &( secPtr + secNum )->startPixel);
}

//!!!!Only works for mixed sections, not continuous sections
//Returns the physical address of the pixel at pixelNum in the mixed section pixel array
//accounts for the segment direction
//b/c sections are stored in flash, we need to use pgm_read_word to fetch their properites
//the start pixel is a uint16_t, pgm_read_word reads 16bit words.
uint16_t Segment::getSecMixPixel( uint16_t pixelNum ){
    if(!dirct){
        //count backwards from the length if the segment is reversed
        pixelNum = pgm_read_word(&secMixPtr->length) - pixelNum - 1;
    }
    return pgm_read_word(&secMixPtr->pixArr[pixelNum]);
}

//returns the length of the specified section (secNum is the index of the section within the segmentSections array)
//b/c sections are stored in flash, we need to use pgm_read_word to fetch their properites
//the length is a uint16_t, pgm_read_word reads 16bit words.
int16_t Segment::getSecLength( uint8_t secNum ){
    //check if the secPtr is not null, if so then we must have normal segment sections
    //otherwise we must have a mixed section pointer
    if(secPtr){
        return pgm_read_word( &( secPtr + secNum )->length);
    } else {
        return pgm_read_word( &secMixPtr->length );
    }
}

#include "Segment.h"

//Constructor for creating a segment with a normal section
Segment::Segment(const segmentSecCont *segSectionArr,  uint8_t numSections, bool direction) :
    secPtr(segSectionArr), numSec(numSections), direct(direction)
	{
		init(numSections);
	}

//Constructor for creating a segment with a mixed section
//note that the you are limited to one mixed section
//This is because a mixed section can contain any number of pixels in any order,
//so you don't need more than one section
Segment::Segment(const segmentSecMix *segSecMix, uint8_t numSections, bool direction) :
    secMixPtr(segSecMix), numSec(numSections), direct(direction)
	{
        init(numSections);
	}

//Initializes core variables for the segment
//and checks if the segment has any single sections
void Segment::init(uint8_t numSections){
    totalLength = getSegTotLen();
    
    //check if any of the sections are single, if they are, flag the segment as having a single section
    //(so it gets caught in segDrawUtils::show())
    for(uint8_t i = 0; i < numSec; i++ ){
        if(secPtr){
            if(pgm_read_word( &( secPtr + i )->single) ){
               hasSingle = true;
               break;
            } 
        } else {
            if(pgm_read_word( &( secMixPtr + i )->single) ){
               hasSingle = true;
               break;
            }
        }
    }
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
//b/c sections are stored in flash, we need to use pgm_read_word to fetch their properties
//the start pixel is a uint16_t, pgm_read_word reads 16bit words.
uint16_t Segment::getSecStartPixel( uint8_t secNum ){
	return pgm_read_word( &( secPtr + secNum )->startPixel);
}

//!!!!Only works for mixed sections, not continuous sections
//Returns the physical address of the pixel at pixelNum in the mixed section pixel array
//accounts for the segment direction
//b/c sections are stored in flash, we need to use pgm_read_word to fetch their properties
//the start pixel is a uint16_t, pgm_read_word reads 16bit words.
uint16_t Segment::getSecMixPixel( uint8_t secNum, uint16_t pixelNum ){
    return pgm_read_word(&(secMixPtr + secNum)->pixArr[pixelNum]);
}

//Returns the value of the "single" var for the specified section
//This indicates if the section is to be treated as a single pixel or not
bool Segment::getSecIsSingle(uint8_t secNum){
    if(secPtr){
        return (pgm_read_word( &( secPtr + secNum )->single));
    } else {
        return (pgm_read_word( &( secMixPtr + secNum )->single));
    }
}

//returns the length of the specified section (secNum is the index of the section within the segmentSections array)
//b/c sections are stored in flash, we need to use pgm_read_word to fetch their properties
//the length is a uint16_t, pgm_read_word reads 16bit words.
//If the segment is being treated as a single pixel, it returns 1 as the sec length
int16_t Segment::getSecLength( uint8_t secNum ){
    //Check if the secPtr is not null, if so then we must have normal segment sections
    //otherwise we must have a mixed section pointer
    //Then we check if the section is single
    if(secPtr){
        if(pgm_read_word( &( secPtr + secNum )->single)){
            return 1;
        } else {
            return pgm_read_word( &( secPtr + secNum )->length);
        }
    } else {
        if(pgm_read_word( &( secMixPtr + secNum )->single)){
            return 1;
        } else {
            return pgm_read_word( &( secMixPtr + secNum )->length );
        }
    }
}

//returns the length of the section, disregards if the section is being treated as a single pixel
//(Used to set all the section's pixel colors)
int16_t Segment::getSecTrueLength( uint8_t secNum ){
    if(secPtr){
        return pgm_read_word( &( secPtr + secNum )->length);
    } else {
        return pgm_read_word( &( secMixPtr + secNum )->length );
    }
}

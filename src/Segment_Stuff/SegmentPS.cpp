#include "SegmentPS.h"

//Constructor for creating a segment with a normal section
SegmentPS::SegmentPS(const segmentSecCont *segSecContArr, uint8_t NumSec, bool Direct)
    : secContPtr(segSecContArr), numSec(NumSec), direct(Direct)  //
{
    init();
}

//Constructor for creating a segment with a mixed section
//note that the you are limited to one mixed section
//This is because a mixed section can contain any number of pixels in any order,
//so you don't need more than one section
SegmentPS::SegmentPS(const segmentSecMix *segSecMixArr, uint8_t NumSec, bool direction)
    : secMixPtr(segSecMixArr), numSec(NumSec), direct(direction)  //
{
    init();
}

//Initializes core variables for the segment
//and checks if the segment has any single sections
void SegmentPS::init() {
    totalLength = getSegTotLen();

    //check if any of the sections are single, if they are, flag the segment as having a single section
    //(so it gets caught in segDrawUtils::show())
    for( uint8_t i = 0; i < numSec; i++ ) {
        if( secContPtr ) {
            if( pgm_read_word(&(secContPtr + i)->single) ) {
                hasSingle = true;
                break;
            }
        } else {
            if( pgm_read_word(&(secMixPtr + i)->single) ) {
                hasSingle = true;
                break;
            }
        }
    }
}

//returns the total length of the segment by summing the length of each segment section
//Treats single sections as length 1
uint16_t SegmentPS::getSegTotLen() {
    uint16_t totalLength = 0;
    for( int i = 0; i < numSec; i++ ) {
        totalLength += abs(getSecLength(i));
    }
    return totalLength;
}

//!!!!Only works for continuous segment sections, not mixed sections
//returns the start pixel of the specified section (secNum is the index of the section within the segmentSecCont array)
//b/c sections are stored in flash, we need to use pgm_read_word to fetch their properties
//the start pixel is a uint16_t, pgm_read_word reads 16bit words.
uint16_t SegmentPS::getSecStartPixel(uint8_t secNum) {
    return pgm_read_word(&(secContPtr + secNum)->startPixel);
}

//!!!!Only works for mixed sections, not continuous sections
//Returns the physical address of the pixel at pixelNum in the mixed section pixel array
//accounts for the segment direction
//b/c sections are stored in flash, we need to use pgm_read_word to fetch their properties
//the start pixel is a uint16_t, pgm_read_word reads 16bit words.
uint16_t SegmentPS::getSecMixPixel(uint8_t secNum, uint16_t pixelNum) {
    return pgm_read_word(&(secMixPtr + secNum)->pixArr[pixelNum]);
}

//Returns the value of the "single" var for the specified section
//This indicates if the section is to be treated as a single pixel or not
bool SegmentPS::getSecIsSingle(uint8_t secNum) {
    if( secContPtr ) {
        return (pgm_read_word(&(secContPtr + secNum)->single));
    } else {
        return (pgm_read_word(&(secMixPtr + secNum)->single));
    }
}

//returns the length of the specified section (secNum is the index of the section within the segmentSectionsPS array)
//b/c sections are stored in flash, we need to use pgm_read_word to fetch their properties
//the length is a uint16_t, pgm_read_word reads 16bit words.
//If the segment is being treated as a single pixel, it returns 1 as the sec length
int16_t SegmentPS::getSecLength(uint8_t secNum) {
    //Check if the secContPtr is not null, if so then we must have normal segment sections
    //otherwise we must have a mixed section pointer
    //Then we check if the section is single
    if( secContPtr ) {
        if( pgm_read_word(&(secContPtr + secNum)->single) ) {
            return 1;
        } else {
            return pgm_read_word(&(secContPtr + secNum)->length);
        }
    } else {
        if( pgm_read_word(&(secMixPtr + secNum)->single) ) {
            return 1;
        } else {
            return pgm_read_word(&(secMixPtr + secNum)->length);
        }
    }
}

//returns the length of the section, disregards if the section is being treated as a single pixel
//(Used to set all the section's pixel colors)
int16_t SegmentPS::getSecTrueLength(uint8_t secNum) {
    if( secContPtr ) {
        return pgm_read_word(&(secContPtr + secNum)->length);
    } else {
        return pgm_read_word(&(secMixPtr + secNum)->length);
    }
}
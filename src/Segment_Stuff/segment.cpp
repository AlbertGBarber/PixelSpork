#include "segment.h"

Segment::Segment( uint8_t numSections, segmentSection *segSectionArr, boolean direction, boolean segActive) :
    numSec(numSections), secPtr(segSectionArr), dirct(direction), active(segActive)
	{
		totalLength = getSegTotLen();
	}

//returns the total length of the segment by summing the length of each segment section
uint16_t  Segment::getSegTotLen(){
  uint16_t totalLength = 0;
  for(int i = 0; i < numSec; i++ ){
    totalLength += abs(getSecLength( i ));
  }
  return totalLength;
}

//returns the start pixel of the specified section (secNum is the index of the section within the segmentSection array)
//b/c sections are stored in flash, we need to use pgm_read_word to fetch their properites
//the start pixel is a uint16_t, pgm_read_word reads 16bit words.
uint16_t  Segment::getSecStartPixel( uint8_t secNum ){
	return pgm_read_word( &( secPtr + secNum )->startPixel);
}

//returns the length of the specified section (secNum is the index of the section within the segmentSection array)
//b/c sections are stored in flash, we need to use pgm_read_word to fetch their properites
//the length is a uint16_t, pgm_read_word reads 16bit words.
int16_t  Segment::getSecLength( uint8_t secNum ){
	return pgm_read_word( &( secPtr + secNum )->length);
}

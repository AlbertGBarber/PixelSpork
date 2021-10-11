#ifndef segment_h
#define segment_h

#include "segmentSection.h"

//for class explanation see segmentSet.h
class Segment {
	
	public:
		Segment(uint8_t numSections, segmentSection *segSectionsArr, boolean direction, boolean segActive = true);
		
	  uint8_t
		numSec;
	 
	  boolean
	  	active = true,
		dirct;
		
	  uint16_t
	    getSecStartPixel( uint8_t secNum ),
		totalLength;
		
	  int16_t
		getSecLength( uint8_t secNum );
		
	  segmentSection 
		*secPtr;
	  
	private:	
	  uint16_t 
	    getSegTotLen();
};

#endif
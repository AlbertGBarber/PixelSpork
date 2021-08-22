#ifndef segment_h
#define segment_h

#include "segmentSection.h"

//for class explanation see segmentSet.h
class Segment {
	
	public:
		Segment(byte numSections, segmentSection *segSectionsArr, boolean direction, boolean segActive = true);
		
	  byte
		numSec;
	 
	  boolean
	  	active = true,
		dirct;
		
	  uint16_t
	    getSecStartPixel( byte secNum ),
		totalLength;
		
	  int16_t
		getSecLength( byte secNum );
		
	  segmentSection 
		*secPtr;
	  
	private:	
	  uint16_t 
	    getSegTotLen();
};

#endif
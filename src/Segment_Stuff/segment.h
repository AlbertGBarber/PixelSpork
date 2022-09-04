#ifndef segment_h
#define segment_h

#include "segmentSection.h"

//for class explanation see segmentSet.h
class Segment {
	
	public:
		//Constructor for creating a segment with a continuous section
		Segment(uint8_t numSections, segmentSecCont *segSectionsArr, bool direction = true, bool segActive = true);

		//Constructor for creating a segment with a mixed section
		Segment(uint8_t numSections, segmentSecMix *segSecMix, bool direction = true, bool segActive = true);
		
	  	uint8_t
			numSec;
	 
	  	bool
		  	hasSingle = false,
	  		active = true,
			dirct,
			getSecIsSingle(uint8_t secNum); //Returns the sections "single" bool var
		
	  	uint16_t
	    	getSecStartPixel( uint8_t secNum ), //only works with continuous sections!!
			getSecMixPixel( uint8_t secNum, uint16_t pixelNum ), //only works with mixed sections!!
			totalLength;
		
		int16_t
			getSecLength( uint8_t secNum ), //Returns the length of the section, "single" sections will be returned as 1
			getSecTrueLength( uint8_t secNum ); //Returns the length of the section, disregards the section's "single" setting
		
		//Pointers to the two types of segment section (see segmentSections.h)
		//We can onyl have one type of section per segment, which is set in the constructor
		//so one of these will always be null (this is important to checking what type of section is in the segment)
	  	segmentSecCont 
			*secPtr = nullptr;
	
		segmentSecMix 
			*secMixPtr = nullptr;
	  
	private:

	  	uint16_t 
	    	getSegTotLen();
		
		void
			init(uint8_t numSections);
};

#endif
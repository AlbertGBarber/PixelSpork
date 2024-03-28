#ifndef SegmentPS_h
#define SegmentPS_h

#include "segmentSectionsPS.h"

//Needed to VALIDLY handle program memory pointer access in both C and C++
//Used for accessing mixed segments pixel numbers.
//See more info here: 
//https://community.arduboy.com/t/progmem-functions-pgm-read-float-and-pgm-read-ptr/5771
//(I don't know how template stuff works, this is a life-saving copy-paste)
template<typename T> T * pgm_read_pointer_PS(T * const * pointer)
{
	return reinterpret_cast<T *>(pgm_read_ptr(pointer));
}

//for class explanation see SegmentSetPS.h
class SegmentPS {

    public:
        //Constructor for creating a segment with a continuous section
        SegmentPS(const segmentSecCont *segSecContArr, uint8_t NumSec, bool Direct = true);

        //Constructor for creating a segment with a mixed section
        SegmentPS(const segmentSecMix *segSecMixArr, uint8_t NumSec, bool Direct = true);

        uint8_t
            numSec;

        bool
            hasSingle = false,
            direct,
            getSecIsSingle(uint8_t secNum);  //Returns the sections "single" bool var

        uint16_t
            getSecStartPixel(uint8_t secNum),                   //only works with continuous sections!!
            getSecMixPixel(uint8_t secNum, uint16_t pixelNum),  //only works with mixed sections!!
            totalLength;                                        //Total length of all sections, treating "single" sections as length 1

        int16_t
            getSecLength(uint8_t secNum),      //Returns the length of the section, "single" sections will be returned as 1
            getSecTrueLength(uint8_t secNum);  //Returns the length of the section, disregards the section's "single" setting

        //Pointers to the two types of segment section (see segmentSectionsPS.h)
        //We can only have one type of section per segment, which is set in the constructor
        //so one of these will always be null (this is important to checking what type of section is in the segment)
        const segmentSecCont
            *secContPtr = nullptr;

        const segmentSecMix
            *secMixPtr = nullptr;
        
        //Pointer for accessing mixed section pixel array
        const uint16_t
            *pixArrPtr;

    private:

        uint16_t
            getSegTotLen();

        void
            init();
};

#endif
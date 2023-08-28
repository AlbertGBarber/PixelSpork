#ifndef segmentSectionsPS_h
#define segmentSectionsPS_h

//need to include arduino here to get it to compile
#if ARDUINO >= 100
    #include "Arduino.h"
#else
    #include "WProgram.h"
    #include "pins_arduino.h"
    #include "WConstants.h"
#endif

/* 
There are two different section types, stored as structs
They are used to build segments. Each segment is composed of one type of section.
(see SegmentSetPS.h for more info)

Each section type works best for separate situations:
    The first type is a length of continuous pixels (segmentSecCont)
    It is created by specifying a starting pixel and a length
    ie a section of length 5 starting at pixel number 3 would be 3, 4, 5, 6, 7
    You may have multiple sections per segment, ie you could have a segment with two sections
    where you skipped over some length of pixels in the segment.
    You can also set the length to negative,
    which will cause the section to be read backwards, but you must set the start pixel to the final pixel in the section
    ie in the example above, if the length was -5, the start pixel would be 7
    This is mainly to account for physical strip arrangements where parts of the strip need to be wired in reverse
    Overall, this section type works best for long lengths of pixels, where all of them are in order
    Such as specifying an entire strip, or splitting the strip into multiple sections

    The second type is a mixed section (segmentSecMix)
    This contains an array of pixel addresses, and the length of the array (number of pixels in the section)
    The pixel addresses can be in any order, 
    but you can only have one section per segment (you shouldn't need more than one)
    and the length must be positive
    ie a section containing pixel numbers 3, 6, 8, 10, etc, with a set length
    This type of section is good for when you want a segment made of mostly single pixels

Creating segments with efficient sections will help your code run faster, and also hopefully make it more clear
Hopefully the choice of section will be obvious in most cases. 
The worst case is if you have a bunch of short (longer than 1) sections. 
Unless you have a lot of pixels, it's probably better to use a mixed section in the worst case
because mixed sections will run faster, but do require more memory.  
*/

//A length of continuous pixels, starting at the start pixel and running for the set length
struct segmentSecCont {
    uint16_t startPixel;
    int16_t length;
    //used to mark if the segment should be treated as a single pixel in effects
    bool single;  //Will be default initialized to 0 if omitted from the section definition in your code
                  //I've looked it up, it seems to be true, although some sources say it will be undefined
                  //But so far it's worked in all my code....
                  //You can't add a default value (ie single = false) b/c the base Arduino compiler doesn't like it (ok for ESP boards tho)

    //support for direction of segment section, not implemented elsewhere
    //avoid using due to increase in memory usage
    //instead, if you have a section facing the wrong way, use a negative section length, with the final pixel as the start pixel
    //bool direct;
};

//A section containing an array of pixel addresses. These do not have to be continuous.
//example definition:
//const PROGMEM uint16_t pixel_arr[] = {0, 2, 1};
//const PROGMEM segmentSecMix segmentSec = { pixel_arr, SIZE(pixel_arr) };
//SegmentPS segment0 = { segmentSec, true };
struct segmentSecMix {
    uint16_t *pixArr;
    uint16_t length;
    //used to mark if the segment should be treated as a single pixel in effects
    bool single;  //Will be default initialized to 0 if omitted from the section definition in your code
                  //(see comment in segmentSecCont above)
};

#endif
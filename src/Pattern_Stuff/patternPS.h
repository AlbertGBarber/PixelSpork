#ifndef patternPS_h
#define patternPS_h

#if ARDUINO >= 100
    #include "Arduino.h"
#else
    #include "WConstants.h"
    #include "WProgram.h"
    #include "pins_arduino.h"
#endif

/* 
A struct for holding a uint8_t array and it's length in one place
this makes passing it to effects and functions easier
(and allows you to do shenanigans by changing the length artificially)
See patternUtils.h for interacting with patterns

generally a pattern will be paired with a palette in an effect, where the
pattern entries will be the indicies of the colors in the palette
ie pattern of {0, 1, 2} would correspond to the first, second, and third colors in the palette

note that the patternArr is a pointer to an array of CRGB colors
this is b/c structs don't allow you to have variable length arrays
If you change this pointer to target a new array YOU MUST set the patterns
"length" and "maxLength" accordingly.

example pattern:
uint8_t pattern_arr[] = {0, 2, 1};
patternPS pattern = { pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr) };
The second SIZE() is used to record the maximum size of the palette array for memory management.
It should always be the same as the actual size of the array. */
struct patternPS {
    uint8_t *patternArr;
    uint16_t length;  
    //The total length of the patternArr, used for memory management
    //To prevent memory fragmentation, as long as the patternArr is large enough,
    //the code will often just adjust the "length" of a pattern, without re-sizing the patternArr.
    //This "hides" the extra patternArr entries, but keeps the memory usage consistent
    //maxLength records the actual length of the patternArr.
    //Note that you can set a pattern to always re-size by setting alwaysResizeObj_PS = true (see Include_Lists -> GlobalVars)
    uint16_t maxLength;
};

#endif
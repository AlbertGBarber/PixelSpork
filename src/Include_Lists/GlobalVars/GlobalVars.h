#ifndef GlobalVars_h
#define GlobalVars_h

//A file for global variables and constants, makes it easy to include them in whatever.

#define D_LED 65535
//D_LED is used to indicate a dummy led (max of uint16_t)
//Pixels with this value will be ignored by color setting functions
//This is critical for handling cases where you try to find the address of a pixel that doesn't exist in the segment set
//ex for a segment set with 300 pixels, trying to get the address of the 301st
//We must be able to return a location that is impossible for all segment sets
//(we can't just return 301 because that could be an actual address in the segment set)
//Hence we use the constant D_LED as a global impossible address marker
//This limits the maximum segment set length to 65534, but that's still waaay more pixels than any current MC can handle

//Warning DO NOT USE D_LED for marking dummy leds in segment sets because some effect's need to read colors from LEDs
//You must create real leds off the strip, see the wiki page Advanced Segment Usage for more.

extern bool alwaysResizeObjPS; //defaulted to false in GlobalVars.cpp
/*
If true, then whenever an effect, utility, etc tries to create an object on the heap (ie using new or malloc)
the code will check the size of the existing object. Only creating a new object if it needs more space.
If false, a new object will always be created.

If you're only running one effect at a time, and not changing it settings during runtime you don't need to worry about this.
Likewise, any effects where fragmentation may be an issue have a note telling you.

!!!NOTE, this does not apply to palettes, which are always -re-created. This was done because usually palettes
are only allocated once at the start of the effect or at the start of your code. Controlling their max size is tedious because
they exist outside of effects. Generally you use a different palette rather than re-sizing the old one.

More Explanation:
If the current object is large enough, instead of creating a new object, we want the current one to be re-used
For example, lets say I want to store an array of LED locations, creating they array dynamically using new.
When I first create the array, I'll record how large it is.
Later, If I want to change the number of LED locations I'm storing, I check if the current array is large enough.
If it isn't, then I free() the array, and re-create it with new.
If it is large enough, then I leave the array as is, but tweak other limits in my code to treat it as smaller.
This helps limit heap fragmentation by limiting how often I'm re-allocating memory.
However this can lead to more memory usage because we may be storing larger objects than needed. 
Memory limitations and heap fragmentation can both cause random crashes, but I have choosen to limit fragmentation more.
If you really need to optimize your memory usage, you can alwaysResizeObjPS to true.

Changing alwaysResizeObjPS will effect:
    Any dynamically sized array or struct created by an effect,
    this includes things like patterns, particleSets, etc
*/

#endif
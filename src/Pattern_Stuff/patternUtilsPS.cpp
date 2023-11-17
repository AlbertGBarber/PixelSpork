#include "patternUtilsPS.h"

using namespace patternUtilsPS;

//sets the pattern Val at the specified index
//the index wraps, so running off the end of the pattern, will put you back at the start
void patternUtilsPS::setVal(patternPS &pattern, uint8_t val, uint16_t index) {
    pattern.patternArr[mod16PS(index, pattern.length)] = val;
}

//returns the Val at a specified index
//the index wraps, so running off the end of the pattern, will put you back at the start
uint8_t patternUtilsPS::getPatternVal(patternPS &pattern, uint16_t index) {
    return pattern.patternArr[mod16PS(index, pattern.length)];
}

//returns a random value from the pattern
uint8_t patternUtilsPS::getRandVal(patternPS &pattern) {
    return pattern.patternArr[random8(pattern.length)];
}

//returns a pointer to the Val in the pattern array at the specified index (wrapping if needed)
//useful for background Val syncing
//Note: maybe remove the wrapping?,
uint8_t *patternUtilsPS::getValPtr(patternPS &pattern, uint16_t index) {
    return &(pattern.patternArr[mod16PS(index, pattern.length)]);
}

//Reverses the order of the pattern colors
//Using the common uint16One etc vars makes this hard to read,
//but it's essentially the same as the reverse() in the paletteUtils
void patternUtilsPS::reverse(patternPS &pattern) {
    uint16One = 0;                   //start index
    uint16Two = pattern.length - 1;  //end index

    //To reverse the pattern we use a common algo
    //We basically swap pairs of values, starting with the end and start of the array, and then moving inwards
    //Once we reach the center of the array, we stop
    while( uint16One < uint16Two ) {
        uint8One = getPatternVal(pattern, uint16One);  //get the start index value
        uint8Two = getPatternVal(pattern, uint16Two);  //get the end index value
        setVal(pattern, uint8Two, uint16One);          //set the end value into the start index
        setVal(pattern, uint8One, uint16Two);          //set the start value into the end index
        //move to the next pair of indexes
        uint16One++;
        uint16Two--;
    }
}

//Randomizes the order of values in a pattern.
//Note that it does not check against the current order, so it's possible to get the same pattern order back. 
//The likely-hood of this is 1/(pattern.length!), ie 1/3! => 1/6 for a pattern length of 3. 
void patternUtilsPS::shuffle(patternPS &pattern) {
    uint16One = pattern.length - 1;  //loop starting val
    //Start from the last element and swap one by one at random 
    //i > 0 is the loop limit b/c we don't need to swap the first element.
    for( uint16_t i = uint16One; i > 0; i-- ) {
        // Pick a random index from 0 to i
        uint16Two = random16(i + 1);  //randIndex
        // Swap arr[i] with the element
        // at random index
        uint8One = getPatternVal(pattern, uint16Two);  //get the value at the random index
        uint8Two = getPatternVal(pattern, i);          //get the value of the current index
        setVal(pattern, uint8Two, uint16Two);          //swap the random index to the current one
        setVal(pattern, uint8One, i);                  //swap current index to the random one
    }
}

//Retuns a random value from the pattern
//Works by picking a random starting index and then looping through the pattern until it finds a 
//value different than the current value, "currentPatternVal".
//Most patterns contain short-ish lengths of repeating values so there shouldn't be too many loop cycles.
//DO NOT use this with a pattern that has all the same values as it will loop through all of them
//and then just spit back out the "currentPatternVal" when none are different.
uint8_t patternUtilsPS::getShuffleVal(patternPS &pattern, uint8_t currentPatternVal) {
    uint16One = random16(pattern.length);  //pick a random starting index

    //Loop through the pattern, offset by the starting index and check pattern values
    //if the value is different than the current value, return it
    //This ensures that you always get a new value back
    for( uint16_t i = 0; i < pattern.length; i++ ) {
        //get pattern val, offset by our random start point (patterns wrap so it won't run off the end of the pattern)
        uint8One = patternUtilsPS::getPatternVal(pattern, i + uint16One); 

        //if we find value different than the current one, we're done and can return it
        if( uint8One != currentPatternVal ) {
            return uint8One;
        }

    }

    //catch return for if the pattern is all one value
    return currentPatternVal;
}

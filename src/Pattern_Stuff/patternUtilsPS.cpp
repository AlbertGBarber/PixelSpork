#include "patternUtilsPS.h"

using namespace patternUtilsPS;

//sets the pattern Val at the specified index
//the index wraps, so running off the end of the pattern, will put you back at the start
void patternUtilsPS::setVal(patternPS *pattern, uint8_t val, uint16_t index){
    pattern->patternArr[ mod16PS( index, pattern->length ) ] = val;
}

//returns the Val at a specified index
//the index wraps, so running off the end of the pattern, will put you back at the start
uint8_t patternUtilsPS::getPatternVal(patternPS *pattern, uint16_t index){
    return pattern->patternArr[ mod16PS( index, pattern->length ) ];
}

//returns a random value from the pattern
uint8_t patternUtilsPS::getRandVal(patternPS *pattern){
    return pattern->patternArr[ random8(pattern->length) ];
}

//returns a pointer to the Val in the pattern array at the specified index (wrapping if needed)
//useful for background Val syncing
//Note: maybe remove the wrapping?, 
uint8_t* patternUtilsPS::getValPtr(patternPS *pattern, uint16_t index){
    return &(pattern->patternArr[ index % pattern->length ]);
}

//randomizes the order of Vals in a pattern
void patternUtilsPS::shuffle(patternPS *pattern){
    uint8_t randVal, tempVal, randIndex;
    uint8_t loopStart = pattern->length - 1;
    // Start from the last element and swap
    // one by one. We don't need to run for
    // the first element that's why i > 0
    for(int i = loopStart; i > 0; i--){
        // Pick a random index from 0 to i
        randIndex = random(i + 1);
        // Swap arr[i] with the element
        // at random index
        tempVal = getPatternVal(pattern, i);
        randVal = getPatternVal(pattern, randIndex);
        setVal(pattern, tempVal, randIndex);
        setVal(pattern, randVal, i);
    }
}

//retuns a random value from the pattern
//the code checks to see if the random index matches the current index (passed in)
//if it does we'll just advance the index by one and return that
//this stops the same color from being chosen again (assuming the pattern doesn't repeat)
uint8_t patternUtilsPS::getShuffleIndex(patternPS *pattern, uint8_t currentPatternVal){
    uint16_t indexGuess = random16(pattern->length);
    uint8_t guessVal = patternUtilsPS::getPatternVal(pattern, indexGuess);
    if( guessVal == currentPatternVal ){
        return patternUtilsPS::getPatternVal(pattern, indexGuess + 1);
    } else {
        return guessVal;
    }
}

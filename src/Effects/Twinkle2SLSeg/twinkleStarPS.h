#ifndef twinkleStarPS_h
#define twinkleStarPS_h

#if ARDUINO >= 100
    #include "Arduino.h"
#else
    #include "WConstants.h"
    #include "WProgram.h"
    #include "pins_arduino.h"
#endif

//A structure for holding a twinkle
//Twinkles are single pixels of a single color that fades in and out
struct twinkleStarPS {
    uint16_t location;     //where the twinkle is on the segments
    CRGB color;            //its color
    uint8_t fadeInSteps;   //how many steps it takes to fade in
    uint8_t fadeOutSteps;  //how many steps it takes to fade out

    //pre-set vars
    uint16_t stepNum = 0;  //the inital step number out of fadeInSteps
    bool active = false;   //if the twinkle is on or not

    //Constructor
    twinkleStarPS(uint16_t Location, CRGB Color, uint8_t FadeInSteps, uint8_t FadeOutSteps )
        : location(Location), color(Color), fadeInSteps(FadeInSteps), fadeOutSteps(FadeOutSteps) //  
    {
        stepNum = 0;
        active = false;
    }
};

/* A struct for holding an array of twinkles
to declare twinklesSet:
    twinklePS *twinkleArr[] = { &twinkle1, &twinkle2, etc};
    twinkleSetPS twinkleSet = {twinkleArr, SIZE(twinkleArr), SIZE(twinkleArr)};

    The second SIZE() is used to record the maximum size of the twinkle array for memory management.
    It should always be the same as the actual size of the array.

    access a twinkle's property via: twinkleSet.twinkleArr[x]->property.
    See code in Twinkle2PS and wiki twinkleStar page for more snippets. */
struct twinkleSetPS {
    twinkleStarPS **twinkleArr;  //pointer to an array of twinkles pointers
    uint16_t length;             //the size of the twinkle array (num of twinkles)

    uint16_t maxLength;  //the maximum length of the twinkle array (used for memory management)

    //Returns the pointer to a twinkle at the specified index
    //Note that the function wraps if your `index` is too large, so you always get a twinkle back.
    twinkleStarPS *getTwinkle(uint16_t index){
        return twinkleArr[mod16PS(index, length)];
    }

    //sets twinkle in the array to the passed in twinkle at the specified index
    void setTwinkle(twinkleStarPS *twinkle, uint16_t index) {
        twinkleArr[mod16PS(index, length)] = twinkle;
    };

    //Sets all twinkles to be inactive and their fade `stepNum` to 0 
    //(doesn't touch their location b/c there's no "neutral" location).
    void reset(){
        for( uint16_t i = 0; i < length; i++ ) {
            twinkleArr[i]->active = false;
            twinkleArr[i]->stepNum = 0;
        }
    };

    //Frees the memory of the twinkle set by freeing all the twinkles and the twinkle array. 
    //**Should only be use if the twinkle set was dynamically allocated (ie using malloc() or "new")**
    void deleteTwinkleSet(){
        if( twinkleArr ) {  //check that the twinkle set array exists
            //we need to delete all the twinkles in the set before deleting the twinkle array
            for( uint16_t i = 0; i < maxLength; i++ ) {
                free(twinkleArr[i]);
            }
            free(twinkleArr);
        }
    };
};

#endif
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
    CRGB color;            //it's color
    uint8_t fadeInSteps;   //how many steps it takes to fade in
    uint8_t fadeOutSteps;  //how many steps it takes to fade out

    //pre-set vars
    uint16_t stepNum = 0;  //the inital step number out of fadeInSteps
    bool active = false;   //if the twinkle is on or not
};

/* A struct for holding an array of twinkles
to declare twinklesSet:
    twinklePS *twinkleArr[] = { &twinkle1, &twinkle2, etc}; //can leave this blank and fill it in using buildTwinklesSet() below
    twinkleSetPS twinkleSet = {twinkleArr, SIZE(twinkleArr), SIZE(twinkleArr)};

    The second SIZE() is used to record the maximum size of the twinkle array for memory management.
    It should always be the same as the actual size of the array.

    access a twinkle's property via: twinkleSet.twinkleArr[x]->property.
    See code in Twinkle2PS for more snippets. */
struct twinkleSetPS {
    twinkleStarPS **twinkleArr;  //pointer to an array of twinkles pointers
    uint16_t length;             //the size of the twinkle array (num of twinkles)

    uint16_t maxLength;  //the maximum length of the twinkle array (used for memory management)

    //sets twinkle in the array to the passed in twinkle at the specified index
    void setTwinkle(twinkleStarPS *twinkle, uint16_t index) {
        twinkleArr[mod16PS(index, length)] = twinkle;
    };
};

#endif
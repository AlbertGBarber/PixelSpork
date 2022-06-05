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
    uint16_t location; //where the twinkle is on the segments
    CRGB color; //it's color
    uint8_t fadeInSteps; //how many steps it takes to fade in
    uint8_t fadeOutSteps; //how many steps it takes to fade out

    //pre-set vars
    uint16_t stepNum = 0; //the inital step number out of fadeInSteps
    bool active = false; //if the twinkle is on or not
};

//a struct for holding an array of twinkles
//to declare twinklesSet:
    //twinklePS *twinkleArr[] = { &twinkle1, &twinkle2, etc}; //can leave this blank and fill it in using buildtwinklesSet() below
    //twinkleSetPS twinkleSet = {twinkleArr, SIZE(twinkleArr)};
    //access a twinkle's property via: twinkleSet.twinkeArr[x]->property.
    //See code in Twinkle2PS for more snippets.
struct twinkleSetPS {
    twinkleStarPS **twinkleArr; //pointer to an array of twinkles pointers
    uint8_t length; //the size of the twinkle array (num of twinkles)

    //sets twinkle in the array to the passed in twinkle at the specified index
    void setTwinkle(twinkleStarPS *twinkle, uint8_t index){
        twinkleArr[index % length] = twinkle;
    };

};

#endif
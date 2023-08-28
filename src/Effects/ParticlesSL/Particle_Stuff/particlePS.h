#ifndef particlePS_h
#define particlePS_h

#include "FastLED.h"

#if ARDUINO >= 100
    #include "Arduino.h"
#else
    #include "WConstants.h"
    #include "WProgram.h"
    #include "pins_arduino.h"
#endif

//represents a single moving pixel particle
//the particle has a direction, speed, and can leave a trail
//Trails blend cleanly into the background color over the trail length
//(like waving a flame around, or a meteor trail)
//Trails have a minimum length of 1 (turned on/off with trail mode)
//Particles have a minimum size of 1
//Trail options:
//    0: no trails
//    1: one trail facing away from the direction of motion (like a comet)
//    2: two trails, facing towards both directions of motion
//    3: one trail facing towards the direction of motion
//    4: Infinite trails that persist after the particle
//For example, with a trail length of 4, the modes will produce:
//(The trail head is *, - are the trail, particle is moving to the right ->)
//  0:     *
//  1: ----*
//  2: ----*----
//  3:     *----
//  4: *****
struct particlePS {
    uint16_t startPosition;  //initial position of the particle on reset
    bool direction;          //the direction of motion (true moves towards the last pixel in the segmentSet)
    uint16_t speed;          //the update rate of the particle (in ms)
    uint16_t size;           //the length of the main body of the particle (min size 1)
    uint8_t trailType;       //the type of trail for the particle (see above for types)
    uint8_t trailSize;       //the length of the trail(s) of the particle (only applies if the pixel has a trail)
    bool bounce;             //sets if the particle wraps to the beginning of the SegmentSetPS once it reaches the end, or if it reverses direction (it bounces)
    uint8_t colorIndex;      //all particles use a palette for color, this is the index of the color in the palette

    //the below variables are generally not set manually, and are used to keep track of the state of the particle
    uint16_t position = startPosition;  //the current position of the particle in the SegmentSetPS (not the actual location on the strip!)
    unsigned long lastUpdateTime = 0;   //the last time the particle was moved

    //for decaying particles
    //life is in ms
    uint16_t life = 3000;
    uint16_t maxLife = life;
};

//a struct for holding an array of particles
//most of the time you are manipulating these rather than individual particles directly
//most of the utility functions in particleUtilsPS.h focus on working with particles in particleSets
//to declare particleSet:
//    //I highly recommend using the particleUtils buildParticleSet() function to create particle sets.
//    //Just remember you must free the set's memory using freeParticleSet() once you are finished with it.

//    //To create a set manually:
//    //particlePS *particleArr[] = { &particle1, &particle2, etc}; //can leave this blank and fill it in using buildParticleSet() below
//    //particleSetPS particleSet = {particleArr, SIZE(particleArr), SIZE(particleArr)};
//    //The second SIZE() is used to record the maximum size of the particle array for memory management.
//    //It should always be the same as the actual size of the array.
struct particleSetPS {
    particlePS **particleArr;  //pointer to an array of particle pointers
    uint8_t length;            //the size of the particle array (num of particles)

    uint8_t maxLength;  //the maximum length of the particle array set (used for memory management)

    //returns the pointer to a particle at the specified index
    particlePS *getParticle(uint8_t index) {
        return particleArr[mod8(index, length)];
    };

    //sets particle in the array to the passed in particle at the specified index
    void setParticle(particlePS *particle, uint8_t index) {
            particleArr[mod8(index, length)] = particle;
    };
};

#endif
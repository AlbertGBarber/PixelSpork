#ifndef particleUtilsPS_h
#define particleUtilsPS_h

#include "particlePS.h"
#include "ColorUtils/colorUtilsPS.h"

//Particles need a bit of an explanation.
//A particle is a moving pixel. Particles can move backwards or forwards along the strip. They move at their own speeds.
//There are a number of other settings to do with trails, particle size, bouncing, etc (see particle.h)
//Because working with just one particle at a time would be boring, I've created particleSets (see particle.h)
//which are storage for a group of particles
//You pass these sets into effects (or the effects create them) and then the effects animate them
//Because particles has a number of options, creating them manually is a pain
//To aleviate this, I've created a number of utility functions for changing particle values
//and creating particle sets
//These functions also give you the option of choosing the values at random, to create random sets of particles

//buildParticleSet() is the main function for creating a set of particles
//while setParticleSetProp() can change a single quality of all the particles in a set 
//(say you wanted to change all their speeds, etc)
//there are then sub-functions for changing a specific particle's aspects within a set

//undestanding these functions will help you manipulate particles efficiently (see .cpp file for comments)

//series of utility functions for interacting with particles
namespace particleUtilsPS{

    //main build particleSet function
    particleSetPS
        buildParticleSet(uint8_t numParticles, uint16_t maxPosition, uint8_t direction, uint16_t baseSpeed, 
                        uint16_t speedRange, uint16_t size, uint16_t sizeRange, uint8_t trailType, uint8_t trailSize, 
                        uint8_t trailRange, uint8_t bounce, uint8_t colorIndex, bool randColor);

    //randomizes the particles in a particle set
    void 
        randomizeParticleSet(particleSetPS *particleSet, uint16_t maxPosition, uint8_t direction, uint16_t baseSpeed, 
                            uint16_t speedRange, uint16_t size, uint16_t sizeRange, uint8_t trailType, uint8_t trailSize, 
                            uint8_t trailRange, uint8_t bounce, uint8_t colorIndex, bool randColor);

    //randomizes properties of a specific particle in a particle set
    void
        randomizeParticle(particleSetPS *particleSet, uint8_t partNum, uint16_t maxPosition, uint8_t direction, uint16_t baseSpeed, 
                                       uint16_t speedRange, uint16_t size, uint16_t sizeRange, uint8_t trailType, uint8_t trailSize, 
                                       uint8_t trailRange, uint8_t bounce, uint8_t colorIndex, bool randColor);

    //functions for deleting or reseting particles
    void 
        resetParticle(particlePS *particle),
        resetParticleset(particleSetPS *particleSet),
        resetParticle(particleSetPS *particleSet, uint8_t partNum),
        deleteParticle(particleSetPS *particleSet, uint8_t partNum),
        deleteAllParticles(particleSetPS *particleSet);

    //functions for changing particle properties
    void
        setParticleSetProp(particleSetPS *particleSet, uint8_t propNum, uint16_t opt1, uint16_t opt2, uint16_t opt3),
        setParticleSetPosition(particleSetPS *particleSet, uint8_t partNum, uint16_t position, bool rand),
        setParticleSetDirection(particleSetPS *particleSet, uint8_t partNum, uint8_t direction),
        setParticleSetSpeed(particleSetPS *particleSet, uint8_t partNum, uint16_t baseSpeed, uint16_t range),
        setParticleSetSize(particleSetPS *particleSet, uint8_t partNum, uint16_t size, uint16_t range),
        setParticleSetTrailType(particleSetPS *particleSet, uint8_t partNum, uint8_t trailType),
        setParticleSetTrailSize(particleSetPS *particleSet, uint8_t partNum, uint8_t trailSize, uint8_t range),
        setParticleSetBounce(particleSetPS *particleSet, uint8_t partNum, uint8_t bounce),
        setParticleSetColor(particleSetPS *particleSet, uint8_t partNum, uint8_t colorIndex, bool randColor);

    //functions for more precise trail mode setting
    void
        setAllTrailRand(particleSetPS *particleSet, bool noTrails, 
                        bool oneTrail, bool twoTrail, bool revTrail, bool infTrail),
        setTrailRand(particleSetPS *particleSet, uint8_t partNum, bool noTrails, 
                    bool oneTrail, bool twoTrail, bool revTrail, bool infTrail);
                
    int8_t
        getDirectStep(bool direction);

    //for setting colors of particles       
    CRGB
        getTrailColor(CRGB color, CRGB targetColor, uint8_t step, uint8_t totalSteps, int8_t dimPow);

    //pre-allocate variables for speed
    static uint8_t
        dimRatio,
        particleSetLength;
    
    static bool
        randColor;
};

#endif
#include "particleUtilsPS.h"
//TODO: .Add function to grow/shrink trails
//      .Change the rates based on external var
//      .Give particles their own color modes?

using namespace particleUtilsPS;

//resets a given particle back to it's starting position and sets it's update time to 0
void particleUtilsPS::resetParticle(particlePS *particle){
    particle->position = particle->startPosition;
    particle->lastUpdateTime = 0;
}

//resets a given particle in a particle set back to it's starting position and sets it's update time to 0
void particleUtilsPS::resetParticle(particleSetPS *particleSet, uint8_t partNum){
    resetParticle(particleSet->particleArr[partNum]);
}

//resets a set of particles back to their starting position and sets their update time to 0
void particleUtilsPS::resetParticleset(particleSetPS *particleSet){
    particleSetLength = particleSet->length;
    for(uint8_t i = 0; i < particleSetLength; i++ ){
        resetParticle(particleSet, i);
    }
}       

//Fills in a given particle set with particles using the passed in options
//The option names are the same as used in the individual setParticle functions and do the same things
//ie maxPosition will be passed to setParticleSetPositions() as the maxPosition input
//See each of the individual functions for what the options do
//NOTE: the particles are created using new, so don't forget to call deleteParticle() or deleteAllParticles()
//if you are finished with a particle or particleSet
particleSetPS particleUtilsPS::buildParticleSet(uint8_t numParticles, uint16_t maxPosition, uint8_t direction, uint16_t baseSpeed, 
                                                uint16_t speedRange, uint16_t size, uint16_t sizeRange, uint8_t trailType, 
                                                uint8_t trailSize, uint8_t trailRange, uint8_t bounce, uint8_t colorIndex, 
                                                bool randColor){
    particlePS **particleArr = new particlePS*[numParticles];
    particleSetPS newParticleSet = { particleArr, numParticles };
    //create a new set of particles
    for(uint8_t i = 0; i < numParticles; i++ ){
        particlePS* p = new particlePS();
        newParticleSet.setParticle(p, i);
    }

    //set the particle properties
    randomizeParticleSet(&newParticleSet, maxPosition, direction, baseSpeed, speedRange, size, sizeRange, trailType, trailSize, trailRange, bounce, colorIndex, randColor);

    //sets the particle positions to their start positions
    //and sets their lastUpdateTime's to 0
    resetParticleset(&newParticleSet);

    return newParticleSet;
}

//Randomizes the properties of a set of particles
//The option names are the same as used in the individual setParticle functions and do the same things
//ie maxPosition will be passed to setParticleSetPositions() as the maxPosition input
//See each of the individual functions for what the options do
void particleUtilsPS::randomizeParticleSet(particleSetPS *particleSet, uint16_t maxPosition, uint8_t direction, uint16_t baseSpeed, 
                                       uint16_t speedRange, uint16_t size, uint16_t sizeRange, uint8_t trailType, uint8_t trailSize, 
                                       uint8_t trailRange, uint8_t bounce, uint8_t colorIndex, bool randColor){
    particleSetLength = particleSet->length;
    for(uint8_t i = 0; i < particleSetLength; i++ ){
        randomizeParticle(particleSet, i, maxPosition, direction, baseSpeed, speedRange, size, 
                          sizeRange, trailType, trailSize, trailRange, bounce, colorIndex, randColor);
    }
}

//Randomizes the properties of a particle (that is a member of a particle set)
//The option names are the same as used in the individual setParticle functions and do the same things
//ie maxPosition will be passed to setParticleSetPositions() as the maxPosition input
//See each of the individual functions for what the options do
void particleUtilsPS::randomizeParticle(particleSetPS *particleSet, uint8_t partNum, uint16_t maxPosition, uint8_t direction, uint16_t baseSpeed, 
                                       uint16_t speedRange, uint16_t size, uint16_t sizeRange, uint8_t trailType, uint8_t trailSize, 
                                       uint8_t trailRange, uint8_t bounce, uint8_t colorIndex, bool randColor){

    setParticleSetPosition(particleSet, partNum, maxPosition, true);
    setParticleSetDirection(particleSet, partNum, direction);
    setParticleSetSpeed(particleSet, partNum, baseSpeed, speedRange);
    setParticleSetSize(particleSet, partNum, size, sizeRange);
    setParticleSetTrailType(particleSet, partNum, trailType);
    setParticleSetTrailSize(particleSet, partNum, trailSize, trailRange);
    setParticleSetBounce(particleSet, partNum, bounce);
    setParticleSetColor(particleSet, partNum, colorIndex, randColor);
}

//sets a single property for all the particles in a set
//Useful as a shorthand way of changing all the particles
//The properties are set by propNum:
//  0: The start positions of the particle (randomizes them)
//  1: The directions of the particles
//  2: The speeds of the particiles
//  3: The size of the particles
//  4: The trail types of the particles
//  5: The trail lengths particles
//  6 The bounce property of the particles
//  7: The palette color index of the particles (pass the palette length as opt2 to choose randomly from the palette)
//This function basically just calls one of the setParticleSet() functions below
//The inputs opt1, opt2, otp3 are used as the inputs for the setParticleSet() functions in the same order as they appear
//So see those functions for exact explanations of their inputs
//set any unused opt's to 0
//For example: setParticleSetProp(particleSet, 0, 100, 0, 0);
//calls setParticleSetPositions() using 100 as the "max" value. opt2 and opt3 are not used
//Another example: setParticleSetProp(particleSet, 4, 1, 5, 2);
//calls setParticleSetTrails() using  1 (opt1) as the trailType, 5 (opt2) ad the trailSize, and 2 (opt3) as the range
void particleUtilsPS::setParticleSetProp(particleSetPS *particleSet, uint8_t propNum, uint16_t opt1, uint16_t opt2, uint16_t opt3){
    particleSetLength = particleSet->length;
    randColor = false;
    if(opt2 != 0){
        randColor = true;
    }
    for(uint8_t i = 0; i < particleSetLength; i++ ){
        switch (propNum) {
            case 0:
                setParticleSetPosition(particleSet, i, opt1, opt2);
                break;
            case 1:
                setParticleSetDirection(particleSet, i, opt1);
                break;
            case 2:
                setParticleSetSpeed(particleSet, i, opt1, opt2);
                break;
            case 3:
                setParticleSetSize(particleSet, i, opt1, opt2);
                break;
            case 4:
                setParticleSetTrailType(particleSet, i, opt1);
                break;
            case 5:
                setParticleSetTrailSize(particleSet, i, opt1, opt2);
                break;
            case 6:
                setParticleSetBounce(particleSet, i, opt1);
                break;
            case 7:
                setParticleSetColor(particleSet, i, opt1, randColor);
        }     
    }
}

//Sets a particle's startPosition to the passed in value
//if rand is true, it will be choosen randomly up to the passed in position
//(generally use the segmentSet length as the position for rand)
void particleUtilsPS::setParticleSetPosition(particleSetPS *particleSet, uint8_t partNum, uint16_t position, bool rand){
    if(rand){
        position = random16(position);
    }
    particleSet->particleArr[partNum]->startPosition = position;
}

//sets a particle's direction based on the passed in direction
//to set the direction randomly, pass in a value > 1
void particleUtilsPS::setParticleSetDirection(particleSetPS *particleSet, uint8_t partNum, uint8_t direction){
    if( direction > 1 ){
        direction = random8(2);
    }
    particleSet->particleArr[partNum]->direction = direction;
}

//sets a particle's speed to the passed in baseSpeed (ms)
//The speed is varied by a random amount no greater than the passed in range
//(the baseSpeed is the fastest a particle will go)
void particleUtilsPS::setParticleSetSpeed(particleSetPS *particleSet, uint8_t partNum, uint16_t baseSpeed, uint16_t range){
    particleSet->particleArr[partNum]->speed = baseSpeed + random16(range + 1);
}

//sets a particle's size to the passed in size
//The size is varied by a random amount no greater than the passed in range
void particleUtilsPS::setParticleSetSize(particleSetPS *particleSet, uint8_t partNum, uint16_t size, uint16_t range){
    //particles must have a minimum size of 1
    if(size == 0){
        size = 1;
    }
    particleSet->particleArr[partNum]->size = size + random16(range + 1);
}

//sets the type of a particle's trail (see particle.h for trail types)
//if you pass in a trailType > 4 (the highest trail type) the trails will be choosen randomly from the 
//first three trail types (0, 1, 2)
//If you want a specific set of trail types choosen randomly, use setParticleSetTrailRand()
void particleUtilsPS::setParticleSetTrailType(particleSetPS *particleSet, uint8_t partNum, uint8_t trailType){
    if(trailType > 4){
        trailType = random8(3);
    }
    particleSet->particleArr[partNum]->trailType = trailType;

}

//does setParticleSetTrailRand(), but for a whole set
void particleUtilsPS::setAllTrailRand(particleSetPS *particleSet, bool noTrails, 
                                              bool oneTrail, bool twoTrail, bool revTrail, bool infTrail){
    particleSetLength = particleSet->length;
    for(uint8_t i = 0; i < particleSetLength; i++ ){
        setTrailRand(particleSet, i, noTrails, oneTrail, twoTrail, revTrail, infTrail);
    }
}

//sets the type of a particle's trail (see particle.h for trail types) randomly according to the passed in flags
//each flag allows a trail type to be choosen randomly
//Flags:
//noTrails -- trail type 0
//oneTrail -- trail type 1
//twoTrail -- trail type 2
//revTrail -- trail type 3
//infTrail -- trail type 4
void particleUtilsPS::setTrailRand(particleSetPS *particleSet, uint8_t partNum, bool noTrails, 
                                              bool oneTrail, bool twoTrail, bool revTrail, bool infTrail){

    uint8_t trailType = 0;
    //booleans = 1 for true and 0 for false, so we will randomly pick from as many true flags as we have
    uint8_t randVal = random8(noTrails + oneTrail + twoTrail + revTrail + infTrail) + 1;

    //with the random value choosen, we need to match it to a trail
    //we go over each trail flag in order, reducing the randVal at each flag if it's set
    //once randVal is 0, we've reached the trail type that matches the randVal
    //Think of it like an extended while loop, where we seaching for the randVal, but only incrementing if the trail flag is set
    if(noTrails && randVal > 0){
        trailType = 0;
        randVal--;
    }
    if(oneTrail && randVal > 0){
        trailType = 1;
        randVal--;
    }
    if(twoTrail && randVal > 0){
        trailType = 2;
        randVal--;
    }
    if(revTrail && randVal > 0){
        trailType = 3;
        randVal--;
    }
    if(infTrail && randVal > 0){
        trailType = 4;
    }

    particleSet->particleArr[partNum]->trailType = trailType;

}

//sets the length of a particle's trail
//The trail length is varied by a random amount no greater than the passed in range
//Trails must have a minimum length of 1
void particleUtilsPS::setParticleSetTrailSize(particleSetPS *particleSet, uint8_t partNum, uint8_t trailSize, uint8_t range){
    if( trailSize == 0 ){
        trailSize = 1;
    }
    particleSet->particleArr[partNum]->trailSize = trailSize + random8(range + 1);
}

//sets a particle's bounc ebehavior on the passed in bounce value
//to set the bounce randomly, pass in a value > 1
void particleUtilsPS::setParticleSetBounce(particleSetPS *particleSet, uint8_t partNum, uint8_t bounce){
    if(bounce > 1){
        bounce = random8(2);
    }
    particleSet->particleArr[partNum]->bounce = bounce;
}

//sets a particle's colorIndex to the passed in value
//If you want the color to be randomly picked from a palette
//pass in the palette's length as colorIndex and set randColor as true
void particleUtilsPS::setParticleSetColor(particleSetPS *particleSet, uint8_t partNum, uint8_t colorIndex, bool randColor){
    if(randColor){
        colorIndex = random8(colorIndex);
    }
    particleSet->particleArr[partNum]->colorIndex = colorIndex;
}

//deletes a particle in a particleSet, should only be used if the particle was generated using new
//ie using the buildParticleSet() function
void particleUtilsPS::deleteParticle(particleSetPS *particleSet, uint8_t partNum){
    delete[] particleSet->particleArr[partNum];
}

//deletes all the particles in a particleSet, should only be used if the particles were generated using new
//ie using the buildParticleSet() function
void particleUtilsPS::deleteAllParticles(particleSetPS *particleSet){
    particleSetLength = particleSet->length;
    for(uint8_t i = 0; i < particleSetLength; i++ ){
        deleteParticle(particleSet, i);
    }
}

//for gettint particle trail colors
//returns a blended color towards the target color based on the input steps and totalSteps
//step == totalSteps is fully blended
//Note that we offset totalSteps by 1, so we never reach full blend (since it would produce background pixels)
//the maximum brightness is scaled by dimPow
//dimPow 0 will produce a normal linear gradient, but for more shimmery waves we can dial the bightness down
//dimpow of 80 gives a good effect
//The body of the particle will still be drawn at full brightness since it's drawn seperately 
CRGB particleUtilsPS::getTrailColor(CRGB color, CRGB targetColor, uint8_t step, uint8_t totalSteps, int8_t dimPow) {
    
    //dimRatio = ( (uint16_t)step * dimPow ) / (totalSteps + 1) ;
    
    //alternate dimming formula for more aggressive dimming (set dimPow between -127 and 127)
    //basically subtracts a term from the step value to simiulate an increase in dimming
    //the subtraction term decreases as we get closer to totalSteps, so we don't bug out and over run
    dimRatio = ( (uint16_t)step * (255 - dimPow) ) / (totalSteps + 1) + dimPow; 

    //ratio = dim8_video(ratio); 
    //dimRatio = triwave8( 128 * (uint16_t)step / (totalSteps) );

    return colorUtilsPS::getCrossFadeColor(color, targetColor, dimRatio);
}

//a quick function for getting direction multipliers based on a bool
//returns either 1 or -1
//uses the convention that true is forward (1) and false is backwards (-1)
int8_t particleUtilsPS::getDirectStep(bool direction){
    return direction - !direction;
}

/* uint8_t particleUtilsPS::getTrailBitwisePack(bool noTrails, bool oneTrail, bool twoTrail, bool revTrail, bool infTrail){
    uint8_t val;
    if(noTrails){
        bitSet(val, 0);
    }
    if(oneTrail){
        bitSet(val, 1);
    }
    if(twoTrail){
        bitSet(val, 2);
    }
    if(revTrail){
        bitSet(val, 3);
    }
    if(infTrail){
        bitSet(val, 4);
    }
    return val;
} */

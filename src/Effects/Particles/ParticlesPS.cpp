#include "ParticlesPS.h"

//constructor for automatically making a particle set according to the passed in options
ParticlesPS::ParticlesPS(SegmentSet &SegmentSet, palletPS *Pallet, CRGB BgColor, uint8_t numParticles, uint8_t direction, uint16_t baseSpeed, 
                        uint16_t speedRange, uint16_t size, uint16_t sizeRange, uint8_t trailType, uint8_t trailSize, 
                        uint8_t trailRange, uint8_t bounce, uint8_t colorIndex, bool randColor):
    segmentSet(SegmentSet), pallet(Pallet)
    {    
        init(BgColor);
        numLEDs = segmentSet.numActiveSegLeds;
        particleSetTemp = particleUtilsPS::buildParticleSet(numParticles, numLEDs, direction, baseSpeed, 
                                                            speedRange, size, sizeRange, trailType, trailSize, 
                                                            trailRange, bounce, colorIndex, randColor);
        particleSet = &particleSetTemp;
        trailEndColors = new CRGB[numParticles];
    }

//constructor for using a particle set you've already made
ParticlesPS::ParticlesPS(SegmentSet &SegmentSet, particleSetPS *ParticleSet, palletPS *Pallet, CRGB BgColor):
    segmentSet(SegmentSet), particleSet(ParticleSet), pallet(Pallet)
    {    
        init(BgColor);
        trailEndColors = new CRGB[particleSet->length];
    }

//destructor
//since particleSetTemp is only ever set using particleUtilsPS::buildParticleSet() (which makes everything using new)
//we need to clean up it's memory. This includes the particles themselves.
//However if particleSetTemp has not been set, then trying to clean it up will probably cause an error
//by default, the particleArr pointer is NULL, so we can check for that to confirm if particleSetTemp has been used or not
ParticlesPS::~ParticlesPS(){
    if(particleSetTemp.particleArr){
        particleUtilsPS::deleteAllParticles(&particleSetTemp);
        delete[] particleSetTemp.particleArr;
    }
    delete[] trailEndColors;
}

//initilizes the core variables of the effect
void ParticlesPS::init(CRGB BgColor){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    //The effect uses the rates of the particles, but all effects must have a Rate var, so we make one up
    uint16_t Rate = 80; 
    bindClassRatesPS();
    //bind background color pointer
    bindBGColorPS();
    //initial fill to clear out any previous effects
    segDrawUtils::fillSegSetColor(segmentSet, *bgColor, bgColorMode);
}

//sets the effect to use a new particle set
//we need to remake the trail end color array for the new particles
//to avoid having left over trails, we'll redraw the background
void ParticlesPS::setParticleSet(particleSetPS *newParticleSet){
    particleSet = newParticleSet;
    delete[] trailEndColors;
    trailEndColors = new CRGB[particleSet->length];
    segDrawUtils::fillSegSetColor(segmentSet, *bgColor, bgColorMode);
}

//resets all the particles to their start locations
void ParticlesPS::reset(){
    particleUtilsPS::resetParticleset(particleSet);
}

//Updates all the particles
//Outline:
//On each update cycle we check each particle to see if it should move, if so, we move it (dealing with bouncing and wrapping)
//Then, if the particle moved, we set the background color on the led that the particle has moved off of (the last pixel in the trail)
//Next, even if the particle has not moved, we re-draw the trails and the particle body
//This prevents another, faster particle from wiping out a slower on, only to have the slower one 
//suddenly re-appear once it's time to move it again
//To minimize the number of re-draws (we don't need to re-draw if no particles have moved) we limit the overall update rate
//to that of the fastest particle. This is re-checked every update.
//Bounce and end behavior:
//For a particle to bounce, it must reverse it's direction once it hits either end of the segmentSet
//However, how/when it bounces is a matter of opinion. I have opted for the following:
//The particle only bounces when it's main body (not trail) reaches an end point.
//This means that a trail in front of the particle disappears off the strip before the bounce. This was done to mimic the classic cylon eye look.
//However, trails behind the particle wrap back on themselves as the particle bounces. 
//This means that the trail naturally fades as like it would for a physical streamer/flame/etc
//For particles where the body size is larger than one, when bounce happens, the entire body reverses direction at once
//This is not visually noticable, and makes coding easier. But it does mean there's no "center" of a particle
void ParticlesPS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= updateRate ) {
        prevTime = currentTime;
        //need to reset the update rate each cycle, to make sure we always set it to the fastest particle
        updateRate = 65535; //max value of a uint16_t. This is an update rate of 65.5 sec.

        //refill the background if directed (if we're using a dynamic rainbow or something)
        if(fillBG || blend){
            segDrawUtils::fillSegSetColor(segmentSet, *bgColor, bgColorMode);
        }

        numParticles = particleSet->length;

        numLEDs = segmentSet.numActiveSegLeds;

        //for each particle, in order:
        //move it to it's next position (if needed)
        //then sets the pixel at the previous location to the BgColor (including trail ends)
        //then draws new trails if needed
        //then draws the particle in it's new position
        //This order helps prevent overwriting different parts of the particles if there are clashes
        //As a particle passes over another, we want to maintain the look of both particles as much as possible
        //This is why we always re-draw all the particles with each update, so that a fast particle doesn't erase a slower one
        //Also we pay special attention when setting the background pixel:
        //If a previous particle from the set has already overwritten the background pixel, then we don't need to set it
        //To track this, we record the color that we expect the background pixel to be (it's the last color in the trail/body)
        //Then when we go to set the background, we check the current color of the pixel
        //If it doesn't match, then we don't overwrite.
        for (uint8_t i = 0; i < numParticles; i++) {
            //get the particle from the set, and record some vars locally for ease of access
            particlePtr = particleSet->particleArr[i];
            position = particlePtr->position; //the current position of the particle
            speed = particlePtr->speed; //the speed of the patrticle 
            direction = particlePtr->direction; //the direction of motion (true moves towards the last pixel in the segementSet)
            size = particlePtr->size; //the length of the main body of the particle
            trailType = particlePtr->trailType; //the type of trail for the particle (see above for types)
            trailSize = particlePtr->trailSize; //the length of the trail(s) of the particle (only applies if the pixel has a trail)
            bounce = particlePtr->bounce; //sets if the particle wraps to the beginning of the segmentSet once it reaches the end, or if it reverses direction (it bounces)
            
            //record the fastest particle for accurate effect updating
            if( speed < updateRate ){
                updateRate = speed;
            }
            
            colorOut = palletUtilsPS::getPalletColor(pallet, particlePtr->colorIndex);

            //if we're bouncing, we don't want anything to wrap past the end/start of the strip
            //so we want to keep the modAmmount larger than the trailLedLocation can reach
            //ie the numLEDs plus the tailLength, as this is will allow the tail to extend off the strip,
            //which will be discarded by the setPixelColor func, as the pixel is out of bounds
            if (bounce) {
                modAmmount = numLEDs + trailSize + 1;
            } else {
                modAmmount = numLEDs;
            }

            //if enough time has passed, we need to move the particle
            movePart = ( ( currentTime - particlePtr->lastUpdateTime ) >= speed );
            //if the particle needs to move, move it and record the time
            if(movePart) {
                particlePtr->lastUpdateTime = currentTime;
                moveParticle(particlePtr);
            }

            //get the multiplier for the direction (1 or -1)
            directStep = particleUtilsPS::getDirectStep(direction);

            //if we have 4 trails or more, we are in infinite trails mode, so we don't touch the previous leds
            //otherwise we need to set the last pixel in the trail to the background
            if (trailType < 4 && (!fillBG && !blend) ) {
                //if we don't have trails, we just need to turn off the first trail pixel
                //otherwise we need to switch the pixel at the end of the trail
                if (trailType == 0 || trailType == 3) {
                    trailLedLocation = getTrailLedLoc(true, 1, modAmmount);
                } else if( trailType == 1 || trailType == 2 ){
                    //in the case of two trails, we only need to set the rear trail, since the 
                    //front one will be overwritten as the particle moves
                    trailLedLocation = getTrailLedLoc(true, trailSize + 1, modAmmount);
                }

                //get the physical pixel location and the color it's meant to be
                segDrawUtils::getPixelColor(segmentSet, &pixelInfo, *bgColor, bgColorMode, trailLedLocation);
                //only turn off the pixel if it hasn't been touched by another particle's trail (or something else)
                //this prevents background holes from being placed in other particles
                if(segmentSet.leds[pixelInfo.pixelLoc] == trailEndColors[i]){
                    segmentSet.leds[pixelInfo.pixelLoc] = pixelInfo.color;
                }
            }

            //if we have trails, draw them
            //if it has two, we draw the trail in front of the particle first, followed by the one behind it
            //this gets the correct look when bouncing with two trails
            if (trailType != 0 && trailType < 4) {
                //draw a trail, extending positively or negativly, dimming with each step, wrapping according to the modAmmount
                //we draw the trail rear first, so that on bounces the brighter part of the trail over-writes the dimmer part
                for (uint8_t k = trailSize; k > 0; k--) {

                    //if we have two trails, we need to draw the negative trail
                    if (trailType == 2 || trailType == 3) {
                        trailLedLocation = getTrailLedLoc(false, k, modAmmount);
                        setTrailColor(trailLedLocation, k);
                    }
                    
                    //draw the positive trail
                    if (trailType == 1 || trailType == 2) {
                        trailLedLocation = getTrailLedLoc(true, k, modAmmount);
                        setTrailColor(trailLedLocation, k);
                        //If we have trails, we need to record the trail end color at the end of the trail
                        if(k == trailSize){
                            trailEndColors[i] = segmentSet.leds[pixelInfo.pixelLoc]; //pixelInfo.color;
                        }
                    }
                }
            }

            //draw the main particle
            //we always start at the particle's head and move opposite the direction of motion
            //Note that if we're bouncing, moveParticle() will shift the particle back by it's size when a bounce happens
            //so we don't need to worry about (position - k * directStep) becoming negative
            for (uint16_t k = 0; k < size; k++) {
                if (bounce) {
                    trailLedLocation = (position - k * directStep);
                } else {
                    //add numLEDs to prevent the value from being negative before the mod
                    //(arduino handles mods of negative numbers weirdly)
                    trailLedLocation = addMod16PS( position, numLEDs - k * directStep, numLEDs); //( (position - k * directStep) + numLEDs) % numLEDs;
                }
                //get the pixel location and color and set it
                segDrawUtils::getPixelColor(segmentSet, &pixelInfo, colorOut, colorMode, trailLedLocation);
                if(blend){
                    segmentSet.leds[pixelInfo.pixelLoc] += pixelInfo.color;
                } else {
                    segmentSet.leds[pixelInfo.pixelLoc] = pixelInfo.color;
                }

                //if we don't have a rear trail, then the next pixel that needs to be set to background
                //is the last pixel in the particle body, so we record it's color
                if( k == (size - 1) && (trailType == 0 || trailType == 3) ){
                    trailEndColors[i] = segmentSet.leds[pixelInfo.pixelLoc];//pixelInfo.color;
                }
            }
        }
        showCheckPS();
    }
}

//Moves the particle according to it's direction
//also handles bounce behavior
//when bouncing, the whole particle body is reversed, so it's head position is 
//shifted back/forward equal to it's size
void ParticlesPS::moveParticle(particlePS *particlePtr) {
    position = particlePtr->position;

    //get the multiplier for the direction (1 or -1)
    directStep = particleUtilsPS::getDirectStep(direction);

    //get the next position of the particle 
    //if we're bouncing, then the particle can step outside of the strip
    //due to the mod ammount, we catch this below
    position = addMod16PS(position, modAmmount + directStep, modAmmount);//(position + directStep + modAmmount) % modAmmount;

    //if we're bouncing, and we're either at the start or end of the strip
    //we need to reverse direction, and shift the particle's position by size from either end
    //Due to how the mod works, particles outside of the strip
    //will have positions >=numLEDs
    if(bounce && position >= numLEDs){
        if(direction){
            position = numLEDs - size - 1;
            direction = false;
        } else{
            position = size;
            direction = true;
        }
        particlePtr->direction = direction;
    }

    //update the particle's position
    particlePtr->position = position;
}

//writes out the trail color according to the pixel number in the trail (ie 0 - trailSize)
//the trail is blended towards background color according to the trailSize
void ParticlesPS::setTrailColor(uint16_t trailLedLocation, uint8_t trailPixelNum){
    //get the physical pixel location and color
    segDrawUtils::getPixelColor(segmentSet, &pixelInfo, colorOut, colorMode, trailLedLocation);
    //blend the color into the background according to where we are in the trail
    colorEnd = segDrawUtils::getPixelColor(segmentSet, pixelInfo.pixelLoc, *bgColor, bgColorMode, pixelInfo.segNum, pixelInfo.lineNum);
    pixelInfo.color = particleUtilsPS::getTrailColor(pixelInfo.color, colorEnd, trailPixelNum, trailSize, dimPow);
    //output the color
    if(blend){
        segmentSet.leds[pixelInfo.pixelLoc] += pixelInfo.color;
    } else {
        segmentSet.leds[pixelInfo.pixelLoc] = pixelInfo.color;
    }                 
}

//returns the position of a trail pixel(local to the segment) based on the trail direction, and the mod ammount
//trailDirect = true => a rear trail
//trailDirect =  false => a front trail
uint16_t ParticlesPS::getTrailLedLoc(bool trailDirect, uint8_t trailPixelNum, uint16_t modAmmount) { 
    //get the multiplier for the direction (1 or -1)
    //sets if the trail will be drawn forwards or backwards
    trailDirectionAdj = particleUtilsPS::getDirectStep(trailDirect);

    //since we draw the body of the particle behind the lead pixel, 
    //we need to offset rear trails by the body size
    sizeAdj = size - 1;

    if(!trailDirect){
        sizeAdj = 0;
    }

    //worked this formula out by hand, basically just adds/subtracts the trail location from the particle location
    //wrapping according to the mod ammount (note that we add the mod ammount to prevent this from being negative,
    //arduino handles mods of negative numbers weirdly)
    //( position + modAmmount - trailDirectionAdj * ( directStep * ( trailPixelNum + sizeAdj ) ) ) % modAmmount;
    trailLocOutput = addMod16PS(position, modAmmount - trailDirectionAdj * ( directStep * ( trailPixelNum + sizeAdj ) ), modAmmount);

    //if we've bounced and have a trail coming from the rear of the particle
    //we produce a fading trail as it bounces (like you'd see on a flame or stream irl)
    //to do this we take the portion of the trail that is currently off the strip 
    //and redirect it back along the strip according to the direction
    if(trailType == 1 && trailDirect && bounce && trailLocOutput >= numLEDs){
        if(direction){
            trailLocOutput = modAmmount - trailLocOutput;
        } else {
            trailLocOutput = (numLEDs - 1) - mod16PS( trailLocOutput, (numLEDs - 1) ); //(numLEDs - 1) - trailLocOutput % (numLEDs - 1);
        }
        //displace the trail by the size of the particle
        trailLocOutput = trailLocOutput - sizeAdj;
    } 

    return trailLocOutput;
}
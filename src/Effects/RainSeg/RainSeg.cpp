#include "RainSeg.h"

//constructor for palette colors, no range options
RainSeg::RainSeg(SegmentSet &SegSet, palettePS &Palette, CRGB BgColor, bool BgPrefill, uint8_t SpawnChance, 
                    uint8_t MaxNumDrops, uint16_t Size, uint8_t TrailType, uint8_t TrailSize, uint16_t Rate):
    palette(&Palette), bgPrefill(BgPrefill), spawnChance(SpawnChance), 
    size(Size), trailType(TrailType), trailSize(TrailSize)
    {    
        speedRange = 0;
        sizeRange = 0;
        trailRange = 0;
        init(MaxNumDrops, BgColor, SegSet, Rate);
	}

//constructor for palette colors with range and trail options
RainSeg::RainSeg(SegmentSet &SegSet, palettePS &Palette, CRGB BgColor, bool BgPrefill, uint8_t SpawnChance, 
                    uint8_t MaxNumDrops, uint16_t Size, uint16_t SizeRange, uint8_t TrailSize,
                    uint8_t TrailRange, bool NoTrails, bool OneTrail, bool TwoTrail, bool RevTrail, 
                    bool InfTrail, uint16_t Rate, uint16_t SpeedRange):
    palette(&Palette), bgPrefill(BgPrefill), spawnChance(SpawnChance), size(Size), 
    sizeRange(SizeRange), trailSize(TrailSize), trailRange(TrailRange), noTrails(NoTrails), oneTrail(OneTrail),
    twoTrail(TwoTrail), revTrail(RevTrail), infTrail(InfTrail), speedRange(SpeedRange) 
    {   
        trailType = 6; //we're picking from the boolean trail options, this is indicated by trailType 6
        init(MaxNumDrops, BgColor, SegSet, Rate);
	}

//constructor for single color, no range options
RainSeg::RainSeg(SegmentSet &SegSet, CRGB Color, CRGB BgColor, bool BgPrefill, uint8_t SpawnChance, 
                    uint8_t MaxNumDrops, uint16_t Size, uint8_t TrailType, uint8_t TrailSize, uint16_t Rate):
    bgPrefill(BgPrefill), spawnChance(SpawnChance), size(Size), trailType(TrailType), trailSize(TrailSize)
    {    
        speedRange = 0;
        sizeRange = 0;
        trailRange = 0;
        //make a single colored palette
        paletteTemp = paletteUtilsPS::makeSingleColorPalette(Color);
        palette = &paletteTemp;
        init(MaxNumDrops, BgColor, SegSet, Rate);
	}

//constructor for single colors with range and trail options
RainSeg::RainSeg(SegmentSet &SegSet, CRGB Color, CRGB BgColor, bool BgPrefill, uint8_t SpawnChance, 
                    uint8_t MaxNumDrops, uint16_t Size, uint16_t SizeRange, uint8_t TrailSize,
                    uint8_t TrailRange, bool NoTrails, bool OneTrail, bool TwoTrail, bool RevTrail, 
                    bool InfTrail, uint16_t Rate, uint16_t SpeedRange):
    bgPrefill(BgPrefill), spawnChance(SpawnChance), size(Size), sizeRange(SizeRange), 
    trailSize(TrailSize), trailRange(TrailRange), noTrails(NoTrails), oneTrail(OneTrail),
    twoTrail(TwoTrail), revTrail(RevTrail), infTrail(InfTrail), speedRange(SpeedRange) 
    {    
        trailType = 6; //we're picking from the boolean trail options, this is indicated by trailType 6
        //make a single colored palette
        paletteTemp = paletteUtilsPS::makeSingleColorPalette(Color);
        palette = &paletteTemp;
        init(MaxNumDrops, BgColor, SegSet, Rate);
	}

RainSeg::~RainSeg(){
    //Free all particles and the particle array pointer
    particleUtilsPS::freeParticleSet(particleSetTemp);
    free(partActive);
    free(trailEndColors);
    free(paletteTemp.paletteArr);
}

//general setup function for class vars
void RainSeg::init(uint8_t MaxNumDrops, CRGB BgColor, SegmentSet &SegSet, uint16_t Rate){
    //bind the rate and segSet pointer vars since they are inherited from BaseEffectPS
    bindSegSetPtrPS();
    bindClassRatesPS();
    //bind background color pointer
    bindBGColorPS();
    setupDrops(MaxNumDrops);
}

//Creates a new set of particles for the effect based on the passed in value for the max number of concurrent drops
//You should call this if you want to change the number of drops or if you change the segment set
//Note that all particles will be reset, and if bgPrefill is true, the background will be filled in to clear any active particles
void RainSeg::setupDrops(uint8_t newMaxNumDrops){

    //must always have at least 1 drop spawning
    if(newMaxNumDrops == 0){
        newMaxNumDrops = 1;
    }
    maxNumDrops = newMaxNumDrops;
    
    //Record the dimensions of the segment set
    numSegs = segSet->numSegs;
    
    //We need to know how many particles we need to store all the possible active drops for each segment
    uint16_t numParticles = numSegs * newMaxNumDrops;
    
    //We only need to make a new particle set and accompanying arrays if the current ones aren't large enough
    //This helps prevent memory fragmentation by limiting the number of heap allocations
    //but this may use up more memory overall.
    if( alwaysResizeObjPS || (numParticles > particleSet->maxLength) ){

        free(trailEndColors);
        trailEndColors = (CRGB*) malloc(numParticles * sizeof(CRGB));

        free(partActive);
        partActive = (bool*) malloc(numParticles * sizeof(bool) );

        //Free all particles and the particle array pointer
        particleUtilsPS::freeParticleSet(particleSetTemp);

        //create a new particle set
        particleSetTemp = particleUtilsPS::buildParticleSet(numParticles, 0, true, *rate, speedRange, size, sizeRange, 
                                                            trailType, trailSize, trailRange, false, palette->length, true);
        particleSet = &particleSetTemp;
    }
    //Set the particle set length to match the number of particles
    //This "hides" any unused particles from the rest of the effect
    particleSet->length = numParticles;

    //for trailType 6, we'll set the particle trails randomly based on the trail flags
    if(trailType == 6){
        particleUtilsPS::setAllTrailRand(*particleSet, noTrails, oneTrail, twoTrail, revTrail, infTrail);
    }

    //set all the particles to inactive
    //and set their spawn positions
    for (uint16_t i = 0; i < numSegs; i++) {
        for (uint8_t j = 0; j < maxNumDrops; j++) {
            particleIndex = i * maxNumDrops + j;
            partActive[particleIndex] = false;
            particlePtr = particleSet->particleArr[particleIndex];
            setDropSpawnPos(particlePtr, i);
        }
    }

    //Flag the background to be filled to clear any leftover active particles
    //This will only trigger the background to fill if bgPrefill is true.
    bgFilled = false;
}

//sets the inital spawn position of a particle
//For particles with no or trailing trails the spawn position is 0
//For particles with leading or double trails spawn position is offset by the trail's size
//(we set the spawn position in the "phantom zone", see update() comments)
void RainSeg::setDropSpawnPos(particlePS *particlePtr, uint8_t segNum){
    partTrailType = particlePtr->trailType;
    sectionEnd = segSet->getTotalSegLength(segNum);
    partTrailSize = particlePtr->trailSize;
    partSize = particlePtr->size;
    if(partTrailType == 2){
        particlePtr->position = sectionEnd + partTrailSize + partSize;
    } else if(partTrailType == 3){
        particlePtr->position = sectionEnd + partSize;
    } else {
        particlePtr->position = 0;
    }
}

//Updates the effect by either moving particles or spawning them for each segment
//Particles always move positively down the segment until disappearing off the end
//How it works:
//1:Particles:
    //Particles from particlePS.h (in the Particles Effect)
    //We have a single set of particles ( created using setupDrops() )
    //Each particle is either active or inactive
    //An active particle is drawn moving on the segSet, while an inactive on is waiting to become activated
    //The active/inactive status of particles is stored in the partActive array
    //There is a single particle set and partActive array for all the particles across all the segments
    //The areas in the array for each segment's particles go up sequentially
        //so for two segments, each with three possible active particles
        //partActive[0],[1],[2] would be the particles in the first segment
        //and [3],[4],[5] would be those in the second segment
        //in general particleIndex = segmentNumber * maxNumDrops + particleNumber
//2: Particle motion
    //It's critical that particles fully go off the strip before being set to inactive, including tails
    //To allow them to move off the strip, we create a "phantom" zone off of the end of the strip
    //where the particles will exist, but anything in the zone will be not be drawn
    //We do this by setting the maximum particle position to sectionEnd + totPartSize;
    //where sectionEnd is the length of the segment and totPartSize = partTrailSize + partSize is the total size of the particle
    //(We do some additional adjustments for twin trailed particles)
    //when drawing, we only draw parts of the particle that are on the segment
//3: Spawning and De-spawning:
    //Once a particle reaches the max position, it is fully off the strip and is set to inactive
    //Any inactive particle can be re-spawned randomly as long as it would not spawn on top of another particle
    //When a particle is re-spawned, its position is set to 0, and it is given a new set of random traits
    //(size, speed, color, etc) depending on the effect options
    //As part of spawning, the particle is also drawn (at 0)
//4: Overall Steps:
    //On each update cycle, for each segment we check all its particle to see if they are active and should move, 
    //if so, we move them
    //Then, if the particle moved, we set the background color on the led that the particle has moved off of (the last pixel in the trail)
    //We also check its position to see if should block any other particles from spawning, if so we set the spawnOkTest flag
    //Next, even if the particle has not moved, we re-draw the trails and the particle body
    //(unless the particle has moved fully off the segment, we set it to inactive and skip drawing it)
    //This prevents another, faster particle from wiping out a slower on, only to have the slower one 
    //suddenly re-appear once it's time to move it again
    //Once all the active particles have been drawn, we go over the inactive ones and try to spawn them
    //(is spawnOkTest has not been flagged)
    //When a particle is spawned it is drawn in the 0 position of the segment and spawnOkTest is flagged
    //to prevent any more spawning
void RainSeg::update(){
    currentTime = millis();

    //we use *rate as the update rate b/c it's the maximum speed any particle have
    if( ( currentTime - prevTime ) >= *rate) {
        prevTime = currentTime;

        //if the bg is to be filled before the particles start, fill it in
        if(fillBG || blend || (bgPrefill && !bgFilled) ){
            bgFilled = true;
            segDrawUtils::fillSegSetColor(*segSet, *bgColor, bgColorMode);
        }

        //for each segment and then each particle, in order:
        //if active:
            //move it to its next position (if needed)
            //set the spawnOkTest flag if the particle is blocking the 0th pixel in the segment
            //then sets the pixel at the previous location to the bgColor (including trail ends)
            //Checks if the particle should still be active after moving (if not, we skip the rest of the steps)
            //then draws new trails if needed
            //then draws the particle in its new position
            //This order helps prevent overwriting different parts of the particles if there are clashes
            //As a particle passes over another, we want to maintain the look of both particles as much as possible
            //This is why we always re-draw all the particles with each update, so that a fast particle doesn't erase a slower one
            //Also we pay special attention when setting the background pixel:
            //If a previous particle from the set has already overwritten the background pixel, then we don't need to set it
            //To track this, we record the color that we expect the background pixel to be (it's the last color in the trail/body)
            //Then when we go to set the background, we check the current color of the pixel
            //If it doesn't match, then we don't overwrite.
        //if not active:
            //Try to spawn the particle if spawnOKTest is true
            //If it's spawned, set it to active and its position to zero
            //and randomize it properties (speed, size, etc)
            //the draw the particle head in the 0th position
        for (uint16_t i = 0; i < numSegs; i++) {
            sectionEnd = segSet->getTotalSegLength(i);
            //sectionStart = 0;
            //reset the spawnOkTest for each segment
            spawnOkTest = true;
            for (uint8_t j = 0; j < maxNumDrops; j++) {
                //the particle's location in the particleSet and partActive arrays
                particleIndex = i * maxNumDrops + j;
                //if the particle is active, we need to increment it and draw trails
                if(partActive[particleIndex]){
                    //get the particle from the set, and record some vars locally for ease of access
                    particlePtr = particleSet->particleArr[particleIndex];
                    partPos = particlePtr->position; //the current position of the particle
                    partSpeed = particlePtr->speed; //the speed of the particle 
                    partSize = particlePtr->size; //the length of the main body of the particle
                    partTrailType = particlePtr->trailType; //the type of trail for the particle (see above for types)
                    partTrailSize = particlePtr->trailSize; //the length of the trail(s) of the particle (only applies if the pixel has a trail)
                    
                    colorOut = paletteUtilsPS::getPaletteColor(*palette, particlePtr->colorIndex);
                    //calculate the maximum position the particle can have before all of it is off the segment
                    totPartSize = partTrailSize + partSize;

                    //for particles with leading or twin trails, we need to do some extra adjustments to the 
                    //maximum particle postion, and introduce an offset b/c the postion of the particle is off the segment
                    //initially
                    posOffset = 0;
                    if(partTrailType == 2 || partTrailType == 3){
                        posOffset = partTrailSize;
                        if(partTrailType == 2){
                            totPartSize += partTrailSize;
                        }
                    }

                    maxPosition = sectionEnd + totPartSize;

                    //if enough time has passed, we need to move the particle
                    movePart = ( ( currentTime - particlePtr->lastUpdateTime ) >= partSpeed );
                    //if the particle needs to move, move it and record the time
                    if(movePart) {
                        particlePtr->lastUpdateTime = currentTime;
                        moveParticle(particlePtr);
                    }
                    
                    //if any part of the particle is in the 0th segment pixel
                    //we need to block any new particles from spawning
                    //headPos takes into account the leading trails for trailTypes 2 and 3
                    headPos = addMod16PS(partPos, posOffset, maxPosition);
                    if( headPos < totPartSize ){
                        spawnOkTest = false;
                    }

                    //if we have 4 trails or more, we are in infinite trails mode, so we don't touch the previous leds
                    //otherwise we need to set the last pixel in the trail to the background
                    if (partTrailType < 4 && (!fillBG && !blend)) {
                        //get the physical pixel location and the color it's meant to be
                        //if we don't have trails, we just need to turn off the first trail pixel
                        //otherwise we need to switch the pixel at the end of the trail
                        if (partTrailType == 0 || partTrailType == 3) {
                            trailLedLocation = getTrailLedLoc(true, 1, maxPosition);
                        } else if( partTrailType == 1 || partTrailType == 2 ){
                            //in the case of two trails, we only need to set the rear trail, since the 
                            //front one will be overwritten as the particle moves
                            trailLedLocation = getTrailLedLoc(true, partTrailSize + 1, maxPosition);
                        }

                        //only try to draw a pixel if it's within the segment
                        if(trailLedLocation < sectionEnd){
                            //get the pixel location in the current segment
                            pixelPosTemp = segDrawUtils::getSegmentPixel(*segSet, i, trailLedLocation);

                            //only turn off the pixel if it hasn't been touched by another particle's trail (or something else)
                            //this prevents background holes from being placed in other particles
                            if(segSet->leds[pixelPosTemp] == trailEndColors[particleIndex]){
                                lineNum = segDrawUtils::getLineNumFromPixelNum(*segSet, trailLedLocation, i);
                                segSet->leds[pixelPosTemp] = segDrawUtils::getPixelColor(*segSet, pixelPosTemp, *bgColor, bgColorMode, i, lineNum);
                            }
                        }
                    }

                    //if a particle has reached the end of the segment + (total particle size), it is now inactive
                    //the reason why we include the particle size is to allow the whole particle to move off the segment
                    //anything that falls outside the segment won't be drawn,
                    //so we can skip drawing the particle, and break out of the loop
                    if ( headPos == maxPosition - 1 ) {
                        partActive[particleIndex] = false;
                        continue;
                    }

                    //if we have trails, draw them
                    //if it has two, we draw the trail in front of the particle first, followed by the one behind it
                    if (partTrailType != 0 && partTrailType < 4) {
                        //draw a trail, extending positively or negatively, dimming with each step
                        for (uint8_t k = partTrailSize; k > 0; k--) {

                            //if we have two trails, we need to draw the negative trail
                            if (partTrailType == 2 || partTrailType == 3) {
                                trailLedLocation = getTrailLedLoc(false, k, maxPosition);
                                if(trailLedLocation < sectionEnd){
                                    drawParticlePixel(trailLedLocation, k, partTrailSize, i, false);
                                }
                            }
                            
                            //draw the positive trail
                            if (partTrailType == 1 || partTrailType == 2) {
                                trailLedLocation = getTrailLedLoc(true, k, maxPosition);
                                //only try to draw a pixel if it's within the segment
                                if(trailLedLocation < sectionEnd){
                                    drawParticlePixel(trailLedLocation, k, partTrailSize, i, false);
                                    //drawParticlePixel gets pixelPosTemp for the current pixel
                                    //If we have trails, we need to record the trail end color at the end of the trail
                                    if(k == partTrailSize){
                                        trailEndColors[particleIndex] = segSet->leds[pixelPosTemp]; //pixelInfo.color;
                                    }
                                }
                            }
                        }
                    }

                    //draw the main particle
                    //we always start at the particle's head and move opposite the direction of motion
                    for (uint16_t k = 0; k < partSize; k++) {
                        
                        trailLedLocation = addMod16PS( partPos, maxPosition - k, maxPosition); //( (position - k * directStep) + numLEDs) % numLEDs;
                        
                        //only try to draw a pixel if it's within the segment
                        if(trailLedLocation < sectionEnd){
                            //get the pixel location and color and set it
                            drawParticlePixel(trailLedLocation, k, partTrailSize, i, true);
                            //drawParticlePixel gets pixelPosTemp for the current pixel
                            //if we don't have a rear trail, then the next pixel that needs to be set to background
                            //is the last pixel in the particle body, so we record it's color
                            if( k == (partSize - 1) && (partTrailType == 0 || partTrailType == 3) ){
                                trailEndColors[particleIndex] = segSet->leds[pixelPosTemp];//pixelInfo.color;
                            }
                        }
                    }
                }
            }

            //with the active particles drawn, we now try to spawn any inactive ones
            //(if we can)
            for (uint8_t j = 0; j < maxNumDrops; j++) {
                particleIndex = i * maxNumDrops + j;
                if(spawnOkTest && !partActive[particleIndex]){
                    //try to spawn particle
                    //if we spawn a particle, stop more from spawning (prevents overlapping),
                    //then set the particle's properties using spawnParticle();
                    if (random8(100) <= spawnChance) {
                        spawnOkTest = false;
                        spawnParticle(particleIndex, i);
                    }
                }
            }
        }
        showCheckPS();
    }
}

//Moves the particle forward by one step
void RainSeg::moveParticle(particlePS *particlePtr) {
    partPos = particlePtr->position;

    //get the next position of the particle 
    //we use mod to keep the particle within the strip bounds
    partPos = addMod16PS(partPos, maxPosition + 1, maxPosition);//(position + 1 + maxPosition) % maxPosition;

    //update the particle's position
    particlePtr->position = partPos;
}

//writes out the pixel color according to the pixel number in the trail / body (ie 0 - trailSize)
//a trail pixel is indicated by setting bodyPixel to false
//the trail is blended towards background color according to the trailSize and the trailPixelNum
//note that colorOut is the palette color of the pixel fetched as part of the update function
void RainSeg::drawParticlePixel(uint16_t trailLedLocation, uint8_t trailPixelNum, uint8_t trailSize, uint8_t segNum, bool bodyPixel){
    //get the physical pixel location and color
    pixelPosTemp = segDrawUtils::getSegmentPixel(*segSet, segNum, trailLedLocation);
    lineNum = segDrawUtils::getLineNumFromPixelNum(*segSet, trailLedLocation, segNum);
    colorTemp = segDrawUtils::getPixelColor(*segSet, pixelPosTemp, colorOut, colorMode, segNum, lineNum);

    if(!bodyPixel){
        //get the target background color and blend towards it
        colorEnd = segDrawUtils::getPixelColor(*segSet, pixelPosTemp, *bgColor, bgColorMode, segNum, lineNum);
        //blend the color into the background according to where we are in the trail
        colorTemp = particleUtilsPS::getTrailColor(colorTemp, colorEnd, trailPixelNum, trailSize, dimPow);
    }
    //output the color
    if(blend){
        segSet->leds[pixelPosTemp] += colorTemp;
    } else {
        segSet->leds[pixelPosTemp] = colorTemp;
    }                 
}

//returns the position of a trail pixel(local to the segment) based on the trail direction, and the mod amount
//trailDirect = true => a rear trail
//trailDirect = false => a front trail
uint16_t RainSeg::getTrailLedLoc(bool trailDirect, uint8_t trailPixelNum, uint16_t maxPosition) { 
    //get the multiplier for the direction (1 or -1)
    //sets if the trail will be drawn forwards or backwards
    trailDirectionAdj = particleUtilsPS::getDirectStep(trailDirect);

    //since we draw the body of the particle behind the lead pixel, 
    //we need to offset rear trails by the body size
    sizeAdj = partSize - 1;

    if(!trailDirect){
        sizeAdj = 0;
    }

    //worked this formula out by hand, basically just adds/subtracts the trail location from the particle location
    //wrapping according to the mod amount (note that we add the mod amount to prevent this from being negative)
    //( position + maxPosition - trailDirectionAdj * ( ( trailPixelNum + sizeAdj ) ) ) % maxPosition;
    return addMod16PS(partPos, maxPosition - trailDirectionAdj * ( trailPixelNum + sizeAdj ), maxPosition);
}

//spawns a particle by taking an inactive particle and resetting its position to 0
//and randomizing its properties
//also draws the first step of the particle
void RainSeg::spawnParticle(uint8_t particleIndex, uint8_t segNum){
    //set the particle to active
    partActive[particleIndex] = true;
    //randomize the particle properties
    particleUtilsPS::randomizeParticle(*particleSet, particleIndex, 0, true, *rate, speedRange, size, sizeRange, 
                                       trailType, trailSize, trailRange, false, palette->length, true);
    if(trailType == 6){
        particleUtilsPS::setTrailRand(*particleSet, particleIndex, noTrails, oneTrail, twoTrail, revTrail, infTrail);
    }

    //reset the particle and set its new spawn location
    particlePtr = particleSet->particleArr[particleIndex];
    particleUtilsPS::resetParticle(particlePtr);
    setDropSpawnPos(particlePtr, segNum);

    //draw the first step of the particle
    //for particles with leading trails, the first step is the end of the trail
    colorOut = paletteUtilsPS::getPaletteColor(*palette, particlePtr->colorIndex);
    partTrailType = particlePtr->trailType;
    if(partTrailType == 2 || partTrailType == 3){
        drawParticlePixel(0, particlePtr->trailSize, particlePtr->trailSize, segNum, true);
    } else {
        drawParticlePixel(0, 0, particlePtr->trailSize, segNum, true);
    }
    //we need to store the trailEndColor for size 1 particles
    //otherwise, when they next update, trailEndColors will be empty
    trailEndColors[particleIndex] = segSet->leds[pixelPosTemp];
}
#include "FireworksPS.h"

//Constructor for effect with palette
FireworksPS::FireworksPS(SegmentSet &SegmentSet, palettePS *Palette, uint8_t MaxNumFireworks, uint8_t MaxNumSparks, 
                         uint8_t SpawnChance, uint16_t LifeBase, uint8_t SpeedDecay, uint16_t Rate, uint16_t SpeedRange):
    segmentSet(SegmentSet), palette(Palette), spawnChance(SpawnChance), lifeBase(LifeBase), speedDecay(SpeedDecay), speedRange(SpeedRange)
    {    
        init(MaxNumFireworks, MaxNumSparks, Rate);
	}

//Constructor for effect with palette of random colors
FireworksPS::FireworksPS(SegmentSet &SegmentSet, uint16_t numColors, uint8_t MaxNumFireworks, uint8_t MaxNumSparks, 
                         uint8_t SpawnChance, uint16_t LifeBase, uint8_t SpeedDecay, uint16_t Rate, uint16_t SpeedRange):
    segmentSet(SegmentSet), spawnChance(SpawnChance), lifeBase(LifeBase), speedDecay(SpeedDecay), speedRange(SpeedRange)
    {    
        paletteTemp = paletteUtilsPS::makeRandomPalette(numColors);
        palette = &paletteTemp;
        init(MaxNumFireworks, MaxNumSparks, Rate);
	}

//constructor for effect with single color
//!!If using pre-build FastLED colors you need to pass them as CRGB( *color code* )
FireworksPS::FireworksPS(SegmentSet &SegmentSet, CRGB Color, uint8_t MaxNumFireworks, uint8_t MaxNumSparks, 
 uint8_t SpawnChance, uint16_t LifeBase, uint8_t SpeedDecay, uint16_t Rate, uint16_t SpeedRange):
    segmentSet(SegmentSet), spawnChance(SpawnChance), lifeBase(LifeBase), speedDecay(SpeedDecay), speedRange(SpeedRange)
    {    
        paletteTemp = paletteUtilsPS::makeSingleColorPalette(Color);
        palette = &paletteTemp;
        init(MaxNumFireworks, MaxNumSparks, Rate);
	}

FireworksPS::~FireworksPS(){
    particleUtilsPS::deleteAllParticles(&particleSetTemp);
    delete[] particleSetTemp.particleArr;
    delete[] fireWorkActive;
    delete[] trailEndColors;
    delete[] paletteTemp.paletteArr;
}

//common initilzation function for core vars
void FireworksPS::init(uint8_t maxNumFireworks, uint8_t maxNumSparks, uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    //bind background color pointer (if needed)
    CRGB BgColor = 0;
    bindBGColorPS();
    //set how long the center "bomb" pixel is alive for in ms.
    //This seems to look good in practice, but you might need to adjust centerLife
    //for more extreme values of lifeBase
    centerLife = lifeBase / 10 + 100;
    setupFireworks(maxNumFireworks, maxNumSparks);
    //do a quick clear of the strip
    segDrawUtils::fillSegSetColor(segmentSet, *bgColor, bgColorMode);
}

//Create the data structures for a set of fireworks
//You should call this if you ever want to change maxNumFireworks or maxNumSparks
//Fireworks need three data structures:
    //A bool array fireworks[maxNumFireworks] that stores if a firework is active or not
    //A particleSet with a paricle array of size maxNumFireworks * (maxNumSparks + 1)
    //A CRGB array of trailEndColors[maxNumFireworks * (maxNumSparks + 1)] to store the trail color for each particle
    //(We have an extra particle for the center "bomb" particle)
//Note that we're using a single 1D array for all the particles from all the fireworks
//This is easier that creating a 2D array with rows for each firework
//To get the particles in a specific firework, we use fireworkNum * maxNumSparks + i where 0 -> i -> maxNumSparks
//The minimium number of fireworks and sparks is 1
void FireworksPS::setupFireworks(uint8_t newMaxNumFireworks, uint8_t newMaxNumSparks){

    //check for any active particles on the segment set
    //we need to clear them before we make a new set of particles
    bool clearStrip = false;
    for (uint8_t i = 0; i < maxNumFireworks; i++) {
        if( fireWorkActive[i] ){
            clearStrip = true;
            break;
        }
    }

    //set the background if we found an active particle to clear the segment set
    if( clearStrip || fillBG || blend ){
        segDrawUtils::fillSegSetColor(segmentSet, *bgColor, bgColorMode);
    }

    //must always have at least 1 firework spawning
    if(newMaxNumSparks == 0){
        newMaxNumSparks = 1;
    }

    //must always have at least 1 spark spawning
    if(newMaxNumFireworks == 0){
        newMaxNumFireworks = 1;
    }

    //delete and re-create all the arrays and the particle set
    maxNumFireworks = newMaxNumFireworks;
    maxNumSparks = newMaxNumSparks + 1;
    uint16_t numParticles = maxNumFireworks * maxNumSparks;

    delete[] trailEndColors;
    trailEndColors = new CRGB[numParticles];

    delete[] fireWorkActive;
    fireWorkActive = new bool[maxNumFireworks];

    particleUtilsPS::deleteAllParticles(&particleSetTemp);
    delete[] particleSetTemp.particleArr;

    particleSetTemp = particleUtilsPS::buildParticleSet(numParticles, 0, true, *rate, speedRange, size, sizeRange, 
                                                        0, 0, 0, false, palette->length, true);
    particleSet = &particleSetTemp;

    //set all the fireworks to inactive, ready to be spawned
    for (uint8_t i = 0; i < maxNumFireworks; i++) {
        fireWorkActive[i] = false;
    }
}

//updates the effect
//Note: This effect uses particles: see more info in particlePS.h and particleUtilsPS.h
//Overall we have a series of fireworks that each have their own set of sparks (particles)
//When a firework is spawned fireWorkActive[i] is set true (see setupFireworks() for more on the firework's data structure)
//and the firework's particles are re-spawned with random lifes, speeds, etc
//(see spawnFirework() for more on spawning)
//The update cycle moves particles for active fireworks, draws them, updates their lifes, and
//confirms if the firework is inactive. If it is, we try to spawn it again.
//The update cycle is actually fairly simple dispite how much code there is.
//One important note is that we re-draw any active particles even if they haven't moved
//Particles dim whenever they are re-drawn, so even slow particles dim fairly quickly
//This may seem like it's not the right way to do it, but it looks good in practice
//because we tie the speed and life together when spawing particles (see spawnFirework())
//The update operations in order:
    //If it's time to update we get check each firework
        //If a firework is active we check each of the sparks (particles) in the firework
            //If they're active (life > 0) then we:
                //Check if it's time to move them, and then do so
                //If they moved, fill-in the background in their previous position
                //Then re-draw the particle itself, we always re-draw
                //any active particles incase their colors have changed, even if they didn't move
                //As part of the drawing, particles have their life decreased
                //If it hits 0 (or the particle runs off the edges of the strip)
                //the particle is deactivated
                //if all the particles in a firework are in-active, then the firework is inactive
        //Once all the fireworks have been drawn, we try to spawn any that are inactive
        //When spawned, all the particles start in one spawn location, and then spread out when updated
void FireworksPS::update(){
    currentTime = millis();

    //The time since the last update cycle. We need this for adjusting particle lives later
    deltaTime = currentTime - prevTime;
    if( deltaTime >= *rate ) {
        prevTime = currentTime;
        
        numLEDs = segmentSet.numLeds;
        //if the bg is to be filled before the particles start, fill it in
        //(such as if you have a background that's changing with time (alla bgColorMode 6))
        if( fillBG || blend ){
            segDrawUtils::fillSegSetColor(segmentSet, *bgColor, bgColorMode);
        }

        //For each firerwork, we check if it's active and then update any of its active particles
        for (uint8_t i = 0; i < maxNumFireworks; i++) {
            if(fireWorkActive[i]){
                //We set the firework as inactive here before we check the sparks (particles)
                //If any particle is active then the firework is active and fireWorkActive[i] will be set true
                fireWorkActive[i] = false;
                //Check each particle, if it's active, move and draw it
                for (uint8_t j = 0; j < maxNumSparks; j++) {
                    //The first particle in the array is always the unmoving center "bomb" particle
                    //(see spawnFirework())
                    //This particle is drawn slightly differently then the others, so we set the flag here
                    firstPart = false;
                    if(j == 0){ firstPart = true; }

                    //the particle's location in the particleSet array
                    particleIndex = i * maxNumSparks + j;
                    //get the pointer to the particle in the particle set
                    particlePtr = particleSet->particleArr[particleIndex];
                    partLife = particlePtr->life;

                    //if the particle is alive, we need to update it
                    if(partLife > 0){
                        //if we have any active particles in the firework, then the firework is active
                        fireWorkActive[i] = true;
                        //record some particle vars locally for ease of access
                        partPos = particlePtr->position; //the current position of the particle
                        partSpeed = particlePtr->speed; //the speed of the patrticle 
                        partDirect = particlePtr->direction; //the direction of motion (true moves towards the last pixel in the segementSet)
                        partSize = particlePtr->size; //the length of the main body of the particle
                        partMaxLife = particlePtr->maxLife; //the particle's maximum life
                        
                        //get the particle's color from the palette
                        colorOut = paletteUtilsPS::getPaletteColor(palette, particlePtr->colorIndex);

                        //The maxPosition is the maxium position of the particle 
                        //This includes a "phantom zone" off the strip of size partSize
                        //Where the particle exists, but isn't drawn. This is to accomodate 
                        //particles of size > 1, so that they fully move off the strip
                        maxPosition = numLEDs + partSize;

                        //if enough time has passed, we need to move the particle
                        movePart = ( ( currentTime - particlePtr->lastUpdateTime ) >= partSpeed );
                        //if the particle needs to move, move it and record the time
                        if(movePart) {
                            particlePtr->lastUpdateTime = currentTime;
                            moveParticle(particlePtr);
                        }

                        //get the multiplier for the direction (1 or -1)
                        directStep = particleUtilsPS::getDirectStep(partDirect);

                        //if we're not filling in the background each cycle
                        //we need to set the previous particle position to the background
                        if ( !fillBG && !blend ) {
                            //get the previous particle location (the trail is always 1 behind the particle)
                            trailLedLocation = getTrailLedLoc(1);
                            //get the physical pixel location and the color it's meant to be
                            segDrawUtils::getPixelColor(segmentSet, &pixelInfo, *bgColor, bgColorMode, trailLedLocation);

                            //only turn off the pixel if it hasn't been touched by another particle (or something else)
                            //this prevents background holes from being placed in other particles
                            if(segmentSet.leds[pixelInfo.pixelLoc] == trailEndColors[particleIndex]){
                                segmentSet.leds[pixelInfo.pixelLoc] = pixelInfo.color;
                            }
                        }

                        //reduce the particle's life by the amount of time passed in ms
                        //once the particle's life hits 0 it is deactivated
                        //(We re-fetch partLife incase it's been set to zero because the particle has moved off the strip)
                        partLife = particlePtr->life;
                        if( int32_t(partLife - deltaTime) <= 0){
                            partLife = 0;
                        } else {
                            partLife -= deltaTime;
                        }
                        particlePtr->life = partLife; 

                        //draw the main particle
                        //we always start at the particle's head and move opposite the direction of motion
                        for (uint16_t k = 0; k < partSize; k++) {
                            
                            //the body postion, for size 1 particles, this is just it position
                            trailLedLocation = addMod16PS( partPos, maxPosition - (k * directStep), maxPosition ); //( (position - k * directStep) + numLEDs) % numLEDs;

                            //Draw the body pixel
                            drawParticlePixel(particlePtr, trailLedLocation);
                            //The pixel that needs to be set to background
                            //is the last pixel in the particle body, so we record it's color
                            if( k == (partSize - 1) ){
                                trailEndColors[particleIndex] = segmentSet.leds[pixelInfo.pixelLoc];
                            }
                        }
                    }
                }
            }
        }

        //With the active fireworks drawn, we now try to spawn any inactive ones
        for (uint8_t i = 0; i < maxNumFireworks; i++) {
            if(!fireWorkActive[i]){
                //try to spawn firework
                if (random8(100) <= spawnChance) {
                    spawnFirework(i);
                }
            }
        }
        showCheckPS();
    }
}

//Moves the particle forward by one step and reduce it's speed slightly
//Any particles that run off the strip have their life set to 0 to de-activate them
void FireworksPS::moveParticle(particlePS *particlePtr) {

    //Reduce the particle's speed. This formula isn't the most elegant
    //but seems to produce a reasonable looking result
    //NOTE that speed is the particle's update rate, so higher speed value => slower particle
    particlePtr->speed += uint32_t( particlePtr->speed * speedDecay ) / 100;
    //particlePtr->speed += particlePtr->speed * deltaTime / 100;

    //get the multiplier for the direction (1 or -1)
    directStep = particleUtilsPS::getDirectStep(partDirect);

    //get the next position of the particle,
    //moderated by maxPosition, which allows the particle to go outside the strip
    //to account for particles with size > 1
    partPos = addMod16PS(partPos, maxPosition + directStep, maxPosition);//(position + directStep + modAmmount) % modAmmount;

    //If a particle reaches its max position then it's off the strip and is deactivated
    //(particles moving either direction will hit maxPosition)
    if(partPos >= maxPosition - 1){
        particlePtr->life = 0;
        partLife = 0;
    }

    //update the particle's position
    particlePtr->position = partPos;
}

//Writes out a particle to the strip
//The color of the pixel starts as the burstColor, then fades to the firework color and finally to the background color
//How much of the blend (out of 255) is taken up by the burst color is set by burstBlendLimit
//This function is called assuming you've already set some particle vars, like colorOut, partLife, etc
void FireworksPS::drawParticlePixel(particlePS *particlePtr, uint16_t trailLedLocation){

    //get the pixel's physical location and adjust for any color modes
    //also fetch the background color at this point (colorFinal)
    segDrawUtils::getPixelColor(segmentSet, &pixelInfo, colorOut, colorMode, trailLedLocation);
    colorFinal = segDrawUtils::getPixelColor(segmentSet, pixelInfo.pixelLoc, *bgColor, bgColorMode, pixelInfo.segNum, pixelInfo.lineNum);
    
    //if the pixel is the first particle in the firework, then it is the center particle
    //which only ever fades from the burst color 
    //(this seems to look better overall than fading like a normal particle)
    if(firstPart){ //&& !matchBurst for TODO
        colorTemp = burstColor;
    } else {
        colorTemp = pixelInfo.color;
    }

    //get the where we are in the color blend based on the particle's life
    //(the blend has 255 steps total)
    dimRatio = 255 - ( (partLife * 255) / particlePtr->maxLife );

    //get the output color
    //if we're in the "burst" part of the blend, we fade from the burst color to the firework color
    //otherwise we fade from the firework color to the background
    //This is all one blend of 255 steps, burstBlendLimit sets what step we switch the blend colors at
    if(dimRatio < burstBlendLimit){ //&& !matchBurst for TODO
        colorFinal = colorUtilsPS::getCrossFadeColor(burstColor, colorTemp, dimRatio, burstBlendLimit);
    } else {
        colorFinal = colorUtilsPS::getCrossFadeColor(colorTemp, colorFinal, dimRatio);
    }
    
    //output the color
    if(blend){
        segmentSet.leds[pixelInfo.pixelLoc] += colorFinal;
    } else {
        segmentSet.leds[pixelInfo.pixelLoc] = colorFinal;
    }                 
}

//returns the previous position end of the particle
//if we had trails trailPixelNum would vary, but in our case it's always 1
uint16_t FireworksPS::getTrailLedLoc( uint8_t trailPixelNum ) { 

    //since we draw the body of the particle behind the lead pixel, 
    //we need to offset rear trails by the body size
    sizeAdj = size - 1;

    //worked this formula out by hand, basically just adds/subtracts the trail location from the particle location
    //wrapping according to the mod ammount (note that we add the mod ammount to prevent this from being negative,
    //arduino handles mods of negative numbers weirdly)
    //( position + maxPosition - ( directStep * ( trailPixelNum + sizeAdj ) ) ) % maxPosition;
    return addMod16PS(partPos, maxPosition - ( directStep * ( trailPixelNum + sizeAdj ) ), maxPosition);
}

//Spawns a new firework by reseting all of its particles
//All the particles are given the same random spawn position and have a color choosen at random
//Their directions are set to alternate for each particle so we always have an even spread in both directions
//Their speeds are set on a sliding random scale so that we always have a mix of fast and slow particles
//This helps prevent too many particles from having the same speed, which happens if you just pick randomly
//We also scale the particle's maxLife by its speed so that particles fade in proportion to their speed
//So slower particles fade more quickly. I think this looks good in practice
//Finally, we also handle the center "bomb" particle that represents the inital explosion
//This particle doesn't move and has its life set to centerLife
void FireworksPS::spawnFirework(uint8_t fireworkNum){
    //set the particle to active
    fireWorkActive[fireworkNum] = true;
    //get a spawn position, not too close to the edge of the strip
    uint16_t spawnPos = random16( numLEDs / spawnRangeDiv, numLEDs - numLEDs / spawnRangeDiv );
    //pick a starting direction at random
    //we will alternate this direction for each subsequent particle to get an even spread on both sides
    bool initDirect = random8(2); 
    uint16_t maxSpeed = *rate + speedRange;
    uint16_t speedRangeTemp;

    //if all the particles are to be the same color, pick it here
    if(!randSparkColors){
        randColorIndex = random8(palette->length);
    }

    //For each particle in the firework, randomize its properties
    //and then set this spawn position and life
    //Note that we skip the first index, that is for the center "bomb" particle set seperately below
    for(uint8_t i = 1; i < maxNumSparks; i++){
        //the index of the particle we're setting
        particleIndex = fireworkNum * maxNumSparks + i;
        
        initDirect = !initDirect; //swap the particle direction
        //if each of the particles is to have a random color, pick it here
        if(randSparkColors){
            randColorIndex = random8(palette->length);
        }

        //To ensure that we have an evenish spread of particles
        //we cap the range that each particle's speed can vary by based on it's array index
        //Particles will start out fast, and gradually be more likley to slow down
        //( since the speed is picked by rate + random(speedRangeTemp) )
        //NOTE that speed is the particle's update rate, so higher speed value => slower particle
        speedRangeTemp = speedRange * i  / maxNumSparks;
        //randomize the particle properties
        particleUtilsPS::randomizeParticle(particleSet, particleIndex, 0, initDirect, *rate, speedRangeTemp, size, sizeRange, 
                                           0, 0, 0, false, randColorIndex, false);    
           
        //reset the particle and set its new spawn location
        particlePtr = particleSet->particleArr[particleIndex];
        particleUtilsPS::resetParticle(particlePtr);
        particlePtr->position = spawnPos;

        //Set the particles life in porportion to its speed, so faster particles live longer,
        //capped by lifeBase + random16(lifeRange) so stop really speedy particles from living too long
        //I think this looks good, but if you want a more even decay you could just use lifeBase + random16(lifeRange)
        //assuming you set lifeRange to something small like 500
        particlePtr->maxLife = min( (maxSpeed - particlePtr->speed) * 10, lifeBase + random16(lifeRange) ); 
        //lifeBase + random16(lifeRange); ->alternate, more even formula
        particlePtr->life = particlePtr->maxLife;

        //we need to store the trailEndColor for size 1 particles
        //otherwise, when they next update, trailEndColors will be empty
        trailEndColors[particleIndex] = burstColor;
    }

    //set up the center "bomb" particle properties
    //this is the first particle in each firework particle array
    //Note that we set the particle speed to 65000 (65 sec), and the lastUpdateTime to millis()
    //so that it probably won't move before it decays
    particleIndex = fireworkNum * maxNumSparks;
    particlePtr = particleSet->particleArr[particleIndex];
    particleUtilsPS::randomizeParticle(particleSet, particleIndex, 0, true, 65000, 0, centerSize, 0, 
                                       0, 0, 0, false, randColorIndex, false); 
    particlePtr->lastUpdateTime = millis();
    particlePtr->maxLife = centerLife;
    particlePtr->life = particlePtr->maxLife;
    //We want this particle to be the center of the explosion, but for larger particles
    //they are always draw with the head at the particle position, and the rest of the body after
    //so we need to offset the particle position to keep it centered
    particlePtr->position = min( spawnPos + particlePtr->size/2, numLEDs );
}
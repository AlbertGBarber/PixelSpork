#include "FirefliesSLPS.h"

//Constructor for effect with pallet
FirefliesSLPS::FirefliesSLPS(SegmentSet &SegmentSet, palletPS *Pallet, uint8_t MaxNumFireflies, uint8_t SpawnChance, 
                         uint16_t LifeBase, uint16_t LifeRange, uint16_t SpeedBase, uint16_t SpeedRange, uint16_t Rate):
    segmentSet(SegmentSet), pallet(Pallet), spawnChance(SpawnChance), lifeBase(LifeBase), 
    lifeRange(LifeRange), speedBase(SpeedBase), speedRange(SpeedRange)
    {    
        init(MaxNumFireflies, Rate);
	}

//Constructor for effect with pallet of random colors
FirefliesSLPS::FirefliesSLPS(SegmentSet &SegmentSet, uint8_t numColors, uint8_t MaxNumFireflies, uint8_t SpawnChance, 
                         uint16_t LifeBase, uint16_t LifeRange, uint16_t SpeedBase, uint16_t SpeedRange, uint16_t Rate):
    segmentSet(SegmentSet), spawnChance(SpawnChance), lifeBase(LifeBase), 
    lifeRange(LifeRange), speedBase(SpeedBase), speedRange(SpeedRange)
    {    
        palletTemp = palletUtilsPS::makeRandomPallet(numColors);
        pallet = &palletTemp;
        init(MaxNumFireflies, Rate);
	}

//constructor for effect with single color
//!!If using pre-build FastLED colors you need to pass them as CRGB( *color code* )
FirefliesSLPS::FirefliesSLPS(SegmentSet &SegmentSet, CRGB Color, uint8_t MaxNumFireflies, uint8_t SpawnChance, 
                         uint16_t LifeBase, uint16_t LifeRange, uint16_t SpeedBase, uint16_t SpeedRange, uint16_t Rate):
    segmentSet(SegmentSet), spawnChance(SpawnChance), lifeBase(LifeBase), 
    lifeRange(LifeRange), speedBase(SpeedBase), speedRange(SpeedRange)
    {    
        palletTemp = palletUtilsPS::makeSingleColorPallet(Color);
        pallet = &palletTemp;
        init(MaxNumFireflies, Rate);
	}


FirefliesSLPS::~FirefliesSLPS(){
    particleUtilsPS::deleteAllParticles(&particleSetTemp);
    delete[] particleSetTemp.particleArr;
    delete[] trailEndColors;
    delete[] particlePrevPos;
    delete[] palletTemp.palletArr;
}

//common initilzation function for core vars
void FirefliesSLPS::init(uint8_t maxNumFireflies, uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    //bind background color pointer (if needed)
    CRGB BgColor = 0;
    bindBGColorPS();
    setupFireflies(maxNumFireflies);
    //do a quick clear of the strip
    segDrawUtils::fillSegSetColor(segmentSet, *bgColor, bgColorMode);
}

//Create the data structures for a set of Fireflies (particles)
//You should call this if you ever want to change maxNumFireflies
//Fireflies need three data structures:
    //A particleSet with a paricle array of size maxNumFireflies * (maxNumSparks + 1)
    //A CRGB array of trailEndColors[maxNumFireflies] to store the trail color for each particle
    //A uint16_t array of particlePrevPos[maxNumFireflies] to store the previous particle locations
void FirefliesSLPS::setupFireflies(uint8_t newMaxNumFireflies){

    //check for any active particles on the segment set
    //we need to clear them before we make a new set of particles
    bool clearStrip = false;
    for (uint8_t i = 0; i < maxNumFireflies; i++) {
        if( particleSet->particleArr[i]->life > 0){
            clearStrip = true;
            break;
        }
    }

    //set the background if we found an active particle to clear the segment set
    if( clearStrip || fillBG || blend ){
        segDrawUtils::fillSegSetColor(segmentSet, *bgColor, bgColorMode);
    }

    //must always have at least 1 firefly spawning
    if(newMaxNumFireflies == 0){
        newMaxNumFireflies = 1;
    }
    maxNumFireflies = newMaxNumFireflies;

    //delete and re-create all the arrays and the particle set
    delete[] trailEndColors;
    trailEndColors = new CRGB[maxNumFireflies];

    delete[] particlePrevPos;
    particlePrevPos = new uint16_t[maxNumFireflies];

    particleUtilsPS::deleteAllParticles(&particleSetTemp);
    delete[] particleSetTemp.particleArr;

    //to allow the effect to work along segment lines, we use the maximum number of lines
    //as the range of the particle's motion
    numLines = segmentSet.maxSegLength;
    particleSetTemp = particleUtilsPS::buildParticleSet(maxNumFireflies, numLines, true, speedBase, speedRange, 1, 0, 
                                                        0, 0, 0, false, pallet->length, true);
    particleSet = &particleSetTemp;

    //set all the Fireflies to inactive, ready to be spawned
    for (uint8_t i = 0; i < maxNumFireflies; i++) {
        particleSet->particleArr[i]->life = 0;
    }
}
//Updates the effect
//Each update, for any active fireflies (particles) we:
    //Update the particle location (even if it hasn't moved) and decrease its life
    //Replace the previous location of the particle with the background
    //Draw the particle, wherever it may be
//For inactive (life == 0) particles, we will try to spawn them
void FirefliesSLPS::update(){
    currentTime = millis();

    //The time since the last update cycle. We need this for adjusting particle lives later
    deltaTime = currentTime - prevTime;
    if( deltaTime >= *rate ) {
        prevTime = currentTime;

        //re-fetch the segment vars in-case they've been modified
        numLines = segmentSet.maxSegLength;
        numSegs = segmentSet.numSegs;
        longestSeg = segmentSet.segNumMaxSegLength;

        //If the bg is to be filled before the particles start, fill it in
        //(such as if you have a background that's changing with time (alla bgColorMode 6))
        if( fillBG || blend ){
            segDrawUtils::fillSegSetColor(segmentSet, *bgColor, bgColorMode);
        }

        //For each firefly (particle) update it if it's life > 0
        //otherwise try to spawn it
        for (uint8_t i = 0; i < maxNumFireflies; i++) {
            //get the pointer to the particle in the particle set
            particlePtr = particleSet->particleArr[i];
            partLife = particlePtr->life;

            //if the particle is alive, we need to update it
            if(partLife > 0){
                        
                //get the particle's color from the pallet
                colorOut = palletUtilsPS::getPalletColor(pallet, particlePtr->colorIndex);

                //update the particle's location
                moveParticle(particlePtr, i);

                //if we're not filling in the background each cycle
                //we need to set the previous particle position to the background
                //Although the effect is set up to work on segment lines, we only use the
                //particle pixel on the longest segment to check when setting the background
                //This minimizes the data we have to store, and avoids a bunch of issues when working 
                //with segments with different lengths
                if ( !fillBG && !blend ) {
                    //The old particle location is stored in particlePrevPos[i]
                    //We use it to get the physical pixel location and the color it's meant to be

                    //get the physical pixel location based on the line and seg numbers
                    //we always get the pixel in the line that's on the longest segment
                    pixelNum = segDrawUtils::getPixelNumFromLineNum(segmentSet, numLines, longestSeg, particlePrevPos[i]);
                    //segDrawUtils::getPixelColor(segmentSet, &pixelInfo, *bgColor, bgColorMode, particlePrevPos[i]);

                    //only turn off the line if it hasn't been touched by another particle (or something else)
                    //this prevents background holes from being placed in other particles
                    //We only check against the pixel on the longest segment, since it will always match 1 to 1
                    //with the particle position
                    //This helps avoid issues with un-equal segments, where multiple lines can converge on a single pixel
                    //but you still only want to draw the last particle in the set for consistency
                    if(segmentSet.leds[pixelNum] == trailEndColors[i]){
                        segDrawUtils::drawSegLineSimple(segmentSet, particlePrevPos[i], *bgColor, bgColorMode);
                        //segmentSet.leds[pixelInfo.pixelLoc] = pixelInfo.color;
                    }
                }

                //Draw the particle
                drawParticlePixel(particlePtr, i);
                    
            } else {
                //try to spawn a firefly
                if (random8(100) <= spawnChance) {
                    spawnFirefly(i);
                }
            }  
        }
        showCheckPS();
    }
}

//Updates the particle location based on the inoise16() output
//It's important that each particle has a unique noise position, otherwise they will all move together
//After some testing, I found the best way to do this was to increment the noise based on the particle's speed
//and the current time. Unfortunatly the speed range is fairly small (2 - 20) unless you want really fast particles
//We also offset the particle's location by its start position. 
//This prevents particles of similar speeds from grouping up
//The overall result is not perfect, but looks pretty good most of the time
//We also reduce the particle's life by the elapsed time each time we update it
//Note, to keep things simple I'm only allowing particles of size 1 with no trails
//Likewise the motion of the particles is goverened entirely by the noise
//So overall I'm not using the particle trialType, trailSize, size, direction, bounce, or lastUpdateTime properties
void FirefliesSLPS::moveParticle(particlePS *particlePtr, uint16_t partNum) {

    partLife = particlePtr->life;
    particlePrevPos[partNum] = particlePtr->position;

    //We want each particle's noise to change uniquely over time
    //After some testing, I found that using the particle's speed worked the best
    //The speed range is only about 2 - 20 before particles start getting really fast
    //we use the partLife as a secondary input to the noise, since this is also unique to each particle
    partPos = inoise16( currentTime * particlePtr->speed, partLife ); //(currentTime * particlePtr->speed)/2  
    //inoise tends to pick values in the middle of the range, so we want to map this section to the strip location
    partPos = constrain(partPos, 13000, 51000);
    partPos = map(partPos, 13000, 51000, 0, numLines);
    //offset the particle's location by its start position. This prevents particles of similar speeds from grouping up
    partPos = addMod16PS( partPos, particlePtr->startPosition, numLines);

    //update the particle's position
    particlePtr->position = partPos;

    //reduce the particle's life by the amount of time passed in ms
    //once the particle's life hits 0 it is deactivated
    if( int32_t(partLife - deltaTime) <= 0){
        partLife = 0;
    } else {
        partLife -= deltaTime;
    }
    particlePtr->life = partLife; 
}

//Writes out a particle to the strip
//Particles have three life stages:
    //At the start they fade up to their color
    //In the middle they remain their full color
    //At the end they dim back down to the background
//The stage depends ratio between the particle's life and max life
//and the fadeThresh value, which sets the threshold ratio for the start and end stages
//The ratio and fadeThresh are both out of 255
//The final color of a particle is also subject to "flicker" to simulate the firefly's light,
//which reduces the brightness according to a noise8() function
void FirefliesSLPS::drawParticlePixel(particlePS *particlePtr, uint16_t partNum){

    //get the where we are in the color blend based on the particle's life
    //(the blend has 255 steps total)
    partLife = particlePtr->life;
    lifeRatio = (partLife * 255) / particlePtr->maxLife;
    partPos = particlePtr->position;
    
    //Get what life stage the particle is in: fading in, solid, or fading out
    //the fading in/out portions are set by fadeThresh, as a number of steps out of the total lifeRatio (255)
    //For fading in/out we need to complete the fade over the fadeThresh number of steps, even though the 
    //lifeRatio is still out of 255
    //So we need to do some extra math to map the values
    fadeMax = 255 - fadeThresh; //the threshold for fading in
    if(lifeRatio >= fadeMax){
        //fading in, we need to scale lifeRatio - fadeMax to fadeThresh
        //lifeRatio - fadeMax gives us a value that goes from 0 to fadeThresh
        dimRatio = 255 - ( (lifeRatio - fadeMax) * 255 ) / fadeThresh;
        fadeType = 0;
    } else if(lifeRatio < fadeThresh) {
        //fading out, we need to scale lifeRatio - fadeThresh to fadeThresh
        //lifeRatio - fadeThresh gives us a value that goes from fadeThresh to 0
        dimRatio = ( (fadeThresh - lifeRatio) * 255 ) / fadeThresh;
        fadeType = 1;
    } else {
        //the middle life stage, just a solid color
        fadeType = 2;
    }

    //add a random "flicker" brightness adjustment to the color
    if(flicker){
        //To look smooth, the brightness is based on at noise function
        flickerBri = inoise8(particlePtr->startPosition, partLife * 10); //millis() * 2
        //to prevent too much flicker, we constrain the bightness
        flickerBri = 255 - constrain(flickerBri, 50, 200);
    }

    //draw the particle's line across the segments
    //note that, to account for color modes, we get the target particle and background colors for each 
    //pixel in the line
    for(uint8_t i = 0; i < numSegs; i++){
        //get the pixel's physical location and adjust for any color modes
        //also fetch the background and particle color at this point 
        pixelNum = segDrawUtils::getPixelNumFromLineNum(segmentSet, numLines, i, partPos);
        //segDrawUtils::getPixelColor(segmentSet, &pixelInfo, colorOut, colorMode, particlePtr->position);
        bgCol = segDrawUtils::getPixelColor(segmentSet, pixelNum, *bgColor, bgColorMode, i, partPos);
        colorFinal = segDrawUtils::getPixelColor(segmentSet, pixelNum, colorOut, colorMode, i, partPos);
        
        //set the color based on if we're fading in or out
        switch(fadeType){
            case 0:
                //fading in, from bgColor to the particle color
                colorFinal = colorUtilsPS::getCrossFadeColor(bgCol, colorFinal, dimRatio);
                break;  
            case 1:
                //fading out, from the particle color to the background
                colorFinal = colorUtilsPS::getCrossFadeColor(colorFinal, bgCol, dimRatio);
                break;
            //case 2:
                //the middle life stage, just a solid color
                //colorFinal = colorFinal;
                //break;
        }

        //dim the color by the flicker brightness
        nscale8x3(colorFinal.r, colorFinal.g, colorFinal.b, flickerBri);

        //output the color
        if(blend){
            segmentSet.leds[pixelNum] += colorFinal;
        } else {
            segmentSet.leds[pixelNum] = colorFinal;
        }    

        //if we're writing out the pixel on the longest segment, we need to record the 
        //color, so we can use it for setting the background in update()
        if(i == longestSeg){
            //Record the output color for setting the background in the next update
            trailEndColors[partNum] = segmentSet.leds[pixelNum];  
        }
    }           
}

//Spawns a firefly by randomizing an extisting particle
//The particle is given a new speed, start position and color index
//The size is locked at one
void FirefliesSLPS::spawnFirefly(uint8_t partNum){

    //randomize the particle properties (position is only up to numLines)
    particleUtilsPS::randomizeParticle(particleSet, partNum, numLines, true, speedBase, speedRange, 
                                       1, 0, 0, 0, 0, false, pallet->length, true);    
           
    //reset the particle and set its new spawn location
    particlePtr = particleSet->particleArr[partNum];
    particleUtilsPS::resetParticle(particlePtr);

    particlePtr->maxLife = lifeBase + random16(lifeRange); 
    particlePtr->life = particlePtr->maxLife;

    drawParticlePixel(particlePtr, partNum);
}
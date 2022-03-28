#include "DissolvePS.h"

//constructor for pattern
DissolvePS::DissolvePS(SegmentSet &SegmentSet, patternPS *Pattern, palletPS *Pallet, uint8_t DMode, uint16_t SpawnRateInc, uint16_t Rate):
    segmentSet(SegmentSet), pattern(Pattern), pallet(Pallet), dMode(DMode), spawnRateInc(SpawnRateInc)
    {    
        init(Rate);
	}

//constructor for pallet as pattern
DissolvePS::DissolvePS(SegmentSet &SegmentSet, palletPS *Pallet, uint8_t DMode, uint16_t SpawnRateInc, uint16_t Rate):
    segmentSet(SegmentSet), pallet(Pallet), dMode(DMode), spawnRateInc(SpawnRateInc)
    {
        setPalletAsPattern();
        init(Rate);
    }

//constructor for randomly choosen colors
DissolvePS::DissolvePS(SegmentSet &SegmentSet, uint8_t DMode, uint16_t SpawnRateInc, uint16_t Rate):
    segmentSet(SegmentSet), dMode(DMode), spawnRateInc(SpawnRateInc)
    {
        //although we're randomly choosing colors, we still make a pallet and pattern 
        //so that if the dMode is changed later, there's still a pallet/pattern to use
        palletTemp = palletUtilsPS::makeRandomPallet(2);
        pallet = &palletTemp;
        setPalletAsPattern();
        init(Rate);
    }

//destructor
DissolvePS::~DissolvePS(){
    delete[] palletTemp.palletArr;
    delete[] pixelArray;
    delete[] patternTemp.patternArr;
}

//inits core variables for the effect
//caculates the setAllThreshold to be 1/10th of the number of leds (rounded up)
void DissolvePS::init(uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    resetPixelArray();
    setAllThreshold = numActiveLeds - ceil( (float) (numActiveLeds) / 10);
}

//resets the pixel array to false
//also resets the core effect variables as needed
void DissolvePS::resetPixelArray(){
    numActiveLeds = segmentSet.numActiveSegLeds;
    delete[] pixelArray;
    pixelArray = new bool[numActiveLeds];
    for(int i = 0; i < numActiveLeds; i++){
        pixelArray[i] = false;
    }
    numMaxSpawn = numMaxSpawnBase;
    numSpawned = 0;
    thresStartPoint = 0;
    randColorPicked = false;
}

//binds the pallet to a new one
void DissolvePS::setPallet(palletPS *newPallet){
    pallet = newPallet;
}

//sets a new pattern for the effect
void DissolvePS::setPattern(patternPS *newPattern){
    pattern = newPattern;
}

//sets the pattern to match the current pallet
//ie for a pallet length 5, the pattern would be 
//{0, 1, 2, 3, 4}
void DissolvePS::setPalletAsPattern(){
    patternTemp = generalUtilsPS::setPalletAsPattern(pallet);
    pattern = &patternTemp;
}

//set a color based on the pattern and dMode
//see effect description for pattern info
//dModes:
    //0: Each dissolve is a solid color following the pattern
    //1: Each dissolve is a set of random colors choosen from the pattern
    //2: Each dissolve is a set of randomly choosen colors
    //3: Each dissolve is a solid color choosen at random
    //4: Each dissolve is a solid color choosen randomly from the pattern
void DissolvePS::pickColor(){
    if(dMode == 0){
        //cycle through the pattern
        currentIndex = patternUtilsPS::getPatternVal(pattern, numCycles);
        color = palletUtilsPS::getPalletColor(pallet, currentIndex );
    } else if(dMode == 1){
        //choose colors randomly from the pattern
        color = palletUtilsPS::getPalletColor(pallet, patternUtilsPS::getRandVal(pattern) );
    } else if(dMode == 2){
        //choose colors randomly
        color = colorUtilsPS::randColor();
    } else {
        //for modes 3 and 4, the colors must only be picked once, since they are choosen randomly
        //hence the ranColorPicked flag
        //(This could also apply to mode 0, but we want to check the color each time for palletBlending)
        if( !randColorPicked ){
            if(dMode == 3){
                color = colorUtilsPS::randColor();
            } else if(dMode == 4) {
                currentIndex = patternUtilsPS::getShuffleIndex(pattern, currentIndex);
                color = palletUtilsPS::getPalletColor( pallet, currentIndex );
            }
            randColorPicked = true;
        }
    }
}

//Updates the effect
//How it works:
    //Our goal is to switch all of the leds from one color to the next at random
    //To record if any led has been switched we use an array of bools, one for each led
    //When an led is switched, it's corrosponding index in the array is switched to true
        //Each cycle we try to switch up to numMaxSpawn leds, picking them at random
        //If we pick an led that has not been switched, we switch it
        //We also increment numSpawned, which tracks how many leds have been switched so far
    //To avoid getting stuck with just a few leds not switched (and missing them every time due to the randomness)
    //We use setAllThreshold, which is the maximum number of leds we'll try to set randomly
    //(setAllThreshold is set by init() based on the length of the segmentSet)
    //If numSpawned passes setAllThreshold, we'll set any remaining leds in order (up to numMaxSpawn leds per cycle)
    //Once all the led have been set (numSpawned >= numActiveLeds)
    //We reset the pixel array, numMaxSpawn, thresStartPoint (used when setting the leds once the threshold is met)
    //We also set hangTimeOn, and increment the numCycles (which tracks how many dissolves we've done, used for setting colors)
    //Hang Time:
        //Hang time holds the current dissolve for a certain period before starting a new one
        //It is triggered after a dissolve is complete using hangTimeOn
        //during hangTime nothing from the effect is drawn or incremented
void DissolvePS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate) {
        
        //code for pausing the effect after a dissolve is finished
        //if we're in hang time, we simply return to break out of the function 
        if(hangTimeOn){
            if( (currentTime - prevTime) > hangTime ){
                hangTimeOn = false;
            } else {
                return;
            }
        }

        prevTime = currentTime;
        
        //try to spawn up to numMaxSpawn leds
        //the leds will either be spawned randomly, or in order along the strip
        //depending on if the theshold for setting all has been reached
        for(uint8_t i = 0; i < numMaxSpawn; i++){
            //if we're not passed the threshold for setting all the leds,
            //choose one randomly and try to set it
            if(numSpawned < setAllThreshold){
                pixelNum = random16( numActiveLeds );
                if(!pixelArray[pixelNum]){
                    spawnLed(pixelNum);
                }
            } else {
                //if we're setting all the leds we run through them all until we find
                //one that's not set, we set it, then end the loop
                //we also record the starting point for the loop next time,
                //since we already know that all the leds up to that point have been set
                for(uint16_t j = thresStartPoint; j < numActiveLeds; j++){
                    if(!pixelArray[j]){
                        spawnLed(j);
                        thresStartPoint = j;
                        break;
                    }
                }
            }
        }

        //check if the disolve is finished
        //if it is we need to reset to start a new dissolve
        //turn the hang time on, and increment the cycle number
        if(numSpawned >= numActiveLeds){
            resetPixelArray();
            hangTimeOn = true;
            numCycles = addmod8(numCycles, 1, pattern->length); //(numCycles +1) % pattern->length
        }

        showCheckPS();
    }

    //Determines if it's time to increase the spawn cap
    //This is checked independent of the effect loop, 
    //to decouple the spawn increase time from the effect rate
    if( ( currentTime - prevSpawnTime ) >= spawnRateInc ){
        prevSpawnTime = currentTime;
        numMaxSpawn++;
    }
}

//Colors an led at the specified pixel number
//records that it's set to the array and also increments the number of pixels spawned
void DissolvePS::spawnLed(uint16_t pixelNum){
    pixelArray[pixelNum] = true;
    pickColor();
    segDrawUtils::setPixelColor(segmentSet, pixelNum, color, colorMode);
    numSpawned++;
}
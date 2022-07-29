#include "DissolveSLPS.h"

//constructor for pattern
DissolveSLPS::DissolveSLPS(SegmentSet &SegmentSet, patternPS *Pattern, palletPS *Pallet, uint8_t DMode, uint16_t SpawnRateInc, uint16_t Rate):
    segmentSet(SegmentSet), pattern(Pattern), pallet(Pallet), dMode(DMode), spawnRateInc(SpawnRateInc)
    {    
        init(Rate);
	}

//constructor for pallet as pattern
DissolveSLPS::DissolveSLPS(SegmentSet &SegmentSet, palletPS *Pallet, uint8_t DMode, uint16_t SpawnRateInc, uint16_t Rate):
    segmentSet(SegmentSet), pallet(Pallet), dMode(DMode), spawnRateInc(SpawnRateInc)
    {
        setPalletAsPattern();
        init(Rate);
    }

//constructor for randomly choosen colors (should only use dMode 2 or 3 with this constructor)
DissolveSLPS::DissolveSLPS(SegmentSet &SegmentSet, uint8_t DMode, uint16_t SpawnRateInc, uint16_t Rate):
    segmentSet(SegmentSet), dMode(DMode), spawnRateInc(SpawnRateInc)
    {
        //although we're randomly choosing colors, we still make a pallet and pattern 
        //so that if the dMode is changed later, there's still a pallet/pattern to use
        palletTemp = palletUtilsPS::makeRandomPallet(3);
        pallet = &palletTemp;
        setPalletAsPattern();
        init(Rate);
    }

//destructor
DissolveSLPS::~DissolveSLPS(){
    delete[] palletTemp.palletArr;
    delete[] pixelArray;
    delete[] patternTemp.patternArr;
}

//inits core variables for the effect
//caculates the setAllThreshold to be 1/10th of the number of leds (rounded up)
void DissolveSLPS::init(uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    resetPixelArray();
    //note that numLines is set in resetPixelArray();
    setAllThreshold = numLines - ceil( (float) (numLines) / 10);
}

//resets the pixel array to false
//also resets the core effect variables as needed
void DissolveSLPS::resetPixelArray(){
    //create an array of bools, one bool for each line in the segment set.
    //These record if the line has switched colors or not.
    numLines = segmentSet.maxSegLength;
    delete[] pixelArray;
    pixelArray = new bool[numLines];
    for(int i = 0; i < numLines; i++){
        pixelArray[i] = false;
    }
    numMaxSpawn = numMaxSpawnBase;
    numSpawned = 0;
    thresStartPoint = 0;
    randColorPicked = false;
}

//sets the pattern to match the current pallet
//ie for a pallet length 5, the pattern would be 
//{0, 1, 2, 3, 4}
void DissolveSLPS::setPalletAsPattern(){
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
CRGB DissolveSLPS::pickColor(){
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
    return color;
}

//Updates the effect
//How it works:
    //Our goal is to switch all of the lines from one color to the next at random
    //To record if any line has been switched we use an array of bools, one for each line
    //When an line is switched, it's corrosponding index in the array is switched to true
        //Each cycle we try to switch up to numMaxSpawn line, picking them at random
        //If we pick an line that has not been switched, we switch it
        //We also increment numSpawned, which tracks how many lines have been switched so far
    //To avoid getting stuck with just a few lines not switched (and missing them every time due to the randomness)
    //We use setAllThreshold, which is the maximum number of lines we'll try to set randomly
    //(setAllThreshold is set by init() based on the length of the segmentSet)
    //If numSpawned passes setAllThreshold, we'll set any remaining lines in order (up to numMaxSpawn lines per cycle)
    //Once all the lines have been set (numSpawned >= numLines)
    //We reset the pixel array, numMaxSpawn, thresStartPoint (used when setting the lines once the threshold is met)
    //We also set hangTimeOn, and increment the numCycles (which tracks how many dissolves we've done, used for setting colors)
    //Hang Time:
        //Hang time holds the current dissolve for a certain period before starting a new one
        //It is triggered after a dissolve is complete using hangTimeOn
        //during hangTime nothing from the effect is drawn or incremented
void DissolveSLPS::update(){
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
        
        //try to spawn up to numMaxSpawn lines
        //the lines will either be spawned randomly, or in order along the strip
        //depending on if the theshold for setting all has been reached
        for(uint8_t i = 0; i < numMaxSpawn; i++){
            //if we're not passed the threshold for setting all the lines,
            //choose one randomly and try to set it
            if(numSpawned < setAllThreshold){
                lineNum = random16(numLines);
                if(!pixelArray[lineNum]){
                    spawnLed(lineNum);
                }
            } else {
                //if we're setting all the lines we run through them all until we find
                //one that's not set, we set it, then end the loop
                //we also record the starting point for the loop next time,
                //since we already know that all the leds up to that point have been set
                for(uint16_t j = thresStartPoint; j < numLines; j++){
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
        if(numSpawned >= numLines){
            resetPixelArray();
            hangTimeOn = true;
            numCycles = addmod8(numCycles, 1, pattern->length);
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

//Colors a line at the specified line number
//records that it's set to the array and also increments the number of lines spawned
void DissolveSLPS::spawnLed(uint16_t lineNum){
    pixelArray[lineNum] = true;
    color = pickColor();
    //write the color out to all the leds in the segment line
    segDrawUtils::drawSegLineSimple(segmentSet, lineNum, color, colorMode);
    //segDrawUtils::setPixelColor(segmentSet, lineNum, color, colorMode);
    numSpawned++;
}
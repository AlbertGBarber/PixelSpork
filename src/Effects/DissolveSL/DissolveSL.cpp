#include "DissolveSL.h"

//constructor for pattern
DissolveSL::DissolveSL(SegmentSet &SegmentSet, patternPS &Pattern, palettePS &Palette, uint8_t RandMode, uint16_t SpawnRateInc, uint16_t Rate):
    segmentSet(SegmentSet), pattern(&Pattern), palette(&Palette), randMode(RandMode), spawnRateInc(SpawnRateInc)
    {    
        init(Rate);
	}

//constructor for palette as pattern
DissolveSL::DissolveSL(SegmentSet &SegmentSet, palettePS &Palette, uint8_t RandMode, uint16_t SpawnRateInc, uint16_t Rate):
    segmentSet(SegmentSet), palette(&Palette), randMode(RandMode), spawnRateInc(SpawnRateInc)
    {
        setPaletteAsPattern();
        init(Rate);
    }

//constructor for randomly choosen colors (should only use randMode 1 or 3 with this constructor)
DissolveSL::DissolveSL(SegmentSet &SegmentSet, uint8_t RandMode, uint16_t SpawnRateInc, uint16_t Rate):
    segmentSet(SegmentSet), randMode(RandMode), spawnRateInc(SpawnRateInc)
    {
        //although we're randomly choosing colors, we still make a palette and pattern 
        //so that if the randMode is changed later, there's still a palette/pattern to use
        paletteTemp = paletteUtilsPS::makeRandomPalette(3);
        palette = &paletteTemp;
        setPaletteAsPattern();
        init(Rate);
    }

//destructor
DissolveSL::~DissolveSL(){
    free(paletteTemp.paletteArr);
    free(patternTemp.patternArr);
    free(pixelArray);
}

//inits core variables for the effect
void DissolveSL::init(uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    setLineMode(lineMode);
}

//sets the pattern to match the current palette
//ie for a palette length 5, the pattern would be 
//{0, 1, 2, 3, 4}
void DissolveSL::setPaletteAsPattern(){
    patternTemp = generalUtilsPS::setPaletteAsPattern(*palette);
    pattern = &patternTemp;
}

//Sets the line mode var, also restarts the dissolve, 
//and sets the setAllTheshold to 1/10 the numLines
void DissolveSL::setLineMode(bool newLineMode){
    lineMode = newLineMode;
    resetPixelArray();
    setAllThreshold = numLines - ceil( (float) (numLines) / 10);
}

//resets the pixel array to false, restarting the dissolve
//also resets the core effect variables as needed
void DissolveSL::resetPixelArray(){
    //if we're in line mode, then we'll be setting whole lines at once
    //otherwise we'll be setting each pixel individually, so we need to set the numLines to match
    //(we use numLines, even though it's more like numPixels for the individual case to keep the code clean)
    if(lineMode){
        numLines = segmentSet.numLines;
    } else {
        numLines = segmentSet.numLeds;
    }

    //To record if a line has been switched or not, we need to create an array of bools
    //(but only if we don't already have an array of the right size)
    if(prevNumLines != numLines){
        free(pixelArray);
        pixelArray = (bool*) malloc(numLines * sizeof(bool));
        prevNumLines = numLines;
    }

    //reset the bool array, indicating that all the lines need to be spawned
    for(uint16_t i = 0; i < numLines; i++){
        pixelArray[i] = false;
    }

    maxNumSpawn = maxNumSpawnBase;
    numSpawned = 0;
    thresStartPoint = 0;
    randColorPicked = false;
    paused = false;
}

//set a color based on the pattern and randMode
//see effect description for pattern info
//randModes:
    //0: Each dissolve is a solid color following the pattern
    //1: Each dissolve is a set of randomly choosen colors
    //2: Each dissolve is a set of random colors choosen from the pattern
    //3: Each dissolve is a solid color choosen at random
    //4: Each dissolve is a solid color choosen randomly from the pattern
CRGB DissolveSL::pickColor(){
    if(randMode == 0){
        //cycle through the pattern
        currentIndex = patternUtilsPS::getPatternVal(*pattern, numCycles);
        color = paletteUtilsPS::getPaletteColor(*palette, currentIndex );
    } else if(randMode == 1){
        //choose colors randomly
        color = colorUtilsPS::randColor();
        //choose colors randomly from the pattern
        color = paletteUtilsPS::getPaletteColor(*palette, patternUtilsPS::getRandVal(*pattern) );
    } else if(randMode == 2){
        //choose colors randomly from the pattern
        color = paletteUtilsPS::getPaletteColor(*palette, patternUtilsPS::getRandVal(*pattern) );
    } else {
        //for modes 3 and 4, the colors must only be picked once, since they are choosen randomly
        //hence the ranColorPicked flag
        //(This could also apply to mode 0, but we want to check the color each time for paletteBlending)
        if( !randColorPicked ){
            if(randMode == 3){
                color = colorUtilsPS::randColor();
            } else if(randMode == 4) {
                currentIndex = patternUtilsPS::getShuffleIndex(*pattern, currentIndex);
                color = paletteUtilsPS::getPaletteColor( *palette, currentIndex );
            }
            randColorPicked = true;
        }
    }
    return color;
}

//Updates the effect
//How it works:
    //Our goal is to switch all of the lines from one color to the next at random
    //To record if any line has been switched we use an array of bools, one for each line (pixelArray)
    //When an line is switched, it's corrosponding index in the array is switched to true
        //Each cycle we try to switch up to maxNumSpawn lines, picking them at random
        //If we pick an line that has not been switched, we switch it
        //We also increment numSpawned, which tracks how many lines have been switched so far
        //maxNumSpawn increases every spawnRateInc ms, to help speed up the spawing over time
    //To avoid getting stuck with just a few lines not switched (and missing them every time due to the randomness)
    //We use setAllThreshold, which is the maximum number of lines we'll try to set randomly
    //(setAllThreshold is set by init() based on the length of the segmentSet)
    //If numSpawned passes setAllThreshold, we'll set any remaining lines in order (up to maxNumSpawn lines per cycle)
    //Once all the lines have been set (numSpawned >= numLines)
    //We reset the pixel array, maxNumSpawn, thresStartPoint (used when setting the lines once the threshold is met)
    //We also set paused, and increment the numCycles (which tracks how many dissolves we've done, used for setting colors)
    //Hang Time:
        //Hang time holds the current dissolve for a certain period before starting a new one
        //It is triggered after a dissolve is complete using paused
        //during pauseTime nothing from the effect is drawn or incremented
void DissolveSL::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate) {
        
        //code for pausing the effect after a dissolve is finished
        //if we're in pause time, we simply return to break out of the function 
        if(paused){
            if( (currentTime - prevTime) > pauseTime ){
                paused = false;
            } else {
                return;
            }
        }

        prevTime = currentTime;
        
        //try to spawn up to maxNumSpawn lines
        //the lines will either be spawned randomly, or in order along the strip
        //depending on if the theshold for setting all has been reached
        for(uint8_t i = 0; i < maxNumSpawn; i++){
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
        //turn the pause time on, and increment the cycle number
        if(numSpawned >= numLines){
            resetPixelArray();
            dissolveCount++;
            paused = true;
            numCycles = addMod16PS(numCycles, 1, pattern->length);
        }

        showCheckPS();
    }

    //Determines if it's time to increase the spawn cap (if we're not paused)
    //This is checked independent of the effect loop, 
    //to decouple the spawn increase time from the effect rate
    if( !paused && ( currentTime - prevSpawnTime ) >= spawnRateInc ){
        prevSpawnTime = currentTime;
        maxNumSpawn++;
    }
}

//Colors a line at the specified line number
//records that it's set to the array and also increments the number of lines spawned
void DissolveSL::spawnLed(uint16_t lineNum){
    pixelArray[lineNum] = true;
    color = pickColor();

    if(lineMode){
        //write the color out to all the leds in the segment line
        segDrawUtils::drawSegLine(segmentSet, lineNum, color, colorMode);
    } else {
        segDrawUtils::setPixelColor(segmentSet, lineNum, color, colorMode);
    }
    numSpawned++;
}
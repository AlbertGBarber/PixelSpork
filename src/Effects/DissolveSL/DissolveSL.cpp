#include "DissolveSL.h"

//constructor for pattern
DissolveSL::DissolveSL(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, uint8_t RandMode,
                       uint16_t SpawnRateInc, uint16_t Rate)
    : pattern(&Pattern), palette(&Palette), randMode(RandMode), spawnRateInc(SpawnRateInc)  //
{
    bgMode = 0;  //since we're using a set pattern, bgMode isn't being used
    init(SegSet, Rate);
}

//constructor for palette as pattern
DissolveSL::DissolveSL(SegmentSetPS &SegSet, palettePS &Palette, uint8_t RandMode, uint8_t BgMode, uint16_t SpawnRateInc, uint16_t Rate)
    : palette(&Palette), randMode(RandMode), bgMode(BgMode), spawnRateInc(SpawnRateInc)  //
{
    setPaletteAsPattern();
    init(SegSet, Rate);
}

//constructor for randomly chosen colors (should only use randMode 1 or 3 with this constructor)
//note that RandMode3CycleLen is only relevant for bgMode 1 and randMode 3.
//it controls how many colors are cycled before the background color is used. (see intro bgMode section for more)
DissolveSL::DissolveSL(SegmentSetPS &SegSet, uint8_t RandMode, uint8_t BgMode, uint16_t RandMode3CycleLen, uint16_t SpawnRateInc, uint16_t Rate)
    : randMode(RandMode), bgMode(BgMode), spawnRateInc(SpawnRateInc)  //
{
    //although we're randomly choosing colors, we still make a palette and pattern
    //so that if the randMode is changed later, there's still a palette/pattern to use
    paletteTemp = paletteUtilsPS::makeRandomPalette(3);
    palette = &paletteTemp;
    setPaletteAsPattern();
    init(SegSet, Rate);
    //we need to bind the randMode 3 cycle limit after init b/c init sets it to the pattern length by default
    randMode3CycleLen = RandMode3CycleLen;
}

//destructor
DissolveSL::~DissolveSL() {
    free(paletteTemp.paletteArr);
    free(patternTemp.patternArr);
    free(pixelArray);
}

//inits core variables for the effect
void DissolveSL::init(SegmentSetPS &SegSet, uint16_t Rate) {
    //bind the rate and segSet pointer vars since they are inherited from BaseEffectPS
    bindSegSetPtrPS();
    bindClassRatesPS();
    setLineMode(lineMode);
    //set the cycle limit for randMode 3 just in case
    randMode3CycleLen = pattern->length - 1;
}

//sets the pattern to match the current palette
//ie for a palette length 5, the pattern would be {0, 1, 2, 3, 4}
//Also injects "blank" spaces in the pattern depending on the bgMode setting.
//The spaces are set to 255 in the pattern, which is recognized as the bgColor for the rest of the effect.
//See setBgMode() below for the bgModes list
void DissolveSL::setPaletteAsPattern() {
    uint8_t spacing = 0;

    //Manage the bgMode
    if( bgMode == 1 ) {
        //We're going to build the pattern to match the palette automatically below,
        //but for bgMode 1, we need an extra space in the pattern
        //Unfortunately, this means we need to "trick" the code into thinking the is palette longer by adjusting its length
        //So we increment the palette length here, and then decrement it once the pattern is made
        //!!This is only ok because it's all being done in one function call. You shouldn't be manipulating palette lengths usually!
        palette->length = palette->length + 1;
    } else if( bgMode == 2 ) {
        //For bgMode 2, we're adding a space between each palette color, we can do this by setting a spacing
        //value of 1 for when we call setPaletteAsPattern() below
        spacing = 1;
    }

    //Set patternTemp to match the palette, possibly with a single space in-between each color
    generalUtilsPS::setPaletteAsPattern(patternTemp, *palette, 1, spacing);

    //For bgMode 1, once the pattern is created, we need to revert the palette back to its original length
    //and also set the final pattern value to 255 (so it's recognized as the background color)
    if( bgMode == 1 ) {
        palette->length = palette->length - 1;
        patternUtilsPS::setVal(patternTemp, 255, patternTemp.length - 1);
    }

    pattern = &patternTemp;
}

//Changes the bgMode to add background spaces to the dissolve pattern
//bgModes:
//  0 -- No spaces (ex: {0, 1, 2, 3, 4}, where the values are palette indexes)
//  1 -- One space added to the end of the pattern (ex: {0, 1, 2, 3, 4, 255})
//  2 -- A space is added after each color (ex: {0, 255, 1, 255, 2, 255, 3, 255, 4, 255})
//(background spaces are denoted by 255 in the patterns)
//Note that changing the bgMode also changes the dissolve pattern to use patternTemp
//and re-writes patternTemp to a new pattern for the bgMode.
void DissolveSL::setBgMode(uint8_t newBgMode) {
    if( newBgMode != bgMode ) {
        bgMode = newBgMode;
        setPaletteAsPattern();
    }
}

//Sets the line mode var, also restarts the dissolve,
//and sets the setAllThreshold to 1/10 the numLines
void DissolveSL::setLineMode(bool newLineMode) {
    lineMode = newLineMode;
    resetPixelArray();
    setAllThreshold = numLines - ceil((float)(numLines) / 10);
}

//resets the pixel array to false, restarting the dissolve
//also resets the core effect variables as needed
//This is called automatically as part of the effect, but if you change the segment set, you should call this
void DissolveSL::resetPixelArray() {
    //if we're in line mode, then we'll be setting whole lines at once
    //otherwise we'll be setting each pixel individually, so we need to set the numLines to match
    //(we use numLines, even though it's more like numPixels for the individual case to keep the code clean)
    if( lineMode ) {
        numLines = segSet->numLines;
    } else {
        numLines = segSet->numLeds;
    }

    //To record if a line has been switched or not, we need to create an array of bools
    //We only need to make a new array if the current one isn't large enough
    //This helps prevent memory fragmentation by limiting the number of heap allocations
    //but this may use up more memory overall.
    if( alwaysResizeObj_PS || (numLines > maxNumLines) ) {
        maxNumLines = numLines;
        free(pixelArray);
        pixelArray = (bool *)malloc(numLines * sizeof(bool));
    }

    //reset the bool array, indicating that all the lines need to be spawned
    for( uint16_t i = 0; i < numLines; i++ ) {
        pixelArray[i] = false;
    }

    maxNumSpawn = maxNumSpawnBase;
    numSpawned = 0;
    thresStartPoint = 0;
    randColorPicked = false;
    paused = false;
}

/* set a color based on the pattern and randMode
see effect description for pattern info
randModes:
    0: Each dissolve is a solid color following the pattern
    1: Each dissolve is a set of randomly chosen colors
    2: Each dissolve is a set of random colors chosen from the pattern
    3: Each dissolve is a solid color chosen at random
    4: Each dissolve is a solid color chosen randomly from the pattern 
    5: Each dissolve is a solid color chosen randomly from the pattern, but a "blank" dissolve is 
       done between each color (see "Inserting Background Steps" below). 
       By default, the same dissolve color won't repeat after a blank, but you can allow
       repeats by setting "randMode5AllowRepeats" to true (making the colors choosen at random).
       Note that bgMode should be set to 0 for this randMode.
Note that in the pattern, a value of 255 will indicates the background color 
(See "Inserting Background Steps" in Intro for randMode + bgMode behaviors)*/
CRGB DissolveSL::pickDissolveColor() {
    if( randMode == 0 ) {
        //cycle through the pattern
        currentIndex = patternUtilsPS::getPatternVal(*pattern, numCycles);
        //get the next dissolve color
        color = getPatternColor(currentIndex);
    } else if( randMode == 1 ) {
        //choose colors randomly
        color = colorUtilsPS::randColor();
    } else if( randMode == 2 ) {
        //choose colors randomly from the pattern for each pixel
        //note that we use a separate variable, tempIndex for tracking, so that we don't overwrite the currentIndex value
        //(incase the randMode is changed, we want to keep the tracking)
        tempIndex = patternUtilsPS::getRandVal(*pattern);
        //get the next dissolve color
        color = getPatternColor(tempIndex);
    } else {
        //for modes 3, 4, and 5 the colors must only be picked once, since they are chosen randomly
        //hence the ranColorPicked flag
        //(This could also apply to mode 0, but we want to check the color each time for paletteBlending)
        if( !randColorPicked ) {
            flipFlop = !flipFlop;  //tracking for bgMode 2 (can't use odd/even cycle check since it resets, so even cycle lengths produce double blanks)
            if( randMode == 3 ) {
                //Pick a random color for the next dissolve
                //for randMode 3, we still want to respect the background mode, but we aren't using the pattern
                //so we need to check the bgMode conditions manually,
                //either by adding a blank once "randMode3CycleLen" number of cycles (user configurable, default to patten length) (bgMode 1),
                //or by adding a blank every other cycle (bgMode 2)
                //If bgMode is 0, no blanks are added
                if( (bgMode == 1 && numCycles == (randMode3CycleLen)) || (bgMode == 2 && flipFlop) ) {
                    color = *bgColor;
                } else {
                    color = colorUtilsPS::randColor();
                }
            } else if( randMode == 4 ) {
                currentIndex = patternUtilsPS::getShuffleVal(*pattern, currentIndex, true); //get a shuffled color index, allowing blanks
                //get the next dissolve color
                color = getPatternColor(currentIndex);
            } else if( randMode == 5 ) {
                //Every other dissolve is picked randomly from the pattern, alternating between the blank color
                //So every color dissolves to the blank color and then to a new random color
                //Note that by default, the same color won't repeat, as the currentIndex is maintained between blanks,
                //but we can change this using "randMode5AllowRepeats", which if true, randomizes the currentIndex each cycle 
                if( flipFlop ) {
                    color = *bgColor;
                    if(randMode5AllowRepeats){
                        currentIndex = random16(pattern->length);
                    }
                } else {
                    currentIndex = patternUtilsPS::getShuffleVal(*pattern, currentIndex, false); //get a shuffled color index, NOT allowing blanks
                    //get the next dissolve color
                    color = getPatternColor(currentIndex);
                }
            }
            randColorPicked = true;
        }
    }
    return color;
}

//Returns the next dissolve color, using a pattern array index
//if the index is 255, then the spacing background color is used
CRGB DissolveSL::getPatternColor(uint8_t patIndex) {
    if( patIndex == 255 ) {
        color = *bgColor;
    } else {
        color = paletteUtilsPS::getPaletteColor(*palette, patIndex);
    }
    return color;
}

//Updates the effect
//How it works:
//Our goal is to switch all of the lines from one color to the next at random
//To record if any line has been switched we use an array of bools, one for each line (pixelArray)
//When an line is switched, it's corresponding index in the array is switched to true
//Each cycle we try to switch up to maxNumSpawn lines, picking them at random
//If we pick an line that has not been switched, we switch it
//We also increment numSpawned, which tracks how many lines have been switched so far
//maxNumSpawn increases every spawnRateInc ms, to help speed up the spawning over time
//To avoid getting stuck with just a few lines not switched (and missing them every time due to the randomness)
//We use setAllThreshold, which is the maximum number of lines we'll try to set randomly
//(setAllThreshold is set by init() based on the length of the segSet)
//If numSpawned passes setAllThreshold, we'll set any remaining lines in order (up to maxNumSpawn lines per cycle)
//Once all the lines have been set (numSpawned >= numLines)
//We reset the pixel array, maxNumSpawn, thresStartPoint (used when setting the lines once the threshold is met)
//We also set paused, and increment the numCycles (which tracks how many dissolves we've done, used for setting colors)
//Hang Time:
//Hang time holds the current dissolve for a certain period before starting a new one
//It is triggered after a dissolve is complete using paused
//during pauseTime nothing from the effect is drawn or incremented
void DissolveSL::update() {
    currentTime = millis();

    if( (currentTime - prevTime) >= *rate ) {

        //code for pausing the effect after a dissolve is finished
        //if we're in pause time, we simply return to break out of the function
        if( paused ) {
            if( (currentTime - prevTime) > pauseTime ) {
                paused = false;
            } else {
                return;
            }
        }

        prevTime = currentTime;

        //try to spawn up to maxNumSpawn lines
        //the lines will either be spawned randomly, or in order along the strip
        //depending on if the threshold for setting all has been reached
        for( uint16_t i = 0; i < maxNumSpawn; i++ ) {
            //if we're not passed the threshold for setting all the lines,
            //choose one randomly and try to set it
            if( numSpawned < setAllThreshold ) {
                lineNum = random16(numLines);
                if( !pixelArray[lineNum] ) {
                    spawnLed(lineNum);
                }
            } else {
                //if we're setting all the lines we run through them all until we find
                //one that's not set, we set it, then end the loop
                //we also record the starting point for the loop next time,
                //since we already know that all the leds up to that point have been set
                for( uint16_t j = thresStartPoint; j < numLines; j++ ) {
                    if( !pixelArray[j] ) {
                        spawnLed(j);
                        thresStartPoint = j;
                        break;
                    }
                }
            }
        }

        //check if the dissolve is finished
        //if it is we need to reset to start a new dissolve
        //turn the pause time on, and increment the cycle number
        if( numSpawned >= numLines ) {
            resetPixelArray();
            dissolveCount++;
            paused = true;

            //randMode 3 doesn't use the pattern, and instead uses randMode3CycleLen
            //to control the pattern length, so we need to adjust the cycle cap.
            if( randMode == 3 ) {
                maxNumCycles = randMode3CycleLen + 1;
            } else {
                maxNumCycles = pattern->length;
            }

            numCycles = addMod16PS(numCycles, 1, maxNumCycles);
        }

        showCheckPS();
    }

    //Determines if it's time to increase the spawn cap (if we're not paused)
    //This is checked independent of the effect loop,
    //to decouple the spawn increase time from the effect rate
    if( !paused && (currentTime - prevSpawnTime) >= spawnRateInc ) {
        prevSpawnTime = currentTime;
        maxNumSpawn++;
    }
}

//Colors a line at the specified line number
//records that it's set to the array and also increments the number of lines spawned
void DissolveSL::spawnLed(uint16_t lineNum) {
    pixelArray[lineNum] = true;
    color = pickDissolveColor();

    if( lineMode ) {
        //write the color out to all the leds in the segment line
        segDrawUtils::drawSegLine(*segSet, lineNum, color, colorMode);
    } else {
        segDrawUtils::setPixelColor(*segSet, lineNum, color, colorMode);
    }
    numSpawned++;
}
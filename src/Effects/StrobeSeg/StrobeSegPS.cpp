#include "StrobeSegPS.h"

//constructor for pallet ver
StrobeSegPS::StrobeSegPS(SegmentSet &SegmentSet, palletPS *Pallet, CRGB BgColor, uint8_t NumPulses, uint16_t PauseTime, 
                        bool SegEach, bool SegDual, bool SegLine, bool SegLineDual, bool SegAll, uint16_t Rate):
    segmentSet(SegmentSet), pallet(Pallet), numPulses(NumPulses), pauseTime(PauseTime), segEach(SegEach), segDual(SegDual), segLineDual(SegLineDual), segLine(SegLine), segAll(SegAll)
    {    
        init(BgColor, Rate);
	}

//constructor for single color
//!!If using pre-build FastLED colors you need to pass them as CRGB( *color code* )
StrobeSegPS::StrobeSegPS(SegmentSet &SegmentSet, CRGB Color, CRGB BgColor, uint8_t NumPulses, uint16_t PauseTime, 
                        bool SegEach, bool SegDual, bool SegLine, bool SegLineDual, bool SegAll, uint16_t Rate):
    segmentSet(SegmentSet), numPulses(NumPulses), pauseTime(PauseTime), segEach(SegEach), segDual(SegDual), segLineDual(SegLineDual), segLine(SegLine), segAll(SegAll)
    {    
        palletTemp = palletUtilsPS::makeSingleColorPallet(Color);
        pallet = &palletTemp;
        init(BgColor, Rate);
	}

//constructor for randomly generated pallet
StrobeSegPS::StrobeSegPS(SegmentSet &SegmentSet, uint8_t numColors, CRGB BgColor, uint8_t NumPulses, uint16_t PauseTime, 
                        bool SegEach, bool SegDual, bool SegLine, bool SegLineDual, bool SegAll, uint16_t Rate):
    segmentSet(SegmentSet), numPulses(NumPulses), pauseTime(PauseTime), segEach(SegEach), segDual(SegDual), segLineDual(SegLineDual), segLine(SegLine), segAll(SegAll)
    {    
        palletTemp = palletUtilsPS::makeRandomPallet(numColors);
        pallet = &palletTemp;
        init(BgColor, Rate);
	}

void StrobeSegPS::init(CRGB BgColor, uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    //bind background color pointer (if needed)
    bindBGColorPS();
    reset();
}

StrobeSegPS::~StrobeSegPS(){
    delete[] palletTemp.palletArr;
}

//sets the newColor flag
//we need to call setCycleCountMax() to setup the correct
//number of cycles to run
void StrobeSegPS::setNewColorBool(bool newColorBool){
    newColor = newColorBool;
    setCycleCountMax();
}

void StrobeSegPS::setpallet(palletPS *newPallet){
    pallet = newPallet;
    setCycleCountMax();
}

//resets the core cycle variables to restart the effect
void StrobeSegPS::reset(){
    pulseCount = 1;
    colorNum = 0;
    pulseMode = -1;
    cycleCount = 0;
    pulseBG = false;
    firstHalf = true;
    pause = false;
    prevGuess = 0;
    setPulseMode();
    setCycleCountMax();
}

//Strobe Modes:
    //0: Strobe each segment in the set one at a time, the direction can be set, and it can be set to reverse after each cycle
    //1: Strobe every other segment
    //2: Strobe each segment line one at a time, the direction can be set, and it can be set to reverse after each cycle
    //3: Strobe every other segment line (radial)
    //4: Strobe all segs at once 
    //What modes get used depends on the mode bool values (segEach, segDual, etc)
    //Each mode that is selected will be used after the previous mode has finished (with a pause in between)
//The strobe of each segment will continue for a set number of on/off cycles (ie making the strobe)
//A full cycle is once each of the segments has been strobed at least one time (gone through a set number of pulses)
//and all colors of the pallet have been used at least once
//Color options and Strobe cycle behavior (set with newColor)
    //A new color can be picked from the pallet either for each new set of pulses, or after a strobe cycle
    //The number of strobe cycles will always be enough to go through all the colors in the pallet,
    //rounded up to a whole number of cycles
    //For example, for a pallet of 3 colors with a segment set having 5 segments, doing strobe mode 0:
    //If newColor is set, 1 full strobe cycle (5 sets of pulses, one for each segment)
    //with each pulse having a new color from the pallet 
    //However if the pallet was length 6, then two full strobe cycles would 
    //happen so that all the colors in the pallet showed up
    //If newColor is not set, then the number of full strobe cycles is the pallet length (number of colors)
    //so that the segment is pulsed in each color one time
    //**The number of cycles is full dependent on the pallet length, even if colors are choosen at random
//Pausing:
//We pause for a set time after each full strobe cycle (or after every set of pulses of pauseEvery is set)
//Ie for mode 0, we'd pause after pulsing each segment once (or after each color if pauseEvery is set)

//Backgrounds:
//By default the background is filled in after the end of every pulse set and duing a pause
//Both these can be disabled with the fillBG and fillBGOnPause flags respectivly
//This causes the last pulse color to persist after the pulse set is done
//which can be used for some neat effects, like a scifi charging cycle or something

//Overall:
//The overall algoritum is a state machine
//If we need to pulse, we write out either the pulse color or the background depending on what was last written out
//When we pulse, we choose the area of the strip to pulse based on a few flag variables and the mode
//Each time we pulse the color (not the background) we advance a pulse counter
//Once this reaches the number of set pulses we choose what to do for the next set of pulses:
//We increment the cycleCount (this tracks how many sets of pulses we've done)
//If this matches cycleCountMax, then we need to pick a new mode and pause (if pulseEvery is false)
//We then need to check if we need to advance the colorNum, which tracks the pallet color index we're at
//to set a new color
//And we check if we need to change the direction of the pulse cycle (for modes 0 and 2)
//We flip the firstHalf flag (sets segments pulsed for modes 1 and 3)
//and finally reset the pulse count to 1
//After pausing we start pulsing again
void StrobeSegPS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        //if we have not reached the number of pulses we need to do a pulse
        if(pulseCount <= numPulses && !pause){ 

            //every other pulse we need to fill in the background instead
            //pulseBG is flipped each time we do a pulse
            if(pulseBG){
                colorOut = *bgColor;
                modeOut = bgColorMode;
            } else {
                //choose color (possibly at random)
                //we do this each pulse cycle to catch any pallet color updates
                pickColor();
                modeOut = colorMode;
                colorOut = colorTemp;
            }

            //de a pulse according to the pulseMode
            switch(pulseMode) {
                case 0:
                    //the next seg moves forward by one for each set of pulses
                    //(cycleCount always increments by 1 after a pulse set)
                    nextSeg = mod16PS( cycleCount, numSegs ); //cycleCount % numSegs;
                    //if we're starting from the last seg moving out
                    //we need to reverse our count
                    if(!direct){
                        nextSeg = numSegs - 1 - nextSeg;
                    }
                    segDrawUtils::fillSegColor(segmentSet, nextSeg, colorOut, modeOut);
                    break;
                case 1:
                    //We need to fill every other seg, alternating for each set of pulses
                    //firstHalf is a bool, so it's either 0 or 1
                    //this will set the loop to either fill in the even or odd segments
                    //firstHalf flips every time we finish a set of pulses
                    for(uint8_t i = firstHalf; i < numSegs; i += 2){
                        segDrawUtils::fillSegColor(segmentSet, i, colorOut, modeOut);
                    }
                    break;
                case 2:
                    //similar to case 0, but we're filling seg lines instead of segments
                    nextSeg = mod16PS( cycleCount, maxSegLength );//cycleCount % maxSegLength;
                    if(!direct){
                        nextSeg = maxSegLength - 1 - nextSeg;
                    }
                    segDrawUtils::drawSegLineSimple(segmentSet, nextSeg, colorOut, modeOut);
                    break;
                case 3:
                    //similar to case 1, but we're filling seg lines instead of segments
                    for(uint8_t i = firstHalf; i < maxSegLength; i += 2){
                        segDrawUtils::drawSegLineSimple(segmentSet, i, colorOut, modeOut);
                    }
                    break;
                default: //(mode 4)
                    //just fill the whole segment set
                    segDrawUtils::fillSegSetColor(segmentSet, colorOut, modeOut);
                    break;
            }

            //One pulse is setting the color and then turning it off again
            //so we only advance the pulse count every other cycle
            //we also toggle the fillBG flag each cycle so every time we advance the pulse count
            //we know we've done one full pulse
            if(!pulseBG){
                pulseCount++;
            }
            pulseBG = !pulseBG;

        } else if(pause) {
            //if we're pausing between pulse sets, then we just have to wait until the pause time is up
            if( ( currentTime - pauseStartTime ) >= pauseTime){
                pause = false;
            }
        } else {
            //if we've finished a set of pulses we need to decide what to do next
            //we advance the cycleCount since we've finished a set of pulses
            cycleCount = addMod16PS( cycleCount, 1, cycleCountMax );//(cycleCount + 1) % cycleCountMax;
            //if the cycleCount is 0 then we've gone through a full strobe cycle for the current mode
            //so we need to move to the next mode and trigger a pause (maybe)
            if( cycleCount == 0 ){
                setPulseMode();
                setCycleCountMax();
                //if we only want to pause once a strobe cycle is finished, do so
                if(!pauseEvery){
                    startPause();
                }    
            }

            //The cycle loop limit is the number of cycles until all the segments have been filled with color once
            //if this has happened we need to chooses a new color and flip the direction (maybe)
            boolTemp = mod16PS( cycleCount, cycleLoopLimit ) == 0; //(cycleCount % cycleLoopLimit == 0);
            //if newColor is true, we always choose a the next color for each pulse cycle
            //otherwise we only do it at the end of a full strobe cycle
            if(newColor || boolTemp){
                //color is picked via pickColor() above
                //color num tracks the pallet color index we're on
                colorNum = addmod8( colorNum, 1, pallet->length ); //(colorNum + 1) % pallet->length;
            }

            //switch the direction if alternate is on and
            //we're at the end of a strobe cycle
            //(for modes 0 and 2)
            if(alternate && boolTemp){
                direct = !direct;
            }

            //if we want to pause once after every pulse set, do so
            if(pauseEvery){
                startPause();
            }

            //reset the pulse vars, and toggle firstHalf (for modes 1 and 3)
            firstHalf = !firstHalf;
            pulseCount = 1;
            //fill the segment set with the background to clear the strip for the next pulse cycle
            //(if desired)
            if(fillBG){
                segDrawUtils::fillSegSetColor(segmentSet, *bgColor, bgColorMode);
            }
            //don't want to start with the background for the next pulse cycle
            pulseBG = false;
        }
        showCheckPS();
    }
}

//advances to the next pulse mode based on the segEach, segDual, segLine, segLineDual, and segAll flags
//segEach =     mode 0
//segDual =     mode 1
//segLine =     mode 2
//segLineDual = mode 3
//segAll =      mode 4
//we want to give the user the option of transitioning smoothly between pulse modes automatically
//the pulse modes that will show up are set by the flags above
//The pulse modes will occur in the order matching their mode numbers above (mode 0 happens before mode 2, etc)
void StrobeSegPS::setPulseMode(){
    if(segEach && pulseMode < 0){
        pulseMode = 0;
    } else if(segDual && pulseMode < 1){
        pulseMode = 1;
    } else if(segLine && pulseMode < 2){
        pulseMode = 2;
    } else if(segLineDual && pulseMode < 3){
        pulseMode = 3;
    } else if(segAll && pulseMode < 4){
        pulseMode = 4;
    } else {
        pulseMode = -1;
        setPulseMode();
    }
}

//sets the reset point for a strobe cycle based on the current pulseMode
//(a strobe cycle is finished once all the segments have been pulsed)
//the reset point is cycleLoopLimit
//while cycleCountMax is sets the total number of strobe cycles to do to show all the colors in the pallet
//This is rounded up to a whole number of strobe cycles
void StrobeSegPS::setCycleCountMax(){
    numSegs = segmentSet.numSegs;
    maxSegLength = segmentSet.maxSegLength;
    if(pulseMode == 0){
        //we need to pulse each seg once, so a full strobe cycle is numSegs
        cycleLoopLimit = numSegs;
    } else if(pulseMode == 1 || pulseMode == 3){
        //every other seg or line is pulsed, so we have a strobe cycle length of 2
        cycleLoopLimit = 2;
    } else if(pulseMode == 2){
        //we need to pulse every line
        cycleLoopLimit = maxSegLength;
    } else if(pulseMode == 4){
        //all the segments are pulsed at once, so the strobe cycle is 1
        cycleLoopLimit = 1;
    }

    //we want to show each color in the pallet at least once
    //so we need to set the number of strobe cycles accordingly
    if(newColor){
        //if we're doing a new color with each pulse then
        //the number of cycles we need to do is ratioed by the number of colors
        //rounded up so we do a whole number of strobe cycles
        cycleCountMax = cycleLoopLimit * ceil( (float)pallet->length / cycleLoopLimit);
    } else {
        //if we're doing one color for each strobe cycle
        cycleCountMax = cycleLoopLimit * pallet->length;
    }
}

//trigger a pause to start by setting pause to true
//and recording the start time
//if pause time is 0, then we skip the pause
void StrobeSegPS::startPause(){
    if(fillBGOnPause){
        segDrawUtils::fillSegSetColor(segmentSet, *bgColor, bgColorMode);
    }
    pause = true && (pauseTime != 0);
    pauseStartTime = millis();
}

//Chooses a color based on the random mode and stores it in colorTemp
//if we're choosing randomly (randMode > 0) then we only want to pick a new color
//at the start of a set of pulses
void StrobeSegPS::pickColor(){
    if(randMode == 0){
        colorTemp = palletUtilsPS::getPalletColor( pallet, colorNum );
    } else if(randMode == 1 && pulseCount <= 1) {
        //choose a completely random color
        colorTemp = colorUtilsPS::randColor();
    } else {
        //choose a color randomly from the pattern (making sure it's not the same as the current color)
        if(pulseCount <= 1) {
            randGuess = random8(pallet->length);
            if(randGuess == prevGuess){
                randGuess = addmod8(prevGuess, 1, pallet->length); //(prevGuess + 1) % pallet->length;
            }
            prevGuess = randGuess;
        }
        colorTemp = palletUtilsPS::getPalletColor( pallet, randGuess );
    }
}
#include "StrobeSLSeg.h"

//constructor for pattern and palette ver
StrobeSLSeg::StrobeSLSeg(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, CRGB BgColor, uint8_t NumPulses,
                         uint16_t PauseTime, uint8_t StrobeColorMode,
                         bool SegEach, bool SegDual, bool SegLine, bool SegLineDual, bool SegAll,
                         uint16_t Rate)
    : pattern(&Pattern), palette(&Palette), numPulses(NumPulses), pauseTime(PauseTime), strobeColorMode(StrobeColorMode),
      segEach(SegEach), segDual(SegDual), segLineDual(SegLineDual), segLine(SegLine), segAll(SegAll)  //
{
    init(BgColor, SegSet, Rate);
}

//constructor for palette ver
StrobeSLSeg::StrobeSLSeg(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, uint8_t NumPulses, uint16_t PauseTime,
                         uint8_t StrobeColorMode, bool SegEach, bool SegDual, bool SegLine, bool SegLineDual, bool SegAll,
                         uint16_t Rate)
    : palette(&Palette), numPulses(NumPulses), pauseTime(PauseTime), strobeColorMode(StrobeColorMode),
      segEach(SegEach), segDual(SegDual), segLineDual(SegLineDual), segLine(SegLine), segAll(SegAll)  //
{
    init(BgColor, SegSet, Rate);
    setPaletteAsPattern();
}

//constructor for single color
StrobeSLSeg::StrobeSLSeg(SegmentSetPS &SegSet, CRGB Color, CRGB BgColor, uint8_t NumPulses, uint16_t PauseTime,
                         bool SegEach, bool SegDual, bool SegLine, bool SegLineDual, bool SegAll, uint16_t Rate)
    : numPulses(NumPulses), pauseTime(PauseTime),
      segEach(SegEach), segDual(SegDual), segLineDual(SegLineDual), segLine(SegLine), segAll(SegAll)  //
{
    strobeColorMode = 0;  //since we only have a single color, all the strobe color modes will do the same thing
    init(BgColor, SegSet, Rate);
    paletteTemp = paletteUtilsPS::makeSingleColorPalette(Color);
    palette = &paletteTemp;
    setPaletteAsPattern();
}

//constructor for randomly generated palette
StrobeSLSeg::StrobeSLSeg(SegmentSetPS &SegSet, uint8_t numColors, CRGB BgColor, uint8_t NumPulses, uint16_t PauseTime,
                         uint8_t StrobeColorMode, bool SegEach, bool SegDual, bool SegLine, bool SegLineDual, bool SegAll,
                         uint16_t Rate)
    : numPulses(NumPulses), pauseTime(PauseTime), strobeColorMode(StrobeColorMode),
      segEach(SegEach), segDual(SegDual), segLineDual(SegLineDual), segLine(SegLine), segAll(SegAll)  //
{
    init(BgColor, SegSet, Rate);
    paletteTemp = paletteUtilsPS::makeRandomPalette(numColors);
    palette = &paletteTemp;
    setPaletteAsPattern();
}

void StrobeSLSeg::init(CRGB BgColor, SegmentSetPS &SegSet, uint16_t Rate) {
    //bind the rate and segSet pointer vars since they are inherited from BaseEffectPS
    bindSegSetPtrPS();
    bindClassRatesPS();
    //bind background color pointer (if needed)
    bindBGColorPS();
    reset();
}

StrobeSLSeg::~StrobeSLSeg() {
    free(paletteTemp.paletteArr);
    free(patternTemp.patternArr);
}

//sets the pattern to match the current palette
//ie for a palette length 5, the pattern would be
//{0, 1, 2, 3, 4}
//Also restarts the current strobe mode.
void StrobeSLSeg::setPaletteAsPattern() {
    generalUtilsPS::setPaletteAsPattern(patternTemp, *palette);
    setPattern(patternTemp);
}

//sets the strobeColorMode
//we need to call setCycleCountMax() to setup the correct
//number of cycles to run
void StrobeSLSeg::setStrobeColorMode(uint8_t newMode) {
    strobeColorMode = newMode;
    setCycleCountMax();
}

//Sets a new pattern, also re-calculates how long the strobe cycle should be
void StrobeSLSeg::setPattern(patternPS &newPattern) {
    pattern = &newPattern;
    setCycleCountMax();
}

//resets the core cycle variables to restart the effect
void StrobeSLSeg::reset() {
    totalCycles = 0;
    pulseCount = 1;
    colorNum = 0;
    strobeMode = -1;
    cycleNum = 0;
    pulseBG = false;
    paused = false;
    setStrobeMode();
    setCycleCountMax();
}

/* Strobe Modes:
    0: Strobe each segment in the set one at a time, the direction can be set, and it can be set to reverse after each cycle
    1: Strobe every other segment
    2: Strobe each segment line one at a time, the direction can be set, and it can be set to reverse after each cycle
    3: Strobe every other segment line
    4: Strobe all segs at once 
    What modes get used depends on the mode bool values (segEach, segDual, etc)
    Each mode that is selected will be used after the previous mode has finished (with a pause in between)
The strobe of each segment will continue for a set number of on/off cycles (ie making the strobe)
A full cycle is complete when each of the segments or segment lines has been strobed at least one time, 
while an individual strobe is one set of pulses only.

For info on Strobe color modes, pausing, and background behaviors, etc, see the Inputs Guide in the .h file.

Overall:
The overall algorithm is a state machine
If we need to pulse, we write out either the pulse color or the background depending on what was last written out
When we pulse, we choose the area of the strip to pulse based on a few flag variables and the mode
Each time we pulse the color (not the background) we advance a pulse counter
Once this reaches the number of set pulses we choose what to do for the next set of pulses:
We increment the cycleNum (this tracks how many sets of pulses we've done)
If this matches cycleCountMax, then we need to pick a new mode and pause (if pulseEvery is false)
We then need to check if we need to advance the colorNum, which tracks the pattern color index we're at
to set a new color
And we check if we need to change the direction of the pulse cycle (alternate)
and finally reset the pulse count to 1
After pausing we start pulsing again */
void StrobeSLSeg::update() {
    currentTime = millis();

    if( (currentTime - prevTime) >= *rate ) {
        prevTime = currentTime;

        //code for pausing the effect after pulse set is finished
        //if we're paused, we simply return to break out of the function
        if( paused ) {
            //if we're pausing between pulse sets, then we just have to wait until the pause time is up
            if( (currentTime - pauseStartTime) >= pauseTime ) {
                paused = false;
            } else {
                return;
            }
        }

        //if we have not reached the number of pulses we need to do a pulse
        if( pulseCount <= numPulses ) {

            //every other pulse we need to fill in the background instead
            //pulseBG is flipped each time we do a pulse
            if( pulseBG ) {
                colorOut = *bgColor;
                modeOut = bgColorMode;
            } else {
                //choose color (possibly at random)
                //we do this each pulse cycle to catch any palette color updates
                pickColor();
                modeOut = colorMode;
                colorOut = colorTemp;
            }

            //do a pulse according to the strobeMode
            switch( strobeMode ) {
                case 0:
                    //the next seg moves forward by one for each set of pulses
                    //(cycleNum always increments by 1 after a pulse set)
                    nextSeg = mod16PS(cycleNum, numSegs); 
                    //if we're starting from the last seg moving out, we need to reverse our count
                    if( !direct ) {
                        nextSeg = numSegs - 1 - nextSeg;
                    }
                    segDrawUtils::fillSegColor(*segSet, nextSeg, colorOut, modeOut);
                    break;
                case 1:
                    //We need to fill each segment at a frequency of sMode1Freq, 
                    //ie every other segment, every third segment, ect.
                    //The set of segments we fill has to advance as part of the strobe cycle, 
                    //so we strobe all the segments. 
                    //we can use cycleNum to track what set of segments we're doing, bounding it by 
                    //the cycleLoopLimit (set in setCycleCountMax() to sMode1Freq)
                    nextSeg = mod16PS(cycleNum, cycleLoopLimit);

                    //if we're starting from the last group of segments, moving in, we need to reverse our count
                    if( !direct ) {
                        nextSeg = cycleLoopLimit - 1 - nextSeg;
                    }
                    //Fill the segments for every "sMode1Freq" segment, nextSeg adjusts what group
                    //of segment we're on
                    for( uint16_t i = nextSeg; i < numSegs; i += sMode1Freq ) {
                        segDrawUtils::fillSegColor(*segSet, i, colorOut, modeOut);
                    }
                    break;
                case 2:
                    //similar to case 0, but we're filling seg lines instead of segments
                    nextSeg = mod16PS(cycleNum, numLines);  //cycleNum % numLines;
                    if( !direct ) {
                        nextSeg = numLines - 1 - nextSeg;
                    }
                    segDrawUtils::drawSegLine(*segSet, nextSeg, colorOut, modeOut);
                    break;
                case 3:
                    //similar to case 1, but we're filling seg lines instead of segments
                    nextSeg = mod16PS(cycleNum, cycleLoopLimit);
                    if( !direct ) {
                        nextSeg = cycleLoopLimit - 1 - nextSeg;
                    }
                    for( uint8_t i = nextSeg; i < numLines; i += sMode3Freq ) {
                        segDrawUtils::drawSegLine(*segSet, i, colorOut, modeOut);
                    }
                    break;
                default:  //(mode 4)
                    //just fill the whole segment set
                    segDrawUtils::fillSegSetColor(*segSet, colorOut, modeOut);
                    break;
            }

            //One pulse is setting the color and then turning it off again
            //so we only advance the pulse count every other cycle
            //we also toggle the fillBg flag each cycle so every time we advance the pulse count
            //we know we've done one full pulse
            if( !pulseBG ) {
                pulseCount++;
            }
            pulseBG = !pulseBG;

        } else {
            //if we've finished a set of pulses we need to decide what to do next
            //we advance the cycleNum since we've finished a set of pulses
            cycleNum = addMod16PS(cycleNum, 1, cycleCountMax);

            //The cycle loop limit is the number of cycles until all the segments have been filled with color once
            //if this has happened we need to chooses a new color and flip the direction (maybe)
            atCycleLim = mod16PS(cycleNum, cycleLoopLimit) == 0;
            //if strobeColorMode is 0, we always choose the next color for each pulse cycle
            //otherwise we only do it at the end of a full strobe cycle
            if( strobeColorMode == 0 || atCycleLim ) {
                //color is picked via pickColor() above
                //color num tracks the pattern color index we're on
                colorNum = addMod16PS(colorNum, 1, pattern->length);
            }

            //switch the direction if alternate is on, we're at the end of a strobe cycle
            //and our current strobe cycle matches the alternate frequency
            //(note that we use normal mod here over mod16PS b/c totalCycles will grow much larger than altFreq.
            //mod16PS works best when the two numbers are fairly close together)
            if( alternate && atCycleLim && ( totalCycles % altFreq == 0 ) ) { 
                direct = !direct;
            }

            //if we want to pause once after every pulse set, do so
            if( pauseEvery ) {
                startPause();
            }

            pulseCount = 1;
            //fill the segment set with the background to clear the strip for the next pulse cycle
            //(if desired)
            if( fillBg ) {
                segDrawUtils::fillSegSetColor(*segSet, *bgColor, bgColorMode);
            }
            //don't want to start with the background for the next pulse cycle
            pulseBG = false;

            //if the cycleNum is 0 then we've gone through a full strobe cycle for the current mode
            //so we need to move to the next mode and trigger a pause (maybe)
            if( cycleNum == 0 ) {
                setStrobeMode();
                setCycleCountMax();
                totalCycles++;
                //if we only want to pause once a strobe cycle is finished, do so
                if( !pauseEvery ) {
                    startPause();
                }
            }
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
void StrobeSLSeg::setStrobeMode() {
    if( segEach && strobeMode < 0 ) {
        strobeMode = 0;
    } else if( segDual && strobeMode < 1 ) {
        strobeMode = 1;
    } else if( segLine && strobeMode < 2 ) {
        strobeMode = 2;
    } else if( segLineDual && strobeMode < 3 ) {
        strobeMode = 3;
    } else if( segAll && strobeMode < 4 ) {
        strobeMode = 4;
    } else {
        strobeMode = -1;
        setStrobeMode();
    }
}

//sets the reset point for a strobe cycle based on the current strobeMode
//(a strobe cycle is finished once all the segments have been pulsed)
//the reset point is cycleLoopLimit
//while cycleCountMax is sets the total number of strobe cycles to do to show all the colors in the pattern
//This is rounded up to a whole number of strobe cycles
void StrobeSLSeg::setCycleCountMax() {
    cycleNum = 0;  //Ensures that we start a fresh strobe cycle
    numSegs = segSet->numSegs;
    numLines = segSet->numLines;

    //Set how long an individual strobe cycle is depending on the strobeMode
    //ie how many cycles it takes to strobe each line/segment/etc one time
    switch( strobeMode ) {
        case 0:
        default:
            //we need to pulse each seg once, so a full strobe cycle is numSegs
            cycleLoopLimit = numSegs;
            break;
        case 1:
            //every "sMode1Freq" segment is pulsed, alternating (ie every 3rd line, other line, etc)
            //so the limit is the frequency
            cycleLoopLimit = sMode1Freq;
            break;
        case 2:
            //we need to pulse every line
            cycleLoopLimit = numLines;
        case 3:
            //every "sMode3Freq" line is pulsed, alternating (ie every 3rd line, other line, etc)
            //so the limit is the frequency
            cycleLoopLimit = sMode3Freq;
            break;
        case 4:
            //all the segments are pulsed at once, so the strobe cycle is 1
            cycleLoopLimit = 1;
            break;
    }

    //Set how long the strobe cycle is based on the strobeColorMode
    if( strobeColorMode == 0 ) {
        //For mode 0 we do a new color with each set of pulses, making sure we cover all the segments and colors
        //the number of cycles we need to do is ratio'd by the number of colors
        //rounded up so we do a whole number of strobe cycles
        cycleCountMax = cycleLoopLimit * ceil((float)pattern->length / cycleLoopLimit);
    } else if( strobeColorMode == 1 ) {
        //For mode 1, we do all full set of strobes in each color from the pattern
        //(ie in strobeMode 0, we'd pulse all the segments in one color, then do the next color, etc)
        cycleCountMax = cycleLoopLimit * pattern->length;
    } else {
        //For mode 2, we do one set of pulses in a single color, and then move on.
        //So they number of cycles is enough to do one set of pulses.
        cycleCountMax = cycleLoopLimit;
    }
}

//trigger a pause to start by setting pause to true
//and recording the start time
//if pause time is 0, then we skip the pause
void StrobeSLSeg::startPause() {
    if( fillBGOnPause ) {
        segDrawUtils::fillSegSetColor(*segSet, *bgColor, bgColorMode);
    }
    paused = true && (pauseTime != 0);
    pauseStartTime = millis();
}

//Chooses a color based on the random mode and stores it in colorTemp
//if we're choosing randomly (randMode > 0) then we only want to pick a new color
//at the start of a set of pulses
void StrobeSLSeg::pickColor() {
    if( randMode == 0 ) {
        palIndex = patternUtilsPS::getPatternVal(*pattern, colorNum);
        colorTemp = paletteUtilsPS::getPaletteColor(*palette, palIndex);
    } else if( randMode == 1 && pulseCount <= 1 ) {
        //choose a completely random color
        colorTemp = colorUtilsPS::randColor();
    } else if( randMode == 2 ) {
        //choose a color randomly from the pattern (making sure it's not the same as the current color)
        if( pulseCount <= 1 ) {
            //Use the current palIndex value to get a shuffled value
            //This may look confusing, but the pattern shuffle function a pattern value and spits out a different one
            //palIndex is only set in pickColor(), so it's safe to store it in itself for the next pickColor() is called
            palIndex = patternUtilsPS::getShuffleVal(*pattern, palIndex);
        }
        colorTemp = paletteUtilsPS::getPaletteColor(*palette, palIndex);
    } else {
        //choose a color randomly from the pattern, allowing repeats
        if( pulseCount <= 1 ) {
            //Choose a color randomly from the pattern (can repeat)
            //We use the spacing (255) value as the current shuffle value
            //so that the shuffle will pick the first actual color it hits
            //we use palIndex b/c it is only set in pickColor(),
            //so it's safe to store it in itself for the next pickColor() is called
            palIndex = patternUtilsPS::getShuffleVal(*pattern, 255);
        }
        colorTemp = paletteUtilsPS::getPaletteColor(*palette, palIndex);
    }
}
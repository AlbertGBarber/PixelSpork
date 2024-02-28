#include "PoliceStrobeSLSeg.h"

//constructor for a traditional two color strobe
PoliceStrobeSLSeg::PoliceStrobeSLSeg(SegmentSetPS &SegSet, CRGB ColorOne, CRGB ColorTwo, CRGB BgColor, uint8_t NumPulses,
                                     uint16_t PauseTime, uint8_t PulseMode, bool SegMode, uint16_t Rate)
    : numPulses(NumPulses), pauseTime(PauseTime), pulseMode(PulseMode), segMode(SegMode)  //
{
    //create a dual color palette for the two strobe colors
    CRGB *newPalette_arr = new CRGB[2];
    newPalette_arr[0] = ColorOne;
    newPalette_arr[1] = ColorTwo;
    paletteTemp = {newPalette_arr, 2};
    palette = &paletteTemp;

    //Set the pattern to match the dual color palette
    setPaletteAsPattern();

    init(BgColor, SegSet, Rate);
}

//Constructor using both pattern and palette
PoliceStrobeSLSeg::PoliceStrobeSLSeg(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, CRGB BgColor,
                                     uint8_t NumPulses, uint16_t PauseTime, uint8_t PulseMode, bool SegMode, uint16_t Rate)
    : pattern(&Pattern), palette(&Palette), numPulses(NumPulses), pauseTime(PauseTime), pulseMode(PulseMode), segMode(SegMode)  //
{
    init(BgColor, SegSet, Rate);
}

//constructor for using any palette for the colors
PoliceStrobeSLSeg::PoliceStrobeSLSeg(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, uint8_t NumPulses,
                                     uint16_t PauseTime, uint8_t PulseMode, bool SegMode, uint16_t Rate)
    : palette(&Palette), numPulses(NumPulses), pauseTime(PauseTime), pulseMode(PulseMode), segMode(SegMode)  //
{
    setPaletteAsPattern();
    init(BgColor, SegSet, Rate);
}

PoliceStrobeSLSeg::~PoliceStrobeSLSeg() {
    free(paletteTemp.paletteArr);
    free(patternTemp.patternArr);
}

//restarts the effect
void PoliceStrobeSLSeg::reset() {
    colorNum = 0;
    pulseCount = 1;
    paused = false;
    //Set the flashHalf based on the chosen pulseMode
    if( pulseMode == 0 || pulseMode == 1 ) {
        flashHalf = true;
    } else {
        flashHalf = false;
    }
}

void PoliceStrobeSLSeg::init(CRGB BgColor, SegmentSetPS &SegSet, uint16_t Rate) {
    //bind the rate and segSet pointer vars since they are inherited from BaseEffectPS
    bindSegSetPtrPS();
    bindClassRatesPS();
    //bind background color pointer (if needed)
    bindBGColorPS();
    reset();
}

//sets the pattern to match the current palette
//ie for a palette length 5, the pattern would be
//{0, 1, 2, 3, 4}
void PoliceStrobeSLSeg::setPaletteAsPattern() {
    generalUtilsPS::setPaletteAsPattern(patternTemp, *palette);
    pattern = &patternTemp;
}

/* Updates the effect
There are three separate pulse modes that must be produced:
    0: Pulse half the strip in each color (alternating halves and colors), then pulse each color on the whole strip
    1: Pulse half the strip in each color (alternating halves and colors)
    2: Pulse the whole strip in each color (alternating colors)
To mimic a police light we pause for a set time after each pulse type (or color of pauseEvery is set)
Ie for mode 0, we'd pause after pulsing all the halve colors (or after each color if pauseEvery is set)
The overall algorithm is a state machine
If we need to pulse, we write out either the pulse color or the background depending on what was last written out
When we pulse, we choose the area of the strip to pulse based on a few flag variables
Each time we pulse the color (not the background) we advance a pulse counter
Once this reaches the number of pulses we choose the next color and pulse mode and possibly start a pause (depending on pauseEvery)
After pausing we start pulsing again
The effect has been adapted to work with both segments and segment lines, depending on segMode
This doesn't change the overall algorithm, we just draw along segments or segment lines,
while also splitting the strobe in half based on the number of segments or segment lines. */
void PoliceStrobeSLSeg::update() {
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

        if( segMode ) {
            //Get the number of segments
            //We're in segMod, so we're drawing the strobes along segment instead of segment lines
            numLines = segSet->numSegs;
        } else {
            //get the current number of segment lines
            numLines = segSet->numLines;
        }

        //we add (numLines % 2) to account for odd length strips ((numLines % 2) is either 1 or 0)
        //replace mod with bitwise "numLines & 1"?. Probably already optimized by compiler......
        halfLength = numLines / 2 + mod16PS(numLines, 2);

        //if we have not reached the number of pulses we need to do a pulse
        if( pulseCount <= numPulses ) {

            //every other pulse we need to fill in the background instead
            //fillBg is flipped each time we do a pulse
            modeOut = colorMode;
            if( pulseBG ) {
                colorOut = *bgColor;
                modeOut = bgColorMode;
            } else {
                pickColor();  //choose color (possibly at random)
                colorOut = colorTemp;
            }

            //we either need to pulse half the strip or the whole thing depending on flashHalf
            if( flashHalf ) {
                //if we're flashing half the strip, we need to set the start and end limits
                //these are based on firstHalf (which switches every time a set of pulses finishes)
                if( firstHalf ) {
                    lightStart = 0;
                    lightEnd = halfLength - 1;  //-1 since we start counting at 0
                } else {
                    lightStart = halfLength;
                    lightEnd = numLines - 1;
                }
            } else {
                //set limits to pulse the whole thing
                lightStart = 0;
                lightEnd = numLines - 1;
            }

            //color the pulsed length along the segment lines or segments
            for( uint16_t i = lightStart; i <= lightEnd; i++ ) {
                if( segMode ) {
                    //In segMode, we draw the pulses along segments
                    segDrawUtils::fillSegColor(*segSet, i, colorOut, modeOut);
                } else {
                    //Not in segMod: we draw the pulses along segment lines
                    segDrawUtils::drawSegLine(*segSet, i, colorOut, modeOut);
                }
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
            colorNum = addMod16PS(colorNum, 1, pattern->length);
            //after choosing the next color, if we're back at the first color
            //then we've pulsed all the colors in the current cycle
            //and we need to set the next pulse mode flags
            if( colorNum == 0 ) {
                if( pulseMode == 0 ) {
                    flashHalf = !flashHalf;
                } else if( pulseMode == 1 ) {
                    flashHalf = true;
                } else if( pulseMode == 2 ) {
                    flashHalf = false;
                }
                //if we only want to pause once all colors have been pulsed, do so
                if( !pauseEvery ) {
                    startPause();
                }
            }

            //if we want to pause once after every color, do so
            if( pauseEvery ) {
                startPause();
            }

            //reset the pulse vars, and toggle firstHalf (so another half is chosen)
            firstHalf = !firstHalf;
            pulseCount = 1;
            //fill the segment set with the background to clear the strip for the next pulse cycle
            if( fillBg ) {
                segDrawUtils::fillSegSetColor(*segSet, *bgColor, bgColorMode);
            }
            pulseBG = false;
        }
        showCheckPS();
    }
}

//trigger a pause to start by setting pause to true
//and recording the start time
//if pause time is 0, then we skip the pause
void PoliceStrobeSLSeg::startPause() {
    if( fillBGOnPause ) {
        segDrawUtils::fillSegSetColor(*segSet, *bgColor, bgColorMode);
    }
    paused = true && (pauseTime != 0);
    pauseStartTime = millis();
}

//Chooses a color based on the random mode and stores it in colorTemp
//if we're choosing randomly (randMode > 0) then we only want to pick a new color
//at the start of a set of pulses
void PoliceStrobeSLSeg::pickColor() {
    if( randMode == 0 ) {
        palIndex = patternUtilsPS::getPatternVal(*pattern, colorNum);
        colorTemp = paletteUtilsPS::getPaletteColor(*palette, palIndex);
    } else if( randMode == 1 && pulseCount <= 1 ) {
        //choose a completely random color
        colorTemp = colorUtilsPS::randColor();
    } else {
        //choose a color randomly from the pattern (making sure it's not the same as the current color)
        if( pulseCount <= 1 ) {
            //Use the current palIndex value to get a shuffled value
            //This may look confusing, but the pattern shuffle function a pattern value and spits out a different one
            //palIndex is only set in pickColor(), so it's safe to store it in itself for the next pickColor() is called
            palIndex = patternUtilsPS::getShuffleVal(*pattern, palIndex);
        }
        colorTemp = paletteUtilsPS::getPaletteColor(*palette, palIndex);
    }
}
#include "PoliceStrobeSL.h"

//constructor for a traditional two color strobe
PoliceStrobeSL::PoliceStrobeSL(SegmentSet &SegmentSet, CRGB ColorOne, CRGB ColorTwo, CRGB BgColor, uint8_t NumPulses, uint16_t PauseTime, uint8_t PulseMode, uint16_t Rate):
    segmentSet(SegmentSet), numPulses(NumPulses), pauseTime(PauseTime), pulseMode(PulseMode)
    {    
        init(BgColor, Rate);
        //create a dual color palette for the tow strobe colors
        CRGB *newPalette_arr = new CRGB[2];
        newPalette_arr[0] = ColorOne;
        newPalette_arr[1] = ColorTwo;
        paletteTemp = {newPalette_arr, 2};
        palette = &paletteTemp;
	}

//constructor for using any palette for the colors
PoliceStrobeSL::PoliceStrobeSL(SegmentSet &SegmentSet, palettePS *Palette, CRGB BgColor, uint8_t NumPulses, uint16_t PauseTime, uint8_t PulseMode, uint16_t Rate):
    segmentSet(SegmentSet), palette(Palette), numPulses(NumPulses), pauseTime(PauseTime), pulseMode(PulseMode)
    {    
        init(BgColor, Rate); 
	}

PoliceStrobeSL::~PoliceStrobeSL(){
    free(paletteTemp.paletteArr);
}

//restarts the effect
void PoliceStrobeSL::reset(){
    colorNum = 0;
    pulseCount = 1;
    pause = false;
    prevGuess = 0;
    //Set the flashHalf based on the choosen pulseMode
    if(pulseMode == 0 || pulseMode == 1){
        flashHalf = true;
    } else {
        flashHalf = false; 
    }
}

void PoliceStrobeSL::init(CRGB BgColor, uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    //bind background color pointer (if needed)
    bindBGColorPS();
    reset();
}

//Updates the effect
//There are three seperate pulse modes that must be produced:
//0: Pulse half the strip in each color (alternating halves and colors), then pulse each color on the whole strip
//1: Pulse half the strip in each color (alternating halves and colors)
//2: Pulse the whole strip in each color (alternating colors)
//To mimic a police light we pause for a set time after each pulse type (or color of pauseEvery is set)
//Ie for mode 0, we'd pause after pulsing all the halve colors (or after each color if pauseEvery is set)
//The overall algoritum is a state machine
//If we need to pulse, we write out either the pulse color or the background depending on what was last written out
//When we pulse, we choose the area of the strip to pulse based on a few flag variables
//Each time we pulse the color (not the background) we advance a pulse counter
//Once this reaches the number of pulses we choose the next color and pulse mode and possibly start a pause (depending on pulseEvery)
//After pausing we start pulsing again
void PoliceStrobeSL::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;
        //get the current number of segment lines and spilt the segments in to halves
        numLines = segmentSet.maxSegLength;
        //we add (numLines % 2) to account for odd length strips ((numLines % 2) is either 1 or 0)
        halfLength = numLines/2 + mod16PS(numLines, 2);

        //if we have not reached the number of pulses we need to do a pulse
        if(pulseCount <= numPulses && !pause){

            //every other pulse we need to fill in the background instead
            //fillBG is flipped each time we do a pulse
            modeOut = colorMode;
            if(pulseBG){
                colorOut = *bgColor;
                modeOut = bgColorMode;
            } else {
                pickColor(); //choose color (possibly at random)
                colorOut = colorTemp;
            }

            //we either need to pulse half the strip or the whole thing depending on flashHalf
            if(flashHalf){
                //if we're flashing half the strip, we need to set the start and end limits
                //these are based on firstHalf (which switches every time a set of pulses finishes)
                if(firstHalf){
                    lightStart = 0;
                    lightEnd = halfLength;
                } else {
                    lightStart = halfLength;
                    lightEnd = numLines - 1;
                }
            } else {
                //set limits to pulse the whole thing
                lightStart = 0;
                lightEnd = numLines - 1;
            }

            //color the pulsed length along the segment lines
            for(uint16_t i = lightStart; i <= lightEnd; i++){
                segDrawUtils::drawSegLineSimple(segmentSet, i, colorOut, modeOut);
            }
            //segDrawUtils::fillSegSetlengthColor(segmentSet, colorOut, modeOut, lightStart, lightEnd);
            
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
            colorNum = addMod16PS(colorNum, 1, palette->length); //(colorNum + 1) % palette->length;
            //after choosing the next color, if we're back at the first color
            //then we've pulsed all the colors in the current cycle
            //and we need to set the next pulse mode flags
            if( colorNum == 0 ){
                if(pulseMode == 0){
                    flashHalf = !flashHalf;
                } else if(pulseMode == 1){
                    flashHalf = true;
                } else if(pulseMode == 2){
                    flashHalf = false;
                }
                //if we only want to pause once all colors have been pulsed, do so
                if(!pauseEvery){
                    startPause();
                }
            }

            //if we want to pause once after every color, do so
            if(pauseEvery){
                startPause();
            }

            //reset the pulse vars, and toggle firstHalf (so another half is choosen)
            firstHalf = !firstHalf;
            pulseCount = 1;
            //fill the segment set with the background to clear the strip for the next pulse cycle
            if(fillBG){
                segDrawUtils::fillSegSetColor(segmentSet, *bgColor, bgColorMode);
            }
            pulseBG = false;
        }
        showCheckPS();
    }
}

//trigger a pause to start by setting pause to true
//and recording the start time
//if pause time is 0, then we skip the pause
void PoliceStrobeSL::startPause(){
    if(fillBGOnPause){
        segDrawUtils::fillSegSetColor(segmentSet, *bgColor, bgColorMode);
    }
    pause = true && (pauseTime != 0);
    pauseStartTime = millis();
}

//Chooses a color based on the random mode and stores it in colorTemp
//if we're choosing randomly (randMode > 0) then we only want to pick a new color
//at the start of a set of pulses
void PoliceStrobeSL::pickColor(){
    if(randMode == 0){
        colorTemp = paletteUtilsPS::getPaletteColor( palette, colorNum );
    } else if(randMode == 1 && pulseCount <= 1) {
        //choose a completely random color
        colorTemp = colorUtilsPS::randColor();
    } else {
        //choose a color randomly from the pattern (making sure it's not the same as the current color)
        if(pulseCount <= 1) {
            randGuess = random8(palette->length);
            if(randGuess == prevGuess){
                randGuess = addmod8(prevGuess, 1, palette->length);//(prevGuess + 1) % palette->length;
            }
            prevGuess = randGuess;
        }
        colorTemp = paletteUtilsPS::getPaletteColor( palette, randGuess );
    }
}
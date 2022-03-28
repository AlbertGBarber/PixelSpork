#ifndef StrobeSegPS_h
#define StrobeSegPS_h

//TODO: make strobe colors based on pattern and pallet?

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

//An effect to strobe segment sets in various patterns, while switching between pallet colors automatically
//A strobe is a rapid blinking of light.

//The strobe colors will be taken from a passed in pallet. There are constuctor options for single or randomly 
//choosen colored strobes
//The are a few options for randomly choosing the strobe colors from the pallet

//Strobe Modes:
    //0: Strobe each segment in the set one at a time, the direction can be set, and it can be set to reverse after each cycle
    //1: Strobe every other segment
    //2: Strobe each segment line one at a time, the direction can be set, and it can be set to reverse after each cycle
    //3: Strobe every other segment line (radial)
    //4: Strobe all segs at once 

//You can configure any combination of the modes to fire one after another
//Which modes get used depends on the mode bool values (segEach, segDual, etc, see constructor notes below)
//Each mode that is selected will be used after the previous mode has finished (with a pause in between)
//The effect will loop, so once the last mode has finished, we'll start again with the first mode
//You can change which modes appear on the fly by changing the mode flags (see constructor inputs)
//A mode is finished when a full strobe cycle in complete, see below:

//The strobe of each segment will continue for a set number of on/off cycles (ie making the strobe)
//A full cycle is when each of the segments has been strobed at least one time (gone through a set number of pulses)
//and all colors of the pallet have been used at least once

//Color options and Strobe cycle behavior (set with newColor flag)
    //A new color can be picked from the pallet either for each new set of pulses, or after a full strobe cycle
    //The number of strobe cycles will always be enough to go through all the colors in the pallet,
    //rounded up to a whole number of cycles
    //For example, for a pallet of 3 colors with a segment set having 5 segments, doing strobe mode 0:
        //If newColor is set, 1 full strobe cycle (5 sets of pulses, one for each segment) will be done
        //with each segment pulsing a new color from the pallet 
        //However if the pallet was length 6, then two full strobe cycles would 
        //happen so that all the colors in the pallet showed up
    //If newColor is not set, then the number of full strobe cycles is the pallet length (number of colors)
    //so that the all the segments are pulsed in each color one time
//**The number of cycles is full dependent on the pallet length, even if colors are choosen at random

//see the code of setCycleCountMax() to work out how many strobe cycles will happen based on your pallet
//cycleCountMax is the value you want

//After a strobe cycle is finished there is a user configurable pause time between cycles
//after which, the next mode will start.

//Pausing:
//We pause for a set time after each full strobe cycle (or after every set of pulses of pauseEvery is set)
//this can be set to 0

//Backgrounds:
//By default the background is filled in after the end of every pulse set and duing a pause
//Both these can be disabled with the fillBG and fillBGOnPause flags respectivly
//This causes the last pulse color to persist after the pulse set is done
//which can be used for some neat effects, like a scifi charging cycle or something

//This effect is fully compatible with color modes, and the bgColor is a pointer, so you can bind it
//to an external color variable
//(see segDrawUtils::setPixelColor)

//There are quite a lot of extra configuration options, help make more dynamic strobes

//Example calls: 
    //StrobeSegPS(ringSegments, &pallet1, 0, 4, 0, true, true, false, false, false, 50);
    //setNewColorBool(true); (sets newColor flag)
    //Will do a set of strobes using pallet1 and strobe modes 0 and 1.
    //The background is blank, there are 4 pulses per strobe with 50ms between each
    //The pause time is 0
    //The effect has been set to do a new color for each set of pulses

    //StrobeSegPS(ringSegments, &pallet1, CRGB::Red, 2, 500, true, false, false, false, true, 100);
    //alternate = true;
    //fillBG = false;
    //fillBGOnPause = false;
    //Will do a set of strobes using pallet1 and strobe modes 0 and 4.
    //The background is red, there are 2 pulses per strobe with 100ms between each
    //The pause time is 500
    //The effect has been set to alternate segment fill directions after each cycle (this only affects mode 0)
    //The total number of strobe cycles will be equal to the pallet length (because newColor was not set)
    //The background will not be filled after each pulse cycle

    //StrobeSegPS(ringSegments, CRGB:Blue, CRGB::Red, 5, 0, false, true, false, true, false, 40);
    //Will do a set of strobes in blue and strobe modes 1 and 3.
    //The background is red, there are 5 pulses per strobe with 40ms between each
    //The pause time is 0

    //StrobeSegPS(ringSegments, 4, 0, 3, 0, false, true, false, false, false, 100);
    //setNewColorBool(true); (sets newColor flag)
    //randMode = 2; 
    //Will do a set of strobes in using a pallet of 4 random colors and strobe mode 1.
    //The background is blank, there are 3 pulses per strobe with 60ms between each
    //The pause time is 0
    //Since the newColor flag has been set and randMode has been set to 2, 
    //a new color from the pallet will be choosen for each pulse cycle
 
//Constructor Inputs:
    //Pallet(optional, see constructors) -- Used for making a strobe from a specific pallet
    //Color(optional, see constructors) -- Used for making a single color strobe
    //numColors (optional, see constructors) -- Used for making a strobe from a randomply generated pallet of length numColors
    //BgColor -- The color between strobe pulses. It is a pointer, so it can be tied to an external variable
    //NumPulses -- The number of pulses per strobe
    //PauseTime -- The pause time between strobe cycles
    //segEach -- If true, strobe mode 0 will be run
    //segDual -- If true, strobe mode 1 will be run
    //segLine -- If true, strobe mode 2 will be run
    //segLineDual -- If true, strobe mode 3 will be run
    //segAll -- If true, strobe mode 4 will be run
    //Rate -- The update rate (ms)

//Functions:
    //reset() -- Restarts the strobe using the first mode
    //setNewColorBool(bool newColorBool) -- Changes the newColor flag
    //                                      The newColor flag determines if a new color is choosen every set of pulses
    //                                      or after every strobe cycle
    //setpallet(palletPS *newPallet) -- Changes the pallet, also re-caculates the number of strobe cycles to do
    //setPulseMode() -- Advances the pulse mode to the next mode (only call this if you manually want to change the mode)
    //setCycleCountMax() -- Re-caculates how many strobe cycles to do based on the pallet length (only need to call manually if you're doing something funky)                                                     
    //update() -- updates the effect

//Other Settings:
    //colorMode (default 0) -- sets the color mode for the random pixels (see segDrawUtils::setPixelColor)
    //bgColorMode (default 0) -- sets the color mode for the background (see segDrawUtils::setPixelColor)
    //fillBG (default true) -- flag to fill the background after each set of pulses
    //fillBGOnPause (default true) -- flag to fill the background during each pause
    //pauseEvery (default false) -- pause after every set of pulses rather than after every full strobe cycle
    //newColor (default false) -- reference only, set using setNewColorBool()
    //direct (default true) -- The direction of the pulses for modes 0 and 2 (true starts at segment 0)
    //alternate (default false) -- Set this to alternate the direction of pulses after each strobe cycle for modes 0 and 2
    //randMode (default 0) -- Sets how colors are choosen from the pallet
    //                     -- 0: Colors will be choosen from the pallet in order (not random)
    //                     -- 1: Colors will be choosen completly at random (not using the pallet)
    //                     -- 2: Colors will be choosen randomly from the pallet, same color will not be choosen in a row

//Flags:
    //pause -- set true if a pause is active

//Reference Vars (read only, don't change these unless you know what you're doing):
    //colorNum -- the pallet index of the color currently being pulsed
    //pulseMode -- The current pulse mode
    //cycleCount -- The current cycle we're on out of cycleCountMax
    //cycleCountMax -- The total number of cycles to pulse all segments with all colors shown somewhere at least once
class StrobeSegPS : public EffectBasePS {
    public:
        //constructor for pallet ver
        StrobeSegPS(SegmentSet &SegmentSet, palletPS *Pallet, CRGB BgColor, uint8_t NumPulses, uint16_t PauseTime, 
                    bool SegEach, bool SegDual, bool SegLine, bool SegLineDual, bool SegAll, uint16_t Rate);

        //constructor for single color ver
        StrobeSegPS(SegmentSet &SegmentSet, CRGB Color, CRGB BgColor, uint8_t NumPulses, uint16_t PauseTime, 
                        bool SegEach, bool SegDual, bool SegLine, bool SegLineDual, bool SegAll, uint16_t Rate);
        
        //constructor for randomly generate pallet ver
        StrobeSegPS(SegmentSet &SegmentSet, uint8_t numColors, CRGB BgColor, uint8_t NumPulses, uint16_t PauseTime, 
                        bool SegEach, bool SegDual, bool SegLine, bool SegLineDual, bool SegAll, uint16_t Rate);

        ~StrobeSegPS();  

        SegmentSet 
            &segmentSet; 
        
        bool
            fillBG = true, //flag to fill the background after each set of pulses
            fillBGOnPause = true, //flag to fill the background during each pause
            pause = false, //if a pause is active
            pauseEvery = false, 
            newColor = false, //reference only, set using setNewColorBool()
            direct = true,
            alternate = false,
            segEach,
            segDual,
            segLineDual,
            segLine,
            segAll;
        
        int8_t
            pulseMode = -1; //current pulse mode, for reference only

        uint8_t 
            colorNum = 0, //pallet index of the color currently being pulsed
            randMode = 0,
            numPulses, 
            colorMode = 0,
            bgColorMode = 0;

        unsigned long
            pauseTime;
        
        uint16_t
            cycleCount = 0,
            cycleCountMax;
        
        palletPS
            *pallet; //the pallet used for the strobe colors

        CRGB 
            bgColorOrig,
           *bgColor; //bgColor is a pointer so it can be tied to an external variable if needed (such as a pallet color)
        
        void 
            reset(),
            setPulseMode(),
            setCycleCountMax(),
            setNewColorBool(bool newColorBool),
            setpallet(palletPS *newPallet),
            update(void);
    
    private:
        unsigned long
            currentTime,
            pauseStartTime,
            prevTime = 0;
        
        uint8_t
            numSegs,
            pulseCount = 1,
            randGuess,
            prevGuess = 0, //the last color index guessed
            modeOut;
        
        uint16_t
            nextSeg,    
            cycleLoopLimit,
            maxSegLength;
        
        bool
            boolTemp,
            pulseBG = false,
            firstHalf = true;
        
        palletPS
            palletTemp; //pallet used for auto generate pallets
        
        CRGB
            colorOut,
            colorTemp;
        
        void
            startPause(),
            init(CRGB BgColor, uint16_t Rate),
            pickColor();
};

#endif
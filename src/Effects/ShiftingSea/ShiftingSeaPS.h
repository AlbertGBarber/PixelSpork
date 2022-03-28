#ifndef ShiftingSeaPS_h
#define ShiftingSeaPS_h

//TODO:
//  -- Add option to use a pattern based on a pallet?

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"
#include "Utils/ShiftingSeaUtilsPS.h"

//Cycles each pixel of a segment set through a pallet of colors. Each pixel is given an random 
//offset to place it somewhere mid-fade between the pallet colors, so the result is a shifting
//sea of pallet colors. 
//There are two modes: 0, where the offsets are choosen randomly from any point of fading between any 
//two consecutive colors of the pallet (so the overall output is a mix of the pallet colors)
//or mode 1, where the offsets are choosen randomly from the fade between the first and second colors
//in mode 1, the pixels will all generally shift from color to color, but some will be ahead of others
//creating a varied look.
//You can specify a grouping for the pixels, this will set the offsets of consecutive pixels to be the same
//the number of pixels grouped together is choosen randomly (up to the grouping amount)
//This makes the effect more uniform, and may look better with larger pallets
//By default, once the offsets are set, they do not change. This can make the effect look a bit repetetive
//to counter this, you can turn on random shifting, which will increment the offset of a pixel
//by up to shiftStep (default 1) if a random threshold is met

//Please note that this effect will not work with the colorModes of segDrawUtils::setPixelColor();
//But you can find a rainbow version of the effect in ShiftingRainbowSeaPS.h

//Also note that the class needs a uint16_t array the length of the number of pixels in the segment in order to work
//So if you are short on ram, you might not be able to run this!

//Example call: 
    //ShiftingSeaPS(mainSegments, &pallet3, 20, 0, 3, 40);
    //Will shift through the colors of pallet3, with 20 steps between each shift, using mode 0
    //grouping pixels by 3, at a rate of 40ms

//Inputs:
    //Pallet-- the pallet from which colors will be choosen
    //GradLength -- (max 255) the number of steps to fade from one color to the next
    //Mode -- The mode of the effect, either 0, or 1. 0 for a the offsets to be picked between any two colors
    //                                                1 for the offsets to be picked from between the first two colors
    //Grouping -- Min value of 1. The maximum number of consecutive pixels that can share the same offset
    //            (the offsets are grouped in randomly lengths up to the Grouping value)
    //Rate -- The update rate (ms)

//Functions:
    //setPallet(*newPallet) -- Sets the pallet used for the random pixels
    //resetOffsets() -- Resets the offset array, recaulating offsets for each pixel, will cause a jump if done mid-effect
    //setTotalEffectLength() -- calculates all possible offset values for the pixels, you shouldn't need to call this
    //setMode(newMode) -- Changes the mode of the effect, also resets the offset array
    //setGradLength(newGradLength) -- Changes the Gradlength to the specified value, adjusting the length of the cross fades
    //update() -- updates the effect

//Other Settings:
    //randomShift (default false) -- Turns on/off the random shift for the pixel offsets (see effect description above)
    //shiftThreshold ( default 15) -- Sets the threshold for if a pixel offset will increment, out of 100, with higher values being more likly
    //                                15 seemed to look good in my tests
    //shiftStep (default 1, min 1) -- The maximum value (is choosen randomly) of the offset increment if the shiftThreshold is met

//Notes:
    //Please note that this effect will not work with the colorModes of segDrawUtils::setPixelColor();
    //But you can find a rainbow version of the effect in ShiftingRainbowSeaPS.h

    //Also note that the class needs a uint16_t array the length of the number of pixels in the segment in order to work
    //So if you are short on ram, you might not be able to run this!
class ShiftingSeaPS : public EffectBasePS {
    public:
        ShiftingSeaPS(SegmentSet &SegmentSet, palletPS *Pallet, uint8_t GradLength, uint8_t Smode, uint8_t Grouping, uint16_t Rate);  

        //destructor
        ~ShiftingSeaPS();

        SegmentSet 
            &segmentSet; 
        
        palletPS
            *pallet;
        
        uint8_t
            shiftThreshold = 15,
            shiftStep = 1,
            grouping,
            sMode,
            gradLength;
        
        uint16_t
            *offsets,
            totalCycleLength, //the total number of possible offsets a pixel can have (one for each fade color)
            cycleNum = 0, //tracks how many update's we've done, max value of totalCycleLength
            numPixels;
        
        bool
            randomShift = false;
        
        void
            setPallet(palletPS *newPallet),
            setTotalEffectLength(), 
            setGradLength(uint8_t newGradLength),
            setMode(uint8_t newMode),
            resetOffsets(),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;

        CRGB
            color,
            currentColor,
            nextColor;

        uint8_t 
            currentColorIndex, 
            gradStep;

        uint16_t
            step = 0;
};

#endif
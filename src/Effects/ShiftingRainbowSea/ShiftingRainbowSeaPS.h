#ifndef ShiftingRainbowSeaPS_h
#define ShiftingRainbowSeaPS_h

#include "SegmentFiles.h"
#include "palletFiles.h"

#include "Effects/EffectBasePS.h"
#include "Effects/ShiftingSea/Utils/ShiftingSeaUtilsPS.h"

//Cycles each pixel of a segment set through a rainbow. Each pixel is given an random 
//offset to place it somewhere mid-rainbow, so the result is a shifting sea of rainbow colors. 
//There are two modes: 0, where the offsets are choosen randomly from any point of the rainbow
//or mode 1, where the offsets are choosen randomly up to a set value (Gradlength, max 255)
//in mode 1, the pixels will all generally shift from color to color, but some will be ahead of others
//creating a varied look, whereas in mode 0 it is more random
//These different modes are set automatically by two seperate constructors.
//You can specify a grouping for the pixels, this will set the offsets of consecutive pixels to be the same
//the number of pixels grouped together is choosen randomly (up to the grouping amount)
//This makes the effect more uniform, and may look better with higher pixel numbers
//By default, once the offsets are set, they do not change. This can make the effect look a bit repetetive
//to counter this, you can turn on random shifting, which will increment the offset of a pixel
//by up to shiftStep (default 1) if a random threshold is met

//Please note that this effect will not work with the colorModes of segDrawUtils::setPixelColor();
//But you can find a non-rainbow version of the effect in ShiftingSeaPS.h

//Also note that the class needs a uint16_t array the length of the number of pixels in the segment in order to work
//So if you are short on ram, you might not be able to run this!

//Example calls: 
    //ShiftingRainbowSeaPS *CFC = new ShiftingRainbowSeaPS(mainSegments, 3, 20);
    //Will shift through the rainbow, using mode 0 (offsets can be anything between 0 and 255), 
    //grouping pixels by 3, at a rate of 20ms

    //ShiftingRainbowSeaPS *CFC = new ShiftingRainbowSeaPS(mainSegments, 10, 3, 20);
    //Will shift through the rainbow, using mode 1, where the offsets are choosen to be in a range of 0 to 10
    //grouping pixels by 3, at a rate of 20ms

//Inputs:
    //GradLength (optional, see constructors, default 30, min 1) -- Used for mode 1, the maximum initial value of an offset
    //Grouping -- Min value of 1. The maximum number of consecutive pixels that can share the same offset
    //            (the offsets are grouped in randomly lengths up to the Grouping value)
    //Rate -- The update rate (ms)

//Functions:

    //resetOffsets() -- Resets the offset array, recaulating offsets for each pixel, will cause a jump if done mid-effect
    //setMode(newMode) -- Changes the mode of the effect, also resets the offset array
    //setGradLength(newGradLength) -- Changes the Gradlength to the specified value, resets the offset array (if in mode 1)
    //update() -- updates the effect

//Other Settings:
    //sat and val (default 255) -- the saturation and value settings for the rainbow
    //Mode -- The mode of the effect, either 0, or 1. 0 for the offsets to be picked at any point in the rainbow
    //                                                1 for the offsets to be picked between 0 and Gradlength
    //randomShift (default false) -- Turns on/off the random shift for the pixel offsets (see effect description above)
    //shiftThreshold (default 15) -- Sets the threshold for if a pixel offset will increment, out of 100, with higher values being more likly
    //                                15 seemed to look good in my tests
    //shiftStep (default 1, min 1) -- The maximum value (is choosen randomly) of the offset increment if the shiftThreshold is met

//Notes:
    //Please note that this effect will not work with the colorModes of segDrawUtils::setPixelColor();
    //But you can find a rainbow version of the effect in ShiftingRainbowSeaPS.h

    //Also note that the class needs a uint16_t array the length of the number of pixels in the segment in order to work
    //So if you are short on ram, you might not be able to run this!
class ShiftingRainbowSeaPS : public EffectBasePS {
    public:
        //constructor for mode 0
        ShiftingRainbowSeaPS(SegmentSet &SegmentSet, uint8_t Grouping, uint16_t Rate);
        //constructor for mode 1  
        ShiftingRainbowSeaPS(SegmentSet &SegmentSet, uint8_t GradLength, uint8_t Grouping, uint16_t Rate);  

        SegmentSet 
            &segmentSet; 
        
        uint8_t
            sat = 255,
            val = 255,
            shiftThreshold = 15,
            grouping = 1,
            mode = 0,
            gradLength = 30,
            shiftStep = 1;
        
        uint16_t
            *offsets,
            cycleNum = 0, //tracks how many update's we've done, max value of totalCycleLength
            numPixels;
        
        bool
            randomShift = false;
        
        void
            setGradLength(uint8_t newGradLength),
            setMode(uint8_t newMode),
            resetOffsets(),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;

        CRGB
            color;

        uint8_t
            step;
        
        void
            init(uint16_t Rate);
};

#endif
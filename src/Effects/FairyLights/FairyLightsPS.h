#ifndef FairyLightsPS_h
#define FairyLightsPS_h

#include "Effects/EffectBasePS.h"
#include "Effects/EffectUtils/EffectUtilsPS.h"

//Colors a number of pixels on at a time, turning them off depending on the mode choosen
//Meant to be similar to classic twinkling fairy lights
//The color of the pixels be set to a single color, chosen randomly, of picked from a pallet 
//Just run an example and you'll see what I mean

//Modes:
    //Mode 0: Turns one pixel on after another and then resets them all at once
    //Mode 1: Turns on each pixel one at a time, then off one at a time
    //Mode 2: Each cycle, a new pixel is turned on while an old is turned off (first on first off)

//Example calls: 
    //FairyLightsPS(mainSegments, pallet, 8, 0, 0, 150);
    //Will choose 8 pixels to cycle to/from colors choosen from the pallet, using a blank background, 
    //Each pixel will be turned on one at a time, before reseting them all at once (mode 0), with 150ms between each cycle

    //FairyLightsPS(mainSegments, CRGB::Red, 10, CRGB::Blue, 1, 100);
    //Will choose 10 pixels to set to red before reseting, using a blue background, 
    //The pixels will be turned on one at a time, and then off one at a time (mode 1), with 100ms between each cycle

    //FairyLightsPS(mainSegments, 12, 0, 2, 80);
    //Will choose 12 pixels each cycle to set to random colors, using a blank backgound, 
    //Each cycle, a new pixel will be turned on, while an old is turned off (mode 2), with 80ms in between each cycle

//Constructor Inputs:
    //Pallet(optional, see constructors) -- the pallet from which colors will be choosen randomly
    //Color(optional, see constructors) -- the color that the randomly choosen pixels will be set to
    //NumTwinkles -- The amount of random pixels choosen for twinkling
    //BgColor -- The color of the background, this is what pixels will fade to and from
    //tMode -- The twinkling mode for the effect (see above for descriptions)
    //Rate -- The update rate (ms)

//Functions:
    //setSingleColor(Color) -- Sets the effect to use a single color for the pixels, will restart the effect
    //setPallet(*newPallet) -- Sets the pallet used for the random pixels
    //reset() -- Resets the startup variables, you probably don't need to ever call this
    //setNumTwinkles(newNumTwinkles) -- sets an new number of pixels to be choosen each cycle, will reset the current set of twinkles
    //genPixelSet() -- creates a new group of pixels for twinkling (you shouldn't need to call this)
    //update() -- updates the effect

//Other Settings:
    //colorMode (default 0) -- sets the color mode for the random pixels (see segDrawUtils::setPixelColor)
    //bgColorMode (default 0) -- sets the color mode for the background (see segDrawUtils::setPixelColor)
    //fillBG (default false) -- sets the background to be redrawn every cycle, useful for bgColorModes that are dynamic
    //reDrawAll (default false) -- Will re-draw all the pixels each cycle, is slower than default, but you need this if you want to layer this effect with another
    //                             (is set true if fillBG is true)

//Notes:
    //Requires an uint16_t array and CRGB array of length NumTwinkles to work, make sure you have the memory for them!
    //If you turn fillBG on the off, make sure to also turn off reDrawAll
class FairyLightsPS : public EffectBasePS {
    public:
        //pallet based constructor
        FairyLightsPS(SegmentSet &SegmentSet, palletPS *Pallet, uint8_t NumTwinkles, CRGB BGColor, uint8_t Tmode, uint16_t Rate);
        
        //single color constructor
        FairyLightsPS(SegmentSet &SegmentSet, CRGB Color, uint8_t NumTwinkles, CRGB BGColor, uint8_t Tmode, uint16_t Rate);
        
        //random colors constructor
        FairyLightsPS(SegmentSet &SegmentSet, uint8_t NumTwinkles, CRGB BGColor, uint8_t Tmode, uint16_t Rate); 

        //destructor
        ~FairyLightsPS();

        SegmentSet 
            &segmentSet; 
        
        uint8_t
            cycleNum = 0,
            numTwinkles,
            tmode,
            colorMode = 0,
            bgColorMode = 0;
        
        uint16_t
            *pixelSet;
        
        CRGB
            bgColorOrig = 0,
            *bgColor, //bgColor is a pointer so it can be tied to an external variable if needed (such as a pallet color)
            color,
            *colorSet;
        
        palletPS
            *pallet;

        bool
            reDrawAll = false,
            fillBG = false,
            turnOff = false; //tracks if we're turning the pixels on or off, depending on the mode
        
        void
            setSingleColor(CRGB Color),
            setPallet(palletPS *newPallet),
            setNumTwinkles(uint8_t newNumTwinkles),
            genPixelSet(),
            init(CRGB BgColor, uint16_t Rate),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        uint8_t
            palletLength,
            cycleLimit,
            loopStart,
            loopEnd;

        pixelInfoPS
            pixelInfo{0, 0, 0, 0};
        
        palletPS
            palletTemp;
        
        void
            pickColor(uint16_t pixelNum),
            modeZeroSet(),
            modeOneSet(),
            modeTwoSet();
};

#endif
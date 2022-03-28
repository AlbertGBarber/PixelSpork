#ifndef GlimmerPS_h
#define GlimmerPS_h


#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

//The goal of the effect is to fade a random set of leds in and out, between the glimmerColor and the bgColor
//(note that the bgColor is the dominant color in the effect)
//each led fades a random amount towards glimmerColor, but all leds fade together (their fades finish at the same time)
//the fade amounts are restricted between fadeMin and fadeMax
//The effect can be set so that one or two sets of leds are fading in/out at any one time
//With one set, it is faded in then out be for choosing a new set of leds
//while with two sets, one set is being faded in while the other is being faded out
//Note that the effect needs one uint16_t and one uint8_t array in order to work
//With one set of leds, the array lengths are the number of fading leds
//With two sets, the length is doubled

//Both glimmerColor and bgColor are pointers, so you can bind them to external color vars

//TwoPixelSets setting:
    //True: Two sets of leds with be driven at once, with one set fading in while the other is fading out
    //False: Only one set of leds with be driven at a time, with the set being fully faded in then out before a new set is picked

//Example calls: 
    //GlimmerPS(mainSegments, 8, CRGB::Blue, 0, true, 10, 50);
    //Will choose 8 pixels to cycle to/from off to CRGB::Blue 
    //Two sets of pixels will be faded in/out at one time, going through 10 fade steps, with 50ms between each step

    //GlimmerPS(mainSegments, 10, CRGB::Blue, CRGB::Red, false, 20, 100, 255, 60);
    //Will choose 10 pixels to to cycle to/from CRGB::Red to CRGB::Blue 
    //One set of pixels will be faded in then out before a new set is choosen
    //The range of the fades will be between 100 and 255, with 20 fade steps, with 60ms between each step

//Constructor Inputs:
    //NumTwinkles -- The amount of random pixels choosen for fading
    //GlimmerColor -- The color the pixels will be faded towards
    //BgColor -- The color of the background, this is the color that not-fading pixels will be
    //TwoPixelSets -- (bool) sets if one or two sets of pixels will be fading in/out at once (true for two sets)
    //FadeMin (optional, max 255, default 50) -- The minimum amount that a pixel will fade by
    //Fademax (optional, max 255, default 255) -- The maximum amount that a pixel will fade by
    //FadeSteps -- The number of steps it takes a pixel to fade (one step every update cycle)
    //Rate -- The update rate (ms)

//Functions:
    //setNumPixels(newNumPixels) -- Changes the number of pixels fading, resets the current fade cycle
    //setTwoSets(newSetting) -- Changes the TwoPixelSets variable, if different, it will reset the current fade cycle
    //setupPixelArray() and fillPixelArray() -- Both generate a new set of pixels for fading, you should not need to call these
    //update() -- updates the effect

//Other Settings:
    //colorMode (default 0) -- sets the color mode for the random pixels (see segDrawUtils::setPixelColor)
    //bgColorMode (default 0) -- sets the color mode for the background (see segDrawUtils::setPixelColor)
    //fillBG (default false) -- sets the background to be redrawn every cycle, useful for bgColorModes that are dynamic
    //fadeMin  and fadeMax -- (see Costructor inputs)

//Flags / Counters: 
    //fadeIn -- Tracks is the set of leds is fading in or not, for two sets, this varaible will be toggled mid-set
    //step -- Tracks the fade step we're on, max value is fadeSteps, is reset to 0 once a fade is finished

//Notes:
    //Requires uint16_t array and uint8_t arrays of length up to 2*NumPixels to work, make sure you have the memory for them!
    
class GlimmerPS : public EffectBasePS {
    public:
        GlimmerPS(SegmentSet &SegmentSet, uint16_t NumPixels, CRGB GlimmerColor, CRGB BgColor, bool TwoPixelSets, uint8_t FadeSteps, uint16_t Rate);  

        GlimmerPS(SegmentSet &SegmentSet, uint16_t NumPixels, CRGB GlimmerColor, CRGB BgColor, bool TwoPixelSets, uint8_t FadeSteps, uint8_t FadeMin, uint8_t FadeMax, uint16_t Rate);

        ~GlimmerPS();
        
        SegmentSet 
            &segmentSet; 
        
        uint8_t 
            colorMode = 0,
            bgColorMode = 0,
            *totFadeSteps,
            fadeSteps,
            step = 0,
            fadeMin = 50,
            fadeMax = 255;

        uint16_t
            *fadePixelLocs,
            numPixels;
        
        bool 
            fadeIn,
            fillBG = false,
            twoPixelSets;
        
        CRGB 
            colorOrig,
            *glimmerColor, //glimmerColor is a pointer so it can be tied to an external variable if needed (such as a pallet color)
            bgColorOrig,
            *bgColor; //bgColor is a pointer so it can be tied to an external variable if needed (such as a pallet color)
        
        void 
            setupPixelArray(),
            fillPixelArray(),
            setNumPixels(uint16_t newNumPixels),
            setTwoSets(bool newSetting),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        uint16_t
            arrayLength,
            numActiveLeds;
        
        pixelInfoPS
            pixelInfo{0, 0, 0, 0};
        
        CRGB 
            color,
            targetColor;
        
        bool
            firstFade = false;
        
        void 
            init(CRGB GlimmerColor, CRGB BgColor, uint16_t Rate),
            advancePixelArray();
};

#endif
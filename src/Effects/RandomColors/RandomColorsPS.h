#ifndef RandomColorsPS_h
#define RandomColorsPS_h

#include "SegmentFiles.h"
#include "palletFiles.h"

#include "Effects/EffectBasePS.h"

//Fades sets of randomly chosen pixels in and out (like FastLED TwinkleFox)
//The color of the pixels be set to a single color, chosen randomly, of picked from a pallet 
//The amount of fade in and out steps are controlled by FadeInSteps, and FadeOutSteps
//Please note that the total number of pixels colored at one time will be NumPixels * (FadeInSteps + FadeOutSteps)
//Just run an example and you'll see what I mean

//Example call: 
    //RandomColorsPS *CFC = new RandomColorsPS(mainSegments, CRGB::Red, 2, 0, 4, 4, 70);
    //Will choose 2 pixels each cycle to fade to/from red, using a blank background

//Inputs:
    //Pallet(optional, see constructors) -- the pallet from which colors will be choosen randomly
    //Color(optional, see constructors) -- the color that the randomly choosen pixels will be set to
    //NumPixels -- The amount of random pixels choosen each cycle 
    //BgColor -- The color of the background, this is what pixels will fade to and from
    //FadeInSteps and FadeOutSteps -- The number of steps taken to fade pixels in and out (min value of 1, max of 255)
    //Rate -- The update rate

//Functions:
    //setSteps(newfadeInSteps, newfadeOutSteps) -- Sets the number of fade in and out steps, will restart the effect
    //                                             !!DO NOT set the number of steps directly
    //setSingleColor(Color) -- Sets the effect to use a single color for the pixels, will restart the effect
    //setPallet(*newPallet) -- Sets the pallet used for the random pixels
    //reset() -- Resets the startup variables, you probably don't need to ever call this
    //setNumPixels(newNumPixels) -- sets an new number of pixels to be choosen each cycle, will restart the effect
    //update() -- updates the effect

//Other Settings:
    //colorMode (default 0) -- sets the color mode for the random pixels (see segDrawUtils::setPixelColor)
    //bgColorMode (default 0) -- sets the color mode for the background (see segDrawUtils::setPixelColor)
    //fillBG (default false) -- sets the background to be redrawn every cycle, useful for bgColorModes that are dynamic

//Notes:
    //In order for the effect to work, it needs to keep track of all the active random pixels until they fade out
    //To do this it uses a pair of 2D uint16_t and CRGB arrays of size numPixels by (FadeInSteps + FadeOutSteps)
    //This takes up a good amount of ram, so try to limit your pixel number and fade lengths
    //In practice, fewer pixels with longer fades will keep ram minimal, while also looking good
    //If you don;t have enough ram, check the RandomColorsLowRam version of this effect
    
    //You cannot dynamically change the fade lengths or number of random pixels without restarting the effect
    //It would be very complicated to allow this due to having to resize/copy the arrays, while also fading away
    //all active pixels fully
    //(it might look ok to initialize the arrays with random values, but idk)
    //You can do this with the RandomColorsLowRam version of this effect, but it is more limited 
class RandomColorsPS : public EffectBasePS {
    public:
        //Constructor for a full pallet effect
        RandomColorsPS(SegmentSet &SegmentSet, palletPS *Pallet, uint16_t NumPixels, CRGB BgColor, uint8_t FadeInSteps, uint8_t FadeOutSteps, uint16_t Rate);  
        //Constructor for a using a single color
        RandomColorsPS(SegmentSet &SegmentSet, CRGB Color, uint16_t NumPixels, CRGB BgColor, uint8_t FadeInSteps, uint8_t FadeOutSteps, uint16_t Rate);
        //Constructor for choosing all colors at random
        RandomColorsPS(SegmentSet &SegmentSet, uint16_t NumPixels, CRGB BgColor, uint8_t FadeInSteps, uint8_t FadeOutSteps, uint16_t Rate);

        uint16_t
            numPixels;

        CRGB 
            bgColor;

        bool 
            fillBG = false;
        
        uint8_t 
            colorMode = 0,
            bgColorMode = 0;

        uint8_t 
            fadeInSteps,
            fadeOutSteps;

        SegmentSet 
            &segmentSet; 
        
        palletPS
            *pallet;
        
        void 
            setSteps(uint8_t newfadeInSteps, uint8_t newfadeOutSteps),
            setSingleColor(CRGB Color),
            setPallet(palletPS *newPallet),
            reset(),
            setNumPixels(uint16_t newNumPixels),
            initPixelArrays(),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        uint8_t
            totalSteps = 0,
            totFadeSteps;

        uint16_t
            **ledArray = nullptr;

        bool
            startUpDone = false;
        
        CRGB
            **colorIndexArr = nullptr,
            color; //this variable is slightly abused, since it is set by multiple functions

        pixelInfoPS
            pixelInfo{0, 0, 0, 0};
        
        palletPS
            palletTemp;
        
        void
            init(uint8_t FadeInSteps, uint8_t FadeOutSteps, uint16_t Rate),
            incrementPixelArrays(),
            pickColor(uint16_t pixelNum);
};

#endif
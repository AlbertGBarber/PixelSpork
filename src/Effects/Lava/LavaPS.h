#ifndef LavaPS_h
#define LavaPS_h

//TODO -- Add rainbow mode? May not look that good.

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

//An effect based on the lava.ino code created by Scott Marley here: https://github.com/s-marley/FastLED-basics/tree/main/6.%20Noise/lava
//See more about noise here: https://github.com/s-marley/FastLED-basics/tree/main/6.%20Noise
//A fairly simple, but great looking effect that uses FastLED noise functions to produce a lava-esque effect
//You can also supply your own pallet for the effect to add your own colors
//or choose to have a pallet of random color be created

//You can customize how the effect looks by adjusting the blendSteps and blendScale values
//I encourage playing with these, since they can change the effect a lot

//Example calls: 
    //LavaPS(mainSegments, 10);
    //Will do a lava effect, updating at 10ms

    //LavaPS(mainSegments, 40, 20, 10);
    //Will do a lava effect, with 40 blendSteps
    //and a blendScale of 20 (see inputs for info below)
    //at an update rate of 10ms

    //LavaPS(mainSegments, &pallet1, 40, 80, 10);
    //Will do a lava effect using pallet1 for colors, 
    //with 40 blendSteps
    //and a blendScale of 80
    //at an update rate of 10ms

    //LavaPS(mainSegments, 3, 40, 80, 10);
    //Will do a lava effect with a pallet of 3 randomly choosen colors
    //with 40 blendSteps
    //and a blendScale of 80
    //at an update rate of 10ms

//Constructor inputs: 
    //pallet (optional, see constructors) -- A custom pallet passed to the effect, the default is the 
    //                                       lava colors pallet encoded below
    //numColors (optional, see constructors) -- How many colors will be in the randomly created pallet
    //blendSteps (optional, see constructors) -- Sets how many steps are used to blend between each color
    //                                           Basically changes how fast the colors blend
    //                                           Between 20 - 100 looks good
    //blendScale (optional, see constructors) -- Sets how large the areas that a blended are
    //                                           (test it out yourself to see what I mean)
    //                                           Between 10 - 100 looks good
    //Rate -- The update rate (ms) note that this is synced with all the particles.

//Functions:
    //update() -- updates the effect 

//Other Settings:
    //brightnessScale (default 150) -- Sets the noise range for the brightness
    //                                 It doesn't seem to change much in my testing
class LavaPS : public EffectBasePS {
    public:
        LavaPS(SegmentSet &SegmentSet, uint16_t Rate);  

        LavaPS(SegmentSet &SegmentSet, uint16_t blendSteps, uint16_t BlendScale, uint16_t Rate);  

        LavaPS(SegmentSet &SegmentSet, palletPS *Pallet, uint16_t BlendSteps, uint16_t BlendScale, uint16_t Rate);
        
        //constructor for a randomly created pallet
        LavaPS(SegmentSet &SegmentSet, uint8_t numColors, uint16_t BlendSteps, uint16_t BlendScale, uint16_t Rate);
    
        ~LavaPS();

        SegmentSet 
            &segmentSet; 
        
        uint16_t
            blendSteps = 30,
            blendScale = 80,
            brightnessScale = 150;
        
        //the default lava pallet, basically a blend from dark red to yellow
         CRGB 
            lavalPallet_arr[5] = { CRGB::DarkRed, CRGB::Maroon, CRGB::Red, CRGB::Orange, CRGB(245, 202, 10) };
        
        palletPS
            palletTemp,
            *pallet;

        void 
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        uint8_t
            brightness,
            numSegs;

        uint16_t
            index,
            totBlendLength,
            totSegLen,
            pixelNum;
        
        bool
            randPalletCreated;
        
        CRGB 
            colorOut;
        
        void
            init(uint16_t Rate);
};

#endif
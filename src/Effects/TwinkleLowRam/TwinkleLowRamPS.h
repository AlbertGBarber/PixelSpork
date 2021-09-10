#ifndef TwinkleLowRamPS_h
#define TwinkleLowRamPS_h

#include "Effects/EffectBasePS.h"
#include "Effects/EffectUtils/EffectUtilsPS.h"

//Draws sets of randomly chosen pixels (like FastLED TwinkleFox)
//This is mostly the same as TwinklePS, but uses much less ram, and has a few restrictions
//Like, TwinklePS, the color of the pixels can be set to a single color, chosen randomly, or picked from a pallet 
//However, the pixels can either be set to fade out, or switch to a background color, but not both
//so if you choose to fade out the background will be set to 0
//(this is because the whole segmentSet is faded each cycle)

//Example call: 
    //TwinklePS *CFC = new TwinklePS(mainSegments, CRGB::Red, 5, 0, true, 50, 70);
    //Will choose 5 pixels to fade from red at 50 rate every cycle

//Inputs:
    //Pallet(optional, see constructors) -- the pallet from which colors will be choosen randomly
    //Color(optional, see constructors) -- the color that the randomly choosen pixels will be set to
    //NumPixels -- The amount of random pixels choosen each cycle 
    //BgColor -- The color of the background, this is what pixels will fade to and from
    //Sparkle -- Turns the fading on and off (on will produce a blank background)
    //FadeOutRate -- The number of steps taken to fade pixels out, if turned on (min 1, max 255), higher = fasted fade
    //Rate -- The update rate

//Functions:
    //setSingleColor(Color) -- Sets the effect to use a single color for the pixels, will restart the effect
    //setPallet(*newPallet) -- Sets the pallet used for the random pixels
    //update() -- updates the effect

//Other Settings:
    //colorMode (default 0) -- sets the color mode for the random pixels (see segDrawUtils::setPixelColor)
    //bgColorMode (default 0) -- sets the color mode for the background (see segDrawUtils::setPixelColor)
    //fillBG (default true) -- sets the background to be redrawn every cycle
class TwinkleLowRamPS : public EffectBasePS {
    public:
        //Constructor for a full pallet effect
        TwinkleLowRamPS(SegmentSet &SegmentSet, palletPS *Pallet, uint16_t NumPixels, CRGB BgColor, bool Sparkle, uint8_t FadeOutRate, uint16_t Rate);  
        //Constructor for a using a single color
        TwinkleLowRamPS(SegmentSet &SegmentSet, CRGB Color, uint16_t NumPixels, CRGB BgColor, bool Sparkle, uint8_t FadeOutRate, uint16_t Rate);
        //Constructor for choosing all colors at random
        TwinkleLowRamPS(SegmentSet &SegmentSet, uint16_t NumPixels, CRGB BgColor, bool Sparkle, uint8_t FadeOutRate, uint16_t Rate);
        
        //destructor
        ~TwinkleLowRamPS();

        uint16_t
            numPixels;

        CRGB 
            bgColor;
        
        bool
            fillBG = true,
            sparkle;
        
        uint8_t
            colorMode = 0,
            bgColorMode = 0;

        uint8_t 
            fadeOutRate;

        SegmentSet 
            &segmentSet; 
        
        palletPS
            *pallet;
        
        void 
            setSingleColor(CRGB Color),
            setPallet(palletPS *newPallet),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        uint8_t 
            palletLength;

        CRGB
            color; //this variable is slightly abused, since it is set by multiple functions

        palletPS
            palletTemp;
        
        void
            init(uint16_t Rate);
};

#endif
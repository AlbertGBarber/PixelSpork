#ifndef PacificaPS_h
#define PacificaPS_h

//TODO: -- Add lava palette as an option?

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
//The source of the palettes for the effect
#include "pacificaPalette/pacificaPalettePS.h"

/*

  "Pacifica"
  Gentle, blue-green ocean waves.
  By Mark Kriegsman and Mary Corey March.
  For Dan.


An effect based on the pacific code here: https://github.com/FastLED/FastLED/blob/master/examples/Pacifica/Pacifica.ino
A gentle effect of blue-green ocean waves rolling across the strip
The effect runs as is; there's no customization
Almost all of the code is directly copied from the original linked above
If you have any questions about how it works, please direct them to Mark Kriegsman

The palettes can be found in "pacificaPalette.h"

Recommend brightness >80 and rate > 20ms
For low brightness values I recommend turing off dithering using
FastLED.setDither(0);

The effect is a bit computationally heavy

Technically, I've made the background color a pointer so you can bind it externally
You probably shouldn't do this tho, b/c it heavily influences the effect

Example calls: 
    PacificaPS pacifica(mainSegments, 40);
    That's it, updates at 40ms

Functions:
    update() -- updates the effect 

*/
class PacificaPS : public EffectBasePS {
    public:
        PacificaPS(SegmentSet &SegSet, uint16_t Rate);  
        
        uint8_t 
            //Produces a total blend length of 240 for the whole palette, matches the original code
            numSteps = 240 / pacificaPal1PS.length;
        
        CRGB 
            bgColorOrig = CRGB( 2, 6, 10 ), //CRGB(10, 0, 0); for lava colors? Messing with this is tricky.
            *bgColor = &bgColorOrig; //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)
        
        void 
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0,
            deltaTime,
            deltaTime1,
            deltaTime2,
            deltaTime21;
        
        uint8_t
            baseThreshold,
            wave,
            threshold,
            lightLvl,
            overage,
            overage2;

        uint16_t 
            totBlendLength,
            totSegLen,
            numSegs,
            pixelNum,
            sCIStart1,
            sCIStart2,
            sCIStart3, 
            sCIStart4,
            speedFactor1,
            speedFactor2,
            ci,
            waveAngle,
            waveScaleHalf,
            s16,
            cs,
            index,
            sIndex16;
        
        CRGB
            colorOut;
        
        void 
            doOneLayer( palettePS *palette, uint16_t ciStart, uint16_t waveScale, uint8_t bri, uint16_t iOff),
            addWhitecaps(),
            deepenColors();
};

#endif
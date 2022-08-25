#ifndef PacificaSLPS_h
#define PacificaSLPS_h

//TODO: -- Add lava palette as an option?

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
//The source of the palettes for the effect
#include "Effects/Pacifica/pacificaPalettePS.h"

/*

 "Pacifica" (segLine version)
  Gentle, blue-green ocean waves.
  By Mark Kriegsman and Mary Corey March.
  For Dan.

An effect based on the pacific code here: https://github.com/FastLED/FastLED/blob/master/examples/PacificaSL/PacificaSL.ino
A gentle effect of blue-green ocean waves rolling across the strip
This version of Pacifica differs from the normal because we draw the colors along seg lines
This produces uniform waves that shift across the whole segment set

The effect runs as is; there's no customization
Almost all of the code is directly copied from the original linked above
(with modifications by me to work with segment sets)
If you have any questions about how it works, please direct them to Mark Kriegsman

The palettes can be found in "Effects/Pacifica/pacificaPalette.h"

Recommend brightness > 80 and rate > 20ms
For low brightness values I recommend turing off dithering using
FastLED.setDither(0);

I've made the addWhiteCaps() function optional. You can turn it on by setting addWhiteCaps true
It brightnes the areas where waves meet. 
For all segment shapes except matricies the code will now do this automatically.
So you should only need to turn addWhiteCaps() on for matricies
I've also added a constant thresholdMax value that caps the white cap light level
To prevent them from getting too white

The effect is a bit computationally heavy

Technically, I've made the background color a pointer so you can bind it externally
You probably shouldn't do this tho, b/c it heavily influences the effect

Example calls: 
    PacificaSLPS(ringSegments, 40);
    //That's it, updates at 40ms
    //(addWhiteCaps is default false)

    PacificaSLPS(ringSegments, true, 40);
    //Sets addWhiteCaps to true, updates at 40ms

Functions:
    update() -- updates the effect 

Other Settings:
    addWhiteCaps (default false) -- If true, the addWhiteCaps() function will  be called as part of the update cycle
    thresholdMax (default 230) -- caps the white cap light level to prevent pixels from getting too white
                                  as part of the addWhiteCaps() function
*/
class PacificaSLPS : public EffectBasePS {
    public:
        //Normal constructor
        PacificaSLPS(SegmentSet &SegmentSet, uint16_t Rate);  

        //constructor with addWhiteCaps setting
        PacificaSLPS(SegmentSet &SegmentSet, bool AddWhiteCaps, uint16_t Rate);

        SegmentSet 
            &segmentSet; 
        
        bool
            addWhiteCaps = false;
        
        uint8_t 
            //Produces a total blend length of 240 for the whole palette, matches the original code
            numSteps = 240 / pacificaPal1PS.length,
            thresholdMax = 230; //cap on light level for addWhiteCaps(), stops colors from going full white
        
        CRGB 
            bgColorOrig,
           *bgColor; //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)

        void 
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0,
            deltams,
            deltams1,
            deltams2,
            deltams21;
        
        uint8_t
            basethreshold,
            wave,
            threshold,
            lightLvl,
            overage,
            overage2;

        uint16_t 
            numLines,
            numSegs,
            totBlendLength,
            pixelNum,
            sCIStart1,
            sCIStart2,
            sCIStart3, 
            sCIStart4,
            speedfactor1,
            speedfactor2,
            ci,
            waveangle,
            wavescale_half,
            s16,
            cs,
            index,
            sindex16;
        
        CRGB
            colorOut;
        
        void 
            init(uint16_t Rate),
            doOneLayer( palettePS *palette, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff),
            addWhitecaps(),
            deepenColors();
};

#endif
#ifndef PacificaHueSL_h
#define PacificaHueSL_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
//The source of the palettes for the effect
#include "PacificaHuePalPS.h"

/*

 "Pacifica" (segLine and hue version)
  Gentle, blue-green ocean waves.
  By Mark Kriegsman and Mary Corey March.
  For Dan.

An effect based on the pacific code here: https://github.com/FastLED/FastLED/blob/master/examples/PacificaSL/PacificaSL.ino
A gentle effect of blue-green ocean waves rolling across the strip
This version of Pacifica differs from the normal because we draw the colors along seg lines
This produces uniform waves that shift across the whole segment set
This version also supports different hues, so you can change the pacifica colors

Recommend brightness > 80 and rate > 20ms
For low brightness values I recommend turing off dithering using
FastLED.setDither(0);

The effect is a bit computationally heavy

Almost all of the code is directly copied from the original linked above
(with modifications by me to work with segment sets and hues)
If you have any questions about how it works, please direct them to Mark Kriegsman

I've made the addWhiteCaps() function optional. You can turn it on by setting addWhiteCaps true
It brightnes the areas where waves meet. It works best for blue hues, 
For all segment shapes except matricies the code will now do this automatically.
So you should only need to turn addWhiteCaps() on for matricies
I've also added a constant thresholdMax value that caps the white cap light level
To prevent them from getting too white

To get the effect to work with multiple colors the palettes needed to be heavily modified.
The palettes and info about hues can be found in "PacificaHuePal.h"
Basically the hue sets where our base color is in the rainbow
The palettes then form their colors around that base color
This means that you'll always end up with colors that are close to each other in the rainbow
(ie you can't have green and purple together)
This keeps with the look of the original effect
    Some hue ranges are:
        Red/Orange: 240 to ~10 
        Orange/Yellow: 10 to ~40
        Yellow/Green: 40 to ~90
        Green/Blue: 90 to ~150
        Blue/Purple: 150 to ~180
        Purple/Red: 180 to 240
        
By default the hue is set to 130, which closly matches the colors from the original effect.

You can also set the hue to cycle through colors over time at hueRate (ms) (see constructors below)
A hue rate of 0 will stop the hue cycle
The hue is updated as part of the effect, so the hueRate should be slower than the effect's update rate
Note that the hueRate is a pointer (like the overall effect Rate), so you can bind it to an external variable if wanted.
The rate passed into the constructors is hueRateOrig

Example calls: 
    PacificaHueSL(ringSegments, 40);
    That's it, updates at 40ms
    (addWhiteCaps is default false, and the hue will default to 130 to match the original pacifica colors)

    PacificaHueSL(ringSegments, true, 130, 40);
    Sets addWhiteCaps to true and the hue to 130, updates at 40ms

    PacificaHueSL(ringSegments, true, 50, 500, 40);
    Sets addWhiteCaps to true, with the hue starting at 50 and incrementing every 500ms
    The effect updates at 40ms

Functions:
    setHue(uint8_t newHue) -- Changes the hue value
    update() -- updates the effect 

Other Settings:
    addWhiteCaps (default false) -- If true, the addWhiteCaps() function will  be called as part of the update cycle
    thresholdMax (default 230) -- caps the white cap light level to prevent pixels from getting too white
                                  as part of the addWhiteCaps() function
    PacificaPalette -- The PacificaHuePal object. You shouldn't need to access this.

Reference vars:
    hue -- The current hue setting, use setHue() to change
    numSteps -- How many steps there are per color gradient. You shouldn't need to change this.
*/
class PacificaHueSL : public EffectBasePS {
    public:
        //Normal constructor
        PacificaHueSL(SegmentSet &SegmentSet, uint16_t Rate);  

        //constructor with addWhiteCaps setting
        PacificaHueSL(SegmentSet &SegmentSet, bool AddWhiteCaps, uint8_t Hue, uint16_t Rate);

        //constructor with addWhiteCaps and hue rate settings
        //The hue will be the initial hue of the palette
        PacificaHueSL(SegmentSet &SegmentSet, bool AddWhiteCaps, uint8_t Hue, uint16_t HueRate, uint16_t Rate);

        ~PacificaHueSL();

        SegmentSet 
            &segmentSet; 

        PacificaHuePalPS
            *PacificaPalette;
        
        bool
            addWhiteCaps = false;
        
        uint8_t 
            hue, //for reference, call setHue() to change the hue
            numSteps, //probably shouldn't change this
            thresholdMax = 230; //cap on light level for addWhiteCaps(), stops colors from going full white

        uint16_t
            hueRateOrig, //Will be set to the inital hue rate as passed into constructors
            *hueRate; //Is initially pointed to hueRateOrig, but can be pointed to an external variable if wanted

        void 
            setHue(uint8_t newHue),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0,
            prevHueTime = 0,
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
            totBlendLength = 240, //use a value of 240 taken from the original pacifica code, changing this is tricky
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
            *bgColor, //bgColor is a pointer, it is tied to the bgColor in the PacificaHuePalPS
            colorOut;
        
        void 
            init(uint16_t HueRate, uint16_t Rate),
            doOneLayer( palettePS *palette, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff),
            addWhitecaps();
};

#endif
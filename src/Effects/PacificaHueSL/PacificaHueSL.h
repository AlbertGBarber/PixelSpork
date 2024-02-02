#ifndef PacificaHueSL_h
#define PacificaHueSL_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
//The source of the palettes for the effect
#include "PacificaHuePal/PacificaHuePalPS.h"

/*

 "Pacifica" (segLine and hue version)
  Gentle, blue-green ocean waves.
  By Mark Kriegsman and Mary Corey March.
  For Dan.

An effect based on the pacific code here: https://github.com/FastLED/FastLED/blob/master/examples/PacificaSL/PacificaSL.ino
This is an alternate version of the original PacificaPS.h,
that I've adapted to work in 2D, while also allowing you to change the overall color of the waves. 
This produces uniform waves that shift across the whole segment set, 
which shift between a colors in a limited hue range. 

Recommend brightness > 80 and rate > 20ms
For low brightness values I recommend turning off dithering using
FastLED.setDither(0);

Like the original, the effect is a bit computationally heavy

Almost all of the code is directly copied from the original linked above
(with modifications by me to work with segment sets and hues)
If you have any questions about how it works, please direct them to Mark Kriegsman

Inputs Guide:

The main input for the effect is the `hue`, which sets where our base color is in the rainbow.
 The effect's palettes then form their colors around that base color. 
 While this produces a nice gentle wave effect like the original effect, 
 but tinted in a specific color, it does mean that you cannot have colors from opposite ends of the spectrum 
 together (like green and purple for example). 
 If you want that, you may have luck with the LavaPS effect or one of the noise effects. 

Some hue ranges are:
    Red/Orange: 240 to ~10 
    Orange/Yellow: 10 to ~40
    Yellow/Green: 40 to ~90
    Green/Blue: 90 to ~150
    Blue/Purple: 150 to ~180
    Purple/Red: 180 to 240
        
By default the hue is set to 130, which closely matches the colors from the original effect.

You can also set the hue to cycle through colors over time at hueRate (ms) (see constructors below)
A hue rate of 0 will stop the hue cycle.
The hue is updated as part of the effect, so the hueRate should be slower than the effect's update rate.
Note that the hueRate is a pointer (like the overall effect Rate), so you can bind it to an external variable if wanted.
The rate passed into the constructors is hueRateOrig

In the original code, waves are shifted towards white where they meet using the function `addWhiteCaps()`. 
I've made this function optional for a few reasons:

    1. In the code, pixel colors are added to one another. For a 1D line or a rectangular matrix this is fine, 
       because each pixel is part of a single Segment Line, so the `addWhiteCaps()` is needed to form the wave peaks. 
       However for an uneven segment set with different length segments, a single pixel may exist in multiple 
       lines at once. In this case, the code is already forming the wave peaks by adding the colors of the 
       overlapping pixels multiple times, making `addWhiteCaps()` overkill. 

    2. `addWhiteCaps()` was specifically written for the original blue-green colors of the original Pacifica effect. 
        I have been unable to adapt it to work well with all hues. So in some cases, it's best to leave it off. 

In addition to the above, I've also allowed you to cap the white light level 
of the pixels using `thresholdMax` as part of `addWhiteCaps()`.

Example calls: 
    PacificaHueSL pacificaHue(mainSegments, 40);
    That's it, updates at 40ms
    (addWhiteCaps is default false, and the hue will default to 130 to match the original pacifica colors)

    PacificaHueSL pacificaHue(mainSegments, true, 130, 40);
    Sets addWhiteCaps to true and the hue to 130, updates at 40ms

    PacificaHueSL pacificaHue(mainSegments, true, 50, 500, 40);
    Sets addWhiteCaps to true, with the hue starting at 50 and incrementing every 500ms
    The effect updates at 40ms

Constructor inputs: 
    addWhiteCaps (optional, default false) -- If true, the addWhiteCaps() function will be called as part of the update cycle
    hue (optional, default 130, max 255) -- The hue used in the effect (see hue notes above). Use `setHue()` to change later.
    hueRate (optional, default 0) -- How quickly the hue shifts (ms).
                                     By default it's bound the effect's local variable, `hueRateOrig`. 
                                     (see Inputs Guide Notes).
    rate -- The update rate of the effect (ms)

Other Settings:
    thresholdMax (default 230) -- caps the white cap light level to prevent pixels from getting too white
                                  as part of the addWhiteCaps() function
                                Lower -> lower light cap.
    PacificaPalette -- The PacificaHuePal object. You shouldn't need to access this.

Functions:
    setHue(newHue) -- Changes the hue value
    update() -- updates the effect 

Reference vars:
    hue -- The current hue setting, use setHue() to change
    numSteps -- How many steps there are per color gradient. You shouldn't need to change this.
*/
class PacificaHueSL : public EffectBasePS {
    public:
        //Normal constructor (does default Pacifica)
        PacificaHueSL(SegmentSetPS &SegSet, uint16_t Rate);

        //constructor with addWhiteCaps setting
        PacificaHueSL(SegmentSetPS &SegSet, bool AddWhiteCaps, uint8_t Hue, uint16_t Rate);

        //constructor with addWhiteCaps and hue rate settings
        //The hue will be the initial hue of the palette
        PacificaHueSL(SegmentSetPS &SegSet, bool AddWhiteCaps, uint8_t Hue, uint16_t HueRate, uint16_t Rate);

        ~PacificaHueSL();

        PacificaHuePalPS
            *PacificaPalette = nullptr;

        bool
            addWhiteCaps = false;

        uint8_t
            hue,                 //for reference, call setHue() to change the hue (defaulted to 130 in PacificaHuePalPS.h)
            numSteps,            //probably shouldn't change this
            thresholdMax = 230;  //cap on light level for addWhiteCaps(), stops colors from going full white

        uint16_t
            hueRateOrig,         //Will be set to the inital hue rate as passed into constructors
            *hueRate = nullptr;  //Is initially pointed to hueRateOrig, but can be pointed to an external variable if wanted

        void
            setHue(uint8_t newHue),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0,
            prevHueTime = 0,
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
            numLines,
            numSegs,
            totBlendLength = 240,  //use a value of 240 taken from the original pacifica code, changing this is tricky
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
            *bgColor = nullptr,  //bgColor is a pointer, it is tied to the bgColor in the PacificaHuePalPS
            colorOut;

        void
            init(uint16_t HueRate, SegmentSetPS &SegSet, uint16_t Rate),
            doOneLayer(palettePS *palette, uint16_t ciStart, uint16_t waveScale, uint8_t bri, uint16_t iOff),
            addWhitecaps();
};

#endif
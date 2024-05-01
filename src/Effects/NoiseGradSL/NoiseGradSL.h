#ifndef NoiseGradSL_h
#define NoiseGradSL_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

/*
An effect where palette colors are drawn as a gradient across segment lines
The gradient is shifted based on the output of the FastLED Perlin noise function,
while the brightness is randomly set in blobs across the segment lines.

The length of the color gradient can be set to shift over time in quick bursts
This lets the effect stay calm for a while, and then quickly shift to a new look

You can change the variables freely, although this may result in jumps in the effect.

The noise colors will be blended towards a background color.
Changing the background color will "tint" the overall output, 
because all the colors are blended towards the background color as part of the effect. 
Supports color modes for the background color.

Inputs guide:
    Overall the effect has two components:
        1) A gradient of palette colors that shifts along the segment set in random jumps
        2) Spots of brightness that grow and change with time

    Both these factors have their own independent settings, so you can tune the effect to your liking. 
    
    Blend settings: 
        1) blendStepsBase: The minimum number of gradient steps between palette colors. 
                           To make the effect more interesting, the number of gradient steps (`blendSteps`)
                           can be set to shift over time, which changes how fast palette colors blend. 
                           The total number of `blendSteps` is calculated as 
                           `blendSteps = blendStepBase + random(blendStepsRange)`, 
                           so `blendStepBase` sets the smallest number of steps the gradients can have. 
        2) blendStepsRange: The maximum amount added to blendStepBase. Sets the upper limit for blendSteps
                            If you set blendStepsRange to 0 then there will be no shifting and blendSteps 
                            will stay at it's current value.
        3) blendRate: How often (in ms) the blendSteps are shifted. Constantly shifting the blendSteps looks choppy
                      so instead we only shift it every blendRate time, but do so quickly. So the effect has moments of calm
                      before jumping to a new look.
        4) phaseScale: The gradient is constantly offset by a noise value. PhaseScale sets how quickly the noise changes, 
                       ie, how wobbly the gradient is. Lower values => faster changing, min value of 1.
                       Recommend between 5 and 20
        5) freqScale:  How quickly the gradient moves across the segments. Lower value => faster. Min value of 1.
                       Recommend between 5 and 20
        
        I recommend keeping the total possible steps to be less than 20. 
        At higher values the shifting becomes quite slow. This may depend on your segments though.
        
        **Warning**, do **NOT** set the `blendSteps` directly if your `blendStepsRange` is greater than 0.
        Doing so may cause the `blendSteps` to increase/decrease infinitely. 

        blendRate is a pointer, so you can bind it externally.
        The blendRate you passed in is stored in blendRateOrig.

    Brightness Spots settings: 
        The brightness patches are generated with noise, similar to the Lava effect.
        1) briScale: Sets how "zoomed-in" the noise is, and in turn how large the brightness patches are.
                     Higher value => smaller patches. Recommend between 10 and 50.
        2) briFreq: Defaulted to 5. Sets how quickly the brightness changes. Smaller value => faster change.
                    Min value of 1. 
        3) doBrightness: Default true. Turns the brightness spots on/off. 

Example calls: 
    For this example, I'm showing you how to set the blendStepsRange so that one complete palette gradient 
    fits into the segment set. This tends to look good because you always have all the colors showing, and they'll
    either spread out into one full wave, or make multiple smaller waves as the blendSteps shifts.
    For this, you want your "max blend steps" * "number of palette colors" to equal the number of segment lines.
    uint8_t numColors = 3;
    uint8_t minSteps = 3;
    uint8_t stepsRange = ( mainSegments.numLines / numColors ) - minSteps;
    In my instance the mainSegments have a maximum length of 24, so stepsRange is 5.

    NoiseGradSL noiseGrad(mainSegments, numColors, 0, minSteps, stepsRange, 10, 20, 30, 5000, 80);
    Will produce an effect using a palette of 3 random colors
    The background is blank.
    There are a minimum of 3 blendSteps and range of 5 for a maximum blendSteps of 8 (as calculated above)
    The phaseScale is 10, the freqScale is 20, and the briScale is 30.
    The blendSteps will be shifted every 5000ms (5 sec).
    The effect updates at 80ms.
    
    NoiseGradSL noiseGrad(mainSegments, cybPnkPal_PS, 0, 8, 16, 5, 20, 10, 3000, 80);
    NoiseGradSL.bgColorMode = 6; //put in Arduino Setup()
    Will produce an effect using colors from cybPnkPal_PS
    The background is blank (but using bgColorMode of 6, ie a shifting rainbow)
    There are a minimum of 8 blendSteps and range of 16 for a maximum blendSteps of 24
    The phaseScale is 5, the freqScale is 20, and the briScale is 10.
    The blendSteps will be shifted every 3000ms (3 sec).
    The effect updates at 80ms.
   
Constructor inputs: 
    palette (optional, see constructors) -- A custom palette passed to the effect, the default is the 
                                           lava colors palette encoded below
    numColors (optional, see constructors) -- How many colors will be in the randomly created palette
    bgColor --  The color of the background pixels.
    blendStepsBase -- The minimum number of gradient steps between palette colors. (See Inputs Guide above)
    blendStepsRange -- The maximum amount added to blendStepBase for shifting. (See Inputs Guide above)
    phaseScale -- Sets how must the gradient wobbles. Must be greater than 0. (See Inputs Guide above)
    freqScale -- Sets how long the waves are. Must be greater than 0. (See Inputs Guide above)
    briScale -- Sets how "zoomed-in" the brightness noise is. (See Inputs Guide above)
    blendRate -- How often we shift to new blendSteps values (ms). blendRate is a pointer, so you can bind it to an external var.
                 The blendRate you pass to the constructor is stored in blendRateOrig, which is set to the pointer's target by default.
    rate -- The update rate (ms)

Other Settings:
    bgColorMode (default 0) -- sets the color mode for the spacing pixels (see segDrawUtils::setPixelColor)
    briFreq (default 5) -- Sets how quickly the brightness changes. Smaller value => faster change. Min value of 1.
    blendSteps -- How many steps are taken to blend between palette colors. 
                  Set to blendStepBase during effect creation.
                  Changes over time if `blendStepsRange` is greater than 0.
    doBrightness (default true) -- Turns the brightness spots on/off. 
    
Functions:
    update() -- updates the effect 

*/
class NoiseGradSL : public EffectBasePS {
    public:
        //Constructor with palette
        NoiseGradSL(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, uint16_t BlendStepsBase, uint16_t BlendStepsRange,
                    uint8_t PhaseScale, uint8_t FreqScale, uint8_t BriScale, uint16_t BlendRate, uint16_t Rate);

        //Constructor with randomly generated palette
        NoiseGradSL(SegmentSetPS &SegSet, uint8_t numColors, CRGB BgColor, uint16_t BlendStepsBase, uint16_t BlendStepsRange,
                    uint8_t PhaseScale, uint8_t FreqScale, uint8_t BriScale, uint16_t BlendRate, uint16_t Rate);

        ~NoiseGradSL();

        uint8_t
            bgColorMode = 0,
            phaseScale,
            freqScale,
            briScale,
            briFreq = 5;

        uint16_t
            blendStepsBase,
            blendStepsRange,
            blendSteps = 10; //will be set to blendStepsBase during effect creation

        uint16_t
            blendRateOrig,
            *blendRate = nullptr;

        bool
            doBrightness = true;

        CRGB
            bgColorOrig,
            *bgColor = nullptr;  //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)

        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0};  //Must init structs w/ pointers set to null for safety

        void
            update(void);

    private:
        unsigned long
            currentTime,
            blendShiftTime,
            prevTime = 0,
            prevBlendShiftTime = 0;

        int8_t
            blendStepsStep = 0;

        uint8_t
            index,
            noisePhase,
            phaseOffset = 0,
            bri;

        uint16_t
            blendStepsTarget,
            colorIndex,
            numLines,
            colorOffset = 0,
            colorOffsetTot,
            pixelNum,
            numSegs,
            totBlendLength;

        CRGB
            colorTarget,
            colorOut;

        void
            shiftBlendSteps(),
            init(CRGB BgColor, uint16_t BlendRate, SegmentSetPS &SegSet, uint16_t Rate);
};

#endif
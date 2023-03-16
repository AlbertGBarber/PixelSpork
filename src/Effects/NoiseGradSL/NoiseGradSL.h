#ifndef NoiseGradSL_h
#define NoiseGradSL_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

/*
An effect where palette colors are drawn as a gradient across segment lines
The graident is shifted based on the output of a noise function,
while the brightness is randomly set in blobs across the segment lines.

The length of the color gradient can be set to shift over time in quick bursts
This lets the effect stay calm for a while, and then quickly shift to a new look

This effect is NOT compatible with color modes, but the background color is.

You can change the variables freely, although this may result in jumps in the effect.

Inputs guide:
    Overall the effect has two components:
        1) A graident of palette colors that shifts along the segment set in random jumps
        2) Spots of brightness that grow and change with time
    
    Blend settings: 
        1) blendStepsBase: The minimum number of gradient steps between palette colors. The actual number of steps is
                           stored in blendSteps, which is shifted over time (unless blendStepsRange is 0)
        2) blendStepsRange: The maxium amount added to blendStepBase. Sets the upper limit for blendSteps
                            ie blendSteps = blendStepBase + random(blendStepsRange);
                            If you set blendStepsRange to 0 then there will be no shifting and blendSteps will stay at it's current value.
        3) blendRate: How often (in ms) the blendSteps are shifted. Constantly shifting the blendSteps looks choppy
                      so instead we only shift it every blendRate time, but do so quickly. So the effect has moments of calm
                      before jumping to a new look.
        4) PhaseScale: The gradient is constantly offset by a noise value. PhaseScale sets quickly the noise changes, 
                       ie, how wobbly the gradient is. Lower values => faster changing, min value of 1.
                       Recommend between 5 and 20
        5) FreqScale:  How quickly the gradient moves across the segments. Lower value => faster. Min value of 1.
                       Recommend between 5 and 20
        
        I recommend keeping you blendRange less than 20. Otherwise you might not notice the shifting very much.
        May depend on your segments though.
        
        !!Do NOT set the blendSteps directly after turning on shiftBlendSteps() (Having blendStepsRange > 0)
        if you do, be sure to set blendSteps = blendStepsTarget

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
    uint8_t numColors = 3;
    uint8_t minBase = 3;
    uint8_t maxBase = ringSegments.numLines / numColors - minBase;
    NoiseGradSL(ringSegments, numColors, 0, minBase, maxBase, 10, 20, 30, 5000, 80);
    For this example, I'm showing you how to set the blendStepsRange so that you have one complete palette gradient 
    fit into the segment set. This tends to look good because you always have all the colors showing, and they'll
    either spread out into one full wave, or make multiple smaller waves as the blendSteps shifts.
    In my instance the ringSegments have a maximum length of 24, so maxBase is 5.
    Will produce an effect using a palette of 3 random colors
    The background is blank.
    There are a minimum of 3 blendSteps and range of 5 for a maximum blendSteps of 8
    The phaseScale is 10, the freqScale is 20, and the briScale is 30.
    The blendSteps will be shifted every 5000ms (5 sec).
    The effect updates at 80ms.
    
    NoiseGradSL(ringSegments, palette1, 0, 8, 16, 5, 20, 10, 3000, 80);
    NoiseGradSL.bgColorMode = 6;
    Will produce an effect using colors from palette1
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
    blendStepsRange -- The maxium amount added to blendStepBase for shifting. (See Inputs Guide above)
    phaseScale -- Sets how must the gradient wobbles. Must be greater than 0. (See Inputs Guide above)
    freqScale -- Sets how long the waves are. Must be greater than 0. (See Inputs Guide above)
    briScale -- Sets how "zoomed-in" the brightness noise is. (See Inputs Guide above)
    blendRate -- How often we shift to new blendSteps values (ms). blendRate is a pointer, so you can bind it to an external var.
                 The blendRate you pass to the constructor is stored in blendRateOrig, which is set to the pointer's target by default.
    rate -- The update rate (ms)

Other Settings:
    bgColorMode (default 0) -- sets the color mode for the spacing pixels (see segDrawUtils::setPixelColor)
    briFreq (default 5) -- Sets how quickly the brightness changes. Smaller value => faster change. Min value of 1.
    blendSteps (defaulted to 10, but set in init() ) -- How many steps are taken to blend between palette colors
                                                        Automatically set when shifting.
    doBrightness (default true) -- Turns the brightness spots on/off. 

Functions:
    update() -- updates the effect 
*/
class NoiseGradSL : public EffectBasePS {
    public:
        //Constructor with palette
        NoiseGradSL(SegmentSet &SegmentSet, palettePS &Palette, CRGB BgColor, uint16_t BlendStepsBase, uint16_t BlendStepsRange,
                    uint8_t PhaseScale, uint8_t FreqScale, uint8_t BriScale, uint16_t BlendRate, uint16_t Rate);  

        //Constructor with randomly generated palette
        NoiseGradSL(SegmentSet &SegmentSet, uint8_t numColors, CRGB BgColor, uint16_t BlendStepsBase, uint16_t BlendStepsRange,
                    uint8_t PhaseScale, uint8_t FreqScale, uint8_t BriScale, uint16_t BlendRate, uint16_t Rate);
        
        ~NoiseGradSL();

        SegmentSet 
            &segmentSet; 
        
        uint8_t 
            bgColorMode = 0,
            phaseScale, 
            freqScale, 
            briScale,
            briFreq = 5; 

        uint16_t
            blendStepsBase,
            blendStepsRange,
            blendSteps = 10;
        
        uint16_t 
            blendRateOrig,
            *blendRate = nullptr;
        
        bool
            doBrightness = true;

        CRGB 
            bgColorOrig,
            *bgColor = nullptr; //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)
        
        palettePS 
            paletteTemp,
            *palette = nullptr;

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
            init(CRGB BgColor, uint16_t BlendRate, uint16_t Rate);
};

#endif
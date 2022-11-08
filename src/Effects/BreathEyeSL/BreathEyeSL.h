#ifndef BreathEyeSL_h
#define BreathEyeSL_h

//TODO -- Combine this effect with twinkle2 for multiple eyes, ditch the breath routine and just use the twinkle fading

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
//#include "MathUtils/mathUtilsPS.h"
/*
Similar to the classic breathing/heartbeat effect where segment set slowly fades between a main color and a background color.
But instead of the whole segment set being one color, a gradient is used from a central point.
This creates something akin to a robotic glowing eye that fades in and out.
Other than that, the effect is largly identical to BreathPS.
With the addition of being able to set the eye's center position and how large it is.
The effect is set to use multiple colors based on a given pattern and palette.
The effect will cycle through the colors with each "breath"
There are also a few random color modes and a rainbow mode (more on those below)

Note that the effect uses segment lines, so the eye will be 2d for a set with multiple segments.

The eye is made of a center point and two "arms" sprouting from each side.
The two arms fade to the background as towards their ends, while the center eye is fully bright (up to the current breath value)
The you can set the center point with eyePos, the center point size with eyeCenterSize (the actual center size will be double the value),
and the arm length with eyeHalfSize. The eyePos can be set to be choosen randomly after every fade, making the eye jump around.
If it's not choosen randomly, the eyePos will be defaulted to the center point of the longest segment.
The centerEyeSize will be defaulted to 1/8 the eyeHalfSize (see init()), which seemed to look good in my tests.
Note that the total size of the eye is eyeHalfSize * 2 
(the arms always start from the eye center pos, so they will be overlapped if the eyeCenterSize is > 0)

You can also have the eyeHalfSize be set randomly for each eye cycle by setting randEyeSize true
and setting a maximum eyeHalfSize by setting eyeHalfSizeMax (which is defaulted to eyeHalfSize on init())
The eyes will have a random size from eyeHalfSize to eyeHalfSizeMax.

You can either have the eye wrap or end if it runs off either end of the segment set. 

The background color is a pointer so you can bind it to an external variable.

The speed of the breathing is set by breathFreq.
I've set this to be a pointer like the effect update rate.
So you can bind it externally. 
By default the breathFreq will be bound to breathFreqOrig, 
so change that if you want to change the speed directly.

I recommend a breathFreq between 5 and 20, otherwise it gets quite slow.

Your update rate should not be too much more than your breathing freq, or the effect will look choppy.

You can also set how far the eye fades by setting maxBreath (default 255) to mimic more of a pulse kind of look
by preventing the eye from fading all the way to the background.
Note that this works best when using a single color with a fixed eye position, 
otherwise the effect will look "jumpy" since colors will change without fully fading.
If you use a random eye position, be sure to set fillBG to true to clear out the previous eye fade.

Note that you can change variables freely while the effect is running.

Does not work with color modes from segDrawUtils::setPixelColor().

randModes:
    0: Colors will be choosen in order from the pattern (not random)
    1: Colors will be choosen completely at random
    2: Colors will be choosen randomly from the palette (not allowing repeats)
    3: Colors will be choosen randomly from the palette (allowing repeats)
    4: Colors will be from the rainbow (the hue is offset by rainbowRate each time a color is choosen)

Example calls: 
    uint8_t pattern_arr = {0, 1, 4};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr)};
    BreathPS(mainSegments, &pattern, &palette3, 0, 10, true, true, 10, 50);
    Does a breathing cycle using the colors from palette3, following the pattern above
    The background is blank
    The eyeHalfSize is 10, the eye wraps, and the eye position will be set randomly for each cycle.
    The breathFreq is 10, the effect updates at 50ms

    BreathPS(mainSegments, &palette3, 0, 8, true, false, 5, 50);
    Does a breathing cycle using the colors from palette3 in order
    The background is blank
    The eyeHalfSize is 8, the eye wraps,
    The eye position will be fixed (defaulting to the center point of the longst segment),
    The breathFreq is 5, the effect updates at 50ms

    BreathPS(mainSegments, CRGB::Red, CRGB::Blue, 200, 10, false, true, 5, 50);
    Does a breathing cycle in red only
    The background is blue
    The maximum fade amount is capped at 200
    The eyeHalfSize is 10, the eye does not wrap, and the eye position will be set randomly for each cycle.
    The breathFreq is 5, the effect updates at 50ms

    BreathPS(mainSegments, 0, 25, 5, true, true, 10, 50);
    Does a breathing cycle using a rainbow
    The background is blank
    The rainbow hue advances 25 steps with each breath
    (the rainbowRate is how the rainbow's color changes with each breath, out of 255)
    The eyeHalfSize is 5, the eye wraps, and the eye position will be set randomly for each cycle.
    The breathFreq is 10, the effect updates at 50ms

Constructor Inputs
    pattern(optional, see constructors) -- A pattern is struct made from a 1-d array of palette indexes ie {0, 1, 3, 6, 7} 
                                          and the length of the array 
                                          (see patternPS.h)                                     
    palette(optional, see constructors) -- The repository of colors used in the pattern, or can be used as the pattern itself
    color(optional, see constructors) -- Will do the breathing in a single color. 
                                         Note that the color will be placed in a palette called paletteTemp.
                                         If you pass in 0 as the color then the effect will switch to randMode 1 (full random)
    maxBreath (optional, default 255) -- How far the breath color will fade towards the background
                                         Only used as a constructor input when creating a single color breath
                                         Using lower than 255 with multiple colors causes the colors to jump with each breath.
                                         !!Must be greater than minBreath (default 60, see other settings below)
    rainbowRate(optional, default 20) -- How the rainbow's color changes with each breath, out of 255
                                         Only used for the rainbow randColor mode (4)
    bgColor -- The color that the breaths will fade to (usually blank).
    eyeHalfSize -- How long each arm of the eye is (see effect description above)
    wrap -- If the eye wraps or is cut off at either end of the segment set
    randEyePos -- If the eye position is choosen randomly each cycle (if not, it will default to the center point of the longest segment)
    breathFreq -- The speed of the breathing, between 5 and 20 works well.
    Rate -- update rate (ms)

Functions:
    setPaletteAsPattern() -- Sets the effect pattern to match the current palette
    reset() -- restarts the effect
               Note that the effect may start mid breath after reseting.
    update() -- updates the effect

Other Settings:
    eyePos -- The center position of the eye, defaulted to the center of the longest segment
    eyeHalfSizeMax (default to eyeHalfSize) -- The upper limit of the eyeHalfSize, only used if randEyeSize is set to true
    eyeCenterSize (default 1/8 of the eyeHalfSize) -- The size of the center eye portion (see description bove)
    dimPow (default 80, min -127, max 127) -- Adjusts the rate of dimming for the trails (see dimPow in particlesPS.h)
    randEyeSize -- If true then the eyeHalfSize will be randomized between eyeHalfSize and eyeHalfSizeMax for each cycle
    fillBG -- sets if the background is to be filled before after each fade, 
              only needed if your maxBreath isn't 255 and you have multiple colors
    randMode (default 0, unless set by a constructor) -- see mode notes above
    minBreath (default 60, min 0) -- The minimum breath fade amount. Should be less than maxBreath (see construtors above)
                                     60 was taken from the original code by ldirko.
    breathFreqOrig -- The default target of the effect's breathFreq pointer variable.
                      Will be set to the passed in breathFreq from the constructor
                      (see breathFreq notes above)
    breathEndOffset (default 5) -- The brightness threshold difference from maxBreath for changing breath colors,
                                   shouldn't need to change this, see comments in update() function
    rainbowSat (default 255) -- The saturation of the rainbow mode 
    rainbowVal (default 255) -- The value of the rainbow mode (as part of setting the rainbow color)
*/
class BreathEyeSL : public EffectBasePS {
    public:
        //Consturctor for using a pattern and palette
        BreathEyeSL(SegmentSet &SegmentSet, patternPS *Pattern, palettePS *Palette, CRGB BgColor, uint16_t EyeHalfSize, bool Wrap, bool RandEyePos, uint8_t BreathFreq, uint16_t Rate);

        //Constructor for using palette as pattern
        BreathEyeSL(SegmentSet &SegmentSet, palettePS *Palette, CRGB BgColor, uint16_t EyeHalfSize, bool Wrap, bool RandEyePos, uint8_t BreathFreq, uint16_t Rate);

        //Constructor for a single color breath (pass in 0 as the color to trigger randMode 2, fully random)
        BreathEyeSL(SegmentSet &SegmentSet, CRGB color, CRGB BgColor, uint8_t MaxBreath, uint16_t EyeHalfSize, bool Wrap, bool RandEyePos, uint8_t BreathFreq, uint16_t Rate); 

        //Constructor for rainbow mode
        BreathEyeSL(SegmentSet &SegmentSet, CRGB BgColor, uint8_t RainbowRate, uint16_t EyeHalfSize, bool Wrap, bool RandEyePos, uint8_t BreathFreq, uint16_t Rate);

        ~BreathEyeSL();

        SegmentSet 
            &segmentSet; 
        
        int8_t 
            dimPow = 80;

        uint8_t
            breathFreqOrig,
            *breathFreq, //pointer to the breath frequency (dby default it's bound to the the Rate)
            minBreath = 60, //The minimum breath fade amount (0 is min, should be less than maxBreath)
            maxBreath = 255, //How far the breath color will fade towards the background (255 is max)
            breathEndOffset = 5, //The offset from maxBreath after which a new color is choosen (see comments in update() code)
            rainbowSat = 255,
            rainbowVal = 255,
            rainbowRate = 20,
            randMode = 0; //see description above
        
        uint16_t
            eyePos,
            eyeHalfSize,
            eyeHalfSizeMax,
            eyeCenterSize = 0;
        
        bool
            fillBG = false, //sets if the background is to be filled before after each fade, 
                            //only needed if your maxBreath isn't 255 and you have multiple colors
            wrap,
            randEyePos,
            randEyeSize = false;
        
        CRGB 
            bgColorOrig,
            *bgColor; //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)

        palettePS
            *palette,
            paletteTemp;
        
        patternPS
            *pattern,
            patternTemp;
        
        void 
            setPaletteAsPattern(),
            reset(void),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        CRGB 
            colorOut,
            dimColor,
            getEyeColor(uint16_t lineNum),
            breathColor; //the color we are fading to
        
        uint8_t
            hue = 0, //for rainbow mode
            ratio,
            bWave,
            breathEndVal,
            breath;
        
        uint16_t
            numLines,
            eyeHalf, //Stores eyeHalfSize or a randomized eyeHalfSize
            lineNumFor,
            lineNumRev,
            palIndex = 0,
            patternIndex = 0;

        bool
            lockColor = false;
        
        void
            getNextColor(),
            init(CRGB BgColor, uint8_t BreathFreq, uint16_t Rate);
};

#endif
#ifndef PoliceStrobeSLPS_h
#define PoliceStrobeSLPS_h

//TODO: add color pattern to strobe?

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

/*
An effect to strobe a strip to mimic police lights, with some additional options
A strobe is a rapid blinking of light.

The base constuctor takes two colors, but you can optionally pass in a palette instead
The are a few options for randomly choosing the strobe colors from the palette

Strobe Modes:
    0: Pulse half the segmentSet in each color (alternating halves and colors), then pulse each color on the whole segmentSet
    1: Pulse half the segmentSet in each color (alternating halves and colors)
    2: Pulse the whole segmentSet in each color (alternating colors)

The strobe of each color will continue for a set number of on/off cycles (ie making the strobe)

If using a palette, all the colors will be cycled through before reseting 
For mode 0, this means all the colors will be strobed in halves, then strobed on the full strip

The effect is adapted to work on segment lines for 2D use, but you can keep it 1D by
passing in a segmentSet with only one segment containing the whole strip.
ie depending on the mode, halve the segment lines will be pulsed at a time.

Also see StrobeSeg for a segment based strobe effect. Lets you create more complex strobe patterns.

Pausing:
After a strobe cycle is finished there is a user configurable pause time between cycles
(or after every set of pulses of pauseEvery is set)
after which, the strobe will restart
this can be set to 0

Backgrounds:
By default the background is filled in after the end of every pulse set and duing a pause
Both these can be disabled with the fillBG and fillBGOnPause flags respectively
This causes the last pulse color to persist after the pulse set is done

This effect is fully compatible with color modes, and the bgColor is a pointer, so you can bind it
to an external color variable
(see segDrawUtils::setPixelColor)

Example calls: 
    PoliceStrobeSLPS(mainSegments, CRGB::Red, CRGB::Blue, 0, 1, 0, 1, 200);
    Does a classic police set of lights
    Blinks each half of the strip for one pulse between red and blue, at a rate 0f 200ms
    The background is blank
    There is no pause between the cycles

    PoliceStrobeSLPS(mainSegments, &palette1, CRGB:Purple, 4, 500, 0, 50);
    A more dynamic strobe
    Will strobe all the colors in the palette1, with 4 pulses at 50ms each
    strobe mode 0 is used, so the strobe will alternate between strobing halfs of the strip
    and the whole strip
    There is a 500ms pause between cycles
    The background color is purple
 
Constructor Inputs:
    Palette(optional, see constructors) -- Used for making a strobe from a specific palette
    ColorOne(optional, see constructors) -- Used for making a dual color strobe
    ColorTwo(optional, see constructors) -- Used for making a dual color strobe
    BgColor -- The color between strobe pulses. It is a pointer, so it can be tied to an external variable
    NumPulses -- The number of pulses per strobe
    PauseTime -- The pause time between strobe cycles
    PulseMode -- The type of strobe that will be used (see modes above)
    Rate -- The update rate of the strobe (ms)

Functions:
    reset() -- Restarts the effect
    update() -- updates the effect

Other Settings:
    colorMode (default 0) -- sets the color mode for the random pixels (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the background (see segDrawUtils::setPixelColor)
    fillBG (default true) -- flag to fill the background after each set of pulses
    fillBGOnPause (default true) -- flag to fill the background during each pause
    randMode (default 0) -- Sets how colors are choosen from the palette
                        -- 0: Colors will be choosen from the palette in order (not random)
                        -- 1: Colors will be choosen completly at random (not using the palette)
                        -- 2: Colors will be choosen randomly from the palette, same color will not be choosen in a row

Flags:
    pause -- set true if a pause is active

Reference Vars (read only, don't change these unless you know what you're doing):
    colorNum -- the palette index of the color currently being pulsed

Notes:
    When using the dual color constructor option, the colors will be formed into a palette within the effect
    This is stored as paletteTemp and is then bound to the effect palette.
    So if you want to change the colors, you'll have to change them in paletteTemp.
    paletteUtilsPS::setColor(<your strobe effect name>->paletteTemp, <your new color>,  color index (0 or 1)); 
*/
class PoliceStrobeSLPS : public EffectBasePS {
    public:

        //Constructor for a traditional two color strobe
        PoliceStrobeSLPS(SegmentSet &SegmentSet, CRGB ColorOne, CRGB ColorTwo, CRGB BgColor, uint8_t NumPulses, uint16_t PauseTime, uint8_t PulseMode, uint16_t Rate);  

        //Constructor for using any palette for the colors
        PoliceStrobeSLPS(SegmentSet &SegmentSet, palettePS *Palette, CRGB BgColor, uint8_t NumPulses, uint16_t PauseTime, uint8_t PulseMode, uint16_t Rate);

        ~PoliceStrobeSLPS();

        SegmentSet 
            &segmentSet; 

        bool
            fillBG = true,
            fillBGOnPause = true,
            pause = false,
            pauseEvery = false;
        
        uint8_t 
            colorNum = 0, //palette index of the color currently being pulsed
            randMode = 0,
            pulseMode,
            numPulses,
            colorMode = 0,
            bgColorMode = 0;
            
        unsigned long
            pauseTime;
        
        CRGB 
            bgColorOrig,
           *bgColor; //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)
        
        palettePS
            paletteTemp,
            *palette; //the palette used for the strobe colors
        
        void 
            reset(),
            update(void);
    
    private:
        unsigned long
            currentTime,
            pauseStartTime,
            prevTime = 0;

        uint8_t
            pulseCount = 1,
            randGuess,
            prevGuess = 0, //the last color index guessed
            modeOut,
            numSegs;

        uint16_t
            lightStart,
            lightEnd,
            halfLength,
            numLines;
        
        bool 
            pulseBG = false,
            firstHalf = true,
            flashHalf;
        
        CRGB
            colorOut,
            colorTemp;
        
        void
            init(CRGB BgColor, uint16_t Rate),
            startPause(),
            pickColor();
};

#endif
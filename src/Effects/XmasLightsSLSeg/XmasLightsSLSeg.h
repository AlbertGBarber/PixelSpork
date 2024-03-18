#ifndef XmasLightsSLSeg_h
#define XmasLightsSLSeg_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
//#include "MathUtils/mathUtilsPS.h"

/*
The goal of this effect is to mimic classic twinkling Christmas lights. Ie, a pattern of repeating colors
where lights blink on and off occasionally. I implemented this by using random chance to toggle the state of 
each pixel during each update cycle, with different probabilities for a pixel turning off vs turning on.
By playing around with the two chance values, you can produce a wide variety of twinkling effects with 
different on/off ratios.

The effect is fully compatible with Color Modes (see segDrawUtils::getPixelColor()) for both the main and 
background ("off") pixels. The background color is a pointer so you can bind it externally.

The effect is adapted for 2D use, you can change how it draws using "segMode" (see Inputs Guide below).

Inputs Guide and Notes:

    For settings that most mimic typical Christmas lights, I recommend a chance ratio of 10 to 1 in favor of 
    pixels turning on. Ex, if your turning on chance is 100, your turning off chance should be 10. 
    (Note that by default the chances are out of 1000). 

    The exact chance values for a look you like will depend on the size of your strip and your update rate 
    (faster updates will increase the chance of a pixel toggling), but I recommend starting
    with the first example call below. 

    The effect uses a pattern and palette for its colors. You can set the length of the pattern colors
    via colorLength. Ex, for a colorLength of 4, the length of each pattern color will be 4 pixels.

    The background color is user controllable, and represents the "off" state of the pixels. 
    Black or white works best for a traditional Christmas effect. 
    
    You can also control the initial state of the pixels (all pattern or all background) using `bgPrefill`.
    This is important because pixel colors are only filled in as their states change, 
    so if the effect starts as all background, the color pattern will slowly be filled in and visa versa. 
    To accommodate dynamic Color Modes, you can also set all the pixels to be re-drawn each update using `fillBg`.

    segModes:
        0 -- The pattern and twinkles will be drawn using segment lines (each line will be a single color).
        1 -- The pattern and twinkles will be drawn using whole segments (each segment will be a single color).
        2 -- The pattern and twinkles will be drawn linearly along the segment set (1D).
    
    Note that the effect uses an array of uint8_t's of size (numLines/numSegs/numLEDs)/8 rounded up (based on segMode)
    to track the state of the pixels. It is allocated dynamically. 
    To prevent memory fragmentation, it is only re-sized if needed.
    If you plan on changing the segMode during runtime, you should start the array at it's max length 
    (probably segMode 2).

Example calls: 
    XmasLightsSLSeg xmasLights(mainSegments, cybPnkPal_PS, 0, 1, 100, 10, 0, 100);
    Does a set of xmas lights using the colors from the cybPnkPal_PS palette.
    The length of each palette color is 1.
    The background color is black.
    There's is a 10% (100/1000) chance of a pixel turning on
    and a 1% (10/1000) chance of a pixel turning off
    This should produce a more traditional Christmas light effect
    The segMode is 0, so the twinkles will be drawn on segment lines
    The effect updates at a rate of 100ms.

    uint8_t pattern_arr = {0, 2, 1};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};

    XmasLightsSLSeg xmasLights(mainSegments, pattern, cybPnkPal_PS, CRGB::Red, 4, 100, 100, 0, 100);
    Does a set of xmas lights using the colors from the cybPnkPal_PS palette, 
    laid out according to the pattern above. The length of each palette color is 4.
    The background color is red.
    There's is a 10% (100/1000) chance of a pixel turning on
    and a 10% (100/1000) chance of a pixel turing off, for a more "chaotic" effect.
    The segMode is 0, so the twinkles will be drawn on segment lines.
    The effect updates at a rate of 100ms.

Constructor Inputs:
    pattern(optional, see constructors) -- The color pattern used for the effect.
    palette(optional, see constructors) -- The palette of colors used for the effect.
    bgColor -- The "off" color of the pixels. It is a pointer, so it can be tied to an external variable
    colorLength (min of 1) -- The length of the pattern colors (ex 4, will produce bands of color 4 pixels in length).
    onChance -- The chance (out of 1000, see chanceBasis) that an off pixel will turn on.
    offChance -- The chance (out of 1000, see chanceBasis) that an on pixel will turn off.
    segMode -- Sets if the twinkles will be along segment lines, whole segments, or single LEDs (see segModes in intro).
               Can be changed later using setSegMode().
    rate -- The update rate for the effect.

Other Settings:
    colorMode (default 0) -- Sets the color mode for the "on" pixels (see segDrawUtils::setPixelColor).
    bgColorMode (default 0) -- Sets the color mode for the "off" pixels (see segDrawUtils::setPixelColor).
    chanceBasis (default 1000) -- The pixel on/off toggle probability threshold. 
                                  A pixel will change if "random(spawnBasis) <= (onChance or offChance)".
    fillBg (default false) -- If true, all the pixels will be re-drawn with each update, useful for dynamic Color Modes.
    bgPrefill (default false) -- If true, the background ("off" color) will be pre-filled for the first update(),
                                 If false, the color pattern will be pre-filled.

Functions:
    reset() -- Resets the effect. On the next update, triggers the pixels to be pre-filled it the color pattern 
                                  or background based on bgPrefill.
    setPaletteAsPattern() -- Sets the effect pattern to match the current palette.
    setSegMode(newSegMode) -- Sets the segMode for the effect. Will call setupTwinkleArray(), triggering a reset().
    setupTwinkleArray() -- Dynamically sizes and allocates an array for tracking the pixel's states. 
                           You only need to call this if you change the size of the segment set. 
                           Will trigger a `reset()`.
   update() -- Updates the effect.

Reference Vars:
    segMode -- Sets if the twinkles will be along segment lines, whole segments, or single LEDs (see segModes above).
               Set using using setSegMode().
*/
class XmasLightsSLSeg : public EffectBasePS {
    public:
        //Constructor with just a palette
        XmasLightsSLSeg(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, uint16_t ColorLength,  
                     uint16_t OnChance, uint16_t OffChance, uint8_t SegMode, uint16_t Rate);

        //Constructor with pattern and palette
        XmasLightsSLSeg(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, CRGB BgColor, uint16_t ColorLength,  
                     uint16_t OnChance, uint16_t OffChance, uint8_t SegMode, uint16_t Rate);

        ~XmasLightsSLSeg();

        uint8_t 
            colorMode = 0,
            bgColorMode = 0,
            segMode; //for reference, set using setSegMode()

        uint16_t 
            onChance,
            offChance,
            chanceBasis = 1000,
            colorLength; //min of 1
        
        bool
            fillBg = false,
            bgPrefill = false; //If true, the background will be pre-filled, if false, the pattern will be pre-filled

        CRGB
            bgColorOrig,
            *bgColor = nullptr; //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color) 
        
        palettePS
            *palette = nullptr;
        
        patternPS
            *pattern = nullptr,
            patternTemp = {nullptr, 0, 0};  //Must init structs w/ pointers set to null for safety;

        void
            reset(),
            setPaletteAsPattern(),
            setSegMode(uint8_t newSegMode),
            setupTwinkleArray(),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        uint8_t 
            *twinkleArr = nullptr,
            bitState,
            bitPos,
            modeOut,
            paletteIndex;
        
        uint16_t
            numTwinkles,
            chance,
            twinkArrLen,
            twinkArrLenMax = 0,
            twinkArrPos,
            patternIndex;
        
        bool
            firstUpdate = true,
            drawNow = false;

        CRGB 
            colorOut;

        void
            init(CRGB BgColor, SegmentSetPS &SegSet, uint16_t Rate),
            initialFill(),
            draw(uint16_t i);
};      

#endif
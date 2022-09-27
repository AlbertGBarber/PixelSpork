#ifndef SegWavesFast_h
#define SegWavesFast_h

//TODO -- add constructor for random options?
//     -- When picking random colors, use getShuffleIndex instead of just randoming

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

/* 
Similar to SegWaves, but runs faster with some restrictions
Like a set of waves moving in a direction
It is a segment based version of the streamerFast effect

The restrictions when compared to SegWaves are listed below:
1: There is no fading to shift the waves along the segments
2: Changing the palette on the fly will have a delayed effect on the colors
  The exisiting colors will shift off the segments before new ones shift on
  This prevents this effect from playing well with paletteBlend functions
3: The same restrictions as (2) apply to changing the pattern
4: Changing the direction of the effect may break it temporarily
5: This effect is not compatible with colorModes for either the waves or the background

Basically the effect works by setting the color of the first segment, 
then for each subsequent segment, it copies the color of the next segment in line
So any changes you make to colors will only show up at the first segment, and will be shifted along the segments

However, as a bonus, this effect supports random colored waves
where the colors for the waves are choosen at random as the enter the segments
This is controlled by the randMode setting

Otherwise, all the settings are/functions are the same as SegWavesFast

Patterns work the same as with other effects, they are a pattern of palette array indexes
ie a pattern of {0, 1, 2} would be the first three colors of a palette
to indicate a background segment (ie set to the BgColor) we use 255 in the pattern
This does mean if your palette has 255 colors, you'll lose the final color, but you shouldn't have palettes that large

For example, lets say we wanted to do the first two colors of our palette,
each as length 4 waves, with 3 background leds inbetween each
we would make a pattern as : {0, 0, 0, 0, 255, 255, 255, 1, 1, 1, 1, 255, 255, 255}

For simple patterns like the previous example, I have writtern a few constructors for the effect
that automate the pattern creation, so you don't have to do it yourself (see constructor notes below)

I have also included a shortcut for making a single wave (a wave of length 1, with only one wave active on the segment set a once)
For any of the constructors with a waveThickness input, just pass in 0, and a single wave pattern will be build automatically.

Note that while each entry in the pattern is a uint8_t,
if you have a lot of colors, with long waves, your patterns may be quite large
so watch your memory usage

Example calls: 
    uint8_t pattern_arr = {0, 255, 255, 255, 1, 1, 255, 255};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr)};
    SegWavesFast(mainSegments, &pattern, &palette3, 0, true, 120);
    Will do a set of waves using the first two colors in the palette
    The wave will begin with 1 segment of color 0, with three spaces after, followed by 2 pixels of color 1, followed by 2 spaces
    The bgColor is zero (off)
    The waves will move from the last segment to the first
    The waves will update at a 120ms rate

    uint8_t pattern_arr = {1, 2, 3};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr)};
    SegWavesFast(mainSegments, &pattern, &palette3, 3, 4, 0, false, 120);
    Will do a wave using the first three colors of the palette (taken from the pattern)
    Each wave will be length 3, followed by 4 spaces, bgColor is 0 (off)
    The waves will move from the first to last segment.
    The effect updates at a rate of 120ms

    SegWavesFast(mainSegments, &palette3, 3, 4, CRGB::Red, true, 120);
    Will do a wave using all the colors in palette3, each wave will be length 3, with 4 spaces inbetween
    The bgColor is red
    The waves will move from the last segment to the first
    The waves will update at a 120ms rate

    SegWavesFast(mainSegments, CRGB::Blue, 2, 2, CRGB::Red, 0, true, 140);
    Will do a blue waves with length 2 and 2 spaces inbetween
    The bgColor is red
    The waves will move from the last segment to the first
    The effect updates at a rate of 140ms
 
Constructor Inputs:
    pattern(optional, see constructors) -- The pattern used for the waves, made up of palette indexes 
    palette(optional, see constructors) -- The repository of colors used in the pattern
    Color(optional, see constructors) -- Used for making a single color wave
    WaveThickness (optional, see constructors, max 255) -- The number pixels a wave color is. Used for automated pattern creation.
    Spacing (optional, see constructors, max 255) -- The number of pixels between each wave color (will be set to bgColor).  Used for automated pattern creation.
    BgColor -- The color of the spacing pixels. It is a pointer, so it can be tied to an external variable
    direct --  The direction the wave move
               True will make the waves move from the last to first segment, false, the reverse.e 
    Rate -- The update rate (ms)

Functions:
    reset() -- Restarts the wave pattern
    setPatternAsPattern(*inputPattern, colorLength, spacing) -- Takes an input pattern and creates a wave pattern from it using the current palette
                                                                Ex: uint8_t pattern_arr = {1, 2, 3};
                                                                   patternPS pattern = {pattern_arr, SIZE(pattern_arr)};
                                                                   setPatternAsPattern(&pattern, 3, 4) 
                                                                   Will do a wave using the first three colors of the palette (taken from the pattern)
                                                                   Each wave will be length 3, followed by 4 spaces
    setPaletteAsPattern(uint8_t colorLength, uint8_t spacing) -- Like the previous function, but all of the current palette will be used for the pattern                                                       
    makeSingleWave() -- Creates a wave pattern so that there's only a single wave of thickness 1 active on the segment set at one time
    update() -- updates the effect

Other Settings:
    randMode (default 0) -- Sets the type of how colors are choosen:
                         -- 0: Colors will be choosen in order from the pattern (not random)
                         -- 1: Colors will be choosen completely at random
                         -- 2: Colors will be choosen randomly from the palette (allowing repeats)
                         -- 3: Colors will be choosen at random from the palette,
                               but the same color won't be repeated in a row

Flags:
    initFillDone -- Indicates if the strip has been pre-filled with the effect's color outputs 
                    This needs to happen before running the first update cycle
                    If false, preFill() will be called when first updating
                    Set true once the first update cycle has been finished

Notes:
    If the constructor made your pattern, it will be stored in patternTemp
    same goes for the palette 
*/
class SegWavesFast : public EffectBasePS {
    public:
        //constructor for using the passed in pattern and palette for the wave
        SegWavesFast(SegmentSet &SegmentSet, patternPS *Pattern, palettePS *Palette, CRGB BgColor, bool Direct, uint16_t Rate);

        //constructor for building the wave pattern from the passed in pattern and the palette, using the passed in colorLength and spacing
        SegWavesFast(SegmentSet &SegmentSet, patternPS *Pattern, palettePS *Palette, uint8_t WaveThickness, uint8_t Spacing, CRGB BgColor, bool Direct, uint16_t Rate);
        
        //constructor for building a wave using all the colors in the passed in palette, using the colorLength and spacing for each color
        SegWavesFast(SegmentSet &SegmentSet, palettePS *Palette, uint8_t WaveThickness, uint8_t Spacing, CRGB BgColor, bool Direct, uint16_t Rate);

        //constructor for doing a single colored wave, using colorLength and spacing
        SegWavesFast(SegmentSet &SegmentSet, CRGB Color, uint8_t WaveThickness, uint8_t Spacing, CRGB BgColor, bool Direct, uint16_t Rate);  
    
        ~SegWavesFast();

        uint8_t
            nextPattern,
            prevPattern,
            randMode = 0,
            cycleCount = 0;
            
        bool
            initFillDone = false,
            direct;

        CRGB 
            prevColor, //The color of the neweststreamer (excluding the background color)
            bgColorOrig,
           *bgColor; //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)

        SegmentSet 
            &segmentSet; 
        
        patternPS
            patternTemp,
            *pattern;
        
        palettePS
            paletteTemp,
            *palette;
        
        void 
            setPatternAsPattern(patternPS *inputPattern, uint8_t colorLength, uint8_t spacing),
            setPaletteAsPattern(uint8_t colorLength, uint8_t spacing),
            reset(),
            makeSingleWave(),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        int8_t
            loopStep;
        
        uint16_t
            numSegs,
            startLimit,
            coloredSeg,
            pixelNumber;
        
        int32_t
            endLimit;

        CRGB 
            nextColor,
            pickStreamerColor(uint8_t patternIndex);
        
        void
            initalFill(),
            getDirection(),
            init(CRGB BgColor, uint16_t Rate);
};  

#endif
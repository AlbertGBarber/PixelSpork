#ifndef FairyLightsSLSeg_h
#define FairyLightsSLSeg_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

/* 
Colors a number of segment lines (or segments) on at a time, turning them off depending on the mode choosen
Meant to be similar to classic twinkling fairy lights
The color of the lines (twinkles) be set to a single color, chosen randomly, of picked from a palette 
Just run an example and you'll see what I mean

tModes:
    Mode 0: Turns one twinkle on after another and then resets them all at once
    Mode 1: Turns on each twinkle one at a time, then off one at a time
    Mode 2: Each cycle, a new twinkle is turned on while an old is turned off (first on first off)

segMode:
The effect is adapted to work on segment lines, whole segments for 2D use, or on single pixels (1D)
This is contolled by the segMode var:
    0: Twinkles will be drawn on segment lines
    1: Twinkles will be drawn on whole segments
    2: Twinkles will be drawn on individual pixels

For segment sets with different segment lengths you may need to set fillBG to true
otherwise you might end up with artifacts where multiple lines cross depending on the tMode

This effect is fully compatible with color modes, and the bgColor is a pointer, so you can bind it
to an external color variable

Requires an uint16_t array and CRGB array of length NumTwinkles to work, make sure you have the memory for them!
If you turn fillBG on the off, make sure to also turn off reDrawAll


Example calls: 
    FairyLightsSLSeg(mainSegments, palette, 5, 0, 0, 1, 150);
    Will choose 5 segments (segMode 1) to cycle to/from colors choosen from the palette, using a blank background, 
    Each segment will be turned on one at a time, before reseting them all at once (mode 0), with 150ms between each cycle
    (segments are used over segment lines because segMode is true)

    FairyLightsSLSeg(mainSegments, CRGB::Red, 10, CRGB::Blue, 1, 0, 100);
    Will choose 10 lines (segMode 0) to set to red before reseting, using a blue background, 
    The lines will be turned on one at a time, and then off one at a time (mode 1), with 100ms between each cycle

    FairyLightsSLSeg(mainSegments, 20, 0, 2, 3, 80);
    Will choose 20 pixels (segMode 2) each cycle to set to random colors, using a blank backgound, 
    (note this sets randMode = 1)
    Each cycle, a new line will be turned on, while an old is turned off (mode 2), with 80ms in between each cycle

Constructor Inputs:
    Palette(optional, see constructors) -- the palette from which colors will be choosen randomly
    Color(optional, see constructors) -- the color that the randomly choosen twinkles will be set to
    NumTwinkles -- The amount of random twinkles choosen for twinkling
    BgColor -- The color of the background, this is what twinkles will fade to and from
    tMode -- The twinkling mode for the effect (see above for descriptions)
    segMode -- Sets if twinkles will be drawn on segment lines, whole segments or individual pixels
               (See segMode notes above)
    Rate -- The update rate (ms)

Functions:
    setSingleColor(Color) -- Sets the effect to use a single color for the twinkles, will restart the effect
    reset() -- Resets the startup variables, you probably don't need to ever call this
    setNumTwinkles(newNumTwinkles) -- sets an new number of twinkles to be choosen each cycle, will reset the current set of twinkles
    setSegMode(newSegMode) -- Sets if twinkles will be drawn on segment lines, whole segments or individual pixels
                              (See segMode notes above) (will reset the current set of twinkles)
    genPixelSet() -- creates a new group of twinkles for twinkling (you shouldn't need to call this)
    update() -- updates the effect

Other Settings:
    colorMode (default 0) -- sets the color mode for the random twinkles (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the background (see segDrawUtils::setPixelColor)
    randMode (default 0) -- sets how colors will be picked
                           0: Picks colors from the palette
                           1: Picks colors at random
    fillBG (default false) -- sets the background to be redrawn every cycle, useful for bgColorModes that are dynamic
    reDrawAll (default false) -- Will re-draw all the twinkles each cycle, is slower than default, but you need this if you want to layer this effect with another
                                (is set true if fillBG is true)

Reference Vars:
    turnOff -- Tracks if we're turning the twinkles on or off, depending on the mode 
*/
class FairyLightsSLSeg : public EffectBasePS {
    public:
        //palette based constructor
        FairyLightsSLSeg(SegmentSet &SegmentSet, palettePS *Palette, uint8_t NumTwinkles, CRGB BGColor, uint8_t Tmode, uint8_t SegMode, uint16_t Rate);
        
        //single color constructor
        FairyLightsSLSeg(SegmentSet &SegmentSet, CRGB Color, uint8_t NumTwinkles, CRGB BGColor, uint8_t Tmode, uint8_t SegMode, uint16_t Rate);
        
        //random colors constructor
        FairyLightsSLSeg(SegmentSet &SegmentSet, uint8_t NumTwinkles, CRGB BGColor, uint8_t Tmode, uint8_t SegMode, uint16_t Rate); 

        //destructor
        ~FairyLightsSLSeg();

        SegmentSet 
            &segmentSet; 
        
        uint8_t
            segMode, //(for reference, use setSegMode() to set)
            randMode = 0,
            cycleNum = 0,
            numTwinkles, //for reference only, set using setNumTwinkles()
            tMode,
            colorMode = 0,
            bgColorMode = 0;
        
        uint16_t
            *twinkleSet;
        
        CRGB
            bgColorOrig,
            *bgColor; //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)
        
        palettePS
            *palette;

        bool
            reDrawAll = false,
            fillBG = false,
            turnOff = false; //tracks if we're turning the twinkles on or off, depending on the mode
        
        void
            setSingleColor(CRGB Color),
            setNumTwinkles(uint8_t newNumTwinkles),
            setSegMode(uint8_t newSegMode),
            genPixelSet(),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        uint8_t
            paletteLength,
            cycleLimit,
            loopStart,
            loopEnd;
        
        uint16_t
            twinkleRange;
        
        palettePS
            paletteTemp;
        
        CRGB 
            color,
            *colorSet,
            pickColor();
        
        void
            modeZeroSet(),
            modeOneSet(),
            modeTwoSet(),
            init(CRGB BgColor, uint16_t Rate),
            drawTwinkle(uint8_t twinkleNum, CRGB tColor, uint8_t cMode);
};

#endif
#ifndef FairyLightsSLSeg_h
#define FairyLightsSLSeg_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

/* 
Fills in a set number of random segment lines, segments, or single pixels one at a time before turning them off in sequence.
There are multiple sequence modes for turning the pixels on/off. 
Overall, the effect is meant to be similar to some classic fairy light twinkle modes. 
The color of the twinkles can be set to a single color, chosen randomly, or picked from a palette.
Just run an example and you'll see what I mean.

This effect is fully compatible with color modes, and the bgColor is a pointer, so you can bind it
to an external color variable

tModes:
    tMode sets the on/off sequence for the pixels.
    0: Turns one twinkle on after another and then resets them all at once
    1: Turns on each twinkle one at a time, then off one at a time
    2: Each cycle, a new twinkle is turned on while an old is turned off (first on first off)

Note that you can have the effect prefill a set of pixels on the first update using the "prefill" setting,
causing the effect to act as if a full cycle had been completed and we're ready to turn off the pixels.
See Other Settings below.

randModes (default 0):
    randMode sets how each twinkle color will be set.
    0: Picks colors from the palette.
    1: Picks colors at random.

segMode:
    The effect is adapted to work on segment lines, whole segments for 2D use, or on single pixels (1D).
    This is controlled by "segMode":
        0: Twinkles will be drawn on segment lines
        1: Twinkles will be drawn on whole segments
        2: Twinkles will be drawn on individual pixels

`segmode` can be changed during runtime using `setSegMode( uint8_t newSegMode );`.

When drawing on segment lines (`segMode` 0), for segment sets with varying segment lengths, 
you may see "artifacts" where twinkles are not fully turned off. 
This happens because of overlapping segment lines. 
Thankfully you can fix this by setting `fillBg` to true, which forces all the twinkles to be re-drawn each update cycle.
Note that if you turn `fillBg` off, make sure to also turn off `reDrawAll` (see Other Settings below for more info).

Requires an uint16_t array and CRGB array of length `numTwinkles` to work, make sure you have the memory for them. 
These are allocated dynamically, so, to avoid memory fragmentation, when you create the effect, 
you should set `numTwinkles` to the maximum value you expect to use. 
See the Effects Advanced wiki page for more on memory management. 

Example calls: 
    FairyLightsSLSeg fairyLights(mainSegments, cybPnkPal_PS, 0, 20, 2, 2, 80);
    fairyLights.prefill = true; //place in Arduino setup() function
    Will choose 20 pixels (segMode 2) each cycle to set to colors 
    from the cybPnkPal_PS palette,
    with a black background
    Each cycle, a new pixel will be turned on, while an old is turned off (tMode 2), 
    with 80ms in between each cycle

    FairyLightsSLSeg fairyLights(mainSegments, CRGB::Red, CRGB::Blue, 10, 1, 0, 100);
    Will choose 10 lines (segMode 0) to set to red before resetting, using a blue background, 
    The lines will be turned on one at a time, and then off one at a time (tMode 1),
    with 100ms between each cycle

    FairyLightsSLSeg fairyLights(mainSegments, 5, 0, 0, 1, 150);
    Will choose 5 segments (segMode 1) to cycle to/from to random colors 
    (no input color/palette, so randMode = 1).
    The background is black. 
    Each segment will be turned on one at a time, before resetting them all at once (tMode 0), 
    with 150ms between each cycle
    (for this effect, make sure your segment set has multiple segments)

Constructor Inputs:
    palette(optional, see constructors) -- the palette from which colors will be chosen randomly
    color(optional, see constructors) -- For the single color constructor, sets the color that the twinkles will be. 
                                         Note that the effect will create a local palette, `paletteTemp` for the color. 
    numTwinkles -- The number of random twinkles chosen for twinkling. Can be later adjusted by calling `setNumTwinkles()`.
    bgColor -- The color of the background, this is what twinkles will fade to and from
    tMode -- The twinkling mode for the effect (see intro for notes)
    segMode -- Sets if twinkles will be drawn on segment lines, whole segments or individual pixels
               (See segMode notes above) Can be changed later using setSegMode().
    rate -- The update rate (ms)

Other Settings:
    colorMode (default 0) -- sets the color mode for the random twinkles (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the background (see segDrawUtils::setPixelColor)
    randMode (default 0) -- sets how colors will be picked (See notes in intro)
    fillBg (default false) -- sets the background to be redrawn every cycle, useful for bgColorModes that are dynamic
    reDrawAll (default false) -- If true, the effect will re-draw all the twinkles each cycle, 
                                 but does not re-draw the whole background.
                                 You need this if you want change the number of twinkles during runtime.
                                 It is automatically set true if `fillBg` is true.
    prefill (default false) -- If true, the effect pre-draws a full set of twinkles on the first update,
                               as if a full cycle had been completed and we're ready to turn off the pixels.
                               Won't work for tMode 0 since all the pixels will be immediately turned off.
                               (Which is fine, since that's what the mode does)

Functions:
    setSingleColor(Color) -- Sets the effect to use a single color for the twinkles, will restart the effect. 
                             Uses the local palette, paletteTemp for the color.
    setNumTwinkles(newNumTwinkles) -- Changes the number of twinkles, re-spawns them, and calls reset() to clear out any existing pixels.
    setSegMode(newSegMode) -- Sets if twinkles will be drawn on segment lines, whole segments or individual pixels
                              (See segMode notes above) (will reset() the current set of twinkles)
    reset() -- Restarts the effect by clearing our any existing pixels and spawning a new set. 
               (will also trigger prefill if set)
    update() -- updates the effect

Reference Vars:
    segMode -- (see constructor notes above) set using setSegMode()
    numTwinkles -- The amount of random twinkles chosen for twinkling, set using setNumTwinkles()
    cycleNum -- How many twinkles have been drawn (resets once numTwinkles updates have been done)

Flags:
    turnOff -- Tracks if we're turning the twinkles on or off, depending on the tmode. (true is turning off)
*/
class FairyLightsSLSeg : public EffectBasePS {
    public:
        //Palette based constructor
        FairyLightsSLSeg(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, uint16_t NumTwinkles,
                         uint8_t Tmode, uint8_t SegMode, uint16_t Rate);

        //Single color constructor
        FairyLightsSLSeg(SegmentSetPS &SegSet, CRGB Color, CRGB BgColor, uint16_t NumTwinkles,
                         uint8_t Tmode, uint8_t SegMode, uint16_t Rate);

        //Random colors constructor
        FairyLightsSLSeg(SegmentSetPS &SegSet, uint16_t NumTwinkles, CRGB BgColor, uint8_t Tmode,
                         uint8_t SegMode, uint16_t Rate);

        //destructor
        ~FairyLightsSLSeg();

        uint8_t
            segMode,  //for reference, use setSegMode() to set
            randMode = 0,
            tMode,
            colorMode = 0,
            bgColorMode = 0;

        uint16_t
            cycleNum = 0,  //for reference, how many twinkles have been drawn (resets once numTwinkles updates have been done)
            numTwinkles,   //for reference only, set using setNumTwinkles()
            *twinkleSet = nullptr;
        
        bool
            prefill = false,
            reDrawAll = false,
            fillBg = false,
            turnOff = false;  //tracks if we're turning the twinkles on or off, depending on the mode

        CRGB
            bgColorOrig,
            *bgColor = nullptr,  //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)
            *colorSet = nullptr;

        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0};  //Must init structs w/ pointers set to null for safety

        void
            setSingleColor(CRGB Color),
            setNumTwinkles(uint16_t newNumTwinkles),
            setSegMode(uint8_t newSegMode),
            reset(void),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0;

        uint8_t
            paletteLength;

        uint16_t
            cycleLimit,
            maxNumTwinkles = 0,  //used for tracking the memory size of the twinkle arrays
            loopStart,
            loopEnd,
            twinkleRange;
        
        bool
            firstUpdate = true;

        CRGB
            color,
            pickColor();

        void
            modeZeroSet(),
            modeOneSet(),
            modeTwoSet(),
            spawnTwinkles(),
            preFillTwinkles(),
            init(CRGB BgColor, SegmentSetPS &SegSet, uint16_t Rate),
            drawTwinkle(uint16_t twinkleNum, CRGB &tColor, uint8_t cMode);
};

#endif
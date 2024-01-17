#ifndef ColorWipeSeg_h
#define ColorWipeSeg_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

/*
An effect that colors one segment pixel in after another until all segments in a set are colored. 
This effect is similar to ColorWipeSLSeg.h, but instead of wiping whole lines or segments, it wipes
each pixel of each segment in order.

ie for a set with five segments, the first would be wiped one pixel at a time, then the second segment, etc.
Note that you really need a segment set with multiple segments for this effect to work well.

Wipes have a direction, either starting from the first or last segment pixel. This can be set to alternate with each wipe.

Likewise, you can set the wipe to start at either the first or last segment.
You can't set this to alternate, instead change the order of the segments in the set.

The effect uses a pattern and palette for colors and various styles to display them:
Styles:
    0: The colors alternate with each segment. Ie if you have 4 segments, each will be a different color from the pattern.
    1: The colors alternates with each segment pixel.
    2: The colors alternate with each segment line.

Note that the colors are set based on the number of segments and pixel's wiped, not by using the segment and pixel numbers directly.
This prevents wipes from being the same for different wipe directions.

The effect is compatible with colorModes. They will override the style setting.

Once all the wipes are finished, the "done" flag will be set, and the effect is is over until it is reset.
Or the effect can be set to loop, automatically resetting, see below for more info.

Looping:
    To keep wiping continuously, you can set the wipes to loop, which will automatically reset the effect repeatedly.
    The number of full wipes completed is stored in loopCount.

    There are a various of options for configuring how the wipe loops, allowing you to change the wipe direction, 
    wipe a background color, etc when looping. You can configure these options all at once using setUpLoop().
    Or you can set them individually.

    The loop options give you a lot of flexibility in creating different effects, I encourage you to play with them!
    If you can't setup exactly what you want, you can always create multiple ColorWipes, and invoke them manually, 
    swapping/resetting them out as they are "done".

The Loop Options: 
    looped -- Sets if the wipes loop or not. Looping wipes automatically restart every time a wipe is finished.
              The other variables are only relevant if the wipe is looping, because they modify subsequent loops.
    bgLoop -- If true, then the background color (default 0) will be used as the color wipe every other loop
              Ie, we wipe a color and then wipe off, looping
    shiftPatLoop -- If true, the pattern will be shifted by 1 with every loop, 
                    changing the colors of each segment / line with each wipe
                    (this is done with an offset, it does not change the existing pattern)
    patShiftDir -- Sets the direction the pattern shifts in, forwards (true) or backwards (false)
    altWipeDirLoop -- If true, then the wipe direction will be flipped every loop
                      (note that this modifies the original startingDirect value)
    bgAltLoop -- Only used if altWipeDirLoop is true. 
                 If true, the the wipe direction will only swap on colored wipes, not on background ones.
                 Ie colored wipe->background wipe->(wipe direction flips)->colored wipe->background wipe->(flip)->etc
                 If false, then the wipe direction will flip every loop
                 ie colored wipe->(wipe direction flips)->background wipe->(wipe direction flips)->colored wipe->etc
    altSegWipeDirLoop -- If true, then the segment set direction will be flipped every loop
                         This is different than flipping the wipe direction, since it makes the first wipe 
                         start at the opposite end of the segment set, rather than having the wipe just move in the opposite direction
                    
The bgColor is a pointer, so you can bind it to an external color variable.
By default it is bound to bgColorOrig, which is set to 0 (blank color).

You can adjust most of the effect variables on the fly, but changing the segWipeDir will only take effect once a wipe loops.

Example calls: 
    ColorWipeSeg colorWipeSeg(mainSegments, cybPnkPal_PS, 0, false, false, true, 60);
    Will do a color wipe along mainSegment's segments using colors from cybPnkPal_PS
    (A pattern will be generated to match the palette)
    The style is 0, each segment will alternate colors according to the pattern.
    Alternate is false, so the wipes will all wipe in the same direction.
    The wipe direction is false, while the segWipeDir is true
    so wipes will go from the first to last segment, starting at the end of each segment
    The effect updates at 60ms
    
    uint8_t pattern_arr = {0, 1, 2};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};
    ColorWipeSeg colorWipeSeg(mainSegments, cybPnkPal_PS, pattern, 1, true, true, true, 60);
    Will do a color wipe along mainSegment's segments using colors from cybPnkPal_PS according to pattern
    The style is 1, each segment pixel will alternate colors according to the pattern.
    Alternate is true, so the wipes will alternate wipe directions for each segment.
    Both the wipe direction and segWipeDir are true.
    So wipes will go from the first to last segment, with the first wipe starting at the end the first segment
    and then alternating starting points after that.
    The effect updates at 60ms

    ColorWipeSeg colorWipeSeg(mainSegments, CRGB(CRGB::Red), 2, false, false, false, 60);
    Will do a color wipe along mainSegment's segments using CRGB::Red as the only color
    The style is 2, each segment line will alternate colors according to the pattern.
    (although this doesn't matter since the color is just red)
    Alternate is false, so the wipes will all wipe in the same direction.
    Both the wipe direction and segWipeDir are false.
    So wipes will go from the last to first segment, starting at the end of each segment
    The effect updates at 60ms

Constructor Inputs:
    palette(optional, see constructors) -- The set of colors that color wipes will use
    pattern(optional, see constructors) -- A pattern is struct made from a 1-d array of palette indexes ie {0, 1, 3, 6, 7} 
                                          and the length of the array (see patternPS.h)   
    color (optional, see constructors) -- A single color for all the wipes.
                                          The color will be placed in the effect's local palette, `paletteTemp`. 
                                          The local pattern, `patternTemp`, will be set to match the palette (ie a single entry for the single color).
    style -- How the colors are displayed for the wipe(s) (see notes in intro above)
    alternate -- If true, wipes will alternate directions (ie if one wipe is going forward, the next will reverse)
    wipeDirect -- The direction of the first wipe, true is forward
    segWipeDir -- The order the segments are wiped in, true is first to last
    rate -- The update rate of the effect (ms)

Functions:
    reset() -- Resets the effect vars, restarting the wipes
    setPaletteAsPattern() -- Sets the effect pattern to match the current palette
    resetLoop() -- Resets the current loop, will switch to the next loop, only relevant when looping (you shouldn't need to call this)
    setUpLoop(looped, bgLoop, shiftPatLoop, patShiftDir, altWipeDirLoop, bgAltLoop, altSegWipeDirLoop) 
             -- A quick way of setting all the loop variables (see intro for notes on loops)
    update() -- updates the effect

Other Settings:
    colorMode (default 0) -- sets the color mode for the waves (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the spacing pixels (see segDrawUtils::setPixelColor)
    bgColor (default 0) -- See intro notes under looping
    
Looping Settings:
    looped (default false) -- Sets if the effect should loop once a wipe is finished (see intro looping notes)
    shiftPatLoop (default false) -- (see intro looping notes)
    patShiftDir (default true) -- (see intro looping notes)
    altSegWipeDirLoop (default false) -- (see intro looping notes)
    altWipeDirLoop (default false) -- (see intro looping notes)
    bgLoop (default false) -- (see intro looping notes)
    bgAltLoop (default false) -- (see intro looping notes)

Reference Vars:
    loopCount -- How many full wipe cycles we've done, useful for tracking wipes when looping
    pixelCount -- How many pixels have been wiped so far (resets when looped)
    segWipeCount -- How many segments have been wiped so far (resets when looped)
    patOffset -- The current offset of the pattern. Used to shift the pattern colors. Only used when looping.

Flags:
    done -- Set true when the wipe cycle is finished. Not set if we're looping.
*/
class ColorWipeSeg : public EffectBasePS {
    public:
        //Constructor using pattern and palette
        ColorWipeSeg(SegmentSetPS &SegSet, palettePS &Palette, patternPS &Pattern, uint8_t Style,
                     bool Alternate, bool WipeDirect, bool SegWipeDir, uint16_t Rate);

        //Constructor using palette alone
        ColorWipeSeg(SegmentSetPS &SegSet, palettePS &Palette, uint8_t Style, bool Alternate, bool WipeDirect,
                     bool SegWipeDir, uint16_t Rate);

        //Constructor for a single color wipe
        ColorWipeSeg(SegmentSetPS &SegSet, CRGB WipeColor, uint8_t Style, bool Alternate, bool WipeDirect,
                     bool SegWipeDir, uint16_t Rate);

        ~ColorWipeSeg();

        CRGB
            bgColorOrig = 0,          //default background color (blank)
            *bgColor = &bgColorOrig;  //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)

        uint8_t
            style,
            colorMode = 0,
            bgColorMode = 0;

        uint16_t
            pixelCount = 0,    //for reference
            segWipeCount = 0,  //for reference
            patOffset = 0,     //for reference
            loopCount = 0;     //for reference

        //General bools
        bool
            done = false,
            alternate,
            wipeDirect,
            segWipeDir;

        //loop settings
        bool
            looped = false,
            altSegWipeDirLoop = false,
            altWipeDirLoop = false,
            bgLoop = false,
            bgAltLoop = false,  //only matters if altWipeDirLoop is true
            shiftPatLoop = false,
            patShiftDir = false;

        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0};  //Must init structs w/ pointers set to null for safety

        patternPS
            *pattern = nullptr,
            patternTemp = {nullptr, 0, 0};  //Must init structs w/ pointers set to null for safety

        void
            reset(),
            setPaletteAsPattern(),
            resetLoop(),
            setUpLoop(bool nLooped, bool nBgLoop, bool nShiftPatLoop, bool nPatShiftDir, bool nAltWipeDirLoop,
                      bool nBgAltLoop, bool nAltSegWipeDirLoop),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0;

        int8_t
            wipeStep,
            segStep,
            patternStep;

        uint8_t
            palIndex,
            modeOut;

        uint16_t
            currentSeg,
            lastSeg,
            segLength,
            numSegs,
            currentPixel,
            endPixel,
            pixelNum,
            lineNum;

        bool
            startingDirect,
            bgWipe = false;

        CRGB
            colorOut;

        void
            setUpWipeOrder(),
            setupSegWipe(),
            init(SegmentSetPS &SegSet, uint16_t Rate);
};

#endif
#ifndef ColorWipeSL_h
#define ColorWipeSL_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

/* 
An effect that colors one segment line in after another until all lines are colored. 
The effect can be configured to wipe in specific lengths (these being repeated down the segment set)
Ie you could have a segment set with 24 lines, and use a wipe length of 6, producing 24/6 = 4 wipes.
The wipes can be set to either all happen simultaneously or in sequence.

Wipes have a direction. This can be set to alternate with each wipe.

(If you're wipe length is the length of the segment set lines, then you'll just have one wipe)

The effect uses a pattern and palette for colors and various styles to display them:
Styles:
    0: The colors alternate with each wipe. Ie if you have 4 wipe sections, each will be a different color from the pattern
    1: The colors alternates with each segment line.
    2: The colors alternate with each segment.
The colors follow the pattern. To change the colors you have to eithee change the pattern or the palette.

The effect is compatible with colorModes. They will override the style setting.

Once all the wipes are finished, the "done" flag will be set, and the effect is is over until it is reset.

To keep wiping continuously, you can set the wipes to loop, which will automatically reset the effect repeatedly.
The number of full wipes completed is stored in wipeCount. (ie if you have 4 wipes, wipe count will increment every time those 4 wipes are finished)

There are a various of options for configuring how the wipe loops, allowing you to change the wipe direction, 
wipe a background color, etc when looping. You can configure these options all at once using setUpLoop().
Or you can set them indiviually.

The loop options are: 
    looped -- Sets if the wipes loop or not. Looping wipes automatically restart everytime a wipe is finished.
              The other variables are only relevant if the wipe is looping, because they modify subsequent loops.
    shiftPatLoop -- If true, the pattern will be shifted forward by 1 with every loop, 
                    changing the colors of each segment / line with each wipe
                    (this is done with an offset, it does not change the existing pattern)
    bgLoop -- If true, then the background color (default 0) will be used as the color wipe every other loop
              Ie, we wipe a color and then wipe off, looping
    altWipeDirLoop -- If true, then the wipe direction will be flipped after each loop
                   (note that this modifies the original startingDirect value)
    bgAltLoop -- Only used if altWipeDirLoop is true. 
                 If true, the the wipe direction will only swap on colored wipes, not on background ones.
                 Ie colored wipe->background wipe->(wipe direction flips)->colored wipe->background wipe->(flip)->etc
                 If false, then the wipe direction will flip every loop
                 ie colored wipe->(wipe direction flips)->background wipe->(wipe direction flips)->colored wipe->etc
    altSegDirLoop -- If true, then the segment set direction will be flipped with each loop
                     This is different than flipping the wipe direction, since it makes the first wipe 
                     start at the opposite end of the segment set, rather than having the wipe just move in the oppsite direction 
                     !!Note that is modifies the direction setting for the segment set    
                    
The bgColor is a pointer, so you can bind it to an external color variable.
By default it is bound to bgColorOrig, which is set to 0 (blank color).

You can adjust most of the effect variables (wipeDirect, simult, etc) on the fly. 
The only restriction is the wipeLength, which must be set using setWipeLength().

Example calls: 

    ColorWipeSL(mainSegments, &palette1, 0, 1, false, false, true, 140);
    Will do a color wipe along mainSegment's lines using colors from palette1
    (A pattern will be generated to match the palette)
    Only one wipe will happen, it is the full length of the segment set
    (passing 0 in as the wipe length automatically sets the wipe length to the number of lines in the segment set)
    The style is 1, each line will alternate colors according to the pattern.
    Both simlut and alternate are false (they don't matter for a single wipe)
    The wipe will move in the positive direction, updating at 140ms

    ColorWipeSL(mainSegments, &palette1, &pattern1, 8, 0, true, false, false, 140);
    Will do a color wipe along mainSegment's lines using colors from palette1, according to pattern1
    The wipe length is 8 (assumed to be shorter than the segment set num lines)
    The style is 0, each wipe will alternate colors according to the pattern.
    The wipes will occur simultaneously with each wipe being in the same direction
    The wipes will move in the negative direction, updating at 140ms

    ColorWipeSL(mainSegments, CRGB(CRGB::Red), 8, 0, false, true, false, 140);
    Will do a color wipe along mainSegment's lines using CRGB::Red as the only color
    The wipe length is 8 (assumed to be shorter than the segment set num lines)
    The style is 0 (doesn't matter b/c we only have one color)
    The wipes will occur in sequence with each wipe alternating direction
    The first wipe will move in the negative direction, updating at 140ms

Constructor Inputs:
    palette(optional, see constructors) -- The set of colors that color wipes will use
    pattern(optional, see constructors) -- A pattern is struct made from a 1-d array of palette indexes ie {0, 1, 3, 6, 7} 
                                          and the length of the array (see patternPS.h)   
    color (optional, see contructors) -- A single color for all the wipes
    wipeLength -- Set how long each wipe is. The effect will always do enough wipes to fill the segment set
                  (passing 0 in as the wipe length automatically sets the wipe length to the number of lines in the segment set)
    style -- How the colors are displayed for the wipe(s) (see notes in intro above)
    simult -- If true, then all wipes will happen at the same time, otherwise they will happen one after another
    alternate -- If true, wipes will alternate directions (ie if one wipe is going forward, the next will reverse)
    wipeDirect -- The direction of the first wipe, true is forward
    Rate -- The update rate of the effect (ms)

Functions:
    reset() -- Resets the effect vars, restarting the wipes
    setPaletteAsPattern() -- Sets the effect pattern to match the current palette
    setWipeLength(newWipeLength) -- Sets the wipe length, does NOT restart the effect
    resetLoop() -- Resets the current loop, will switch to the next loop, only relevant when looping (you shouldn't need to call this)
    setUpLoop(nLooped, nShiftPatLoop, nBgLoop, nAltWipeDirLoop, nBgAltLoop, nAltSegDirLoop) 
             -- A quick way of setting all the loop variables (see intro for notes on loops)
    update() -- updates the effect

Other Settings:
    colorMode (default 0) -- sets the color mode for the waves (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the spacing pixels (see segDrawUtils::setPixelColor)
    bgColor (default 0) -- See intro notes under looping
    looped (default false) -- Sets if the effect should loop once a wipe is finished (see intro looping notes)
    altSegDirLoop (default false) -- (see intro looping notes)
    altWipeDirLoop (default false) -- (see intro looping notes)
    bgLoop (default false) -- (see intro looping notes)
    bgAltLoop (default false) -- (see intro looping notes)
    shiftPatLoop (default false) -- (see intro looping notes)

Reference Vars:
    wipeCount -- How many full wipe cycles we've done, useful for tracking wipes when looping
    wipeLength -- The length of each wipe (set using setWipeLength())
    patOffset -- The current offset of the pattern. Used to shift the pattern colors. Only used when looping.

Flags:
    done -- Set true when the wipe cycle is finished. Not set if we're looping.

 */
class ColorWipeSL : public EffectBasePS {
    public:
        ColorWipeSL(SegmentSet &SegmentSet, palettePS *Palette, patternPS *Pattern, uint16_t WipeLength, uint8_t Style,
                    bool Simult, bool Alternate, bool WipeDirect, uint16_t Rate);  
        
        ColorWipeSL(SegmentSet &SegmentSet, palettePS *Palette, uint16_t WipeLength, uint8_t Style,
                    bool Simult, bool Alternate, bool WipeDirect, uint16_t Rate); 

        ColorWipeSL(SegmentSet &SegmentSet, CRGB WipeColor, uint16_t WipeLength, uint8_t Style,
                    bool Simult, bool Alternate, bool WipeDirect, uint16_t Rate); 
                
        ~ColorWipeSL();

        SegmentSet 
            &segmentSet;

        CRGB 
            bgColorOrig = 0, //default background color (blank)
            *bgColor = &bgColorOrig; //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color) 
        
        uint8_t
            style,
            colorMode = 0,
            bgColorMode = 0;
        
        uint16_t
            wipeLength, //for reference
            wipeCount = 0, //for reference
            patOffset = 0; //for reference
        
        bool 
            done = false,
            simult,
            alternate,
            wipeDirect, 
            looped = false,
            altSegDirLoop = false,
            altWipeDirLoop = false,
            bgLoop = false,
            bgAltLoop = false, //only matters if altWipeDirLoop is true
            shiftPatLoop = false;
        
        palettePS
            *palette = nullptr,
            paletteTemp;
        
        patternPS
            *pattern = nullptr,
            patternTemp;
        
        void 
            reset(),
            setPaletteAsPattern(),
            setWipeLength(uint16_t newLength),
            resetLoop(),
            setUpLoop(bool nLooped, bool nShiftPatLoop, bool nBgLoop, bool nAltWipeDirLoop, bool nBgAltLoop, bool nAltSegDirLoop ),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        uint8_t 
            palIndex,
            modeOut;
        
        uint16_t
            numLines,
            numSegs,
            pixelNum,
            lineNum,
            numWipes,
            wipeLine,
            wipeNumSeq;
        
        bool
            startingDirect,
            bgWipe = false;
        
        CRGB 
            colorOut;
        
        void 
            init(uint16_t Rate);
};

#endif
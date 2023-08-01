#ifndef ColorWipeSLSeg_h
#define ColorWipeSLSeg_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

/* 
An effect that colors one segment line, (or segment) in after another until all lines/segments are colored. 
The effect can be configured to wipe in specific lengths (these being repeated down the segment set)
Ie you could have a segment set with 24 lines, and use a wipe length of 6, producing 24/6 = 4 wipes.
The wipes can be set to either all happen simultaneously or in sequence.
The effect can either be configured to wipe whole segments, or segment lines one by one (see segMode notes below)

Wipes have a direction. This can be set to alternate with each wipe.

(If you're wipe length is the length of the segment set lines or segments, then you'll just have one wipe)

The effect can also be set to loop, repeating the wipes continuously, with a variety of configuration options.

The effect uses a pattern and palette for colors and various styles to display them:
Styles:
    0: The colors alternate with each wipe. Ie if you have 4 wipe sections, each will be a different color from the pattern
    1: The colors alternates with each segment line.
    2: The colors alternate with each segment.
The colors follow the pattern. To change the colors you have to either change the pattern or the palette.

The effect is compatible with colorModes. They will override the style setting.

Once all the wipes are finished, the "done" flag will be set, and the effect is is over until it is reset.

SegMode, Wipe Lengths, and Update Rates:
    segMode configures the effect to wipe whole segments, or segment lines one by one.
    Each segMode has it's own wipeLength (segWipeLen or lineWipeLen).
    When you first create the effect, depending on the segMode you pass in,
    either segWipeLen or lineWipeLen will be set to wipeLength you pass in.
    The other length will be set to 0 (so that a whole wipe is done).
    When switching segModes, the wipeLength will be set to segWipeLen or lineWipeLen.
    This ensures that when looping (see below), we have a specific wipe length for segment or line wipes.
    You can change the segWipeLen and lineWipeLen manually at any time. 

    You can change the segMode by calling setSegMode(newSegMode), 
    which also changes the wipeLength to either segWipeLen or lineWipeLen (recalculating numWipes)

    Because the number of segments and segment lines will probably not be equal, I've included
    a time offset for when segMode is true: segRateAdj. This is added to the effect's update rate
    allowing you to slow down the updates by a fixed amount, while still being able to vary the effect's rate like other effects.
    The is mostly useful when looping:
    For example, say I have a segment set with 5 segments, and 20 segment lines. 
    I have the ColorWipe set to loop, switch segModes when it does. 
    I have my update rate set to 80ms, which looks good with my segment lines, but is too fast for the whole segments.
    So I set segRateAdj to 100ms, making the overall update rate in segMode 180ms (100 + 80).
    
    Note that I'm assuming that most segment sets have more lines than segments, so you'd want to slow down the segment wiping.
    segRateAdj CAN be negative, to increase the update rate, but you must be EXTREMELY careful to keep it less that <<rate>>. 
    
    segRateAdj's range is +/-32,767(ms).

Looping:
    To keep wiping continuously, you can set the wipes to loop, which will automatically reset the effect repeatedly.
    The number of full wipes completed is stored in loopCount. (ie if you have 4 wipes, loopCount will increment every time those 4 wipes are finished)

    There are a various of options for configuring how the wipe loops, allowing you to change the wipe direction, 
    wipe a background color, etc when looping. You can configure these options all at once using setUpLoop().
    Or you can set them individually.

    The loop options give you a lot of flexibility in creating different effects, I encourage you to play with them!
    If you can't setup exactly what you want, you can always create multiple ColorWipes, and invoke them manually, 
    swapping/resetting them out as they are "done".

The Loop Options: 
    looped -- Sets if the wipes loop or not. Looping wipes automatically restart every time a wipe is finished.
              The other variables are only relevant if the wipe is looping, because they modify subsequent loops.
    bgLoopFreq (min 2) -- Sets how often a bgWipe is done. ie 2 will be every other loop, 3, every 3 loops, etc
                          The minimum is 2, because wiping the background every loop (1) isn't useful.
                          Note that if bgLoop is false, no background wipes will be done!
                          I suggest starting with this at 2.
    bgLoop -- If true, then the background color (default 0) will be used as the color wipe every <<bgLoopFreq>> loop
              Ie, we wipe a color and then wipe off, looping
    loopFreq (min 1) -- Sets on what loops shiftPatLoop, altWipeDirLoop, and altSegDirLoop trigger
                        ie 1 will trigger them every loop, 2 every other loop, etc
                        This allows you to build some really neat effects just by changing the freq.
                        I suggest starting with this at 1.
    shiftPatLoop -- If true, the pattern will be shifted forward by 1 with every <<loopFreq>> loop, 
                    changing the colors of each segment / line with each wipe
                    (this is done with an offset, it does not change the existing pattern)
    altWipeDirLoop -- If true, then the wipe direction will be flipped every <<loopFreq>> loop
                      (note that this modifies the original startingDirect value)
    bgAltLoop -- Only used if altWipeDirLoop is true. 
                 If true, the the wipe direction will only swap on colored wipes, not on background ones.
                 Ie colored wipe->background wipe->(wipe direction flips)->colored wipe->background wipe->(flip)->etc
                 If false, then the wipe direction will flip every loop
                 ie colored wipe->(wipe direction flips)->background wipe->(wipe direction flips)->colored wipe->etc
    altSegDirLoop -- If true, then the segment set direction will be flipped every <<loopFreq>> loop
                     This is different than flipping the wipe direction, since it makes the first wipe 
                     start at the opposite end of the segment set, rather than having the wipe just move in the opposite direction    
    altSegModeLoop -- If true, will switch segMode setting the effect from wiping segment lines to whole segments, or visa versa.
                      When swapping, the wipeLength will be set to segWipeLen or lineWipeLen depending on the segMode.
                      see setWipeLength() for info on how these are set. 
                      Note that altSegModeLoop triggers every loop. 
                      For me, this seemed like the best option rather than tying it to a freq. 
                      It seemed weird to want to switch segModes for multiple loops, you might as well just create two different ColorWipes.   
                    
The bgColor is a pointer, so you can bind it to an external color variable.
By default it is bound to bgColorOrig, which is set to 0 (blank color).

You can adjust most of the effect variables (wipeDirect, simult, etc) on the fly. 
The only restriction are the wipeLength and segMode, which must be set using setWipeLength(), and setSegMode().

Example calls: 
    ColorWipeSLSeg colorWipeSL(mainSegments, cybPnkPal, 0, 1, false, false, true, false, 140);
    Will do a color wipe along mainSegment's lines using colors from cybPnkPal
    (A pattern will be generated to match the palette)
    Only one wipe will happen, it is the full length of the segment set
    (passing 0 in as the wipe length automatically sets the wipe length to the number of lines in the segment set)
    The style is 1, each line will alternate colors according to the pattern.
    Both simlut and alternate are false (they don't matter for a single wipe)
    The wipe will move in the positive direction, updating at 140ms
   
    uint8_t pattern_arr = {0, 1, 2};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};
    ColorWipeSLSeg colorWipeSL(mainSegments, cybPnkPal, pattern, 8, 0, true, false, false, false, 140);
    Will do a color wipe along mainSegment's lines using colors from cybPnkPal, according to pattern1
    The wipe length is 8 (assumed to be shorter than the segment set num lines)
    The style is 0, each wipe will alternate colors according to the pattern.
    The wipes will occur simultaneously with each wipe being in the same direction
    The wipes will move in the negative direction, updating at 140ms

    ColorWipeSLSeg colorWipeSL(mainSegments, CRGB(CRGB::Red), 2, 0, true, true, false, true, 140);
    Will do a color wipe along mainSegment's segments using CRGB::Red as the only color
    The wipe length is 2 (assumed to be shorter than the segment set num segments)
    The style is 0 (doesn't matter b/c we only have one color)
    The wipes will occur in sequence with each wipe alternating direction
    The first wipe will move in the negative direction, updating at 140ms

Constructor Inputs:
    palette(optional, see constructors) -- The set of colors that color wipes will use
    pattern(optional, see constructors) -- A pattern is struct made from a 1-d array of palette indexes ie {0, 1, 3, 6, 7} 
                                          and the length of the array (see patternPS.h)   
    color (optional, see constructors) -- A single color for all the wipes
    wipeLength -- Set how long each wipe is. The effect will always do enough wipes to fill the segment set
                  (passing 0 in as the wipe length automatically sets the wipe length to the number of lines in the segment set)
    style -- How the colors are displayed for the wipe(s) (see notes in intro above)
    simult -- If true, then all wipes will happen at the same time, otherwise they will happen one after another
    alternate -- If true, wipes will alternate directions (ie if one wipe is going forward, the next will reverse)
    wipeDirect -- The direction of the first wipe, true is forward
    segMode -- If true, the wipes will use whole segments, otherwise they'll use segment lines
    Rate -- The update rate of the effect (ms)

Functions:
    reset() -- Resets the effect vars, restarting the wipes
    setPaletteAsPattern() -- Sets the effect pattern to match the current palette
    setWipeLength(newWipeLength) -- Sets the wipe length, does NOT restart the effect
    setSegMode(newSegMode) -- Changes the segMode and changes the wipeLength to either segWipeLen or lineWipeLen (recalculating numWipes) (see segMode notes above)
    resetLoop() -- Resets the current loop, will switch to the next loop, only relevant when looping (you shouldn't need to call this)
    setUpLoop(bool nLooped, uint8_t nBgLoopFreq, bool nBgLoop, uint8_t nLoopFreq, bool nShiftPatLoop,
              bool nAltWipeDirLoop, bool nBgAltLoop, bool nAltSegDirLoop, bool nAltSegModeLoop) 
             -- A quick way of setting all the loop variables (see intro for notes on loops)
    update() -- updates the effect

Other Settings:
    colorMode (default 0) -- sets the color mode for the waves (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the spacing pixels (see segDrawUtils::setPixelColor)
    bgColor (default 0) -- See intro notes under looping
    segDirect (default true) -- The direction of the segments for the wipe, ie if false, the first line will be treated as the last line, etc
    segRateAdj (default 0) -- The adjustment rate for segMode (see segMode Notes above )
    lineWipeLen (default 0) -- The length for line wipes (segMode false), (see segMode Notes above)
    segWipeLen (default 0) -- The length for seg wipes (segMode true), (see segMode Notes above)

Looping Settings:
    looped (default false) -- Sets if the effect should loop once a wipe is finished (see intro looping notes)
    shiftPatLoop (default false) -- (see intro looping notes)
    loopFreq (default 1) -- (see intro looping notes)
    bgLoopFreq (default 2) -- (see intro looping notes)
    altSegDirLoop (default false) -- (see intro looping notes)
    altWipeDirLoop (default false) -- (see intro looping notes)
    bgLoop (default false) -- (see intro looping notes)
    bgAltLoop (default false) -- (see intro looping notes)

Reference Vars:
    loopCount -- How many full wipe cycles we've done, useful for tracking wipes when looping
    wipeLength -- The length of each wipe (set using setWipeLength())
    segMode -- (see segMode notes above) ( set using setSegMode() )
    patOffset -- The current offset of the pattern. Used to shift the pattern colors. Only used when looping.

Flags:
    done -- Set true when the wipe cycle is finished. Not set if we're looping.

 */
class ColorWipeSLSeg : public EffectBasePS {
    public:
        //Constructor using pattern and palette
        ColorWipeSLSeg(SegmentSet &SegSet, palettePS &Palette, patternPS &Pattern, uint16_t WipeLength, uint8_t Style,
                       bool Simult, bool Alternate, bool WipeDirect, bool SegMode, uint16_t Rate);  
        
        //Constructor using palette alone 
        ColorWipeSLSeg(SegmentSet &SegSet, palettePS &Palette, uint16_t WipeLength, uint8_t Style,
                       bool Simult, bool Alternate, bool WipeDirect, bool SegMode, uint16_t Rate); 

        //Constructor for a single color wipe
        ColorWipeSLSeg(SegmentSet &SegSet, CRGB WipeColor, uint16_t WipeLength, uint8_t Style,
                       bool Simult, bool Alternate, bool WipeDirect, bool SegMode, uint16_t Rate); 
                
        ~ColorWipeSLSeg();

        SegmentSet 
            &SegSet;

        CRGB 
            bgColorOrig = 0, //default background color (blank)
            *bgColor = &bgColorOrig; //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color) 
        
        uint8_t
            style,
            colorMode = 0,
            bgColorMode = 0;
        
        uint16_t
            wipeLength, //for reference
            loopCount = 0, //for reference
            patOffset = 0, //for reference
            lineWipeLen = 0,
            segWipeLen = 0;
        
        int16_t
            segRateAdj = 0; //Max of +/-32,767. Be careful if you set this negative!!!!, must be less than rate.

        //General bools
        bool
            done = false,
            simult,
            alternate,
            wipeDirect,
            segDirect = true;

        //segMode settings
        bool
            segMode; //for reference

        //loop settings
        bool
            looped = false,
            altSegDirLoop = false,
            altWipeDirLoop = false,
            bgLoop = false,
            bgAltLoop = false, //only matters if altWipeDirLoop is true
            shiftPatLoop = false,
            altSegModeLoop = false;
        
        uint8_t
            loopFreq = 1, //min value 1
            bgLoopFreq = 2; //min value 2
        
        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0}; //Must init structs w/ pointers set to null for safety
        
        patternPS
            *pattern = nullptr,
            patternTemp = {nullptr, 0, 0}; //Must init structs w/ pointers set to null for safety
        
        void 
            reset(),
            setPaletteAsPattern(),
            setWipeLength(uint16_t newLength),
            setSegMode(bool newSegMode),
            resetLoop(),
            setUpLoop(bool nLooped, uint8_t nBgLoopFreq, bool nBgLoop, uint8_t nLoopFreq, bool nShiftPatLoop,
                      bool nAltWipeDirLoop, bool nBgAltLoop, bool nAltSegDirLoop, bool nAltSegModeLoop),
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
            segOrLineNum,
            segOrLineLimit,
            numWipes,
            wipeStep,
            wipeNumSeq;
        
        bool
            startingDirect,
            bgWipe = false;
        
        CRGB 
            colorOut;
        
        void 
            init(uint16_t Rate),
            doLineWipe(uint16_t wipeNum, uint16_t wipeStep, uint16_t lineNum),
            doSegWipe(uint16_t wipeNum, uint16_t wipeStep, uint16_t segNum);
};

#endif
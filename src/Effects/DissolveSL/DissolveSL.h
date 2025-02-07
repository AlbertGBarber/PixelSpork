#ifndef DissolveSL_h
#define DissolveSL_h

//TODO
//  --Change bool array to uint8_t's where the bools are stored as the bits of the uint8_t's
//    Will have to calculate the length of the array and write function for getting/setting

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

/* 
Morphs the segment set from one color to the next by setting each segment line at random, one at a time.
Includes various options for color selection (see modes) 
Colors can be chosen using a palette and pattern, or selected randomly.
You can also automatically insert a background color (default black) between each dissolve,
or at the end of a color pattern.

Once a dissolve is finished, the effect can be set to pause for a period using the `pauseTime` setting.

The effect is adapted to work on segment lines for 2D use. (see Inputs Guide below).

You can freely use colorModes from segDrawUtils::setPixelColor(), but they don't make much sense
unless you are running an offset in the SegmentSetPS or using colorModes 5 or 6.

Warning: requires an array of bools of equal size to the number of pixels in the segment set.
So be careful of your memory usage!

Inputs Guide:

    Line Modes: 
        The effect is adapted to work on segment lines for 2D use. 
        
        You can use `lineMode` to control how pixels are filled in: 
            * true (default): Whole segment lines will be dissolved, rather than each pixel.
            * false: Individual pixels will be dissolved (only really useful when using certain color modes).
    
    Controlling How Pixels Spawn/Dissolve:
        The effect can be accelerated to set more lines/pixels at once by adjusting "spawnRateInc".
        By default we start by spawning one segment line at a time, increasing the number every spawnRateInc ms,
        so the spawning steadily accelerates. This helps keep the spawning consistent, since we're picking 
        repeatedly at random.
        Setting spawnRateInc close-ish (up to double?) to the update rate looks the best.

        Once a certain threshold has been met, "setAllThreshold"  
        (default of 1/10 of the number of lines in the segment set remaining)
        All the remaining lines are set. This prevents us from getting stuck looking for the last line.
        This may sound weird, but in practice it looks quite good.

        Note, I know it would be better to remove the chance of picking a line that's already dissolved,
        but that would require more memory to track the dissolved lines' locations 
        (an array of uint16_t's vs an array of bools). 
        Since the current effect seems to work, albeit with a bit of manual tweaking, I see no reason to change it.

        You can increase the starting number of lines set at once (maxNumSpawnBase), which will
        accelerate the dissolving, and may be good on longer segment sets.

    Inserting Background Dissolves:
        Using the "bgMode" setting, you can configure the effect to dissolve to a "blank" color
        either between every colored dissolve, or at the end of a full pattern of colors. 

        The blank color is default to black, but can be set using via the effects `*bgColor` pointer, 
        or bgColorOrig (the default target of the bgColor pointer).

        The modes work by inserting "blanks" into the effect's dissolve color pattern; 
        they are indicated by a value of 255. 
        You can also use the blank marker in your own patterns 
        (as opposed to those generated automatically by the effect, see constructors below). 

        For example, a pattern of {0, 1, 2, 255, 3} would dissolve through palette colors 
        0, 1, 2, then a blank, and then to color 3 before repeating.
       
        bgMode's (uint8_t):
            0: No blanks (ex: {0, 1, 2, 3, 4}, where the values are palette indexes).
            1: One blank added to the end of the pattern (ex: {0, 1, 2, 3, 4, 255}).
            2: A blank is added after each color (ex: {0, 255, 1, 255, 2, 255, 3, 255, 4, 255}).

        You can change the bgMode using the setBgMode() function. Note that this will rewrite the effect's local
        color pattern (patternTemp) and tell the effect to use it. 
        Changing the bgMode while the effect is running may have weird effects. 
        You can call resetPixelArray() to reset the effect.
    
    Choosing Colors randomly:
        You can opt to have the dissolve colors chosen at random in various combinations using the "randMode" setting.
        Note that there is some interplay between randMode and bgMode which you should be aware of
        (see "Background and Random Color Behavior" below).

            randMode's:
                0: Each dissolve is a solid color following the pattern (not random).
                1: Each dissolve is a set of randomly chosen colors (dif color for each pixel).
                2: Each dissolve is a set of random colors chosen from the pattern (dif color for each pixel/line).
                3: Each dissolve is a solid color chosen at random.
                4: Each dissolve is a solid color chosen randomly from the pattern (non-repeating).
                5: Each dissolve is a solid color chosen randomly from the pattern, but a "blank" dissolve is 
                   done between each color (see "Inserting Background Dissolves" below). 
                   By default, the same dissolve color won't repeat after a blank, but you can allow
                   repeats by setting "randMode5AllowRepeats" to true (making the colors chosen at random).
                   Note that bgMode should be set to 0 for this randMode.

            You should be able switch freely between randModes on the fly, but there may be some initial color repeating
            (the random modes will set up a random palette/pattern as a fallback).

    Background and Random Color Behavior:
        There is some interplay between the random and background modes, as both modify what colors are shown when.
        The background behavior for each random mode is listed below.

        For randMode:
            0: The background mode works as described above.
            1: The background mode is ignored. There will be no blank dissolves.
            2: The blank color can be chosen from the pattern, but the effect will not cycle to fully blank. 
               (Note the more blanks in your pattern, the more likely a pixel will be dissolve to blank. 
               So be careful when using randMode 2 and bgMode 2).
            3: The background mode works as described above, 
               however, for bgMode 1, the number of dissolves before the blank is set by "randMode3CycleLen".
               Ie, if randMode3CycleLen is 3, then the effect will dissolve through 3 random colors before the blank color.
               randMode3CycleLen is defaulted to the input pattern length, or can specified in some constructors.
            4: Each dissolve is a random color from the pattern, with the blank color being a possible choice.  
            5: The background mode is ignored. A blank dissolve always happens between random colors.

Example calls: 
    uint8_t pattern_arr = {0, 2, 1};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};
    DissolveSL dissolve(mainSegments, pattern, cybPnkPal_PS, 0, 150, 70);
    Will dissolve colors from the cybPnkPal_PS palette, following the pattern above using randMode 0 (see above) 
    with the number of lines set on one cycle increasing every 150ms with the effect updating every 70ms.

    DissolveSL dissolve(mainSegments, cybPnkPal_PS, 0, 2, 100, 100);
    Will dissolve colors using from the cybPnkPal_PS palette in order, 
    using randMode 0 (not random) and bgMode 2 (see Inputs Guide above), so there will be a blank dissolve between colors. 
    The number of lines set on one cycle increases every 100ms with the effect updating every 100ms.

    DissolveSL dissolve(mainSegments, cybPnkPal_PS, 4, 0, 100, 100);
    Will dissolve using colors from the cybPnkPal_PS using randMode 4 and bgMode 0 (see Inputs Guide above) 
    with the number of lines set on one cycle increasing every 100ms with the effect updating every 100ms.

    DissolveSL dissolve(mainSegments, 3, 0, 3, 150, 70);
    Will dissolve using random colors set according to randMode 3
    (use randMode 2 or 3 with this constructor)
    The bgMode is 0 (no blanks), with a "randMode3CycleLen" of 3, although this is ignored (see Inputs Guide above)
    The number of lines set on one cycle increases every 150ms with the effect updating every 70ms.

Constructor Inputs
    pattern(optional, see constructors) -- The color pattern the effect will use. 
                                           Note that any entries of 255 in the pattern will be set to the background color.   
    palette(optional, see constructors) -- The repository of colors used in the pattern, or can be used as the pattern itself.
    randMode -- The randMode that will be used for the dissolves (see above)
    bgMode -- Controls if "blank" spaces are added to the shift pattern. 
              See "Inserting Background Dissolves" in Inputs Guide above. 
              Can be changed later using `setBgMode()`.
    randMode3CycleLen (optional, see constructors, default to pattern length if not used) --
                      Only relevant when using bgMode 1 and randMode 3. 
                      Sets how many colors are cycled through before the background color is used. 
                      (See "Inserting Background Dissolves" in Inputs Guide above)
    spawnRateInc -- The rate increase at which the total number of lines that are set each cycle (ms)
                    Setting this close-ish (up to double?) to the update rate looks the best.
    rate -- update rate (ms)

Other Settings:
    *bgColor and bgColorOrig (default 0) -- The color used by "blank" spaces. By default the bgColor is pointed to bgColorOrig.
    colorMode (default 0) -- sets the color mode for the random pixels (see segDrawUtils::setPixelColor)
    setAllThreshold (defaults to 1/10th of the segment set length) -- The number of segment lines that will be set 
                                                                      randomly before any remaining lines are force 
                                                                      set at once. This is a fail safe so that 
                                                                      you don't get stuck with one line that is 
                                                                      never randomly picked, preventing the dissolve 
                                                                      from ending.
    pauseTime (default 0ms) -- The length of time that the effect will wait between dissolves.
                               If the pause time is active, it is indicated with the paused flag
    maxNumSpawnBase (default 1) -- The starting value of the number of segment lines set in one cycle. 
                                   Higher numbers may work better for longer segment set lengths.
    randMode5AllowRepeats (default false) -- Only used in randMode 5. If true, then dissolve colors from the 
                                             pattern can repeat between blanks. 
                                             If false, a new color will always be chosen. 
                                
Functions:
    setPaletteAsPattern() -- Sets the effect pattern to match the current palette, 
                             will also add "blank" spaces depending on bgMode (see intro).
    setBgMode(newBgMode) -- Sets the bgMode to control "blank" pattern spaces.
                            See "Inserting Background Dissolves" in Inputs Guide above.
    resetPixelArray() -- Resets the dissolved state of the segment lines, effectively restarting the current dissolve.
    setLineMode(newLineMode) -- Sets the line mode (see intro LineMode notes), 
                                also restarts the dissolve, and sets the setAllThreshold to 1/10 the numLines
    update() -- updates the effect

Reference vars:
    dissolveCount -- The number of dissolves we've done (does not reset automatically).
    bgMode -- (see constructor vars above). Set using setBgMode().
    numCycles -- How many dissolves we've finished (resets when we've gone through the whole pattern).
    lineMode (default true) -- (See intro Line Mode notes). Set using setLineMode().

Flags:
    paused -- While true, the effect is paused. Note that the effect is not re-drawn while paused.
*/
class DissolveSL : public EffectBasePS {
    public:
        //constructor for pattern
        DissolveSL(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, uint8_t RandMode,
                   uint16_t SpawnRateInc, uint16_t Rate);

        //constructor for palette as pattern
        DissolveSL(SegmentSetPS &SegSet, palettePS &Palette, uint8_t RandMode, uint8_t BgMode, 
                   uint16_t SpawnRateInc, uint16_t Rate);

        //constructor for randomly chosen colors (should only use randMode 2 or 3 with this constructor)
        //note that RandMode3CycleLen is only relevant for bgMode 1 and randMode 3. 
        //it controls how many colors are cycled before the background color is used. (see intro bgMode section for more)
        DissolveSL(SegmentSetPS &SegSet, uint8_t RandMode, uint8_t BgMode, uint16_t RandMode3CycleLen,
                   uint16_t SpawnRateInc, uint16_t Rate);

        ~DissolveSL();

        uint8_t
            randMode,
            colorMode = 0,
            bgMode, //for reference, set this using setBgMode()
            maxNumSpawnBase = 1;

        uint16_t
            setAllThreshold,
            randMode3CycleLen,
            pauseTime = 0,
            spawnRateInc,
            dissolveCount = 0,  //The number of full dissolves we've done
            numCycles = 0;      //how many update cycles we've been through (resets when we've gone through the whole pattern)

        bool
            lineMode = true,  //for reference, set using setLineMode()
            paused = false,
            randMode5AllowRepeats = false,
            *pixelArray = nullptr;
        
        CRGB
            bgColorOrig = 0,  //default "blank" color for spaces
            *bgColor = &bgColorOrig;

        patternPS
            *pattern = nullptr,
            patternTemp = {nullptr, 0, 0};  //Must init structs w/ pointers set to null for safety

        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0};  //Must init structs w/ pointers set to null for safety

        void
            setPaletteAsPattern(),
            setBgMode(uint8_t newBgMode),
            resetPixelArray(),
            setLineMode(bool newLineMode),
            update(void);

    private:
        unsigned long
            currentTime,
            prevSpawnTime = 0,
            prevTime = 0;

        uint8_t
            currentIndex = 0, 
            tempIndex;

        uint16_t
            thresStartPoint = 0,
            numLines,
            maxNumSpawn,  //How many lines we'll try to spawn each cycle (starts as maxNumSpawnBase and increases with time)
            maxNumLines = 0,  //used for tracking the memory size of the pixelArray
            maxNumCycles = 0,
            lineNum,
            numSpawned = 0,
            pixelNum;

        bool
            flipFlop = true,
            randColorPicked = false;

        CRGB
            pickDissolveColor(),
            getPatternColor(uint8_t patIndex),
            color;

        void
            init(SegmentSetPS &SegSet, uint16_t Rate),
            spawnLed(uint16_t pixelNum);
};

#endif
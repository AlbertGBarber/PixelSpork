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
Colors can be chosen using a palette and pattern, or selected randomly
The effect can be accelerated to set more lines at once by adjusting spawnRateInc
By default we start by spawning one segment line at a time, increasing the number every spawnRateInc ms
so the spawning steadily accelerates. This helps keep the spawning consistent, since we're picking at random
Once a certain threshold has been met (default of 1/10 of the number of lines in the segment set remaining)
All the remaining lines are set. This prevents us from getting stuck looking for the last line.
Once a morph is finished, the effect can be set to pause for a period using the pauseTime variable.

Note, I know it would be better to remove the chance of picking a line that's already dissolved,
but that would require more memory to track the dissolved line's locations 
(an array of uint16_t's vs an array of bools). 
Since the current effect seems to work, albeit with a bit of manual tweaking, I see no reason to change it.

You can increase the starting number of lines set at once (maxNumSpawnBase), which will
accelerate the morphing, and may be good on longer segment sets.

The effect is adapted to work on segment lines for 2D use. 
You can use `lineMode` to control how pixels are filled in: 
* true (default): Whole segment lines will be dissolved, rather than each pixel.
* false: Individual pixels will be dissolved (only really useful when using certain color modes).

randModes:
    0: Each dissolve is a solid color following the pattern
    1: Each dissolve is a set of randomly chosen colors (dif color for each pixel)
    2: Each dissolve is a set of random colors chosen from the pattern (dif color for each pixel)
    3: Each dissolve is a solid color chosen at random
    4: Each dissolve is a solid color chosen randomly from the pattern

You should be able switch freely between randModes on the fly (the random modes will set up a random palette/pattern as a fallback)

You can freely use colorModes from segDrawUtils::setPixelColor(), but they don't make much sense
unless you are running an offset in the SegmentSetPS or using colorModes 5 or 6.

Example calls: 
    uint8_t pattern_arr = {0, 2, 1};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};
    DissolveSL dissolve(mainSegments, pattern, cybPnkPal_PS, 0, 150, 70);
    Will dissolve from color 0 in the palette to color 2, to color 1, etc using randMode 0 (see below) 
    with the number of leds set on one cycle increasing every 150ms with the effect updating every 70ms

    DissolveSL dissolve(mainSegments, cybPnkPal_PS, 4, 100, 100);
    Will dissolve from one palette color to the next using randMode 4 (see below) 
    with the number of leds set on one cycle increasing every 100ms with the effect updating every 100ms

    DissolveSL dissolve(mainSegments, 3, 150, 70);
    Will dissolve using random colors set according to randMode 3
    (use randMode 2 or 3 with this constructor)
    with the number of leds set on one cycle increasing every 150ms with the effect updating every 70ms

Constructor Inputs
    pattern(optional, see constructors) -- A pattern is struct made from a 1-d array of palette indexes ie {0, 1, 3, 6, 7} 
                                          and the length of the array 
                                          (see patternPS.h)   
    palette(optional, see constructors) -- The repository of colors used in the pattern, or can be used as the pattern itself
    randMode -- The randMode that will be used for the dissolves (see above)
    spawnRateInc -- The rate increase at which the total number of lines that are set each cycle (ms)
                    Setting this close-ish (up to double?) to the update rate looks the best
    rate -- update rate (ms)

Other Settings:
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
                                
Functions:
    setPaletteAsPattern() -- Sets the effect pattern to match the current palette
    resetPixelArray() -- Resets the dissolved state of the segment lines, effectively restarting the current dissolve.
    setLineMode(newLineMode) -- Sets the line mode (see intro LineMode notes), 
                                also restarts the dissolve, and sets the setAllThreshold to 1/10 the numLines
    update() -- updates the effect

Reference vars:
    dissolveCount -- The number of dissolves we've done  (does not reset automatically).
    numCycles -- How many dissolves we've finished (resets when we've gone through the whole pattern).
    lineMode (default true) -- (See intro lineMode notes) !!FOR reference only, set using setLineMode().

Flags:
    paused -- If true, then the effect is paused. Note that the effect is not re-draw while paused.

Notes:
    Requires an array of bools of equal size to the number of pixels in the segment set
    So be careful of your memory usage 
*/
class DissolveSL : public EffectBasePS {
    public:
        //constructor for pattern
        DissolveSL(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, uint8_t RandMode,
                   uint16_t SpawnRateInc, uint16_t Rate);

        //constructor for palette as pattern
        DissolveSL(SegmentSetPS &SegSet, palettePS &Palette, uint8_t RandMode, uint16_t SpawnRateInc, uint16_t Rate);

        //constructor for randomly chosen colors (should only use randMode 2 or 3 with this constructor)
        DissolveSL(SegmentSetPS &SegSet, uint8_t RandMode, uint16_t SpawnRateInc, uint16_t Rate);

        ~DissolveSL();

        uint8_t
            randMode,
            colorMode = 0,
            maxNumSpawnBase = 1;

        uint16_t
            setAllThreshold,
            pauseTime = 0,
            spawnRateInc,
            dissolveCount = 0,  //The number of full dissolves we've done
            numCycles = 0;      //how many update cycles we've been through (resets when we've gone through the whole pattern)

        bool
            lineMode = true,  //for reference, set using setLineMode()
            paused = false,
            *pixelArray = nullptr;

        patternPS
            *pattern = nullptr,
            patternTemp = {nullptr, 0, 0};  //Must init structs w/ pointers set to null for safety

        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0};  //Must init structs w/ pointers set to null for safety

        void
            setPaletteAsPattern(),
            resetPixelArray(),
            setLineMode(bool newLineMode),
            update(void);

    private:
        unsigned long
            currentTime,
            prevSpawnTime = 0,
            prevTime = 0;

        uint8_t
            currentIndex = 0;

        uint16_t
            thresStartPoint = 0,
            numLines,
            maxNumSpawn,  //How many lines we'll try to spawn each cycle (starts as maxNumSpawnBase and increases with time)
            maxNumLines = 0,  //used for tracking the memory size of the pixelArray
            lineNum,
            numSpawned = 0,
            pixelNum;

        bool
            randColorPicked = false;

        CRGB
            pickColor(),
            color;

        void
            init(SegmentSetPS &SegSet, uint16_t Rate),
            spawnLed(uint16_t pixelNum);
};

#endif
#ifndef DissolveSLPS_h
#define DissolveSLPS_h

//TODO
//  --Change bool array to uint8_t's where the bools are stored as the bits of the uint8_t's
//    Will have to caculate the length of the array and write function for getting/setting
//  --Add a set number of dissolves?, like CrossFadeCycle

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

/* 
Morphs the segment set from one color to the next by setting each segment line one at a time at random
Includes various options for color selection (see modes) 
Colors can be choosen using a pallet and pattern, or selected randomly
The effect can be accelerated to set more lines at once by adjusting spawnRateInc
By default we start by spawning one segment line at a time, increasing the number every spawnRateInc ms
so the spawing steadily accelerates. This helps keep the spawing consistent, since we're picking at random
Once a certain threshold has been met (default of 1/10 of the number of lines in the segment set remaining)
All the remaining lines are set. This prevents us from getting stuck looking for the last line.
Once a morph is finished, the effect can be set to pause for a period using the hangTime variable.

You can increase the starting number of lines set at once (numMaxSpawnBase), which will
acclerate the morphing, and may be good on longer segment sets.

The effect is adapted to work on segment lines for 2D use, but you can keep it 1D by
passing in a segmentSet with only one segment containing the whole strip.

Each segment line will be filled in, rather than each pixel.

Example calls: 
    uint8_t pattern_arr = {0, 1, 4};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr)};
    DissolveSLPS(mainSegments, &pattern, pallet, 0, 150, 70);
    Will dissolve from color 0 in the pallet to color 1, to color 4, etc using dMode 0 (see below) 
    with the number of leds set on one cycle increasing every 150ms with the effect updating every 70ms

    DissolveSLPS(mainSegments, pallet, 4, 100, 100);
    Will dissolve from one pallet color to the next using dMode 4 (see below) 
    with the number of leds set on one cycle increasing every 100ms with the effect updating every 100ms

    DissolveSLPS(mainSegments, 3, 150, 70);
    Will dissolve using random colors set according to dMode 3
    (use dMode 2 or 3 with this constructor)
    with the number of leds set on one cycle increasing every 150ms with the effect updating every 70ms

dModes:
    0: Each dissolve is a solid color following the pattern
    1: Each dissolve is a set of random colors choosen from the pattern
    2: Each dissolve is a set of randomly choosen colors
    3: Each dissolve is a solid color choosen at random
    4: Each dissolve is a solid color choosen randomly from the pattern

You should be able switch freely between dModes on the fly (the random modes will set up a random pallet/pattern as a fallback)

You can freely use colorModes from segDrawUtils::setPixelColor(), but they don't make much sense
unless you are running an offset in the segmentSet or using colorModes 5 or 6.

Constructor Inputs
    pattern(optional, see constructors) -- A pattern is struct made from a 1-d array of pallet indexes ie {0, 1, 3, 6, 7} 
                                          and the length of the array 
                                          (see patternPS.h)   
    pallet(optional, see constructors) -- The repository of colors used in the pattern, or can be used as the pattern itself
    dMode -- The dMode that will be used for the dissolves (see above)
    spawnRateInc -- The rate increase at which the total number of leds that are set each cycle (ms)
                   Setting this closeish (up to double?) to the update rate looks the best
    Rate -- update rate (ms)

Functions:
    setPalletAsPattern() -- Sets the effect pattern to match the current pallet
    resetPixelArray() -- effectivly restarts the current dissolve
    update() -- updates the effect

Other Settings:
    setAllThreshold (defaults to 1/10th of the segment set length) -- The number of leds that will be set randomly
                                                                     Before they are set in order
                                                                     This is a fail safe so that you don't get stuck with one led that is never
                                                                     randomly picked, so the dissolve doesn't end
    hangTime (default 0ms) -- The length of time that the effect will wait between dissolves, useful for adjusting the next dissolve color/settings
                             If the hang time is active, it is indicated with the hangTimeOn flag
    colorMode (default 0) -- sets the color mode for the random pixels (see segDrawUtils::setPixelColor)
    numMaxSpawnBase (default 1) -- The starting value of the number of pixels set in one cycle
                                  Higher numbers may work better for longer pixel lengths

Notes:
    Requires an array of bools of equal size to the number of pixels in the segment set
    So be careful of your memory usage 
*/
class DissolveSLPS : public EffectBasePS {
    public:
        //constructor for pattern
        DissolveSLPS(SegmentSet &SegmentSet, patternPS *Pattern, palletPS *Pallet, uint8_t DMode, uint16_t SpawnRateInc, uint16_t Rate); 

        //constructor for pallet as pattern
        DissolveSLPS(SegmentSet &SegmentSet, palletPS *Pallet, uint8_t DMode, uint16_t SpawnRateInc, uint16_t Rate); 
        
        //constructor for randomly choosen colors (should only use dMode 2 or 3 with this constructor)
        DissolveSLPS(SegmentSet &SegmentSet, uint8_t DMode, uint16_t SpawnRateInc, uint16_t Rate);
    
        ~DissolveSLPS();

        uint16_t
            setAllThreshold,
            hangTime = 0,
            spawnRateInc;
        
        uint8_t
            dMode,
            colorMode = 0,
            numCycles = 0, //how many update cycles we've been through, for reference
            numMaxSpawn,
            numMaxSpawnBase = 1;

        bool
            randColorPicked = false,
            hangTimeOn = false,
            *pixelArray;

        SegmentSet 
            &segmentSet; 
        
        patternPS
            patternTemp,
            *pattern;

        palletPS
            palletTemp,
            *pallet;
        
        void
            setPalletAsPattern(),
            resetPixelArray(),
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
            lineNum,
            numSpawned = 0,
            pixelNum;
        
        CRGB 
            pickColor(),
            color;

        void
            init(uint16_t Rate),
            spawnLed(uint16_t pixelNum);
};

#endif
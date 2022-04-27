#ifndef DissolvePS_h
#define DissolvePS_h

//TODO
//  --Change bool array to uint8_t's where the bools are stored as the bits of the uint8_t's
//    Will have to caculate the length of the array and write function for getting/setting
//  --Add a set number of dissolves?, like CrossFadeCycle

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

//Morphs the segment set from one color to the next by setting each led one at a time at random
//Includes various options for color selection (see modes) 
//Colors can be choosen using a pallet and pattern, or selected randomly
//The effect can be accelerated to set more leds at once by adjusting spawnRateInc
//Once a morph is finished, the effect can be set to pause for a period using the hangTime variable

//Example calls: 
    //uint8_t pattern_arr = {0, 1, 4};
    //patternPS pattern = {pattern_arr, SIZE(pattern_arr)};
    //DissolvePS(mainSegments, &pattern, pallet, 0, 150, 70);
    //Will dissolve from color 0 in the pallet to color 1, to color 4, etc using dMode 0 (see below) 
    //with the number of leds set on one cycle increasing every 150ms with the effect updating every 70ms

    //DissolvePS(mainSegments, pallet, 4, 100, 100);
    //Will dissolve from one pallet color to the next using dMode 4 (see below) 
    //with the number of leds set on one cycle increasing every 100ms with the effect updating every 100ms

    //DissolvePS(mainSegments, 3, 150, 70);
    //Will dissolve using random colors set according to dMode 3
    //(use dMode 2 or 3 with this constructor)
    //with the number of leds set on one cycle increasing every 150ms with the effect updating every 70ms

//dModes:
    //0: Each dissolve is a solid color following the pattern
    //1: Each dissolve is a set of random colors choosen from the pattern
    //2: Each dissolve is a set of randomly choosen colors
    //3: Each dissolve is a solid color choosen at random
    //4: Each dissolve is a solid color choosen randomly from the pattern

//You should be able switch freely between dModes on the fly (the random modes will set up a random pallet/pattern as a fallback)

//You can freely use colorModes from segDrawUtils::setPixelColor(), but they don't make much sense
//unless you are also using a rainbowOffsetCycle()

//Constructor Inputs
    //Pattern(optional, see constructors) -- A pattern is struct made from a 1-d array of pallet indexes ie {0, 1, 3, 6, 7} 
    //                                       and the length of the array 
    //                                       (see patternPS.h)   
    //Pallet(optional, see constructors) -- The repository of colors used in the pattern, or can be used as the pattern itself
    //DMode -- The dMode that will be used for the dissolves (see above)
    //SpawnRateInc -- The rate increase at which the total number of leds that are set each cycle (ms)
    //                Setting this closeish to the update rate looks the best
    //Rate -- update rate (ms)

//Functions:
    //setPattern(*newPattern) -- Sets the passed in pattern to be the effect pattern
    //setPallet(*newPallet) -- Sets the pallet to the passed in pallet
    //setPalletAsPattern() -- Sets the effect pattern to match the current pallet
    //resetPixelArray() -- effectivly restarts the current dissolve
    //update() -- updates the effect

//Other Settings:
    //setAllThreshold (defaults to 1/10th of the segment set length) -- The number of leds that will be set randomly
    //                                                                  Before they are set in order
    //                                                                  This is a fail safe so that you don't get stuck with one led that is never
    //                                                                  randomly picked, so the dissolve doesn't end
    //hangTime (default 0ms) -- The length of time that the effect will wait between dissolves, useful for adjusting the next dissolve color/settings
    //                          If the hang time is active, it is indicated with the hangTimeOn flag
    //colorMode (default 0) -- sets the color mode for the random pixels (see segDrawUtils::setPixelColor)
    //numMaxSpawnBase (defaaut 1) -- The starting value of the number of pixels set in one cycle
    //                               Higher numbers may work better for longer pixel lengths

//Notes:
    //Requires an array of bools of equal size to the number of pixels in the segment set
    //So be careful of your memory usage
class DissolvePS : public EffectBasePS {
    public:
        DissolvePS(SegmentSet &SegmentSet, patternPS *Pattern, palletPS *Pallet, uint8_t DMode, uint16_t SpawnRateInc, uint16_t Rate); 

        DissolvePS(SegmentSet &SegmentSet, palletPS *Pallet, uint8_t DMode, uint16_t SpawnRateInc, uint16_t Rate); 
        
        DissolvePS(SegmentSet &SegmentSet, uint8_t DMode, uint16_t SpawnRateInc, uint16_t Rate);
    
        ~DissolvePS();

        uint16_t
            setAllThreshold,
            hangTime = 0,
            spawnRateInc;
        
        uint8_t
            dMode,
            colorMode = 0,
            numCycles = 0,
            numMaxSpawn,
            numMaxSpawnBase = 1;

        bool
            randColorPicked = false,
            hangTimeOn = false,
            *pixelArray;

        SegmentSet 
            &segmentSet; 
        
        patternPS
            *pattern;

        palletPS
            *pallet;
        
        void
            setPattern(patternPS *newpattern),
            pickColor(),
            setPallet(palletPS *newPallet),
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
            numActiveLeds,
            numSpawned = 0,
            pixelNum;
        
        CRGB 
            color;

        patternPS
            patternTemp;

        palletPS
            palletTemp;
        
        void
            init(uint16_t Rate),
            spawnLed(uint16_t pixelNum);
};

#endif
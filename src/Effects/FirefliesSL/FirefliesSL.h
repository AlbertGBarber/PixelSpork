#ifndef FirefliesSL_h
#define FirefliesSL_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"
#include "Effects/ParticlesSL/Particle_Stuff/particleUtilsPS.h"

/* 
A 1D simulation of fireflies: a set of twinkling particles moving in random patterns across the strip
You have options for the number of particles, how fast they move, how long the live for and how often they spawn
Firefly colors can be set to a single color or picked randomly from a palette
You also have the option of having the constructor make a random palette for you

In general you can change most variables on the fly except for maxnumFireflies
You can also turn the twinking off

This effect is fully compatible with color modes, and the bgColor is a pointer, so you can bind it
to an external color variable. By default it is bound to bgColorOrig, which is set to 0 (blank color).

If you have a non-zero background color be sure to set fillBG to true!!

The effect is adapted to work on segment lines for 2D use, but you can keep it 1D by
passing in a segmentSet with only one segment containing the whole strip.

Due to the way the effect is programmed, if two flies meet each other, by default one will overwrite the other
You can adjust this behavior by turning on "blend", which will add particle colors together as they pass by each other
However this does have one draw backs:
1: For colored backgrounds, the particles colors are added to the background colors.
  This will in most cases significantly change the particle colors 
  For example, blue particles running on a red background will appear purple (blue +  red = purple)
  Overall I do not recommend using blend for colored backgrounds
2: Using blend on segment sets with lines of un-equal length may look weird, because
  pixels may be added to many times where multiple lines converge/overlap

Note that this effect does require three seperate arrays: 
    A particleSet with a particle array of size maxNumFireflies
    A CRGB array of trailEndColors[maxNumFireflies] to store the trail color for each particle
    A uint16_t array of particlePrevPos[maxNumFireflies] to store the previous particle locations
So watch your memory usage

Inputs guide:
To get a good variety of fireflies you want to add a good amount of range for each firefly's settings
Fireflies rely on the FastLED noise functions for their movement, 
They produce a noise output based on a few inputs, however, same inputs => same outputs, 
so two fireflys with the same settings, we look and move exactly the same
To prevent this fireflies are always spawned with a set of random values based on a set of range settings
But using small ranges, will result in similar looking flies.
A firefly has 5 core inputs:
    spawnChance: How likely an inactive firefly is to spawn each update cycle (a percent out of 100)
    lifeBase: The minimum time (in ms) a firefly will live for
    lifeRange: Sets the maxmium possible ammount added to lifeBase. 
              The overall firefly life is caculated as lifeBase + random(lifeRange);
    speedBase: The slowest speed a particle will move (this is a multiplier, see more below)
    speedRange: Sets the maxmium possible ammount added to speedBase. 
              The overall firefly speed is caculated as speedBase + random(speedRange);
There are some other, secondary variables listed in the Other Settings section below,
but you probably won't need to tweak these initally

Lifebase is used to set minium limit on all the fireflies life, but you still want to give them time to move
So lifeBase should probably be 2000+ (2 sec), while the range should be a bit larger than the base ie 3000ish
A larger life range will help give your freflies more variation.
SpeedBase and speedRange do most of the work in seperating fireflies, but the ranges for this is fairly small
I recommend starting with a combined (speedBase + speedRange) of (1 - 15) Any higher, and your fireflies may move too fast
I'd set my range to at least 5 to get a good variation. If you set it too low fireflies will tend to move together.

Example calls: 

    FirefliesSL(mainSegments, CRGB(222, 144, 9), 15, 5, 2000, 3000, 3, 6, 80);
    Will do a set of fireflies using a yellow-orange color
    There are a maximum of 15 fireflies active at one time, and each has a 5 percent chance of spawning per cycle
    The fireflies have a base life of 2000ms, with a range of 3000ms (for a max life of 5000ms)
    The fireflies have a base speed of 3, with a range of 6 (for a max speed of 9)
    The effect updates at 80ms
    !!When using a single color constructor, if you use a pre-build FastLED color make sure you pass in as
    CRGB(*colorCode*), ex CRGB(CRGB::Red)
    Otherwise it will get confused and call the random palette constructor

    FirefliesSL(mainSegments, &palette1, 5, 20, 3000, 4000, 6, 14, 70);
    Will do a set of fireflies using colors from palette1
    There are a maximum of 5 fireflies active at one time, and each has a 20 percent chance of spawning per cycle
    The fireflies have a base life of 3000ms, with a range of 4000ms (for a max life of 7000ms)
    The fireflies have a base speed of 6, with a range of 14 (for a max speed of 20)
    The effect updates at 70ms

    FirefliesSL(mainSegments, 2, 20, 5, 3000, 3000, 2, 5, 80);
    Will do a set of fireflies using a random palette of 2 colors
    There are a maximum of 20 fireflies active at one time, and each has a 5 percent chance of spawning per cycle
    The fireflies have a base life of 3000ms, with a range of 3000ms (for a max life of 6000ms)
    The fireflies have a base speed of 2, with a range of 5 (for a max speed of 7)
    The effect updates at 80ms

Constructor Inputs:
    palette(optional, see constructors) -- The set of colors that firefly colors will be picked from
    numColors (optional, see contructors) -- The number of randomly choosen colors for fireflys
    color (optional, see contructors) -- A single color for all the fireflys
    maxNumfireflies -- The maximum number of simultaneous fireflies  that can be active at one time
    spawnChance -- How likely an inactive firefly is to spawn each update cycle (a percent out of 100)
    lifeBase -- The minimum time (in ms) a firefly will live for
    lifeRange -- Sets the maxmium possible ammount added to lifeBase. (ms)
                 The overall firefly life is caculated as lifeBase + random(lifeRange);
    speedBase -- The slowest speed a particle will move (this is a multiplier, see more below) (ms)
    speedRange -- Sets the maxmium possible ammount added to speedBase. (ms)
                  The overall firefly speed is caculated as speedBase + random(speedRange);
    rate -- The update rate for the effect (ms)

Functions:
    setupfireflies(maxNumfireflies) -- Create the data structures for a set of fireflies
                                       You should call this if you ever want to change maxNumfireflies
    update() -- updates the effect

Other Settings:
    colorMode (default 0) -- sets the color mode for the waves (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the spacing pixels (see segDrawUtils::setPixelColor)
    fillBG (default false) -- Sets whether to fill the background in every update cycle,
                              You should set this true if you are using an animated background mode
    blend (default false) -- Causes sparks to add their colors to the strip, rather than set them
                             See explanation of this in more detail above in effect intro
    flicker (default true) -- Turns the firefly brightness flicker on or off
    bgColorOrig (default 0) -- The default color of the background (bound to the bgColor pointer by default)
    *bgColor (default bound to bgColorOrig) -- The color of the background, is a pointer so it can be bound to an external variable 
    fadeThresh (default 50) -- The number of steps (out of 255) for the flyflies to fade in or out
                               The fireflies spend 255 steps in total fading in, staying at a peak color, then fading out
                               The value of fadeThresh reduces how long the they will stay at peak color
                               Note that setting it to 255 will only doa fade in, and then the fireflys will disappear instantly
                               128 will cause them to fade in then out with no pause at the peak 

Reference Vars:
    maxNumFireflies -- The maximum number of fireflies that can be active at any one time
                       call setupFireflies() to change
*/
class FirefliesSL : public EffectBasePS {
    public:
        //Constructor for effect with palette
        FirefliesSL(SegmentSet &SegmentSet, palettePS &Palette, uint8_t MaxNumFireflies, uint8_t SpawnChance, 
                    uint16_t LifeBase, uint16_t LifeRange, uint16_t SpeedBase, uint16_t SpeedRange, uint16_t Rate);

        //Constructor for effect with palette of random colors
        FirefliesSL(SegmentSet &SegmentSet, uint8_t numColors, uint8_t MaxNumFireflies, uint8_t SpawnChance, 
                    uint16_t LifeBase, uint16_t LifeRange, uint16_t SpeedBase, uint16_t SpeedRange, uint16_t Rate); 

        //constructor for effect with single color
        //!!If using pre-build FastLED colors you need to pass them as CRGB( *color code* ) 
        FirefliesSL(SegmentSet &SegmentSet, CRGB Color, uint8_t MaxNumFireflies, uint8_t SpawnChance, 
                    uint16_t LifeBase, uint16_t LifeRange, uint16_t SpeedBase, uint16_t SpeedRange, uint16_t Rate);

        ~FirefliesSL();

        SegmentSet 
            &segmentSet; 

        uint8_t
            spawnChance,
            colorMode = 0,
            bgColorMode = 0,
            fadeThresh = 50,
            maxNumFireflies = 0; //For reference only!, call setupFireflies() to change
        
        uint16_t
            lifeBase, 
            lifeRange,
            speedBase,
            speedRange;

        bool
            fillBG = false,
            blend = false,
            flicker = true;

        CRGB 
            bgColorOrig = 0, //default background color (blank)
            *bgColor = &bgColorOrig; //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)

        palettePS
            paletteTemp,
            *palette = nullptr;

        particleSetPS 
            *particleSet = nullptr, //the particle set used in the effect
            particleSetTemp; //storage for self created particle sets

        void 
            setupFireflies(uint8_t newMaxNumFireflies),
            update(void);
    
    private:
        
        unsigned long
            currentTime,
            prevTime = 0;

        uint8_t
            lifeRatio,
            flickerBri,
            fadeMax,
            fadeType,
            dimRatio;

        uint16_t 
            partLife,
            partPos,
            deltaTime,
            numLines,
            numSegs,
            pixelNum,
            longestSeg,
            *particlePrevPos = nullptr;
        
        particlePS
            *particlePtr = nullptr;
            
        CRGB 
            *trailEndColors = nullptr, //used to store the last colors of each trail, so the background color can be set
            colorFinal,
            colorOut,
            bgCol;
        
        void
            init(uint8_t maxNumFireflies, uint16_t Rate),
            moveParticle(particlePS *particlePtr, uint16_t partNum),
            drawParticlePixel(particlePS *particlePtr, uint16_t partNum),
            spawnFirefly(uint8_t fireflyNum);
};
#endif
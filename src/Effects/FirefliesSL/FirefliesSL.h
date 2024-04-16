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

In general you can change most variables on the fly except for maxNumFireflies

This effect is fully compatible with color modes, and the bgColor is a pointer, so you can bind it
to an external color variable. By default it is bound to bgColorOrig, which is set to 0 (blank color).

By default the background color is black (0). If you set a non-zero background color be sure to set `fillBg` to true, 
otherwise each pixel will only be filled after a firefly touches it.

The effect is adapted to work on segment lines for 2D use. 
Note that the firefly simulation is _1D_, using a 2D segment set will cause each firefly to drawn on a 
whole segment line (which still looks neat, but is not the same as a full 2D set of fireflies).

Due to the way the effect is programmed, if two flies meet each other, by default one will overwrite the other
You can adjust this behavior by turning on "blend", which will add particle colors together as they pass by each other
However this does have one draw backs:
1: For colored backgrounds, the particles colors are added to the background colors.
  This will, in most cases significantly change the particle colors 
  For example, blue particles running on a red background will appear purple (blue +  red = purple)
  Overall I do not recommend using blend for colored backgrounds
2: Using blend on segment sets with lines of unequal length may look weird, because
  pixels may be added to many times where multiple lines converge/overlap

Note that this effect does require three separate arrays: 
    A particleSet with a particle array of size maxNumFireflies
    A CRGB array of trailEndColors[maxNumFireflies] to store the trail color for each particle
    A uint16_t array of particlePrevPos[maxNumFireflies] to store the previous particle locations
So watch your memory usage

Inputs guide:
    To get a good variety of fireflies you want to add a good amount of range for each firefly's settings
    Fireflies rely on the FastLED noise functions for their movement, 
    The functions produce a noise output based on a few inputs, however, same inputs => same outputs, 
    so two fireflies with the same settings, we look and move exactly the same
    To prevent this fireflies are always spawned with a set of random values based on a set of range settings
    But using small ranges, will result in similar looking flies.
    A firefly has 5 core inputs:
        spawnChance: How likely an inactive firefly is to spawn each update cycle (a percent out of 100)
        lifeBase: The minimum time (in ms) a firefly will live for
        lifeRange: Sets the maximum possible amount added to lifeBase. 
                   The overall firefly life is calculated as lifeBase + random(lifeRange);
        speedBase: The slowest speed a particle will move (this is a multiplier, see more below)
        speedRange: Sets the maximum possible amount added to speedBase. 
                    The overall firefly speed is calculated as speedBase + random(speedRange);
    There are some other, secondary variables listed in the Other Settings section below,
    but you probably won't need to tweak these initially

    LifeBase is used to set minimum limit on all the fireflies life, but you still want to give them time to move
    So lifeBase should probably be 2000+ (2 sec), while the range should be a bit larger than the base ie 3000ish
    A larger life range will help give your fireflies more variation.

    "speedBase" and "speedRange" do most of the work in separating fireflies.
    They work by multiplying the noise input, placing the fireflies on different noise paths. 
    So the "speed" isn't a time, but more like a scale of motion.
    A "good" speed and range vary a lot depending on how large your segment set is.
    I recommend starting with a speed of 10 and a range of 8.
    Make sure your range is at a good portion of your base speed,
    If you set it too low fireflies will tend to move together.

    Spawning:
        A firefly will spawn if random(spawnBasis) <= spawnChance. spawnBasis is default to 1000, so you can go down
        to sub 1% percentages. Note that the effect tries to spawn any inactive firefly with each update().
        This means that how densely your fireflies spawn depends a lot on the
        effect update rate and how many fireflies you have. 
        Even with quite low percentages, fireflies will probably spawn quite often.

        Fireflies fade in and out as they spawn and de-spawn. The percentage of their life that they spend fading 
        is controlled by `fadeThresh`, defaulted to 50 out of 255. 
        See the "Other Settings" entry below for more details.

    Flickering:
        By default, the fireflies vary their brightness randomly to "flicker". 
        You can disable this by setting `flicker` to false.

Example calls: 
    FirefliesSL fireflies(mainSegments, CRGB(222, 144, 9), 15, 50, 2000, 3000, 8, 5, 80);
    Will do a set of fireflies using a yellow-orange color
    There are a maximum of 15 fireflies active at one time, and each has a 5 percent chance of spawning per cycle (50/1000)
    The fireflies have a base life of 2000ms, with a range of 3000ms (for a max life of 5000ms)
    The fireflies have a base speed of 8, with a range of 5 (for a max speed of 13)
    The effect updates at 80ms
    !!When using a single color constructor, if you use a pre-build FastLED color make sure you pass in as
    CRGB(*colorCode*), ex CRGB(CRGB::Red)
    Otherwise it will get confused and call the random palette constructor

    FirefliesSL fireflies(mainSegments, cybPnkPal_PS, 5, 200, 3000, 4000, 6, 14, 70);
    Will do a set of fireflies using colors from the cybPnkPal_PS palette
    There are a maximum of 5 fireflies active at one time, and each has a 20 percent chance of spawning per cycle (200/1000)
    The fireflies have a base life of 3000ms, with a range of 4000ms (for a max life of 7000ms)
    The fireflies have a base speed of 6, with a range of 14 (for a max speed of 20)
    The effect updates at 70ms

    FirefliesSL fireflies(mainSegments, 2, 20, 50, 3000, 3000, 4, 5, 80);
    Will do a set of fireflies using a random palette of 2 colors
    There are a maximum of 20 fireflies active at one time, and each has a 5 percent chance of spawning per cycle (50/1000)
    The fireflies have a base life of 3000ms, with a range of 3000ms (for a max life of 6000ms)
    The fireflies have a base speed of 4, with a range of 5 (for a max speed of 9)
    The effect updates at 80ms

Constructor Inputs:
    palette(optional, see constructors) -- The set of colors that firefly colors will be picked from
    numColors (optional, see constructors) -- The number of randomly chosen colors for fireflies
    color (optional, see constructors) -- A single color for all the fireflies
    maxNumFireflies -- The maximum number of simultaneous fireflies  that can be active at one time
    spawnChance -- How likely an inactive firefly is to spawn each update cycle (a percent out of 100)
    lifeBase -- The minimum time (in ms) a firefly will live for
    lifeRange -- Sets the maximum possible amount added to lifeBase. (ms)
                 The overall firefly life is calculated as lifeBase + random(lifeRange);
    speedBase -- The slowest speed a particle will move (this is a multiplier, see more below) (ms)
    speedRange -- Sets the maximum possible amount added to speedBase. (ms)
                  The overall firefly speed is calculated as speedBase + random(speedRange);
    rate -- The update rate for the effect (ms)

Other Settings:
    colorMode (default 0) -- sets the color mode for the waves (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the spacing pixels (see segDrawUtils::setPixelColor)
    fillBg (default false) -- Sets whether to fill the background in every update cycle,
                              You should set this true if you are using an animated background mode
    blend (default false) -- Causes sparks to add their colors to the strip, rather than set them
                             See explanation of this in more detail above in effect intro
    flicker (default true) -- Turns the firefly brightness flicker on or off
    bgColorOrig (default 0) -- The default color of the background (bound to the bgColor pointer by default)
    *bgColor (default bound to bgColorOrig) -- The color of the background, is a pointer so it can be bound to an external variable 
    fadeThresh (default 50) -- The number of steps (out of 255) for the fireflies to fade in or out
                               The fireflies spend 255 steps in total fading in, staying at a peak color, then fading out
                               The value of `fadeThresh` sets how many steps they spend fading
                               Note that setting it to 255 will only do fade in, and then the fireflies will disappear instantly
                               128 will cause them to fade in then out with no pause at the peak 
    spawnBasis (default 1000) -- The spawn probability threshold. 
                                 A firefly will spawn if "random(spawnBasis) <= spawnChance".
                            
Functions:
    setupFireflies(maxNumFireflies) -- Create the data structures for a set of fireflies
                                       You should call this if you ever want to change maxNumFireflies
                                       Will also clear any active fireflies from the segment set by filling in the background
    update() -- updates the effect

Reference Vars:
    maxNumFireflies -- The maximum number of fireflies that can be active at any one time
                       call setupFireflies() to change
*/
class FirefliesSL : public EffectBasePS {
    public:
        //Constructor for effect with palette
        FirefliesSL(SegmentSetPS &SegSet, palettePS &Palette, uint16_t MaxNumFireflies, uint16_t SpawnChance,
                    uint16_t LifeBase, uint16_t LifeRange, uint16_t SpeedBase, uint16_t SpeedRange, uint16_t Rate);

        //Constructor for effect with palette of random colors
        FirefliesSL(SegmentSetPS &SegSet, uint8_t numColors, uint16_t MaxNumFireflies, uint16_t SpawnChance,
                    uint16_t LifeBase, uint16_t LifeRange, uint16_t SpeedBase, uint16_t SpeedRange, uint16_t Rate);

        //constructor for effect with single color
        //!!If using pre-build FastLED colors you need to pass them as CRGB( *color code* ) -> ex CRGB(CRGB::Blue)
        FirefliesSL(SegmentSetPS &SegSet, CRGB Color, uint16_t MaxNumFireflies, uint16_t SpawnChance,
                    uint16_t LifeBase, uint16_t LifeRange, uint16_t SpeedBase, uint16_t SpeedRange, uint16_t Rate);

        ~FirefliesSL();

        uint8_t
            colorMode = 0,
            bgColorMode = 0,
            fadeThresh = 50;

        uint16_t
            spawnChance,
            spawnBasis = 1000, //spawn change scaling (random(spawnBasis) <= spawnChance controls spawning)
            maxNumFireflies = 0,  //For reference only!, call setupFireflies() to change
            lifeBase,
            lifeRange,
            speedBase,
            speedRange,
            *particlePrevPos = nullptr;

        bool
            fillBg = false,
            blend = false,
            flicker = true;

        CRGB
            bgColorOrig = 0,            //default background color (blank)
            *bgColor = &bgColorOrig,    //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)
            *trailEndColors = nullptr;  //used to store the last colors of each trail, so the background color can be set

        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0};  //Must init structs w/ pointers set to null for safety

        particleSetPS
            *particleSet = nullptr,          //the particle set used in the effect
            particleSetTemp = {nullptr, 0};  //storage for self created particle sets (init to an empty set for safety)

        void
            setupFireflies(uint16_t newMaxNumFireflies),
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
            longestSeg;

        particlePS
            *particlePtr = nullptr;

        CRGB
            colorFinal,
            colorOut,
            bgCol;

        void
            init(uint16_t maxNumFireflies, SegmentSetPS &SegSet, uint16_t Rate),
            moveParticle(particlePS *particlePtr, uint16_t partNum),
            decayParticle(particlePS *particlePtr, uint16_t partNum),
            drawParticlePixel(particlePS *particlePtr, uint16_t partNum),
            spawnFirefly(uint16_t fireflyNum);
};
#endif
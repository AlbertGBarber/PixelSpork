#ifndef FireworksPS_h
#define FireworksPS_h

//TODO -- Add more options for setting the burst color
//        1. Let it be set to match firework color?
//        2. Make it match colorMode? -- Do 2 and 3 together in drawParticlePixel? (would need to modify the blend limit as well)

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"
#include "Effects/ParticlesSL/Particle_Stuff/particleUtilsPS.h"

/* 
A 1D firework simulation inspired heavily by https://www.reddit.com/r/arduino/comments/c3sd46/i_made_this_fireworks_effect_for_my_led_strips/
Spawns multiple fireworks across the strip that explode into a series of sparks
There are a lot of options for adjusting how the fireworks look
I suggest you look at the inputs guide below, or try out one of the constructor examples

Firework colors are picked randomly from a palette. 
You have the option of having the constructor make a random palette for you
By default all a firework's sparks are the same color, but you can set them all to be random (from the palette)
using randSparkColors.

All fireworks "burst" initially before fading to their spark color
The burst color is stored as burstColor, which is a pointer, so you can bind it to an eternal variable
By default it is bound to burstColOrig, which is set to white.

In general you can change most variables on the fly except for maxFireworks and maxNumSparks.

This effect is fully compatible with color modes.

The bgColor is a pointer, so you can bind it to an external color variable.
By default it is bound to bgColorOrig, which is set to 0 (blank color).

If you have a non-zero background color be sure to set fillBg to true!!

Due to the way the effect is programmed, if two sparks meet each other, by default one will overwrite the other
You can adjust this behavior by turning on "blend", which will add particle colors together as they pass by each other
However this does have two draw backs:
    1: Due to the way particle trails are drawn, this forces the background to be re-drawn each update cycle,
       which may have a performance impact depending on your strip length, update rate, etc
    2: For colored backgrounds, the particles colors are added to the background colors.
       This will in, most cases, significantly change the particle colors 
       For example, blue particles running on a red background will appear purple (blue +  red = purple)
       This can be used to create some nice effects, (like ocean-ish of lava-ish looking things),
       But overall I do not recommend using blend for colored backgrounds

Note that this effect does require three separate arrays: 
    A bool array fireworks[maxNumFireworks] that stores if a firework is active or not
    A particleSet with a particle array of size maxNumFireworks * (maxNumSparks + 1)
    A CRGB array of trailEndColors[maxNumFireworks * (maxNumSparks + 1)] to store the trail color for each particle
So watch your memory usage

Inputs guide:
    The fireworks have 7 core inputs:
        maxNumFireworks: The maximum number of fireworks that can exist on the strip simultaneously.
        maxNumSparks: The number of sparks each firework produces
        spawnChance: How likely an inactive firework is to spawn each update cycle (a percent out of 100)
        lifeBase: The maximum time (in ms) a firework's sparks will live for (asides from a bit or randomness from lifeRange)
        speedDecay: How fast the sparks slow down. This is a percent of their current speed -> speed = speed - speed * speedDecay
        speed: The base spark speed (speed is the update time in ms, so lower rate => higher speed)
        speedRange: The range added to a spark's base speed. A spark's speed is set when it spawns as `speed + random( speedRange )`.
    
   More Settings Details:
    * "lifeBase" sets the maximum spark life. You want to give the sparks time to move,
      so lifeBase should probably be 2000+ (2 sec). You should also know that the effect adds a random bit of extra 
      life to each spark, set by lifeRange (default 500, ie 0.5 sec)

    * "speedDecay" is the percent the speed decreases each time the spark is drawn. 
      I recommend starting between 5 - 20, and adjusting as needed. A lower value will let the sparks spread out more.

    * "speed and speedRange" do a lot of work in shaping how the firework looks. Overall, "speed" sets how fast the average spark moves,
      while speedRange sets how different spark speeds are from one another.
      So you can have low speeds and low ranges to make a slow, clumped up firework, or have high speeds and high range
      so that the sparks move quick and spread out more.
      I recommend starting with a speed of 40-60 and a speedRange of 300 - 600.
      For higher speed ranges, increasing the number of sparks will help "fill out" the firework.

    The fade rate of the sparks is proportional to their speed, so faster sparks fade slower.
    This seems produce a good look.

    Fireworks also have a central "bomb" particle, which doesn't move, and decays quickly.
    This makes the fireworks look more like an explosion.
    The settings for this spark all use the "center" in their name.

    There are some other, secondary variables, but you probably won't need to tweak these initially.

    Spawning:
        A firework will spawn if random(spawnBasis) <= spawnChance. spawnBasis is default to 1000, so you can go down
        to sub 1% percentages. Note that the effect tries to spawn any inactive fireworks with each update().
        This means that how densely your fireworks spawn depends a lot on the
        effect update rate and how many fireworks you have. Even with quite low percentages, 
        at the default update rate (see below), fireworks will probably spawn quite often.

        Note that to help avoid fireworks from spawning very close to either end of the strip, 
        their spawn range is reduced by numLEDs / spawnRangeDiv from each end of the strip.
        By default, spawnRangeDiv is 10. A higher spawnRangeDiv will expand the spawn area.

    Update Rate:
        Note that unlike other effects, the update rate is preset for you at 5ms, 
        which helps catch each of the spark's updates on time (hopefully!). 
        You see, each spark has its own speed (update rate), but we still want be able to update the 
        effect with a single `update()` call. I could have ignored the "rate" setting, 
        and just updated all the spark whenever you call `update()`. 
        However, the sparks are re-drawn every `update()`, even if they haven't moved, 
        so this becomes problematic, especially when working with multiple effects or segment sets, 
        where you want more control over when you update. Instead I opted to treat the effect as normal,
        and keep the overall update rate, but default it to a very fast 5ms. 
        Hopefully this default will catch most sparks on time. 
        You are free to change the update rate as needed by setting `rateOrig`.

Example calls: 

    FireworksPS fireworks(mainSegments, 5, 3, 10, 100, 2000, 20, 40, 300);
    Will do a set of fireworks using colors from a randomly generated palette of 5 colors
    There is a maximum of 3 fireworks active at one time.
    Each firework has 10 sparks
    There is a 10% chance a new firework spawns during an update (100/1000)
    The sparks have a maximum life of 2000ms (+ a random(500) from the default life range)
    The spark's speed decays at 20 percent per update
    The fastest particles will update at 40ms, while the slowest will be 40 + 300ms

    FireworksPS fireworks(mainSegments, cybPnkPal_PS, 3, 20, 100, 4000, 5, 40, 500);
    Big fireworks with long lasting sparks
    Will do a set of fireworks using colors from the cybPnkPal_PS palette
    There is a maximum of 3 fireworks active at one time.
    Each firework has 20 sparks
    There is a 10% chance a new firework spawns during an update (100/1000)
    The sparks have a maximum life of 4000ms (+ a random(500) from the default life range)
    The spark's speed decays at 5 percent per update
    The fastest particles will update at 40ms, while the slowest will be 40 + 500ms

    FireworksPS fireworks(mainSegments, CRGB(CRGB::White), 5, 4, 500, 2000, 10, 80, 80);
    Small, but fast fireworks on a shifting background
    fireworks.bgColorMode = 6; //Put in Arduino setup()
    fireworks.fillBg = true; //Put in Arduino setup()
    Will do a set of fireworks using white as the color
    There is a maximum of 5 fireworks active at one time.
    Each firework has 4 sparks
    There is a 50% chance a new firework spawns during an update (500/1000)
    The sparks have a maximum life of 2000ms (+ a random(500) from the default life range)
    The spark's speed decays at 10 percent per update
    The fastest particles will update at 80ms, while the slowest will be 80 + 80ms
    !!If using a pre-built FastLED color you need to pass it as CRGB( *color code* ) -> ex CRGB(CRGB::Blue)

Constructor Inputs:
    palette(optional, see constructors) -- The set of colors that firework colors will be picked from
    numColors (optional, see constructors) -- The number of randomly chosen colors for fireworks
    color (optional, see constructors) -- A single color for all the fireworks
                                          If using a pre-built FastLED color you need to pass is as CRGB( *color code* ) -> ex CRGB(CRGB::Blue)
    maxNumFireworks -- The maximum number of simultaneous fireworks that can be active at one time
    maxNumSparks -- The number of sparks spawned per firework
    spawnChance -- The likely-hood of a firework spawning (percent out of 100, larger is more likely)
    lifeBase -- The longest time a spark can exist (in ms)
    speedDecay -- The percent a spark's speed is reduced by per update cycle (out of 100)
    speed -- The base spark speed (speed is the update time in ms, so lower rate => higher speed).
    speedRange -- The cap for the slowest speed for the sparks ( speed is speed + random(speedRange) ) (ms)

Other Settings:
    colorMode (default 0) -- sets the color mode for the waves (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the spacing pixels (see segDrawUtils::setPixelColor)
    burstBlendLimit (default 100) -- The number of steps (out of 255) to blend from the burst color to the spark color                        
    lifeRange ( default 500 (ms) ) -- Added to the lifeBase as random(lifeRange) to set the maximum spark life
                                      Adds a bit of variation so not all the sparks decay at the same time at lower caps
    centerLife (default lifeBase/10 + 100 ) -- How long the central "bomb" lives for in ms
    size (default 1) -- The size of the sparks, only change this for big fireworks
    sizeRange (default 0) -- A random factor for setting the spark sizes. Spark sizes are calculated as `size + random(sizeRange)`.
                             Sparks will be size + random(sizeRange)
                             Only change this for big fireworks
    centerSize (default 3) -- How large the center "bomb" burst is 
    fillBg (default false) -- Sets whether to fill the background in every update cycle,
                              You should set this true if you are using an animated background mode
    blend (default false) -- Causes sparks to add their colors to the strip, rather than set them
                             See explanation of this in more detail above in effect intro
    randSparkColors (default false) -- If true, each spark will have its own color picked from the palette
    spawnBasis (default 1000) -- The spawn probability threshold. 
                                 A firework will spawn if "random(spawnBasis) <= spawnChance".
    spawnRangeDiv (default 10) -- Sets what range of the strip fireworks spawn in: from numLEDs / spawnRangeDiv to ( numLEDs - (numLEDs / spawnRangeDiv) )
    *burstColor and burstColOrig (default CRGB::White) -- The color of the initial firework burst, is a pointer. 
                                                          It is bound to the burstColOrig by default.
    *bgColor and bgColorOrig (default 0) -- The color of the background, is a pointer. 
                                            It is bound to the bgColorOrig by default.
    *rate and rateOrig -- Update rate (ms). Defaulted to 5ms (see Update note in Inputs Guide). 
                          Is a pointer, allowing you to bind it to an external variable. 
                          By default it's bound the effect's local variable, `rateOrig`. 

 Functions:
    setupFireworks(maxFireworks, maxSparks) -- Create the data structures for a set of fireworks
                                               You should call this if you ever want to change maxNumFireworks or maxNumSparks
                                               Will also clear any active fireworks from the segment set by filling in the background
    update() -- updates the effect

*/
class FireworksPS : public EffectBasePS {
    public:
        //Constructor for fireworks using a palette
        FireworksPS(SegmentSetPS &SegSet, palettePS &Palette, uint8_t MaxNumFireworks, uint8_t MaxNumSparks,
                    uint16_t SpawnChance, uint16_t LifeBase, uint8_t SpeedDecay, uint16_t Speed, uint16_t SpeedRange);

        //Constructor for fireworks using a palette of random colors
        FireworksPS(SegmentSetPS &SegSet, uint16_t numColors, uint8_t MaxNumFireworks, uint8_t MaxNumSparks,
                    uint16_t SpawnChance, uint16_t LifeBase, uint8_t SpeedDecay, uint16_t Speed, uint16_t SpeedRange);

        //Constructor for fireworks of a single color
        //!!If using a pre-built FastLED color you need to pass it as CRGB( *color code* ) -> ex CRGB(CRGB::Blue)
        FireworksPS(SegmentSetPS &SegSet, CRGB Color, uint8_t MaxNumFireworks, uint8_t MaxNumSparks,
                    uint16_t SpawnChance, uint16_t LifeBase, uint8_t SpeedDecay, uint16_t Speed, uint16_t SpeedRange);

        ~FireworksPS();

        uint8_t
            spawnRangeDiv = 10,  //sets what range of the strip fireworks spawn in: from numLEDs / spawnRangeDiv to (numLEDs - numLEDs / spawnRangeDiv)
            colorMode = 0,
            bgColorMode = 0,
            maxNumFireworks,
            maxNumSparks,
            burstBlendLimit = 100,
            speedDecay;

        uint16_t
            spawnChance,
            spawnBasis = 1000, //spawn change scaling (random(spawnBasis) <= spawnChance controls spawning)
            lifeBase,
            lifeRange = 500,
            centerLife,  //set to lifeBase/10 + 100 in init()
            speed,
            speedRange,
            size = 1,
            sizeRange = 0,
            centerSize = 3;  //how large the center "bomb" burst is

        bool
            fillBg = false,
            blend = false,
            randSparkColors = false,
            *fireWorkActive = nullptr;

        CRGB
            *trailEndColors = nullptr,    //used to store the last colors of each trail, so the background color can be set
            burstColOrig = CRGB::White,   //default burst color
            *burstColor = &burstColOrig,  //the burstColor is a pointer, so it can be tied to and external var if needed
            bgColorOrig = 0,              //default background color
            *bgColor = &bgColorOrig;      //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)

        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0};  //Must init structs w/ pointers set to null for safety

        particleSetPS
            *particleSet = nullptr,          //the particle set used in the effect
            particleSetTemp = {nullptr, 0, 0};  //storage for self created particle sets (init to an empty set for safety)

        void
            setupFireworks(uint8_t newMaxNumFireworks, uint8_t newMaxNumSparks),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0;

        int8_t
            directStep,
            trailDirectionAdj;

        uint8_t
            sizeAdj,
            dimRatio,
            randColorIndex;

        uint16_t
            particleIndex,
            partMaxLife,
            partLife,
            partPos,
            partSpeed,
            partSize,
            maxPosition,
            trailLedLocation,
            deltaTime,
            numLEDs,
            getTrailLedLoc(uint8_t trailPixelNum);

        bool
            partDirect,
            firstPart,
            movePart;  //flag for if a particle should move this cycle

        particlePS
            *particlePtr = nullptr;

        pixelInfoPS
            pixelInfo = {0, 0, 0, 0};

        CRGB
            colorFinal,
            colorOut,
            colorTemp;

        void
            init(uint8_t maxNumFireworks, uint8_t maxNumSparks, SegmentSetPS &SegSet),
            moveParticle(particlePS *particlePtr),
            drawParticlePixel(particlePS *particlePtr, uint16_t trailLedLocation),
            spawnFirework(uint8_t fireworkNum);
};

#endif
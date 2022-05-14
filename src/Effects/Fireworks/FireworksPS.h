#ifndef FireworksPS_h
#define FireworksPS_h

//TODO -- Add more options for setting the burst color
//        1. Make it a pointer so you can address it externally?
//        2. Let it be set to match firework color?
//        3. Make it match colorMode? -- Do 2 and 3 together in drawParticlePixel? (would need to modify the blend limit as well)

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"
#include "Effects/Particles/particleUtilsPS.h"

//A 1D firework simulation inpired heavily by https://www.reddit.com/r/arduino/comments/c3sd46/i_made_this_fireworks_effect_for_my_led_strips/
//Spawns multiple fireworks across the strip that explode into a series of sparks
//There are a lot of options for adjusting how the fireworks look
//I suggest you look at the inputs guide below, or try out one of the constructor examples

//Firework colors are picked randomly from a pallet. 
//You have the option of having the constructor make a random pallet for you
//By default all a firework's sparks are the same color, but you can set them all to be random (from the pallet)
//using randSparkColors

//By default, all fireworks "burst" as white initially before fading to their spark color
//The burst color is stored as burstColor, which you may change

//In general you can change most variables on the fly except for maxFireworks and maxNumSparks

//This effect is fully compatible with color modes, and the bgColor is a pointer, so you can bind it
//to an external color variable

//If you have a non-zero background color be sure to set fillBG to true!!

//Due to the way the effect is programmed, if two sparks meet each other, by default one will overwrite the other
//You can adjust this behavior by turning on "blend", which will add particle colors together as they pass by each other
//However this does have two draw backs:
//1: Due to the way particle trails are drawn, this forces the background to be re-drawn each update cycle,
//   which may have a performance impact depending on your strip length, update rate, etc
//2: For colored backgrounds, the particles colors are added to the background colors.
//   This will in most cases significantly change the particle colors 
//   For example, blue particles running on a red background will appear purple (blue +  red = purple)
//   This can be used to create some nice effects, (like oceanish of lavaish looking things),
//   But overall I do not recommend using blend for colored backgrounds

//Note that this effect does require three seperate arrays: 
    //A bool array fireworks[maxNumFireworks] that stores if a firework is active or not
    //A particleSet with a paricle array of size maxNumFireworks * (maxNumSparks + 1)
    //A CRGB array of trailEndColors[maxNumFireworks * (maxNumSparks + 1)] to store the trail color for each particle
//So watch your memory usage

//Inputs guide:
//A firework has 7 core inputs:
    //maxNumFireworks: The maximum number of fireworks that can exist on the strip at once
    //maxNumSparks: The number of sparks each firework produces
    //spawnChance: How likely an inactive firework is to spawn each update cycle (a percent out of 100)
    //lifeBase: The maximum time (in ms) a firework's sparks will live for (asides from a bit or randomness from lifeRange)
    //speedDecay: How fast the sparks slow down. This is a percent of their current speed -> speed = speed - speed * speedDecay
    //Rate: The fastest particle speed (speed is the update time in ms, so lower rate => higher speed)
    //SpeedRange: The slowest speed a particle can have
//There are some other, secondary variables listed in the Other Settings section below,
//but you probably won't need to tweak these initally

//In general the number of fireworks, sparks and spawn chance are up to you based on your strip size
//Lifebase is used to set a limit on all the sparks life, but you still want to give the sparks time to move
//So lifeBase should probably be 2000+ (2 sec). You should also know that the effect adds a random bit of extra 
//life to each spark, set by lifeRange (default 500, ie 0.5 sec)
//speedDecay is the percent the speed decreases each time the particle is drawn. Starting at 5 or 10 
//and then going from there should work. A lower value will let the sparks spread out more.
//Rate and speedRange do a lot of work in shaping how the firework looks. Rate is the fastest possible speed
//for the sparks (rate is the update rate so lower value => higher speed) while speedRange how much the speed varies by
//Overall, rate sets how fast the average spark moves, while speedRange sets how different spark speeds are from one another
//So you can have low speeds and low ranges to make a slow, clumped up firework, or have high speeds and highe range
//so that the sparks move quick and spread out more.
//I recommend starting with a rate of 40-60 and a speedRange of 300 - 600.

//The fade rate of the sparks is proportional to their speed, so faster sparks fade slower
//This seems produce a good look

//For higher speed ranges, increasing the number of sparks will help "fill out" the firework

//Fireworks also have a central "bomb" particle, which doesn't move, and decays quickly
//This makes the fireworks look more like an explosion
//The settings for this spark all use the "center" in their name

//Example calls: 

    //FireworksPS(mainSegments, 5, 3, 10, 10, 2000, 10, 40, 300);
    //Will do a set of fireworks using colors from a randomly generated pallet of 5 colors
    //There is a maximum of 3 fireworks active at one time.
    //Each firework has 10 sparks
    //There is a 10% chance a new firework spawns during an update
    //The sparks have a maximum life of 2000ms (+ a random(500) from the default life range)
    //The spark's speed decays at 10 percent per update
    //The fastest particles will update at 40ms, while the slowest will be 40 + 300ms

    //FireworksPS(mainSegments, &pallet1, 3, 20, 10, 4000, 5, 40, 500);
    //Will do a set of fireworks using colors from a pallet1
    //There is a maximum of 3 fireworks active at one time.
    //Each firework has 20 sparks
    //There is a 10% chance a new firework spawns during an update
    //The sparks have a maximum life of 4000ms (+ a random(500) from the default life range)
    //The spark's speed decays at 5 percent per update
    //The fastest particles will update at 40ms, while the slowest will be 40 + 500ms

    //FireworksPS(mainSegments, CRGB(CRGB::White), 5, 4, 50, 2000, 10, 80, 80);
    //Small, but fast fireworks on a shifting background
    //bgColorMode = 6;
    //fillBG = true;
    //Will do a set of fireworks using white as the color
    //There is a maximum of 5 fireworks active at one time.
    //Each firework has 4 sparks
    //There is a 50% chance a new firework spawns during an update
    //The sparks have a maximum life of 2000ms (+ a random(500) from the default life range)
    //The spark's speed decays at 10 percent per update
    //The fastest particles will update at 80ms, while the slowest will be 80 + 80ms
    //!!If using pre-build FastLED colors you need to pass them as CRGB( *color code* )

//Constructor Inputs:
    //Pallet(optional, see constructors) -- The set of colors that firework colors will be picked from
    //NumColors (optional, see contructors) -- The number of randomly choosen colors for fireworks
    //Color (optional, see contructors) -- A single color for all the fireworks
    //MaxNumFireworks -- The maximum number of simultaneous fireworks that can be active at one time
    //MaxNumSparks -- The number of sparks spawned per firework
    //SpawnChance -- The likelyhood of a firework spawning (percent out of 100, larger is more likley)
    //LifeBase -- The longest time a spark can exist (in ms)
    //SpeedDecay -- The percent a spark's speed is reduced by per update cycle (out of 100)
    //Rate -- The maxium speed for the sparks, also the effect update rate (is a pointer like in other effects)
    //SpeedRange -- The cap for the slowest speed for the sparks (speed is rate + speedRange)

//Functions:
    //setupFireworks(mazFireworks, maxSparks) -- //Create the data structures for a set of fireworks
                                                 //You should call this if you ever want to change maxNumFireworks or maxNumSparks
    //update() -- updates the effect

//Other Settings:
    //colorMode (default 0) -- sets the color mode for the waves (see segDrawUtils::setPixelColor)
    //bgColorMode (default 0) -- sets the color mode for the spacing pixels (see segDrawUtils::setPixelColor)
    //burstBlendLimit (default 100) -- The number of steps (out of 255) to blend from the burst color to the spark color                        
    //lifeRange ( default 500 (ms) ) -- Added to the lifeBase as random(lifeRange) to set the maximum spark life
    //                                  Adds a bit of variation so not all the sparks decay at the same time at lower caps
    //centerLife (default lifeBase/10 + 100 ) -- How long the centeral "bomb" lives for in ms
    //size (default 1) -- The size of the sparks, only change this for big fireworks
    //sizeRange (default 0) -- A random factor for setting the spark sizes 
    //                         Sparks will be size + random(sizeRange)
    //                         Only change this for big fireworks
    //centerSize (default 3) -- How large the center "bomb" burst is 
    //fillBG (default false) -- Sets whether to fill the background in every update cycle,
    //                          You should set this true if you are using an animated background mode
    //blend (default false) -- Causes sparks to add their colors to the strip, rather than set them
    //                         See explanation of this in more detail above in effect intro
    //randSparkColors (default false) -- If true, each spark will have its own color picked from the pallet
    //burstColor (default CRGB::White) -- The color of the inital firework burst
    //spawnRangeDiv (default 5) -- Sets what range of the strip fireworks spawn in: from numLEDs / spawnRangeDiv to (numLEDs - numLEDs / spawnRangeDiv)

class FireworksPS : public EffectBasePS {
    public:
        FireworksPS(SegmentSet &SegmentSet, palletPS *Pallet, uint8_t MaxNumFireworks, uint8_t MaxNumSparks, 
                    uint8_t SpawnChance, uint16_t LifeBase, uint8_t SpeedDecay, uint16_t Rate, uint16_t SpeedRange); 

        FireworksPS(SegmentSet &SegmentSet, uint16_t numColors, uint8_t MaxNumFireworks, uint8_t MaxNumSparks, 
                    uint8_t SpawnChance, uint16_t LifeBase, uint8_t SpeedDecay, uint16_t Rate, uint16_t SpeedRange);

        //!!If using pre-build FastLED colors you need to pass them as CRGB( *color code* )
        FireworksPS(SegmentSet &SegmentSet, CRGB Color, uint8_t MaxNumFireworks, uint8_t MaxNumSparks, 
                    uint8_t SpawnChance, uint16_t LifeBase, uint8_t SpeedDecay, uint16_t Rate, uint16_t SpeedRange);
                
        ~FireworksPS();

        SegmentSet 
            &segmentSet; 

        uint8_t
            spawnChance,
            spawnRangeDiv = 5, //sets what range of the strip fireworks spawn in: from numLEDs / spawnRangeDiv to (numLEDs - numLEDs / spawnRangeDiv)
            colorMode = 0,
            bgColorMode = 0,
            maxNumFireworks,
            maxNumSparks,
            burstBlendLimit = 100,
            speedDecay; 
        
        uint16_t
            lifeBase, 
            lifeRange = 500,
            centerLife, //set to lifeBase/10 + 100 in init()
            speedRange,
            size = 1,
            sizeRange = 0,
            centerSize = 3; //how large the center "bomb" burst is 
        
        bool
            fillBG = false,
            blend = false,
            randSparkColors = false,
            *fireWorkActive;
        
        CRGB 
            burstColor = CRGB::White,
            bgColorOrig,
            *bgColor; //bgColor is a pointer so it can be tied to an external variable if needed (such as a pallet color)

        palletPS
            palletTemp,
            *pallet;

        particleSetPS 
            *particleSet, //the particle set used in the effect
            particleSetTemp; //storage for self created particle sets

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
            movePart; //flag for if a particle should move thise cycle
        
        particlePS
            *particlePtr;

        pixelInfoPS
            pixelInfo{0, 0, 0, 0};

        CRGB 
            *trailEndColors, //used to store the last colors of each trail, so the background color can be set
            colorFinal,
            colorOut,
            colorTemp;
        
        void
            init(uint8_t maxNumFireworks, uint8_t maxNumSparks, uint16_t Rate),
            moveParticle(particlePS *particlePtr),
            drawParticlePixel(particlePS *particlePtr, uint16_t trailLedLocation),
            spawnFirework(uint8_t fireworkNum);
};

#endif
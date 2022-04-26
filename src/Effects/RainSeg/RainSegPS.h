#ifndef RainSegPS_h
#define RainSegPS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "Effects/Particles/particleUtilsPS.h"

//An effect for prodcing a random set of falling particles, like rain, or the classic "Matrix" code animation
//Drops spawn at the start of each segment in the segment set and "fall down" to the end of the segment
//This effect uses particles as the drops. See particlePS.h and particleUtilsPS.h for more details
//There are numerous options for the drops: trail type, speed, size, etc
//There are also more general settings for how often drops spawn, how the background is drawn,
//and if the drops should blend together if they pass over each other.
//You can also configure each drop's properties to be choosen at random from ranges.

//drop colors can either be a static color or picked randomly from a pallet

//This effect is fully compatible with color modes, and the bgColor is a pointer, so you can bind it
//to an external color variable

//Note that update rate of the effect is the maximum speed of the drops.

//Due to the way the effect is programmed, if two drops meet each other, by default one will overwrite the other
//You can adjust this behavior by turning on "blend", which will add particle colors together as they pass by each other
//However this does have two draw backs:
//1: Due to the way particle trails are drawn, this forces the background to be re-drawn each update cycle,
//   which may have a performance impact depending on your strip length, update rate, etc
//2: For colored backgrounds, the particles colors are added to the background colors.
//   This will in most cases significantly change the particle colors 
//   For example, blue particles running on a red background will appear purple (blue +  red = purple)
//   This can be used to create some nice effects, (like oceanish of lavaish looking things),
//   But overall I do not recommend using blend for colored backgrounds

//Example calls: 

    //A "quick" constructor without options for size, trail, or speed ranges and only one trail type supported
    //These are all default to 0
    //There is a similar constructor for using a single color, just replace the pallet with a color in th
    //constructor

    //RainSegPS(mainSegments, &pallet3, CRGB::Red, true, 10, 4, 1, 1, 5, 80);
    //Will spawn drops on the mainSegment set, picking colors from pallet3
    //The background is red, and it will be pre-filled before the drops spawn
    //The drops have a spawn chance of 10/100 (10% chance of spawing each update cycle)
    //There is a maximum of 4 drops running concurrently on each segment
    //The drops have a single trailing trail of length 5
    //The drops move with a base speed of 80ms
    //bgColorMode = 6; => will cycle the background through the rainbow (see segDrawUtils::getPixelColor())

    //A more extensive constructor will all the options for drops
    //Also comes as a pallet variant; replace the color with a pallet in the constructor

    //RainSegPS(mainSegments, CRGB::Green, 0, false, 10, 4, 1, 3, 2, 4, true, true, false, false, false, 60, 40);
    //Will spawn green drops on the mainSegment set
    //The background is off, and it will not be pre-filled before the drops spawn
    //The drops have a spawn chance of 10/100 (10% chance of spawing each update cycle)
    //There is a maximum of 4 drops running concurrently on each segment
    //The drops have a minimum size of 1, with a range of 3
    //The drops have minimum trail size of 2 with a range of 4 (if they have them)
    //Drops can spawn with no trails or a single trial, and will not
    //spawn with double, reversed, or infinite trails
    //Drops will have a base speed of 60ms and a speed range of 40ms 

//Constructor inputs for creating a particle set:
    //pallet(optional) -- The pallet than will be used for the particle colors 
    //color(optional) -- Used in place of a pallet to spawn drops in a single color
    //BgColor -- The background color used for the effect
    //BgPrefill -- If set, then the background will be filled in when the effect first runs
    //             Otherwise the drops will fill it in as they move along
    //SpawnChance -- The chance a drop will spawn (if able) each cycle out of 100, higher val => more likely to spawn
    //MaxNumDrops -- The maximum number of drops that can be active on a segment (not the whole segment Set!) at one time
    //Size -- The minimum size of the drops (min value 1) (doesn't include trails)
    //SizeRange (optional) -- The amount the size can vary from the base size (ie size + random(range))
    //TrailType -- The type of trails used for the drops (see below and particlePS.h),
    //                        pass in 5 to set the trails randomly between 0, 1, and 2 trails
    //                        pass in 6 to set the trails based on the trail flags (see below)
    //TrailSize -- The minimum length of the trails (if the particle has them, min val 1)
    //TrailRange (optional) -- The amount the trailSize can vary from the base size (ie trailSize + random(range))
    //noTrails (optional, default false) -- Used with trailType 6, allows drops with no trails
    //oneTrail (optional, default false) -- Used with trailType 6, allows drops with one trailing trail
    //twoTrail (optional, default false) -- Used with trailType 6, allows drops with two trails
    //revTrail (optional, default false) -- Used with trailType 6, allows drops with one reversed trails
    //infTrail (optional, default false) -- Used with trailType 6, allows drops with infinite trails
    //Rate -- The minimum speed of the drops
    //SpeedRange -- The amount the speed may vary up from the Rate ( ie Rate + random(range) )
 
//Trail Modes:
//Taken from particlePS.h:
//Trails blend cleanly into the background color over the trail length
//(like waving a flame around, or a metor trail)
//Trail options:
//    0: no trails
//    1: one trail facing away from the direction of motion (like a comet)
//    2: two trails, facing towards both directions of motion
//    3: one trail facing towards the direction of motion
//    4: Infinite trails that persist after the particle (no fading)
//For example, with a trail length of 4, the modes will produce:
//(The trail head is *, - are the trail, particle is moving to the right ->)
//  0:     *
//  1: ----* 
//  2: ----*----
//  3:     *----
//  4: *****

//Trail flags:
    //There is a flag for each type of trail. When used with trailType of 6, each drop's trail will be picked randomly
    //from the flags set true
    //ie if noTrails, twoTrail, infTrail are true then drops can spawn with no trails, two trails or infinite trails
    //if no flags are set, drops will spawn with no trails

//DimPow:
//The rate of dimming for the trails can be adjusted using dimPow. This allows you to produce a brighter head
//making the comet effect more noticable
//The range of dimPow is -127 to 127, it's defaulted to 80
//Positive values quicken the dimming, while negative ones slow it down
//setting the negative value below 80, seems to bug it out tho
//Slowing the dimming down is useful for colored backgrounds, as it makes the particles stand out more

//Functions:
    //setupDrops(uint8_t newMaxNumDrops) -- Changes the maximum number of drops, but will also clear any active drops
    //                                      If you want to increase the number of drops without clearing, set the
    //                                      maxNumDrops to your maximum in the constructor. Then lower it before running the
    //                                      effect. You can safely raise it again without reseting (set maxNumDrops manualy)
    //                                      !!If you lower the maximum number of drops, some active drops may be left on the segments
    //update() -- updates the effect 

//Other Settings:
    //colorMode (default 0) -- sets the color mode for the particles (see segDrawUtils::setPixelColor)
    //bgColorMode (default 0) -- sets the color mode for the spacing pixels (see segDrawUtils::setPixelColor)
    //dimPow (default 80, min -127, max 127) -- Adjusts the rate of dimming for the trails (see dimPow above)
    //blend (default false) -- Causes particles to add their colors to the strip, rather than set them
    //                         See explanation of this in more detail above in effect intro
    //fillBG (default false) -- Sets the background to be redrawn every update, useful for bgColorModes that are dynamic
    //                          Warning!: Not compatible with infinite trails (mode 4). They will be drawn over.

//Notes:
    //you can change the drop variables (speed, size, the ranges) on the fly. As drops spawn they will use the 
    //new values
class RainSegPS : public EffectBasePS {
    public:
        //constructor for pallet colors, no range options
        RainSegPS(SegmentSet &SegmentSet, palletPS *Pallet, CRGB BgColor, bool BgPrefill, uint8_t SpawnChance, 
                    uint8_t MaxNumDrops, uint16_t Size, uint8_t TrailMode, uint8_t TrailSize, uint16_t Rate);
        
        //constructor for pallet colors with range and trail options
        RainSegPS(SegmentSet &SegmentSet, palletPS *Pallet, CRGB BgColor, bool BgPrefill, uint8_t SpawnChance, 
                    uint8_t MaxNumDrops, uint16_t Size, uint16_t SizeRange, uint8_t TrailSize,
                    uint8_t TrailRange, bool NoTrails, bool OneTrail, bool TwoTrail, bool RevTrail, 
                    bool InfTrail, uint16_t Rate, uint16_t SpeedRange);
        
        //constructor for single color, no range options
        RainSegPS(SegmentSet &SegmentSet, CRGB Color, CRGB BgColor, bool BgPrefill, uint8_t SpawnChance, 
                    uint8_t MaxNumDrops, uint16_t Size, uint8_t TrailType, uint8_t TrailSize, uint16_t Rate);

        //constructor for single colors with range and trail options        
        RainSegPS(SegmentSet &SegmentSet, CRGB Color, CRGB BgColor, bool BgPrefill, uint8_t SpawnChance, 
                    uint8_t MaxNumDrops, uint16_t Size, uint16_t SizeRange, uint8_t TrailSize,
                    uint8_t TrailRange, bool NoTrails, bool OneTrail, bool TwoTrail, bool RevTrail, 
                    bool InfTrail, uint16_t Rate, uint16_t SpeedRange);
        
        ~RainSegPS();

        SegmentSet 
            &segmentSet; 

        int8_t
            dimPow = 80; //80 range -127 -> 127 -80 good for colored bg's
        
        uint8_t
            maxNumDrops,
            spawnChance,
            colorMode = 0,
            bgColorMode = 0,
            trailType,
            trailSize,
            trailRange = 0;
        
        uint16_t
            speedRange = 0,
            size,
            sizeRange = 0;
        
        bool
            bgPrefill,
            fillBG = false,
            blend = false,
            *partActive;
        
        //trail type flags
        bool
            noTrails = false,
            oneTrail = false,
            twoTrail = false, 
            revTrail = false, 
            infTrail = false;

        CRGB 
            bgColorOrig,
           *bgColor; //bgColor is a pointer so it can be tied to an external variable if needed (such as a pallet color)
        
        palletPS
            palletTemp,
            *pallet;

        particleSetPS 
            *particleSet, //the particle set used in the effect
            particleSetTemp; //storage for self created particle sets

        void 
            setupDrops(uint8_t newMaxNumDrops),
            update(void);
    
    private:
        
        int8_t
            trailDirectionAdj;

        uint8_t
            numSegs = 0, //for first init function call
            partTrailType,
            partTrailSize,
            posOffset,
            sizeAdj,
            dimRatio;

        uint16_t 
            pixelPosTemp,
            particleIndex,
            totPartSize,
            partPos,
            headPos,
            partSpeed,
            lineNum,
            partSize,
            maxPosition,
            trailLedLocation,
            sectionEnd,
            sectionStart = 0,
            getTrailLedLoc(bool trailDirect, uint8_t trailPixelNum, uint16_t maxPosition);

        unsigned long
            currentTime,
            prevTime = 0;
        
        bool
            bgFilled = false,  //flag for if the background has been filled in already
            spawnOkTest = true, //flag for if a particle is able to spawn
            movePart; //flag for if a particle should move thise cycle
        
        particlePS
            *particlePtr;

        CRGB 
            *trailEndColors, //used to store the last colors of each trail, so the background color can be set
            colorEnd,
            colorOut,
            colorTemp,
            desaturate(CRGB color, CRGB targetColor, uint8_t step, uint8_t totalSteps);
        
        void
            init(uint16_t Rate, CRGB BgColor),
            setDropSpawnPos(particlePS *particlePtr, uint8_t segNum),
            moveParticle(particlePS *particlePtr),
            drawParticlePixel(uint16_t trailLedLocation, uint8_t trailPixelNum, uint8_t trailSize, uint8_t segNum, bool bodyPixel),
            spawnParticle(uint8_t particleIndex, uint8_t segNum);
};

#endif
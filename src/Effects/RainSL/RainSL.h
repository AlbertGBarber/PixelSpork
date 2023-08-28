#ifndef RainSL_h
#define RainSL_h

//TODO: -- Try to make core common functions for this and RainSeg?
//      -- Add alternating direction mode?

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "Effects/ParticlesSL/Particle_Stuff/particleUtilsPS.h"

/* 
Like RainSeg, but the drops spawn and move along the segment set lines (perpendicular to the segments)
An effect for producing a random set of falling particles, like rain, or the classic "Matrix" code animation
You can set the drops to spawn at either the first or last segment
They will then "fall" towards the last or first segment respectively
This effect uses particles as the drops. See particlePS.h and particleUtilsPS.h for more details
There are numerous options for the drops: trail type, speed, size, etc
There are also more general settings for how often drops spawn, how the background is drawn,
and if the drops should blend together if they pass over each other.
You can also configure each drop's properties to be chosen at random from ranges.

Drop colors can either be a static color or picked randomly from a palette

This effect is fully compatible with color modes, and the bgColor is a pointer, so you can bind it
to an external color variable

Note that update rate of the effect is the maximum speed of the drops.

Due to the way the effect is programmed, if two drops meet each other, by default one will overwrite the other
You can adjust this behavior by turning on "blend", which will add particle colors together as they pass by each other
However this does have two draw backs:
    1: Due to the way particle trails are drawn, this forces the background to be re-drawn each update cycle,
       which may have a performance impact depending on your strip length, update rate, etc
    2: For colored backgrounds, the particles colors are added to the background colors.
       This will in most cases significantly change the particle colors 
       For example, blue particles running on a red background will appear purple (blue +  red = purple)
       This can be used to create some nice effects, (like ocean-ish of lava-ish looking things),
       But overall I do not recommend using blend for colored backgrounds

Example calls: 

    A "quick" constructor without options for size, trail, or speed ranges and only one trail type supported
    These are all default to 0
    There is a similar constructor for using a single color, just replace the palette with a color in th
    constructor

    RainSL rainSL(mainSegments, cybPnkPal_PS, CRGB::Red, true, 10, 4, 1, 1, 5, 80, true);
    Will spawn drops on the mainSegment set, picking colors from cybPnkPal_PS
    The background is red, and it will be pre-filled before the drops spawn
    The drops have a spawn chance of 10/100 (10% chance of spawning each update cycle)
    There is a maximum of 4 drops running concurrently on each segment
    The drops have a single trailing trail of length 5
    The drops move with a base speed of 80ms
    Drops will spawn at the first segment and move towards the last
    bgColorMode = 6; => will cycle the background through the rainbow (see segDrawUtils::getPixelColor())

    A more extensive constructor will all the options for drops
    Also comes as a palette variant; replace the color with a palette in the constructor

    RainSL rainSL(mainSegments, CRGB::Green, 0, false, 10, 4, 1, 3, 2, 4, true, true, false, false, false, 60, 40, false);
    Will spawn green drops on the mainSegment set
    The background is off, and it will not be pre-filled before the drops spawn
    The drops have a spawn chance of 10/100 (10% chance of spawning each update cycle)
    There is a maximum of 4 drops running concurrently on each segment
    The drops have a minimum size of 1, with a range of 3
    The drops have minimum trail size of 2 with a range of 4 (if they have them)
    Drops can spawn with no trails or a single trial, and will not
    spawn with double, reversed, or infinite trails
     Drops will spawn at the last segment and move towards the first
    Drops will have a base speed of 60ms and a speed range of 40ms 

Constructor inputs for creating a particle set:
    palette(optional) -- The palette than will be used for the particle colors 
    color(optional) -- Used in place of a palette to spawn drops in a single color
    BgColor -- The background color used for the effect
    BgPrefill -- If set, then the background will be filled in when the effect first runs
                Otherwise the drops will fill it in as they move along
    SpawnChance -- The chance a drop will spawn (if able) each cycle out of 100, higher val => more likely to spawn
    MaxNumDrops -- The maximum number of drops that can be active on a segment (not the whole segment Set!) at one time
    Size -- The minimum size of the drops (min value 1) (doesn't include trails)
    SizeRange (optional) -- The amount the size can vary from the base size (ie size + random(range))
    TrailType -- The type of trails used for the drops (see below and particlePS.h),
                           pass in 5 to set the trails randomly between 0, 1, and 2 trails
                           pass in 6 to set the trails based on the trail flags (see below)
    TrailSize -- The minimum length of the trails (if the particle has them, min val 1)
    TrailRange (optional) -- The amount the trailSize can vary from the base size (ie trailSize + random(range))
    noTrails (optional, default false) -- Used with trailType 6, allows drops with no trails
    oneTrail (optional, default false) -- Used with trailType 6, allows drops with one trailing trail
    twoTrail (optional, default false) -- Used with trailType 6, allows drops with two trails
    revTrail (optional, default false) -- Used with trailType 6, allows drops with one reversed trails
    infTrail (optional, default false) -- Used with trailType 6, allows drops with infinite trails
    Rate -- The minimum speed of the drops
    SpeedRange -- The amount the speed may vary up from the Rate ( ie Rate + random(range) )
    Direct -- The direction the drops will fall
 
Trail Modes:
    Taken from particlePS.h:
    Trails blend cleanly into the background color over the trail length
    (like waving a flame around, or a meteor trail)
    Trail options:
    0: no trails
    1: one trail facing away from the direction of motion (like a comet)
    2: two trails, facing towards both directions of motion
    3: one trail facing towards the direction of motion
    4: Infinite trails that persist after the particle (no fading)
    For example, with a trail length of 4, the modes will produce:
    (The trail head is *, - are the trail, particle is moving to the right ->)
    0:     *
    1: ----* 
    2: ----*----
    3:     *----
    4: *****

Trail flags:
    There is a flag for each type of trail. When used with trailType of 6, each drop's trail will be picked randomly
    from the flags set true
    ie if noTrails, twoTrail, infTrail are true then drops can spawn with no trails, two trails or infinite trails
    if no flags are set, drops will spawn with no trails

DimPow:
    The rate of dimming for the trails can be adjusted using dimPow. This allows you to produce a brighter head
    making the comet effect more noticeable
    The range of dimPow is -127 to 127, it's defaulted to 80
    Positive values quicken the dimming, while negative ones slow it down
    setting the negative value below 80, seems to bug it out tho
    Slowing the dimming down is useful for colored backgrounds, as it makes the particles stand out more

Functions:
    setupDrops(newMaxNumDrops) -- Changes the maximum number of drops, but any currently active drops will stop.
                                  If bgPrefill is true, then the background will be re-filled in the next update.
                                  If you want to increase the number of drops without clearing, set the
                                  maxNumDrops to your maximum in the constructor. Then lower it before running the
                                  effect. You can safely raise it again without resetting (set maxNumDrops manually)
                                  !!If you lower the maximum number of drops, some active drops may be left on the segments
                                  !!If you change the segment set, you should re-call this function!!
    update() -- updates the effect 

Other Settings:
    colorMode (default 0) -- sets the color mode for the particles (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the spacing pixels (see segDrawUtils::setPixelColor)
    dimPow (default 80, min -127, max 127) -- Adjusts the rate of dimming for the trails (see dimPow above)
    blend (default false) -- Causes particles to add their colors to the strip, rather than set them
                             See explanation of this in more detail above in effect intro
    fillBG (default false) -- Sets the background to be redrawn every update, useful for bgColorModes that are dynamic
                              Warning!: Not compatible with infinite trails (mode 4). They will be drawn over.

Reference Vars: 
    maxNumDrops -- (see notes above) set using setupDrops();

Notes:
    you can change the drop variables (speed, size, the ranges) on the fly. As drops spawn they will use the new values
*/
class RainSL : public EffectBasePS {
    public:
        //constructor for palette colors, no range options
        RainSL(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, bool BgPrefill, uint8_t SpawnChance,
               uint8_t MaxNumDrops, uint16_t Size, uint8_t TrailMode, uint8_t TrailSize, uint16_t Rate, bool Direct);

        //constructor for palette colors with range and trail options
        RainSL(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, bool BgPrefill, uint8_t SpawnChance,
               uint8_t MaxNumDrops, uint16_t Size, uint16_t SizeRange, uint8_t TrailSize,
               uint8_t TrailRange, bool NoTrails, bool OneTrail, bool TwoTrail, bool RevTrail,
               bool InfTrail, uint16_t Rate, uint16_t SpeedRange, bool Direct);

        //constructor for single color, no range options
        RainSL(SegmentSetPS &SegSet, CRGB Color, CRGB BgColor, bool BgPrefill, uint8_t SpawnChance,
               uint8_t MaxNumDrops, uint16_t Size, uint8_t TrailType, uint8_t TrailSize, uint16_t Rate, bool Direct);

        //constructor for single colors with range and trail options
        RainSL(SegmentSetPS &SegSet, CRGB Color, CRGB BgColor, bool BgPrefill, uint8_t SpawnChance,
               uint8_t MaxNumDrops, uint16_t Size, uint16_t SizeRange, uint8_t TrailSize,
               uint8_t TrailRange, bool NoTrails, bool OneTrail, bool TwoTrail, bool RevTrail,
               bool InfTrail, uint16_t Rate, uint16_t SpeedRange, bool Direct);

        ~RainSL();

        int8_t
            dimPow = 80;  //80 range -127 -> 127 -80 good for colored bg's

        uint8_t
            maxNumDrops = 0,  //for reference only, set using setupDrops()
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
            direct,
            bgPrefill,
            fillBG = false,
            blend = false,
            *partActive = nullptr;

        //trail type flags
        bool
            noTrails = false,
            oneTrail = false,
            twoTrail = false,
            revTrail = false,
            infTrail = false;

        CRGB
            *trailEndColors = nullptr,  //used to store the last colors of each trail, so the background color can be set
            bgColorOrig,
            *bgColor = nullptr;  //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)

        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0};  //Must init structs w/ pointers set to null for safety

        particleSetPS
            *particleSet = nullptr,          //the particle set used in the effect
            particleSetTemp = {nullptr, 0};  //storage for self created particle sets, init to empty for safety

        void
            setupDrops(uint8_t newMaxNumDrops),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0;

        int8_t
            trailDirectionAdj;

        uint8_t
            partTrailType,
            partTrailSize,
            posOffset,
            sizeAdj,
            dimRatio;

        uint16_t
            numLines = 0,  //for first init function call
            numSegs,
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
            getParticlePixelLoc(uint16_t trailLedLocation, uint8_t lineNum),
            getTrailLedLoc(bool trailDirect, uint8_t trailPixelNum, uint16_t maxPosition);

        bool
            bgFilled = false,    //flag for if the background has been filled in already
            spawnOkTest = true,  //flag for if a particle is able to spawn
            movePart;            //flag for if a particle should move this cycle

        particlePS
            *particlePtr = nullptr;

        CRGB
            colorEnd,
            colorOut,
            colorTemp;

        void
            init(uint8_t MaxNumDrops, CRGB BgColor, SegmentSetPS &SegSet, uint16_t Rate),
            setDropSpawnPos(particlePS *particlePtr),
            moveParticle(particlePS *particlePtr),
            drawParticlePixel(uint16_t trailLedLocation, uint8_t trailPixelNum, uint8_t trailSize, uint8_t segNum, bool bodyPixel),
            spawnParticle(uint8_t particleIndex, uint8_t segNum);
};

#endif
#ifndef RainSegPS_h
#define RainSegPS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "Effects/ParticlesSL/Particle_Stuff/particleUtilsPS.h"

/* 
An effect for producing a random set of falling particles, like rain, or the classic "Matrix" code animation
Drops spawn at the start of each segment in the segment set and "fall down" to the end of the segment
This effect uses particles as the drops. See particlePS.h and particleUtilsPS.h for more details

For a similar effect see RainSL, which moves drops along segment lines rather than segment 
(making it "orthogonal" to this effect).

There are numerous options for the drops: trail type, speed, size, etc
There are also more general settings for how often drops spawn, how the background is drawn,
and if the drops should blend together if they pass over each other.
You can also configure each drop's properties to be chosen at random from ranges.
Drop colors can either be a static color or picked randomly from a palette.

This effect is fully compatible with color modes, and the bgColor is a pointer, so you can bind it
to an external color variable

Note that the effect requires an array of particles of size `maxNumDrops * numSegs`. 
It is allocated dynamically, so, to avoid memory fragmentation, when you create the effect, 
you should set `maxNumDrops` to the maximum value you expect to use. 
See https://github.com/AlbertGBarber/PixelSpork/wiki/Effects-Advanced#managing-dynamic-memory-and-fragmentation
for more details. 

Inputs Guide:

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

    Randomizing Trail Types:
        You can have the rain drops chose a random trail type from a customizable selection. 
        To access this feature set `trailType` to 6 (or use one of the trail customization constructors). 
        There is a flag for each type of trail. Each drop's trail will be picked randomly from the flags set true. 
        If no flags are set, drops will spawn with no trails.

        Trail Flags:
            * `noTrails` -- Allows drops with no trails.
            * `oneTrail` -- Allows drops with one trailing trail.
            * `twoTrail` -- Allows drops with two trails.
            * `revTrail` -- Allows drops with one reversed trails.
            * `infTrail` -- Allows drops with infinite trails.

        For example, if `noTrails`, `twoTrail`, `infTrail` are true then drops
        can spawn with no trails, two trails or infinite trails.

    Trail Fading:
        By default, the trails dim quickly in a non-linear fashion. 
        This makes the drop "heads" brighter and standout more, which, in my opinion, 
        looks better then just using a linear fade. You can control the linearity of the 
        trail fades using the "dimPow" setting. A default of 80 is used in this effect. 
        You can read the "dimPow" notes in "particleUtils.h" for more.

    Randomizing Size Settings:
        When a rain drop particle is spawned, it's speed, size, and trail size are set to the effect settings, 
        speed, size, and trailSize. 
        You can opt to randomize these values for each rain drop by setting the following ranges:
        * speedRange -- The amount the speed may vary up from the base speed ( ie `speed + random(speedRange)` ) (ms).
        * trailRange -- The amount the `trailSize` can vary from the base size ( ie `trailSize + random(trailRange)` ).
        * sizeRange -- The amount the size can vary from the base size ( ie `size + random(sizeRange)` ).

    Blending:
        Due to the way the effect is programmed, particles at the end in the particle set will run "in front"
        of those earlier in the set. This means that when two particles pass each other, 
        the later one will be drawn over the earlier one.

        You can adjust this behavior by turning on `blend`, which will add particle colors 
        together as they pass by each other. However this does have a few draw backs:
            1. Due to the way particle trails are drawn, this forces the background to 
               be re-drawn each update cycle, which may have a performance impact depending on your strip length, 
               update rate, etc.
            2. For colored backgrounds, the particles colors are added to the background colors. 
               This will in most cases significantly change the particle colors. 
               For example, blue particles running on a red background will appear purple (blue +  red = purple).
               While this can be used to create some nice effects, (like ocean-ish of lava-ish looking things), 
               overall I do not recommend using blend for colored backgrounds.

    Background Pre-filling:
        As the rain drops move, they naturally fill in the background behind them. 
        This allows you to create a neat start to the effect by letting the first rain drops
        replace whatever was previously on the segment set over time as the spawn in and move. 
        You can also opt to pre-fill the background instead. the `bgPrefill` setting controls 
        this behavior (true will pre-fill the background). The effect also features the more standard `fillBg`, 
        which fills the background every cycle. Finally, note that backgrounds are not drawn by drops 
        with infinite trails (trail mode 4), and that turning on `fillBg` will break their trails.
    
    Spawning:
        A drop will spawn if random(spawnBasis) <= spawnChance. spawnBasis is default to 1000, so you can go down
        to sub 1% percentages. Note that the effect tries to spawn any inactive particles with each update(), although
        only one particle can spawn at a time. This means that how densely your particles spawn depends a lot on the
        effect update rate and how many particles you have. Even with quite low percentages, 
        at the default update rate (see below), particles will probably spawn quite close together. 
    
    Update Rate:
        Note that unlike other effects, the update rate is pre-set for you at 5ms, 
        which helps catch each of the rain drop's updates on time (hopefully!). 
        You see, each rain drop has its own speed (update rate), but we still want be able to update the effect
        with a single update() call. I could have ignored the "rate" setting, and just updated all the 
        rain drops whenever you call update(). However, the drops are re-drawn every update(), even if they haven't moved,
        so this becomes problematic, especially when working with multiple effects or segment sets, where you want more
        control over when you update. Instead I opted to treat the effect as normal, and keep the overall update rate,
        but default it to a very fast 5ms. Usually I set my speeds to multiples of 10, so hopefully this default
        will catch most particles on time. You are free to change the update rate as needed by setting rateOrig.

Example calls: 

    A "quick" constructor without options for size, trail, or speed ranges and only one trail type supported
    All ranges are default to 0
    There is a similar constructor for using a single color, just replace the palette with a color.

    rainSeg.bgColorMode = 6; => optional for next effect, will cycle the background through the rainbow, 
                                place in Arduino setup()
    RainSeg rainSeg(mainSegments, cybPnkPal_PS, 0, true, 10, 4, 1, 1, 5, 80);
    Will spawn drops on the mainSegment set, picking colors from cybPnkPal_PS
    The background is blank, and it will be pre-filled before the drops spawn
    The drops have a spawn chance of 10/1000 (1% chance of spawning each update cycle)
    There is a maximum of 4 drops running concurrently on each segment
    The drops are size 1 and have a single trail trail of length 5 (trail mode 1)
    The drops move with a base speed of 80ms

    A more extensive constructor will all the options for drops
    Also comes as a palette variant; replace the color with a palette in the constructor

    RainSeg rainSeg(mainSegments, CRGB::Green, CRGB::Red, false, 10, 4, 1, 3, 2, 4, true, true, false, false, false, 60, 40);
    Will spawn green drops on the mainSegment set
    The background is red, and it will not be pre-filled before the drops spawn
    The drops have a spawn chance of 10/1000 (1% chance of spawning each update cycle)
    There is a maximum of 4 drops running concurrently on each segment
    The drops have a minimum size of 1, with a range of 3 (for a max size of 4)
    The drops have minimum trail size (if they have them) of 2 with a range of 4 (max size 6)
    Drops can spawn with no trails or a single trial,
    but will not spawn with double, reversed, or infinite trails
    Drops will have a base speed of 60ms and a speed range of 40ms (for a min speed of 100ms)

Constructor inputs for creating a particle set:
    palette(optional) -- The palette than will be used for the particle colors 
    color(optional) -- Used in place of a palette to spawn drops in a single color
    bgColor -- The background color used for the effect
    bgPrefill -- If true, then the background will be filled in when the effect first runs
                Otherwise the drops will fill it in as they move along
    spawnChance -- The chance a drop will spawn (if able) each cycle out of 1000 (see spawnBasis), higher val => more likely to spawn
    maxNumDrops -- The maximum number of rain drop particles that can be active on a segment (not the whole segment Set!) at one time
    size -- The minimum size of the body of the drops (min value 1) (doesn't include trails)
    sizeRange (optional) -- The amount the size can vary from the base size (ie size + random(range))
    trailType -- The type of trails used for the drops (see below and particlePS.h),
                           pass in 5 to set the trails randomly between 0, 1, and 2 trails
                           pass in 6 to set the trails based on the trail flags (see below)
    trailSize -- The length of the rain drop trails (if the particle has them, min val 1)
    trailRange (optional) -- The amount the trailSize can vary from the base size (ie trailSize + random(range))
    noTrails (optional, default false) -- Used with trailType 6, allows drops with no trails
    oneTrail (optional, default false) -- Used with trailType 6, allows drops with one trailing trail
    twoTrail (optional, default false) -- Used with trailType 6, allows drops with two trails
    revTrail (optional, default false) -- Used with trailType 6, allows drops with one reversed trails
    infTrail (optional, default false) -- Used with trailType 6, allows drops with infinite trails
    speed -- The base speed of the drops (ms)
    speedRange -- The amount the speed may vary up from the base speed ( ie speed + random(range) ) (ms)
 
Other Settings:
    colorMode (default 0) -- sets the color mode for the particles (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the spacing pixels (see segDrawUtils::setPixelColor)
    dimPow (default 80, min -127, max 127) -- Adjusts the rate of dimming for the trails (see Fading above)
    blend (default false) -- Causes particles to add their colors to the strip, rather than set them
                             See explanation of this in more detail above in effect intro
    fillBg (default false) -- Sets the background to be redrawn every update, useful for bgColorModes that are dynamic
                              Warning!: Not compatible with infinite trails (mode 4). They will be drawn over.
    spawnBasis (default 1000) -- The spawn probability threshold. 
                                 A drop will spawn if "random(spawnBasis) <= spawnChance".
                            
Functions:
    setupDrops(newMaxNumDrops) -- Changes the maximum number of drops. 
                                  Also calls reset() to reset the drop's spawns. 
                                  Note that any active drops will be left on the segments. 
                                  If `bgPrefill` is true, then the background will be re-filled in 
                                  the next update to clear them. 
                                  !!If you change the segment set, you should re-call 
                                  this function to re-configure the drop particle set!!
    reset() -- Resets the effect by setting all particles to inactive.
               Configures the background to be cleared on the next update if bgPrefill is true.
    update() -- updates the effect 
    
Reference Vars: 
    maxNumDrops -- (see notes above) set using setupDrops();

Notes:
    you can change the drop variables (speed, size, the ranges) on the fly. As drops spawn they will use the new values 
*/
class RainSeg : public EffectBasePS {
    public:
        //constructor for palette colors, no range options
        RainSeg(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, bool BgPrefill, uint16_t SpawnChance,
                uint8_t MaxNumDrops, uint16_t Size, uint8_t TrailMode, uint8_t TrailSize, uint16_t Speed);

        //constructor for palette colors with range and trail options
        RainSeg(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, bool BgPrefill, uint16_t SpawnChance,
                uint8_t MaxNumDrops, uint16_t Size, uint16_t SizeRange, uint8_t TrailSize,
                uint8_t TrailRange, bool NoTrails, bool OneTrail, bool TwoTrail, bool RevTrail,
                bool InfTrail, uint16_t Speed, uint16_t SpeedRange);

        //constructor for single color, no range options
        RainSeg(SegmentSetPS &SegSet, CRGB Color, CRGB BgColor, bool BgPrefill, uint16_t SpawnChance,
                uint8_t MaxNumDrops, uint16_t Size, uint8_t TrailType, uint8_t TrailSize, uint16_t Speed);

        //constructor for single colors with range and trail options
        RainSeg(SegmentSetPS &SegSet, CRGB Color, CRGB BgColor, bool BgPrefill, uint16_t SpawnChance,
                uint8_t MaxNumDrops, uint16_t Size, uint16_t SizeRange, uint8_t TrailSize,
                uint8_t TrailRange, bool NoTrails, bool OneTrail, bool TwoTrail, bool RevTrail,
                bool InfTrail, uint16_t Speed, uint16_t SpeedRange);

        ~RainSeg();

        int8_t
            dimPow = 80;  //80 range -127 -> 127, -80 good for colored bg's

        uint8_t
            maxNumDrops = 0,  //for reference only, set using setupDrops()
            colorMode = 0,
            bgColorMode = 0,
            trailType,
            trailSize,
            trailRange = 0;

        uint16_t
            spawnChance,
            speed,
            speedRange = 0,
            size,
            sizeRange = 0,
            spawnBasis = 1000; //spawn change scaling (random(spawnBasis) <= spawnChance controls spawning)

        bool
            bgPrefill,
            fillBg = false,
            blend = false;

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
            reset(void),
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
            numSegs = 0,  //for first init function call
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

        bool
            bgFilled = false,    //flag for if the background has been filled in already
            spawnOkTest = true,  //flag for if a particle is able to spawn
            movePart,            //flag for if a particle should move this cycle
            isActive(uint8_t particleIndex); //returns true if the particle is active
            
        particlePS
            *particlePtr = nullptr;

        CRGB
            colorEnd,
            colorOut,
            colorTemp;

        void
            init(uint8_t MaxNumDrops, CRGB BgColor, SegmentSetPS &SegSet),
            setDropSpawnPos(particlePS *particlePtr, uint16_t segNum),
            setActive(uint8_t particleIndex, bool isActive),
            moveParticle(particlePS *particlePtr),
            drawParticlePixel(uint16_t trailLedLocation, uint8_t trailPixelNum, uint8_t trailSize, uint16_t segNum, bool bodyPixel),
            spawnParticle(uint8_t particleIndex, uint16_t segNum);
};

#endif
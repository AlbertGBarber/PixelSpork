#ifndef ParticlesSLPS_h
#define ParticlesSLPS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"
#include "Particle_Stuff/particleUtilsPS.h"

/* 
An effect for updating a set of particles
Copying the explanation of particles from particleUtilsPS.h:
A particle is a moving pixel. Particles can move backwards or forwards along the strip. They move at their own speeds.
There are a number of other settings to do with trails, particle size, bouncing, etc (see particle.h)
Because working with just one particle at a time would be boring, I've created particleSets (see particleUtilsPS.h)
which are storage for a group of particles
This effect animates a set of particles
The set can either be provided, or automatically built by the effect.

If you're looking for a preset cylon or knightRider mode see LarsonScanner effect

This effect is fully compatible with color modes, and the bgColor is a pointer, so you can bind it
to an external color variable

The effect is adapted to work on segment lines for 2D use, but you can keep it 1D by
passing in a segmentSet with only one segment containing the whole strip. For 2D segments
particles move along segment lines, with the particle being replicated across all the segments
ie like a wave.

Note that unlike other effects the update rate is not externally controllable, and depends on the 
update rates of the particles

Due to the way the effect is programmed, particles that are placed later in the particleSet will run "in front"
of those eariler in the set. This means that when two particles pass each other, the later one will be drawn 
over the eariler one.
You can adjust this behavior by turning on "blend", which will add particle colors together as they pass by each other
However this does have two draw backs:
1: Due to the way particle trails are drawn, this forces the background to be re-drawn each update cycle,
  which may have a performance impact depending on your strip length, update rate, etc
2: For colored backgrounds, the particles colors are added to the background colors.
  This will in most cases significantly change the particle colors 
  For example, blue particles running on a red background will appear purple (blue +  red = purple)
  This can be used to create some nice effects, (like oceanish of lavaish looking things),
  But overall I do not recommend using blend for colored backgrounds
3: Using blend on segment sets with lines of un-equal length may look weird, because
  pixels may be added to many times where multiple lines converge/overlap

Making a particle set:
A particle set is a struct consisting of an array of particles (technically pointers to particles)
and a length (the number of particles in the array)

For example lets make a particle: particlePS particle1 = {0, true, 80, 1, 1, 5, true, 0}
Based on particlePS.h: this is a particle that starts at pixel 0, is moving forward along the strip at a rate of 80ms,
has a size of 1 and a single trail behind the particle of length 5. It will bounce at each end of the strip.
It will be colored using the 0th element in whatever pallet it is matched with

Now we stick it in a set: 
particlePS *particleArr[] = { &particle1 };
particleSetPS particleSet = {particleArr, SIZE(particleArr)};

we could then pass this particleSet to this effect and it would animate the particle.

You can have any number of particles, each with their own properties running on the strip.
The functions in particleUtilsPS.h let you manipulate particle properties easily 

Because making particles manually is tedious, this effect features a constructor for automatically making a set of particles
including choosing properties at random. I break this down in the constructor inputs below.
The created particle set will be bound to the particleSet pointer in the effect, so you can manipulate it from outside

Please note the following bounce behavior of particles:
For a particle to bounce, it must reverse it's direction once it hits either end of the segmentSet
However, how/when it bounces is a matter of opinion. I have opted for the following:
The particle only bounces when it's main body (not trail) reaches an end point.
This means that a trail in front of the particle disappears off the strip before the bounce. This was done to mimic the classic cylon eye look.
However, trails behind the particle wrap back on themselves as the particle bounces. 
This means that the trail naturally fades as like it would for a physical streamer/flame/etc
For particles where the body size is larger than one, when bounce happens, the entire body reverses direction at once
This is not visually noticable, and makes coding easier. But it does mean there's no "center" of a particle

Example calls: 
    using the particleSet defined above:
    ParticlesSLPS(mainSegments, particleSet, &pallet, CRGB::Red);
    Will animate the single particle as described above, placing it on a red background

    ParticlesSLPS(mainSegments, &pallet3, 0, 3, 2, 60, 50, 1, 3, 2, 3, 0, 2, pallet3.length, true);
    Creates a set of three particles that will run on a blank background with the following properties:
    The particle directions will be choosen at random
    They will have a maximum speed of 60ms, up to 110ms (60 + 50)
    The will have a minimum size of 1, up to 4 (1 + 3)
    They will have two trails of length 3 and no variations in length
    Their bounce properties will be choosen at random
    Their colors will be choosen at random from pallet3

Constructor inputs for creating a particle set:
    pallet -- The pallet than will be used for the particle colors 
    BgColor -- The background color used for the effect
    numParticles -- The number of particles that will be created for the effect
    Direction -- The direction of the particle's motion (pass in any number > 1 to set it randomly)
    baseSpeed -- The minimum speed of the particles
    speedRange -- The amount the speed may vary up from the base speed ( ie baseSpeed + random(range) )
    size -- The minimum size of the particles (min value 1)
    sizeRange -- The amount the size can vary from the base size (ie size + random(range))
    trailType -- The type of trails used for the particles (see below and particlePS.h),
                pass in any number > 4 to set the trails randomly between 0, 1, and 2 trails
                (You may set the trails randomly with more detail using particleUtilsPS::setAllTrailRand() )
    trailSize -- The minimum length of the trails (if the particle has them, min val 1)
    trailRange -- The amount the trailSize can vary from the base size (ie trailSize + random(range))
    bounce -- Whether the particles should reverse direction at either end of the segment set
             pass in any number > 1 to set this randomly
    colorIndex -- The index of the color in the pallet the particles will be
                 If setting randomly, pass in the length of the pallet
    randColor -- If the colors are to be choosen randomly from the pallet (up to the value passed in for colorIndex)

Trail Modes:
Taken from particlePS.h:
Trails blend cleanly into the background color over the trail length
(like waving a flame around, or a metor trail)
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

DimPow:
The rate of dimming for the trails can be adjusted using dimPow. This allows you to produce a brighter head
making the comet effect more noticable
The range of dimPow is -127 to 127, it's defaulted to 80
Positive values quicken the dimming, while negative ones slow it down
setting the negative value below 80, seems to bug it out tho
Slowing the dimming down is useful for colored backgrounds, as it makes the particles stand out more

Functions:
    reset() -- resets all particles to the starting locations
    setParticleSet(particleSetPS *newParticleSet) -- sets the effect to use a new particle set
    update() -- updates the effect 

More functions for adjusting the particles on the fly can be found in particleUtilsPS.h/particleUtilsPS.cpp

Other Settings:
    colorMode (default 0) -- sets the color mode for the particles (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the spacing pixels (see segDrawUtils::setPixelColor)
    dimPow (default 80, min -127, max 127) -- Adjusts the rate of dimming for the trails (see dimPow above)
    blend (default false) -- Causes particles to add their colors to the strip, rather than set them
                            See explanation of this in more detail above in effect intro
    fillBG (default false) -- Sets the background to be redrawn every update, useful for bgColorModes that are dynamic
                             Warning!: Not compatible with infinite trails (mode 4). They will be drawn over.

Notes:
*/
class ParticlesSLPS : public EffectBasePS {
    public:
        //Constructor for having the effect generate a set of particles
        ParticlesSLPS(SegmentSet &SegmentSet, palletPS *Pallet, CRGB BgColor, uint8_t numParticles, uint8_t direction, 
                    uint16_t baseSpeed, uint16_t speedRange, uint16_t size, uint16_t sizeRange, uint8_t trailType, 
                    uint8_t trailSize, uint8_t trailRange, uint8_t bounce, uint8_t colorIndex, bool randColor);  

        //Constructor for using a passed in set of particles
        ParticlesSLPS(SegmentSet &SegmentSet, particleSetPS *ParticleSet, palletPS *Pallet, CRGB BgColor);

        ~ParticlesSLPS();

        SegmentSet 
            &segmentSet; 

        int8_t
            dimPow = 80; //80 range -127 -> 127 -80 good for colored bg's

        uint8_t
            colorMode = 0,
            bgColorMode = 0;
        
        bool
            blend = false, //sets if particles should add onto one another
            fillBG = false;

        CRGB 
            bgColorOrig,
            *bgColor; //bgColor is a pointer so it can be tied to an external variable if needed (such as a pallet color)
        
        palletPS
            *pallet;

        particleSetPS 
            *particleSet, //the particle set used in the effect
            particleSetTemp; //storage for self created particle sets

        void 
            reset(),
            setParticleSet(particleSetPS *newParticleSet),
            update(void);
    
    private:
        //most of these vars are storage for particle properties
        int8_t
            trailDirectionAdj,
            directStep;

        uint8_t
            numSegs,
            trailType,
            numParticles,
            trailSize,
            sizeAdj,
            dimRatio;

        uint16_t 
            speed,
            updateRate = 0, //initilized to 0 to garrentee an initial update
            position,
            size,
            maxPosition,
            trailLocOutput,
            trailLineNum,
            longestSeg,
            numLines,
            pixelNum,
            getTrailLedLoc(bool trailDirect, uint8_t trailPixelNum, uint16_t maxPosition);

        unsigned long
            currentTime,
            prevTime = 0;
        
        bool
            direction,
            bounce,
            movePart;
        
        particlePS
            *particlePtr;

        CRGB 
            *trailEndColors, //used to store the last colors of each trail, so the background color can be set
            colorTarget,
            partColor,
            colorFinal;
        
        void
            init(CRGB BgColor),
            moveParticle(particlePS *particlePtr),
            setTrailColor(uint16_t trailLineNum, uint8_t segNum, uint8_t trailPixelNum);
};

#endif
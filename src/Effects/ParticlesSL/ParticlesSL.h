#ifndef ParticlesSL_h
#define ParticlesSL_h

//KNOWN BUG (seems to be fixed in current FastLED code)
//          -- The first led in the segment set will be set to a static color
//             This bug is very inconsistent, and seems to depend on a memory issue somewhere
//             It seems to mainly affect effects using particles, but exhaustive testing has not been done
//             (only tested on an esp8266)
//             This seems to be a bug with FastLED somewhere? (putting an led[0] = 0 right before calling FastLED.show() didn't stop it)
//             To fix it, you simply need to call a Serial.println(<a program var>) somewhere in your code
//             (probably best to put it in the setup() and print out an random program var)

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"
#include "Particle_Stuff/particleUtilsPS.h"

/* 
An effect for updating a set of particles. A particle is a moving pixel. 
A particle has a direction, speed, and can leave a trail that blends cleanly into the background color over 
the trail length (like waving a flame around, or a comet trail). 
Particles are grouped into particle sets.
Each particle in a set has its own properties, so you can have multiple 
particles with different sizes, speeds, trails, etc. This effect animates a set of particles.

Before using this effect, you should read about the full details of particles see particlePS.h.
For this effect, you have the option of using your own particle set, or have the effect create one for you. 

You have a lot of options when creating a particle set, so you can use this effect to build some really great looks!

The effect is adapted to work on segment lines for 2D use.
For 2D segments particles move along segment lines, with the particle being replicated across 
all the segment, like a wave.

Supports color modes for both the main and background colors.

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

Trail Fading:
    By default, the trails dim quickly in a non-linear fashion. T
    his makes the particle "heads" brighter and standout more, which, in my opinion, 
    looks better then just using a linear fade. You can control the linearity of the trail 
    fades using the "dimPow" setting. A default of 80 is used in this effect. 
    You can read the "dimPow" notes in "particleUtils.cpp, getTrailcolor()" for more.

Blending:
    Due to the way the effect is programmed, particles that are placed later in the particleSet will run "in front"
    of those earlier in the set. This means that when two particles pass each other, the later one will be drawn 
    over the earlier one.

    You can adjust this behavior by turning on "blend", which will add particle colors together as they pass by each other
    However this does have two draw backs:
        1: Due to the way particle trails are drawn, this forces the background to be re-drawn each update cycle,
           which may have a performance impact depending on your strip length, update rate, etc
        2: For colored backgrounds, the particles colors are added to the background colors.
           This will in most cases significantly change the particle colors 
           For example, blue particles running on a red background will appear purple (blue +  red = purple)
           This can be used to create some nice effects, (like ocean-ish of lava-ish looking things),
           But overall I do not recommend using blend for colored backgrounds
        3: Using blend on segment sets with lines of un-equal length may look weird, because
           pixels may be added to many times where multiple lines converge/overlap

Bounce Behavior:
    For a particle to bounce, it must reverse its direction once it hits either end of the segment set.
    However, how/when it bounces is a matter of opinion. 

    I have opted for the following:
        * The particle only bounces when its main body (not trail) reaches an end point.
        * Both the front and rear trails wrap back on themselves as the particle bounces. 
          Ie the head of the trail moves back down the strip, opposite the direction of the particle.
        * The rear trail is always drawn last. In practice this means that particles with two trails 
          mimic the classic "Cylon" scanner look, where the front of the trail disappears off the strip 
          (it is actually wrapping back, but is over written by the rear trail, which is drawn after).
        While for particles with only a rear trail, it naturally fades as like it would for a physical streamer/flame/etc.   
        * For particles with only a front trail the trail also wraps back, but under the particle. 
          This does look a little weird, but there's not a good real world approximation to this kind of particle, so w/e.
        * For particles where the body size is larger than one, when a bounce happens, 
          the entire body reverses direction at once. This is not visually noticeable, 
          and makes coding easier. However, it does mean there's no "center" of a particle.

Update Rate:
    Note that unlike other effects, the update rate is pre-set for you at 5ms, 
    which helps catch each of the particles updates on time (hopefully!). 
    You see, each particle has its own speed (update rate), but we still want be able to update the effect
    with a single update() call. I could have ignored the "rate" setting, and just updated all the 
    particles whenever you call update(). However, the particles are re-drawn every update(), even if they haven't moved,
    so this becomes problematic, especially when working with multiple effects or segment sets, where you want more
    control over when you update. Instead I opted to treat the effect as normal, and keep the overall update rate,
    but default it to a very fast 5ms. Usually I set my speeds to multiples of 10, so hopefully this default
    will catch most particles on time. You are free to change the update rate as needed by setting rateOrig.
    (Note, just setting the update rate to the speed of the fastest particle doesn't work, just think of two
    particles with speeds 40 and 50. The 50 speed particle ends up moving at more like 80 due to the 40ms update rate).

Example calls: 
    using the particleSet defined above:
    ParticlesSL particles(mainSegments, particleSet, cybPnkPal_PS, CRGB::Red);
    Will animate the single particle as described above, placing it on a red background

    ParticlesSL particles(mainSegments, cybPnkPal_PS, 0, 3, 2, 60, 50, 1, 3, 2, 3, 0, 2, cybPnkPal_PS.length, true);
    Creates a set of three particles that will run on a blank background with the following properties:
    The particle directions will be chosen at random
    They will have a maximum speed of 60ms, up to 110ms (60 + 50)
    The will have a minimum size of 1, up to 4 (1 + 3)
    They will have two trails of length 3 and no variations in length
    Their bounce properties will be chosen at random
    Their colors will be chosen at random from cybPnkPal_PS

Constructor inputs for creating a particle set:
    palette -- The palette than will be used for the particle colors 
    BgColor -- The background color used for the effect
    numParticles -- How many particles the effect will have. Note that the particles will be given random start points.
    direction -- The direction of the particle's motion (pass in any number > 1 to set it randomly)
    baseSpeed -- The base speed of the particles (ms)
    speedRange -- The amount the speed may vary up from the base speed ( ie baseSpeed + random(range) ) (ms)
    size -- The minimum size of the particles (min value 1)
    sizeRange -- The amount the size can vary from the base size (ie size + random(range))
    trailType -- The type of trails used for the particles (see below and particlePS.h),
                pass in any number > 4 to set the trails randomly between 0, 1, and 2 trails
                (You may set the trails randomly with more detail using particleUtilsPS::setAllTrailRand() )
    trailSize -- The minimum length of the trails (if the particle has them, min val 1)
    trailRange -- The amount the trailSize can vary from the base size (ie trailSize + random(range))
    bounce -- Whether the particles should reverse direction at either end of the segment set
              pass in any number > 1 to set this randomly
    colorIndex -- The index of the color in the palette the particles will be
                  If setting randomly, pass in the length of the palette
    randColor -- If the colors are to be chosen randomly from the palette (up to the value passed in for colorIndex)

Other Settings:
    colorMode (default 0) -- sets the color mode for the particles (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the spacing pixels (see segDrawUtils::setPixelColor)
    dimPow (default 80, min -127, max 127) -- Adjusts the rate of dimming for the trails (see Fading notes in intro)
    blend (default false) -- Causes particles to add their colors to the strip, rather than set them
                            See explanation of this in more detail above in effect intro
    fillBg (default false) -- Sets the background to be redrawn every update, useful for bgColorModes that are dynamic
                             Warning!: Not compatible with infinite trails (mode 4). They will be drawn over.
    *particleSet -- The effect's particle set. Is a pointer, so you can bind it to an external set. 
                    If the effect builds a particle set for you, `particleSet`, 
                    will be bound to the effect's local set, `particleSetTemp`. 
    *rate and rateOrig -- Update rate (ms). Defaulted to 5ms (see Update note in Inputs Guide). 
                          Is a pointer, allowing you to bind it to an external variable. 
                          By default it's bound the effect's local variable, `rateOrig`. 

Functions:
    reset() -- resets all particles to the starting locations
    setParticleSet(particleSetPS *newParticleSet) -- Sets the effect to use a new particle set
    update() -- updates the effect 

More functions for adjusting the particles on the fly can be found in particleUtilsPS.h/particleUtilsPS.cpp

*/
class ParticlesSL : public EffectBasePS {
    public:
        //Constructor for having the effect generate a set of particles
        ParticlesSL(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, uint16_t numParticles, uint8_t direction,
                    uint16_t baseSpeed, uint16_t speedRange, uint16_t size, uint16_t sizeRange, uint8_t trailType,
                    uint8_t trailSize, uint8_t trailRange, uint8_t bounce, uint8_t colorIndex, bool randColor);

        //Constructor for using a predefined set of particles
        ParticlesSL(SegmentSetPS &SegSet, particleSetPS &ParticleSet, palettePS &Palette, CRGB BgColor);

        ~ParticlesSL();

        int8_t
            dimPow = 80;  //80 range -127 -> 127 -80 good for colored bg's

        uint8_t
            colorMode = 0,
            bgColorMode = 0;

        bool
            blend = false,  //sets if particles should add onto one another
            fillBg = false;

        CRGB
            *trailEndColors = nullptr,  //used to store the last colors of each trail, so the background color can be set
            bgColorOrig,
            *bgColor = nullptr;  //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)

        palettePS
            *palette = nullptr;

        particleSetPS
            *particleSet = nullptr,          //the particle set used in the effect
            particleSetTemp = {nullptr, 0};  //storage for self created particle sets, init to empty for safety.

        void
            reset(),
            setParticleSet(particleSetPS &newParticleSet),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0;

        //most of these vars are storage for particle properties
        int8_t
            trailDirectionAdj,
            directStep;

        uint8_t
            trailType,
            trailSize,
            sizeAdj,
            dimRatio;

        uint16_t
            numParticles,
            speed,
            updateRate = 0,  //initialized to 0 to ensure an initial update
            position,
            size,
            maxPosition,
            trailLocOutput,
            trailLineNum,
            longestSeg,
            numLines,
            numSegs,
            pixelNum,
            getTrailLedLoc(bool trailDirect, uint8_t trailPixelNum, uint16_t maxPosition);

        bool
            direction,
            bounce,
            movePart;

        particlePS
            *particlePtr = nullptr;

        CRGB
            colorTarget,
            partColor,
            colorFinal;

        void
            init(CRGB BgColor, SegmentSetPS &SegSet),
            moveParticle(particlePS *particlePtr),
            setTrailColor(uint16_t trailLineNum, uint16_t segNum, uint8_t trailPixelNum);
};

#endif
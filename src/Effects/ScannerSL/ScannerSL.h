#ifndef ScannerSL_h
#define ScannerSL_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"
#include "Effects/ParticlesSL/Particle_Stuff/particleUtilsPS.h"

/* 
This effect creates a customizable set of "scanner" waves. Think like the classic Cylon visor eye. 
You have a lot of control over what the scanner waves look like, how they bounce, if/when the change colors, etc.
It is a cross between the ParticlesSL and LarsonScannerSL effects, 
being more focused than the former, but more general than the latter. 

The effect is adapted to work on segment lines for 2D use. 
The scanner wave colors will be copied along segment lines.

Supports Color Modes for both the main and background colors.

Note that the effect requires an array of CRGB colors and a particleSet of length `numWaves`. 
These are allocated dynamically, so, to avoid memory fragmentation, 
when you create the effect, you should set `numWaves` to the maximum value you expect to use. 
See the memory management wiki page:
https://github.com/AlbertGBarber/PixelSpork/wiki/Effects-Advanced#managing-dynamic-memory-and-fragmentation 
for more details. 

Inputs Guide:
    The effect is designed to build a set of scanner waves (particles) for you. 
    This allows you to create classic scanner types, like the Cylon and Kitt scanners, 
    but also gives you access to more complex, custom, scanning patterns. 
    The effect creates a set of evenly spaced waves, all with the same trail, speed, size, etc settings. 
    The effect is unique in that it can cycle through wave colors automatically. 
    Note that the waves use a pattern/palette combo for colors like in other effects.
    
    Wave Settings:   
        trailType -- The trail type of the waves (see Trail Types below).

        trailSize -- The length of the trails. Using 0 will turn off the trails for the waves, 
                     (must turn them back on manually by setting the trailType for the effect)

        size -- The size of the main body of the waves (not the trails) (min value 1).

        bounce -- If true, the waves will reverse, "bounce back", at the end of the segment set.
                  If false the waves will wrap back to the start at each segment set end.
    
    Trail Types:
        Taken from particlePS.h:
        Trails blend cleanly into the background color over the trail length
        (like waving a flame around, or a meteor trail)
        Trail options:
        0: no trails
        1: one trail facing away from the direction of motion (like a comet)
        2: two trails, facing towards both directions of motion
        3: one trail facing towards the direction of motion
        4: Infinite trails that persist after the wave (no fading)
        For example, with a trail length of 4, the modes will produce:
        (The trail head is *, - are the trail, wave is moving to the right ->)
        0:     *
        1: ----* 
        2: ----*----
        3:     *----
        4: *****
    
    Wave Colors:
        *Wave colors are tied to the effect palette and pattern (you can set the palette as the pattern).
        *Waves will either alternate colors every time they bounce/wrap, every other time they bounce, or never.
        *Colors can be set to be chosen at random (see randModes below)

        bounceChange -- If bounce is true:
                            If true, the waves will change colors every time they bounce.
                            If false then they will change every other time.
                        If bounce is false:
                            The waves will change colors every time they wrap to 0.
                        
        
        randMode 3 -- To lock the waves colors so that they never change, use `randMode` 3.
        
        blend -- If true, then waves' colors will be blended together when they pass each other. 
                 Not recommended for colored backgrounds.
                 See "blending" entry in ParticlesSL.h for more.
        
        dimPow -- By default, the trails dim quickly in a non-linear fashion. 
                  This makes the particle "heads" brighter and standout more, which, in my opinion, 
                  looks better then just using a linear fade. You can control the linearity of the trail 
                  fades using the "dimPow" setting. A default of 80 is used in this effect. 
                  You can read the "dimPow" notes in "particleUtils.h" for more.
    
    randModes (default 0):
        Sets how wave colors will be chosen when they change.
        0: Colors will be chosen in order from the pattern (not random).
        1: Colors will be chosen randomly from the pattern.
        2: Colors will be chosen at random from the pattern, but the same color won't be repeated in a row.
        3: New colors will not be chosen, the wave colors will be locked to whatever they currently are.
    
    Bounce Behavior:
        For a wave to bounce, it must reverse its direction once it hits either end of the segment set.
        However, how/when it bounces is a matter of opinion. 

        I have opted for the following:
            * The wave only bounces when its main body (not trail) reaches an end point.
            * Both the front and rear trails wrap back on themselves as the wave bounces. 
              Ie the head of the trail moves back down the strip, opposite the direction of the wave.
            * The rear trail is always drawn last. In practice this means that particles with two trails 
               mimic the classic "Cylon" scanner look, where the front of the trail disappears off the strip 
              (it is actually wrapping back, but is over written by the rear trail, which is drawn after).
              While for particles with only a rear trail, it naturally fades as like it would for a 
              physical streamer/flame/etc.   
            * For particles with only a front trail the trail also wraps back, but under the wave. 
              This does look a little weird, but there's not a good real world approximation to this kind of 
              wave, so w/e.
            * For particles where the body size is larger than one, when a bounce happens, 
              the entire body reverses direction at once. This is not visually noticeable, 
              and makes coding easier. However, it does mean there's no "center" of a wave.
        
    Finally, you can also use custom wave configurations by setting the particleSet directly,
    but remember, the effect size, trail, and speed settings apply to all waves. 
    If you want unique waves, you'll need to use the ParticlesSL effect.

The makeWaveSet() Function: (explained here b/c it's long)

    void makeWaveSet(uint16_t numWaves, bool direction, bool alternate, bool makeEndWave);

    The makeWaveSet() function both creates and positions a set of waves evenly across the segment set.
    The function is automatically called as part of the constructors, so you only need to call it manually if you want to
    make a new wave set. Note that `makeWaveSet()` only sets where the waves start and their directions.  
    Does not clear any left-over wave bits from the segment set!
    
    The inputs to the function are also used as inputs to the constructors:

    numWaves -- How many scan waves will be created.

    direction -- The starting direction for the waves (true = forward). The first wave will always use this direction.

    alternate -- If true, the the wave starting directions will alternate with each wave
                 ie, if direction is true, then the first wave will go forward, the next in reverse, etc

    makeEndWave -- Sets how the waves are spaced. It is important for setting the starting positions of your waves
                   so that they remain evenly spaced when moving, depending on if they wrap or bounce at each segment end. 
                   
                   * If you have a single wave, `makeEndWave` has no effect; the wave always starts at 0.
                   
                   * For multiple waves, if makeEndWave is true, then the waves will be spaced
                     such that one wave starts at the end of the segment set.
                     (note that if makeEndWave is true, you should also set alternate true, otherwise the end/start waves will overlap)
                     If makeEndWave is false, then the waves will be spaced evenly across the segment set, starting from 0.
                    
                   * For example, lets say I have 9 leds, and want three waves with size 1, no trails:

                    start -- -- -- -- -- -- -- -- -- (each -- is an LED)

                    If we set makeEndWave true, then the initial setup will look like:

                    start ** -- -- -- ** -- -- -- ** (each ** is an wave)
                    Evenly spaced, but making sure that one wave starts at the end (doing numLines / (numWaves - 1) for spacing)

                    If makeEndWave is false, then the initial setup will look like:

                    start ** -- -- ** -- -- ** -- -- (each ** is an wave)
                    Evenly spaced (doing numLines / numWaves for spacing)

Example calls: 
    ScannerSL scanner(mainSegments, CRGB::Red, 0, 2, 1, 5, 1, true, true, true, true, true, 80);
    Will do a classic "Kitt" scanner, with 2 red particles moving back and forth, meeting in the middle of the segment set
    The background is blank.
    The scan particles will have 1 trail of length 5, a body size of 1
    The particles will start with alternating directions, one particle will start at each end of the segment set
    they will bounce at each segment set end, and blend together when they meet
    The particles move at 80ms

    ScannerSL scanner(mainSegments, cybPnkPal_PS, 0, 3, 2, 4, 2, true, false, false, false, true, false, 80);
    Will automatically create a set of 3 scanner particles
    There will be 3 particles spaced evenly on the segment set (makeEndWave is false)
    The particles will take their colors from the cybPnkPal_PS palette, with a blank background.
    Each particle will have 2 trails of length 4, with a body size of 2
    The particles will start in the forward direction (not set to alternate)
    The particles will not bounce at each end of the segment set, and will wrap instead
    (bounceChange is true, but doesn't matter for wrapping particles)
    Blending is turned off
    The particles move at 80ms

    (note that there is an additional constructor that uses a pattern. This is 
    the same as the above constructor, but a patternPS is included before the palette)

Constructor inputs:
    color(optional, see constructors) -- if used, the scan waves will all be a single color
    pattern(optional, see constructors) -- The pattern used for the waves, made up of palette indexes 
    palette(optional, see constructors) -- The repository of colors used in the pattern
    bgColor -- The background color used for the effect.
    numWaves -- How many scan waves will be created. Can be changed later using set using makeWaveSet().
    trailType -- The trail type of the waves. (See Inputs Guide above)
    trailSize -- The length of the trails. Using 0 will turn off the trails for the waves. 
                 (must turn them back on manually by setting the trailType for the effect).
    size -- The size of the main body of the waves (not the trails) (min value 1). 
    direction -- The starting direction for the waves (true = forward). The first wave will always use this direction.
    alternate -- If true, the the wave starting directions will alternate with each wave
                 ie, if direction is true, then the first wave will go forward, the next in reverse, etc
    makeEndWave -- (See Inputs Guide above)
    bounce -- Sets if the waves reverse ("bounce") at each end of the segment set, if not then they wrap to the start/end.
    bounceChange (set false for the single color constructor) -- Sets when the wave colors change. (See Inputs Guide above)
    blend -- If true, then waves' colors will be blended together when they pass each other. (See Wave Colors in Intro)
    rate -- The update rate (ms) note that this is synced with all the particles.

Other Settings:
    colorMode (default 0) -- sets the color mode for the wave pixels (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the background pixels (see segDrawUtils::setPixelColor)
    fillBg (default false) -- If true, the segment set will be filled with the background every update cycle
    dimPow (default 80, min -127, max 127) -- Adjusts the rate of dimming for the trails (see Wave Colors in Intro)
    randMode (default 0) -- Sets how colors are chosen (see randMode notes above)
    particleSet -- The particleSet used to store the scan particles 
                   (you can use it to directly set the position of the waves)

Functions:
    setPaletteAsPattern() -- Sets the palette to be the pattern (each color in the palette, in order)
    reset() -- Resets all the waves to their start locations and sets their starting colors. 
               Also fills the background to remove any old waves.
    makeWaveSet(numWaves, direction, alternate, makeEndWave) -- Creates a set of evenly space repeating waves
                                                                (see Inputs Guide above)
    update() -- updates the effect 

Reference Vars:
    numWaves -- The number of wave particles. Set using makeWaveSet().
*/
class ScannerSL : public EffectBasePS {
    public:
        //Constructor using a single color
        ScannerSL(SegmentSetPS &SegSet, CRGB Color, CRGB BgColor, uint16_t numWaves, uint8_t TrailType, uint16_t TrailSize,
                  uint16_t Size, bool direction, bool alternate, bool makeEndWave, bool Bounce,
                  bool Blend, uint16_t Rate);

        //Constructor for using a pattern with a custom set of repeating waves
        ScannerSL(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, CRGB BgColor, uint16_t numWaves,
                  uint8_t TrailType, uint16_t TrailSize, uint16_t Size, bool direction, bool alternate, bool makeEndWave,
                  bool Bounce, bool BounceChange, bool Blend, uint16_t Rate);

        //Constructor for using the palette as the pattern with a custom set of repeating waves
        ScannerSL(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, uint16_t numWaves, uint8_t TrailType,
                  uint16_t TrailSize, uint16_t Size, bool direction, bool alternate, bool makeEndWave,
                  bool Bounce, bool BounceChange, bool Blend, uint16_t Rate);

        ~ScannerSL();

        int8_t
            dimPow = 80;  //80 range -127 -> 127, -80 good for colored bg's

        uint8_t
            colorMode = 0,
            bgColorMode = 0,
            randMode = 0,
            trailType,
            trailSize;  //min value of 1

        uint16_t
            numWaves, //The number of wave particles, For reference, set using makeWaveSet().
            size;

        bool
            blend = false,  //sets if particles should add onto one another
            fillBg = false,
            bounce,
            bounceChange;

        CRGB
            *trailEndColors = nullptr,  //used to store the last colors of each trail, so the background color can be set
            bgColorOrig,
            *bgColor = nullptr;  //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)

        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0};  //Must init structs w/ pointers set to null for safety

        patternPS
            *pattern = nullptr,
            patternTemp = {nullptr, 0, 0};  //Must init structs w/ pointers set to null for safety

        particleSetPS
            particleSet = {nullptr, 0, 0};  //Must init structs w/ pointers set to null for safety

        void
            reset(),
            setPaletteAsPattern(),
            makeWaveSet(uint16_t numWaves, bool direction, bool alternate, bool makeEndWave),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0;

        int8_t
            trailDirectionAdj,
            directStep;

        uint8_t
            sizeAdj,
            dimRatio,
            nextPalletIndex;

        uint16_t
            position,
            patternIndexCount = 0,
            maxPosition,
            trailLocOutput,
            trailLineNum,
            longestSeg,
            numLines,
            numSegs,
            pixelNum,
            getTrailLocAndColor(bool trailDirect, uint8_t trailPixelNum, uint16_t maxPosition);

        bool
            direction,
            startDirect;

        particlePS
            *particlePtr = nullptr;

        CRGB
            colorTarget,
            partColor,
            nextColor,
            prevColor,
            trailBgColor,
            colorFinal,
            getPartPixelColor(uint16_t partPixelLoc, bool trailDirect);

        void
            init(CRGB BgColor, SegmentSetPS &SegSet, uint16_t Rate),
            moveParticle(particlePS *particlePtr),
            setPartColor(particlePS *particlePtr),
            setTrailColor(uint16_t trailLineNum, uint16_t segNum, uint8_t trailPixelNum);
};

#endif
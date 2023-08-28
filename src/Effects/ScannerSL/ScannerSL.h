#ifndef ScannerSL_h
#define ScannerSL_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"
#include "Effects/ParticlesSL/Particle_Stuff/particleUtilsPS.h"

/* 
This effect is similar to larson scanner, but gives you the option to use multiple colors and 
automatically build more complex waves. 
Basically the goal of this effect is to create "sonar" or "scanner" waves that travel across the segment set.
Like with particles you can set the trail type, trail size, particle size, speed, etc 
(see particlePS.h for more info on particles) but all the waves share the same values.

(See Inputs Guide below for more)

The effect is fully compatible with colorModes.
The background color is a pointer, so you can bind it externally as needed.

The effect is setup to work using segment lines, so creating 2D effects is easy.

Inputs Guide:
    The code is designed to pre-build a set of scanner waves for you. This allows you to create
    classic scanner types, like the Cylon and Kitt scanners, but also gives you access to more complex scanning patterns.
    
    The makeWaveSet() Function:
        The makeWaveSet() function both creates and positions a set of waves evenly across the segment set.
        The function is automatically called as part of the constructors, so you only need to call it manually if you want to
        make a new wave set. The inputs to the function are also used as inputs to the constructors:

        numWaves -- How many scan waves will be created.

        direction -- The starting direction for the waves (true = forward). The first wave will always use this direction.

        alternate -- If true, the the wave starting directions will alternate with each wave
                     ie, if direction is true, then the first wave will go forward, the next in reverse, etc

        makeEndWave -- Sets how the waves are spaced:
                       The first wave always starts at 0 + (it size)
                       If you have a single wave, this has no effect. The wave always starts at 0.
                       For multiple waves, if makeEndWave is true, then the waves will be spaced
                       such that one wave starts at the end of the segment set.
                       (note that if makeEndWave is true, you should also set alternate true, otherwise the end/start waves will overlap)
                       If makeEndWave is false, then the waves will be spaced evenly across the segment set, starting from 0.
                        
                       For example, lets say I have 9 leds, and want three waves with size 1, no trails:

                       start -- -- -- -- -- -- -- -- -- (each -- is an LED)

                       If we set makeEndWave true, then the initial setup will look like:

                       start ** -- -- -- ** -- -- -- ** (each ** is an wave)
                       Evenly spaced, but making sure that one wave starts at the end (doing numLines / (numWaves - 1) for spacing)

                       If makeEndWave is false, then the initial setup will look like:

                       start ** -- -- ** -- -- ** -- -- (each ** is an wave)
                       Evenly spaced (doing numLines / numWaves for spacing)
        
        Note that makeWaveSet() only sets where the waves start and their directions. 
        
        There are more settings for controlling how the waves bounce, change color, their size, etc.
        These are also constructor inputs, but you are free to change them while the effect is running.
        Note that all waves share these settings, if you want unique waves, you'll need to use the ParticlesPS effect.

    Wave Settings:
    
        trailType -- The trail type of the waves (see particlePS.h) (1 for one trail, 2 for two trails, etc).

        trailSize -- The length of the trails. Using 0 will turn off the trails for the waves, 
                     and you'd need to turn them back on manually by setting the trailType for all the waves.

        size -- The size of the main body of the waves (not the trails) (min value 1)

        bounce -- If true, the waves will bounce back at the end of the segment set.
                  If false the waves will wrap back to the start at each segment set end.
    
    Wave Colors:

        *Wave colors are tied to the effect palette and pattern (you can set the palette as the pattern).
        *Waves will either alternate colors every time they bounce/wrap or every other time they bounce, or never.
        *Colors can be set to be chosen at random (see randModes below)

        bounceChange -- If bounce is true:
                            If true, the waves will change colors every time they bounce.
                            If false then they will change every other time.
                        If bounce is false:
                            The waves will change colors every time they wrap to 0.
                            (To lock wave colors use randMode 3)
        
        blend -- If true, then waves' colors will be blended together when they pass each other.

You can also use custom wave configurations by setting the particleSet directly,
but remember, the effect size and trail settings apply to all waves.
    
randModes:
    0: Colors will be chosen in order from the pattern (not random)
    1: Colors will be chosen randomly from the pattern
    2: Colors will be chosen at random from the pattern, but the same color won't be repeated in a row
    3: New colors will not be chosen, the particle colors will be locked to whatever they currently are.

Example calls: 
    ScannerSL scanner(mainSegments, CRGB::Red, 0, 2, 1, 5, 1, true, true, true, true, true, 80);
    Will do a classic "Kitt" scanner, with 2 red particles moving back and forth, meeting in the middle of the segment set
    The background is blank.
    The scan particles will have 1 trail of length 5, a body size of 1
    The particles will start with alternating directions, one particle will start at each end of the segment set
    they will bounce at each segment set end, and blend together when they meet
    The particles move at 80ms

    ScannerSL scanner(mainSegments, cybPnkPal_PS, 0, 3, 2, 4, 2, true, false, false, false, true, false, 80);
    Will do a scanner, automatically creating a set of 3 evenly spaced scanner particles
    There will be 3 particles spaced evenly on the segment set (makeEndWave is false)
    The particles will take their colors from cybPnkPal_PS, with a blank background.
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
    numWaves -- (See Inputs Guide above)
    trailType -- (See Inputs Guide above)
    trailSize -- (See Inputs Guide above)
    size -- (See Inputs Guide above)
    direction -- (See Inputs Guide above)
    alternate -- (See Inputs Guide above)
    makeEndWave -- (See Inputs Guide above)
    bounce -- (See Inputs Guide above)
    bounceChange(set false for the single color constructor) -- (See Inputs Guide above)
    blend -- (See Inputs Guide above)
    rate -- The update rate (ms) note that this is synced with all the particles.

Functions:
    setPaletteAsPattern() -- Sets the palette to be the pattern (each color in the palette, in order)
    makeWaveSet(numWaves, direction, alternate, makeEndWave) -- Creates a set of evenly space repeating waves
                                                                (see Inputs Guide above)
    update() -- updates the effect 

Other Settings:
    colorMode (default 0) -- sets the color mode for the wave pixels (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the background pixels (see segDrawUtils::setPixelColor)
    dimPow (default 80, min -127, max 127) -- Adjusts the rate of dimming for the trails (see particleUtilsPS.h)
    fillBG (default false) -- If true, the segment set will be filled with the background every update cycle
    particleSet -- The particleSet used to store the scan particles (you can use it to directly customize the waves)
    randMode -- Sets how colors are chosen (see randMode notes above)
*/
class ScannerSL : public EffectBasePS {
    public:

        ScannerSL(SegmentSetPS &SegSet, CRGB Color, CRGB BGColor, uint16_t numWaves, uint8_t TrailType, uint16_t TrailSize,
                  uint16_t Size, bool direction, bool alternate, bool makeEndWave, bool Bounce,
                  bool Blend, uint16_t Rate);

        //Constructor for using a pattern with a custom set of repeating waves
        ScannerSL(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, CRGB BGColor, uint16_t numWaves,
                  uint8_t TrailType, uint16_t TrailSize, uint16_t Size, bool direction, bool alternate, bool makeEndWave,
                  bool Bounce, bool BounceChange, bool Blend, uint16_t Rate);

        //Constructor for using the palette as the pattern with a custom set of repeating waves
        ScannerSL(SegmentSetPS &SegSet, palettePS &Palette, CRGB BGColor, uint16_t numWaves, uint8_t TrailType,
                  uint16_t TrailSize, uint16_t Size, bool direction, bool alternate, bool makeEndWave,
                  bool Bounce, bool BounceChange, bool Blend, uint16_t Rate);

        ~ScannerSL();

        int8_t
            dimPow = 80;  //80 range -127 -> 127 -80 good for colored bg's

        uint8_t
            colorMode = 0,
            bgColorMode = 0,
            randMode = 0,
            trailType,
            trailSize;  //min value of 1

        uint16_t
            size;

        bool
            blend = false,  //sets if particles should add onto one another
            fillBG = false,
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
            particleSet = {nullptr, 0};  //Must init structs w/ pointers set to null for safety

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
            numParticles,
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
            setTrailColor(uint16_t trailLineNum, uint8_t segNum, uint8_t trailPixelNum);
};

#endif
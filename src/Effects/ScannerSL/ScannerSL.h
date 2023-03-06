#ifndef ScannerSL_h
#define ScannerSL_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"
#include "Effects/ParticlesSL/Particle_Stuff/particleUtilsPS.h"

/* 
This effect is similar to larson scanner, but gives you the option to use multiple colors and 
automatically build more complex waves. 
Basically the goal of this effect is to create "sonar" or "scanner" waves that travel across the segment set
Like with particles you can set the trail type, trail size, particle size, speed, etc 
(see particlePS.h for more info on particles)
You can set these properties on the fly, but all the particles share the same values.

Much of the code follows the same principles as ParticlesSL.h, but with the restriction that all particles
have the same properties (speed, trails, etc), but can switch colors as part of the effect

Particles can be set to:
    *Either bounce or wrap at each segment set end
    *Either change colors every time they bounce/wrap, every other time they bounce, or not at all.
    *You can also set the colors to be choosen at random from the pattern using randMode

The particle colors are set according to a passed in color pattern (or be set to copy the palette)

Like larson scanner, there are constructor short cuts for creating classic scan types, 
but in addition, I've included constructors for automatically creating sets of evenly spaced waves.
All you need to do is set the number of waves and the constructor does the rest.

Overall this lets you create a lot of neat "scanner' type effects.

You can also use custom particle configurations be setting the particleSet directly.

The effect is fully compatible with colorModes.
The background color is a pointer, so you can bind it externally as needed.

The effect is setup to work using segment lines, so creating 2D effects is easy.

Three scanner types are built in:
    0: Like the classic cylon scanner, one particle with two trails moving back and forth
    1: Like the cylon scanner, but only using one trail
    2: Like one of the Kit Knight Rider scanners: two particles with single trails
        That move back and forth, intersecting in the center of the strip
        (note that this mode uses blend, see ParticlesPS.h for details)
    
randModes:
    0: Colors will be choosen in order from the pattern (not random)
    1: Colors will be choosen randomly from the pattern
    2: Colors will be choosen at random from the pattern,
       but the same color won't be repeated in a row
    3: new colors will not be choosen, the particle colors will be locked to whatever they currently are.

Example calls: 
    ScannerSL(mainSegments, palette3, 0, 2, 3, 1, false, true, 80);
    Will do a scanner in mode 3 (Kit) using colors from palette3.
    The background is blank.
    The scan particles will have a trails of length 3, a body size of 1
    The particles will wrap (not bounce), changing colors as they wrap
    (bounceChange is true, but this doesn't effect particles that are wrapping)
    The particles move at 80ms

    ScannerSL(mainSegments, palette3, 0, 2, 1, 5, 2, true, false, true, true, false, 80);
    Will do a scanner, automatically creating a set of evenly spaced scanner particles
    There will be two particles spaced evenly on the segment set
    The particles will take their colors from palette3, with a blank background.
    Each particle will have 1 trail of length 5, with a body size of 2
    The particles will start in the forward direction (not set to alternate)
    The particles will bounce at each end of the segment set, changing their colors each time
    Blending is turned off
    The particles move at 80ms

    (note that there are two additional constructor types that use a pattern, these are 
    the same as the above constructors, but a patternPS is included before the palette)

Constructor inputs for using a default scan type:
    pattern(optional, see constructors) -- The pattern used for the streamers, made up of palette indexes 
    palette(optional, see constructors) -- The repository of colors used in the pattern
    bgColor -- The background color used for the effect.
    scanType -- The type of scan type (see above)
    trailSize -- The length of the trails. Using 0 will turn off the trails for the scanner.
    size -- The size of the particles (min value 1)
    bounce -- Sets if the particles will bounce or wrap at each segment set end
    bounceChange -- Sets if the particles change colors every time they bounce (or every other time)
    rate -- The update rate (ms) note that this is synced with all the particles.

Constructor inputs for using the automated evenly spaced waves:
    pattern(optional, see constructors) -- The pattern used for the streamers, made up of palette indexes 
    palette(optional, see constructors) -- The repository of colors used in the pattern
    bgColor -- The background color used for the effect.
    numWaves -- How many particles will be created
    trailType -- The trail type of the particles
    trailSize -- The length of the trails. Using 0 will turn off the trails for the scanner.
    size -- The size of the particles (min value 1)
    direction -- The starting direction for the particles
    alternate -- Sets if every other particle will have its starting direction swapped
    bounce -- Sets if the particles will bounce or wrap at each segment set end
    bounceChange -- Sets if the particles change colors every time they bounce (or every other time)
    blend -- Sets if the particles will be blended togther as they pass over one another
    rate -- The update rate (ms) note that this is synced with all the particles.

Functions:
    setPaletteAsPattern() -- Sets the palette to be the pattern (each color in the palette, inorder)
    setScanType(newType) -- sets the scan mode (note this re-creates all the particles from scratch)
    makeWaveSet(numWaves, direction, alternate) -- Creates a set of evenly space repeating particles
                                                   (note this re-creates all the particles from scratch)
    update() -- updates the effect 

Other Settings:
    colorMode (default 0) -- sets the color mode for the streamer pixels (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the spacing pixels (see segDrawUtils::setPixelColor)
    dimPow (default 80, min -127, max 127) -- Adjusts the rate of dimming for the trails (see particleUtilsPS.h)
    trailSize -- The The length of the trails (min value of 1)
    trailType -- The type of trails for the particles (see particlePS.h)
    size -- The length of the scan particle's body (not trail) (min value of 1)
    bounce -- The bounce property of the particles.
    blend -- Sets if the particles will be blended togther as they pass over one another
    bounceChange -- Sets if the particles change colors every time they bounce (or every other time)
    fillBG (default false) -- If true, the segment set will be filled with the background every update cycle
    particleSet -- The particleSet used to store the scan particles (and passed to the ParticlePS instance)
    randMode -- Sets how colors are choosen (see randMode notes above)
*/
class ScannerSL : public EffectBasePS {
    public:
        //Constructor for using a pattern with one of the default scan types
        ScannerSL(SegmentSet &SegmentSet, patternPS &Pattern, palettePS &Palette, CRGB BGColor, uint8_t ScanType,
                  uint16_t TrailSize, uint16_t Size, bool Bounce, bool BounceChange, uint16_t Rate);

        //Constructor for using the palette as the pattern with one of the default scan types
        ScannerSL(SegmentSet &SegmentSet, palettePS &Palette, CRGB BGColor, uint8_t ScanType, uint16_t TrailSize, 
                  uint16_t Size, bool Bounce, bool BounceChange, uint16_t Rate);

        //Constructor for using a pattern with a custom set of repeating waves
        ScannerSL(SegmentSet &SegmentSet, patternPS &Pattern, palettePS &Palette, CRGB BGColor, uint16_t numWaves,
                  uint8_t TrailType, uint16_t TrailSize, uint16_t Size, bool direction, bool alternate, bool Bounce, 
                  bool BounceChange, bool Blend, uint16_t Rate);

        //Constructor for using the palette as the pattern with a custom set of repeating waves
        ScannerSL(SegmentSet &SegmentSet, palettePS &Palette, CRGB BGColor, uint16_t numWaves, uint8_t TrailType,   
                  uint16_t TrailSize, uint16_t Size,  bool direction, bool alternate, bool Bounce, bool BounceChange, 
                  bool Blend, uint16_t Rate);
        
        ~ScannerSL();
        
        SegmentSet 
            &segmentSet; 

        int8_t
            dimPow = 80; //80 range -127 -> 127 -80 good for colored bg's

        uint8_t
            colorMode = 0,
            bgColorMode = 0,
            randMode = 0,
            trailType,
            trailSize; //min value of 1
        
        uint16_t
            size;
        
        bool
            blend = false, //sets if particles should add onto one another
            fillBG = false, 
            bounce,
            bounceChange;

        CRGB 
            bgColorOrig,
            *bgColor = nullptr; //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)
        
        palettePS
            paletteTemp,
            *palette = nullptr;
        
        patternPS
            patternTemp,
            *pattern = nullptr;

        particleSetPS 
            particleSet;

        void 
            reset(),
            setPaletteAsPattern(),
            setScanType(uint8_t newScanType),
            makeWaveSet(uint16_t numWaves, bool direction, bool alternate),
            update(void);
    
    private:
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
            getTrailLedLoc(bool trailDirect, uint8_t trailPixelNum, uint16_t maxPosition);

        unsigned long
            currentTime,
            prevTime = 0;
        
        bool
            direction,
            startDirect;

        particlePS
            *particlePtr = nullptr;

        CRGB 
            *trailEndColors = nullptr, //used to store the last colors of each trail, so the background color can be set
            colorTarget,
            partColor,
            nextColor,
            prevColor,
            colorFinal,
            getPartPixelColor(uint16_t partPixelLoc, bool trailDirect);
        
        void
            init(CRGB BgColor, uint16_t Rate),
            moveParticle(particlePS *particlePtr),
            setPartColor(particlePS *particlePtr),
            setTrailColor(const CRGB &trailColor, uint16_t trailLineNum, uint8_t segNum, uint8_t trailPixelNum);
};

#endif
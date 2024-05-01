#ifndef LarsonScannerSL_h
#define LarsonScannerSL_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "Effects/ParticlesSL/ParticlesSL.h"

/* 
This effect is intended to be a short-hand for re-creating some classic scanner effects (Cylon, Kitt).
Allows you to pick the scan type, color, the size of the scan particles, etc. 

Overall, it's a wrapper for a ParticlesPS instance that actually controls the effect.
For a similar, more customizable effect, see ScannerSL. 

While this effect does provide some pass-through functions for adjusting some things like the trail length, 
particle size, etc. You can access the underlying "Particle" instance, `scannerInst`. 
You may access it like: `yourScannerName->scannerInst->reset();`.

Likewise the effect has its own particleSet. 
To adjust the particles you can access it via `yourScannerName->particleSet`. 
Same thing with the effect's palette.

the background color of the scannerInst is bound to the background color set in the larson Scanner
The background color is a pointer, so you can bind it externally as needed

Three scanner types are built in:
    0: Like the classic Cylon scanner, one particle with two trails moving back and forth
    1: Like the Cylon scanner, but only using one trail
    2: Like one of the Kitt Knight Rider scanners: two particles with single trails
        That move back and forth, intersecting in the center of the strip
        (note that this mode uses blend, see ParticlesPS.h for details)
    3: A combination of modes 1 and 2; two single trail waves running in opposite directions
       and one double wave. All the particles are the same size. Uses blend.

by default all the scanner particles will bounce back at either strip end
You can change this by calling setBounce( newBounceVal );

Note that because particlesSL works along segment lines, the larsonScanner does too, but be aware that 
for scan mode 2, the colors are added together when particles meet, which may lead to some weird colors 
for segment sets with un-equal length segments

Example calls: 
    LarsonScannerSL larsonScanner(mainSegments, 0, CRGB::Red, 0, 2, 3, 80);
    Will do a scanner in mode 0 (Cylon) using red.
    The scan particle will have a eye size of 2 (the non-trail portion), and a trail length of 3
    The background is blank
    The particle moves at 80ms

Constructor inputs for creating a particle set:
    scanColor -- The color of the scan particles
    scanType -- The type of scan type (see above)
    eyeSize -- The size of the body of the particles (min value 1)
    trailLength -- The length of the trails. Using 0 will turn off the trails for the scanner.
    bgColor -- The background color used for the effect.
    rate -- The update rate (ms). Note that this is synced with all the particles and also the ParticlesSL instance. 

Other Settings:
    palette -- The palette used for the particles color (this is always a single length palette since scanners are a single color)
    *scannerInst -- The local ParticleSL instance
    particleSet -- The particleSet used to store the scan particles (and passed to the ParticleSL instance)

Functions:
    setColorMode(colorMode, bool bgColorMode) -- Sets either the main or background color mode for the particles
                                                 (bgColorMode = true will set the background color mode)
    setColor(color) -- sets the scan particle color
    setScanType(newType) -- sets the scan mode (note this re-creates all the particles from scratch)
    setTrailLength(newTrailLength) -- Changes the particle trail lengths. 
                                      Trails will be turned off if you set the length to 0
    setEyeSize(newEyeSize) -- Sets the body size of the particles.
    setBounce(newBounce) -- Sets the bounce property of the scanner particles.
                            If true, the particles reverse when they reach the end of the segments rather than wrapping.
    reset() -- resets all particles to the starting locations
    update() -- updates the effect 

Reference Vars:
    trailLength -- The The length of the trails. Set using the setTrailLength().
    bounce (default true) -- The bounce property of the scan particles. Set using the setBounce().
    scanType -- The current scan mode. Set using the setScanType().
    eyeSize -- The length of the scan particle's body (not trail). Set using the setEyeSize().
*/
class LarsonScannerSL : public EffectBasePS {
    public:
        LarsonScannerSL(SegmentSetPS &SegSet, uint8_t ScanType, CRGB ScanColor, CRGB BgColor,
                        uint16_t EyeSize, uint8_t TrailLength, uint16_t Rate);

        ~LarsonScannerSL();

        uint8_t
            scanType,
            trailLength;

        bool
            bounce = true;

        uint16_t
            eyeSize;

        CRGB
            bgColorOrig,
            *bgColor = nullptr;  //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)

        palettePS
            palette = {nullptr, 0};  //palette used for ParticleSL instance. Init to empty for safety

        ParticlesSL
            *scannerInst = nullptr;  //pointer to the ParticleSL instance
        
        //READ ONLY, DO NOT CHANGE!
        //must be above the particleSet definition, otherwise the Arduino compiler doesn't link it
        const uint8_t
            maxNumParticles = 3; //We always use the same particle set length regardless of scan modes.
                                 //This helps prevent memory fragmentation when switching scan modes.

        particleSetPS
            particleSet = { nullptr, maxNumParticles, maxNumParticles };  
            //the particle set used in the ParticleSL instance. 
            //Init to empty for safety, but with our max length of 3 
            //so that the ParticlesSL instances boots up correctly
        
        void
            setColorMode(uint8_t colorMode, bool bgColorMode),
            setColor(CRGB color),
            setScanType(uint8_t newType),
            setTrailLength(uint8_t newTrailLength),
            setEyeSize(uint16_t newEyeSize),
            setBounce(bool newBounce),
            reset(void),
            update(void);

    private:
        unsigned long
            prevRate = 0,
            currentTime,
            prevTime = 0;

        uint16_t
            numLines;
        
        void
            scanType3SetTrails();
};

#endif
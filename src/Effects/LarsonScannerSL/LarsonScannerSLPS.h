#ifndef LarsonScannerSLPS_h
#define LarsonScannerSLPS_h

//TODO -- combine this is the scanner function in my orginal library (give ability to change colors, w/ more scan particle positions?)

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "Effects/ParticlesSL/ParticlesSLPS.h"

/* 
This effect is intended to be a short-hand for re-creating some classic scanner effects (cylon, Kit)
Overall, it's a wrapper for a ParticlesPS instance that actually controls the effect
So you should read ParticlesPS.h if you want to adjust any major settings

The underlying ParticlePS instance is called scannerInst
You may access it like: yourScannedName->scannerInst->reset();

The LarsonScannerSL instance also creates it's own particleSet
To adjust the particles you can access it via yourLarsonScannerSL->particleSet
Same thing with the pallet

This effect does provide some pass-through functions for adjusting some things like the 
trail length, particle size, etc

the background color of the scannerInst is bound to the background color set in the larson Scanner
The background color is a pointer, so you can bind it externally as needed

Three scanner types are built in:
    0: Like the classic cylon scanner, one particle with two trails moving back and forth
    1: Like the cylon scanner, but only using one trail
    2: Like one of the Kit Knight Rider scanners: two particles with single trails
        That move back and forth, intersecting in the center of the strip
        (note that this mode uses blend, see ParticlesPS.h for details)

by default all the scanner particles will bounce back at either strip end
You can change this by calling setBounce( newBounceVal );

Note that because partcilesSLPS works along segment lines, the larsonScanner does too, but be aware that 
for scan mode 2, the colors are added together when particles meet, which may lead to some weird colors 
for segment sets with un-equal length segments

Example calls: 
    LarsonScannerSLPS(mainSegments, 0, CRGB::Red, 2, 3, 0, 80);
    Will do a scanner in mode 0 (cylon) using red.
    The scan particle will have a eye size of 2 (the non-trail portion), and a trail length of 3
    The background is blank
    The particle moves at 80ms

Constructor inputs for creating a particle set:
    ScanColor -- The color of the scan particles
    ScanType -- The type of scan type (see above)
    EyeSize -- The size of the particles (min value 1)
    Traillength -- The length of the trails. Using 0 will turn off the trails for the scanner.
    bgColor -- The background color used for the effect.
    Rate -- The update rate (ms) note that this is synced with all the particles.

Functions:
    setColorMode(uint8_t colorMode, bool bgColorMode) -- sets either the main or background color mode for the particles
                                                        (bgColorMode = true will set the background color mode)
    setColor(CRGB color) -- sets the scan particle color
    setScanType(uint8_t newType) -- sets the scan mode (note this re-creates all the particles from scratch)
    setTrailLength(uint8_t newTrailLength) -- Changes the particle trail lengths. 
                                             Trials will be turned off if you set the length to 0
    setEyeSize(uint8_t newEyeSize) -- Sets the eye size of the scanner (the non-trail portion)
    setBounce(bool newBounce) -- Sets the bounce property of the scanner particles, ie sets if the partciles reverse
                                when they reach the end of the segments
    update() -- updates the effect 

Other Settings:
    scanType -- The current scan mode, for reference only. Change it using the setScanType() function.
    trailLength -- The The length of the trails, for reference only. Change it using the setTrailLength() function.
    eyeSize -- The length of the scan particle's body (not trail), for reference only. Change it using the setEyeSize() function.
    bounce (default true) -- The bounce property of the scan particles, for reference only. Change it using the setBounce() function.
    pallet -- The pallet than will be used for the particle color (this is always a single length pallet since scanners are a single color)
    *scannerInst -- The local ParticlePS instance
    particleSet -- The particleSet used to store the scan particles (and passed to the ParticlePS instance)
*/
class LarsonScannerSLPS : public EffectBasePS {
    public:
        LarsonScannerSLPS(SegmentSet &SegmentSet, uint8_t ScanType, CRGB ScanColor, CRGB BgColor, uint8_t EyeSize, uint8_t TrailLength, uint16_t Rate);  

        ~LarsonScannerSLPS();

        SegmentSet 
            &segmentSet;
        
        uint8_t
            scanType,
            trailLength;

        bool 
            bounce = true;

        uint16_t
            eyeSize;

        CRGB 
            bgColorOrig,
            *bgColor; //bgColor is a pointer so it can be tied to an external variable if needed (such as a pallet color)
        
        palletPS
            pallet; //pallet used for ParticlePS instance
        
        ParticlesSLPS
            *scannerInst; //pointer to the ParticlePS instance
            
        particleSetPS
            particleSet; //the particle set used in the ParticlePS instance

        void 
            setColorMode(uint8_t colorMode, bool bgColorMode),
            setColor(CRGB color),
            setScanType(uint8_t newType),
            setTrailLength(uint8_t newTrailLength),
            setEyeSize(uint8_t newEyeSize),
            setBounce(bool newBounce),
            update(void);
    
    private:
        uint16_t
            numLines;

        unsigned long
            prevRate = 0,
            currentTime,
            prevTime = 0;
};

#endif
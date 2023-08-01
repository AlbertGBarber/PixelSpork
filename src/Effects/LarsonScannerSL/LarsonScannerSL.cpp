#include "LarsonScannerSL.h"

LarsonScannerSL::LarsonScannerSL(SegmentSet &SegSet, uint8_t ScanType, CRGB scanColor, CRGB BgColor, uint8_t EyeSize, uint8_t TrailLength, uint16_t Rate):
    SegSet(SegSet), scanType(ScanType), eyeSize(EyeSize), trailLength(TrailLength)
    {    
        //bind the rate and SegSet pointer vars since they are inherited from BaseEffectPS
        bindSegPtrPS();
        bindClassRatesPS();
        //bind background color pointer
        bindBGColorPS();
        //create an instance of ParticlesPS to animate the particles for the scanner
        scannerInst = new ParticlesSL(SegSet, particleSet, palette, BgColor);
        //bind the ParticlesSL instance background color to point to the LarsonScannerSL's background color
        scannerInst->bgColor = bgColor;
        setColor(scanColor);
        setScanType(scanType);
	}

LarsonScannerSL::~LarsonScannerSL(){
    scannerInst->~ParticlesSL();
    //Free all particles and the particle array pointer
    particleSet.length = 2; //set the particle set to it's maximum length (see setScanType())
    particleUtilsPS::freeParticleSet(particleSet);
}

//changes the color mode of the scanner
//bgColorMode determines if you're setting the scanner's bgColorMode or the normal colorMode
void LarsonScannerSL::setColorMode(uint8_t colorMode, bool bgColorMode){
    if(!bgColorMode){
        scannerInst->colorMode = colorMode;
    } else {
        scannerInst->bgColorMode = colorMode;
    }
}

//changes the color of the particles
void LarsonScannerSL::setColor(CRGB color){
    free(palette.paletteArr);
    palette = paletteUtilsPS::makeSingleColorPalette(color);
}

//sets the bounce property on the scanner particles
void LarsonScannerSL::setBounce(bool newBounce){
    bounce = newBounce;
    particleUtilsPS::setParticleSetProp(particleSet, 6, bounce, 0, 0);
}

//builds the particle set for the type of scan
//Scan types:
//  0: Like the classic cylon scanner, one particle with two trails moving back and forth
//  1: Like the cylon scanner, but only using one trail
//  2: Like one of the Kit Knight Rider scanners: two particles with single trails
//     That move back and forth, intersecting in the center of the strip
//     (note that this mode uses blend, see ParticlesPS.h for details)
void LarsonScannerSL::setScanType(uint8_t newScanType){
    numLines = SegSet.numLines;
    scanType = newScanType;

    //Free all particles and the particle array pointer
    //(we will immediately create a new set of the same size)
    particleUtilsPS::freeParticleSet(particleSet);
     
    //Create a particle for the scanner particles.
    //Note that we always create a set with 2 particles (the max used by any of the modes)
    //and then "hide" any extra particles by changing the set's length.
    //This helps prevent memory fragmentation by always storing the same sized set.
    scannerInst->blend = false;
    if(scanType == 0){
        particleSet = particleUtilsPS::buildParticleSet(2, numLines, true, *rate, 0, eyeSize, 0, 2, trailLength, 0, bounce, 0, false);
        particleSet.length = 1; //We only use 1 particle in this mode, so limit the particle set to 1.
    } else if(scanType == 1){
        particleSet = particleUtilsPS::buildParticleSet(2, numLines, true, *rate, 0, eyeSize, 0, 1, trailLength, 0, bounce, 0, false);
        particleSet.length = 1; //We only use 1 particle in this mode, so limit the particle set to 1.
    } else if(scanType == 2){
        particleSet = particleUtilsPS::buildParticleSet(2, numLines, true, *rate, 0, eyeSize, 0, 1, trailLength, 0, bounce, 0, false);
        particleUtilsPS::setParticleSetPosition(particleSet, 0, 0, false);
        particleUtilsPS::setParticleSetDirection(particleSet, 0, false);
        particleUtilsPS::setParticleSetPosition(particleSet, 1, numLines - 1, false);
        scannerInst->blend = true; //need to turn on blend so that the two particles don't overwrite each other when they meet
    }
    //can't have 0 length trails, so we need to change the trail type
    if(trailLength == 0){
        particleUtilsPS::setParticleSetProp(particleSet, 4, 0, 0, 0);
    }

    scannerInst->setParticleSet(particleSet);
    scannerInst->reset();
}

//changes the trail length of the scan particle
void LarsonScannerSL::setTrailLength(uint8_t newTrailLength){
    trailLength = newTrailLength;
    particleUtilsPS::setParticleSetProp(particleSet, 5, trailLength, 0, 0);
    //can't have 0 length trails, so we need to change the trail type
    if(trailLength == 0){
        particleUtilsPS::setParticleSetProp(particleSet, 4, 0, 0, 0);
    }
}

//changes the size of the main body of the scan particle
void LarsonScannerSL::setEyeSize(uint8_t newEyeSize){
    eyeSize = newEyeSize;
    particleUtilsPS::setParticleSetProp(particleSet, 3, eyeSize, 0, 0);
}

//updates the effect
//basically just calls the ParticlePS's instance update
//but also makes sure all of the scanner particles have the same rate as the effect update rate
//(since the effect's update rate is a pointer, so can be changed externally)
void LarsonScannerSL::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;
        
        //makes sure that the scan particles rate is the same as the effect's update rate
        if(prevRate != *rate){
            prevRate = *rate;
            particleUtilsPS::setParticleSetProp(particleSet, 2, *rate, 0, 0);
        }

        scannerInst->update();
    }
}
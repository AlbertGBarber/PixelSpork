#include "LarsonScannerSL.h"

LarsonScannerSL::LarsonScannerSL(SegmentSetPS &SegSet, uint8_t ScanType, CRGB scanColor, CRGB BgColor,
                                 uint16_t EyeSize, uint8_t TrailLength, uint16_t Rate)
    : scanType(ScanType), eyeSize(EyeSize), trailLength(TrailLength)  //
{
    //bind the rate and segSet pointer vars since they are inherited from BaseEffectPS
    bindSegSetPtrPS();
    bindClassRatesPS();
    //bind background color pointer
    bindBGColorPS();
    //create an instance of ParticlesPS to animate the particles for the scanner
    scannerInst = new ParticlesSL(*segSet, particleSet, palette, BgColor);
    //bind the ParticlesSL instance background color to point to the LarsonScannerSL's background color
    scannerInst->bgColor = bgColor;
    //bind the scanner's update rate to the Larson Scanner's.
    //All the particles have the same update rate, so there's no need to use the particleSL's default fast rate
    scannerInst->rate = rate;
    setColor(scanColor);
    setScanType(scanType);
}

LarsonScannerSL::~LarsonScannerSL() {
    scannerInst->~ParticlesSL();
    //Free all particles and the particle array pointer
    particleSet.length = maxNumParticles;  //set the particle set to it's maximum length (see setScanType())
    particleUtilsPS::freeParticleSet(particleSet);
    free(palette.paletteArr);
}

//resets all particles to the starting locations
void LarsonScannerSL::reset(void) {
    scannerInst->reset();
}

//changes the color mode of the scanner
//bgColorMode determines if you're setting the scanner's bgColorMode or the normal colorMode
void LarsonScannerSL::setColorMode(uint8_t colorMode, bool bgColorMode) {
    if( !bgColorMode ) {
        scannerInst->colorMode = colorMode;
    } else {
        scannerInst->bgColorMode = colorMode;
    }
}

//changes the color of the particles
void LarsonScannerSL::setColor(CRGB color) {
    free(palette.paletteArr);
    palette = paletteUtilsPS::makeSingleColorPalette(color);
}

//sets the bounce property on the scanner particles
void LarsonScannerSL::setBounce(bool newBounce) {
    bounce = newBounce;
    particleUtilsPS::setParticleSetProp(particleSet, 6, bounce, 0, 0);
}

/* builds the particle set for the type of scan
Scan types:
    0: Like the classic Cylon scanner, one particle with two trails moving back and forth
    1: Like the Cylon scanner, but only using one trail
    2: Like one of the Kitt Knight Rider scanners: two particles with single trails
        That move back and forth, intersecting in the center of the strip
        (note that this mode uses blend, see ParticlesPS.h for details) 
    3: A combination of modes 1 and 2; two single trail waves running in opposite directions
       and one double wave. All the particles are the same size. Uses blend.*/
void LarsonScannerSL::setScanType(uint8_t newScanType) {
    numLines = segSet->numLines;
    scanType = newScanType;

    //Free all particles and the particle array pointer
    //(we will immediately create a new set of the same size)
    particleUtilsPS::freeParticleSet(particleSet);

    //Create a particle for the scanner particles.
    //Note that we always create a set with 2 particles (the max used by any of the modes)
    //and then "hide" any extra particles by changing the set's length.
    //This helps prevent memory fragmentation by always storing the same sized set.
    scannerInst->blend = false;

    switch( scanType ) {
        case 0:
        default:
            particleSet = particleUtilsPS::buildParticleSet(maxNumParticles, numLines, true, *rate, 0, eyeSize, 0, 2, trailLength, 0, bounce, 0, false);
            particleSet.length = 1;  //We only use 1 particle in this mode, so limit the particle set to 1.
            break;
        case 1:
            particleSet = particleUtilsPS::buildParticleSet(maxNumParticles, numLines, true, *rate, 0, eyeSize, 0, 1, trailLength, 0, bounce, 0, false);
            particleSet.length = 1;  //We only use 1 particle in this mode, so limit the particle set to 1.
            break;
        case 2:
            particleSet = particleUtilsPS::buildParticleSet(maxNumParticles, numLines, true, *rate, 0, eyeSize, 0, 1, trailLength, 0, bounce, 0, false);
            particleSet.length = 2;  //We only use 2 particles in this mode, so limit the particle set to 2.

            //One particle starting at the start of the segment set
            //We reverse its direction verses the other particle
            particleUtilsPS::setParticleSetStartPos(particleSet, 0, 0, false);
            particleUtilsPS::setParticleSetDirection(particleSet, 0, false);

            //One particle starting at the end of the segment set
            particleUtilsPS::setParticleSetStartPos(particleSet, 1, numLines - 1, false);
            scannerInst->blend = true;  //need to turn on blend so that the two particles don't overwrite each other when they meet
            break;
        case 3:
            particleSet = particleUtilsPS::buildParticleSet(maxNumParticles, numLines, true, *rate, 0, eyeSize, 0, 1, trailLength, 0, bounce, 0, false);
            particleSet.length = 3;
            //One particle starting at the start of the segment set
            //We reverse its direction verses the other particles
            particleUtilsPS::setParticleSetStartPos(particleSet, 0, 0, false);
            particleUtilsPS::setParticleSetDirection(particleSet, 0, false);

            //One particle starting at the end of the segment set
            particleUtilsPS::setParticleSetStartPos(particleSet, 1, numLines - 1, false);

            //One particle starting in the middle of the segment set with double trails 
            //We halve the trail length so the overall particle matches the others
            //If the trailLength is odd, we add 1 to the particle's body eyeSize to compensate
            particleUtilsPS::setParticleSetStartPos(particleSet, 2, numLines/2, false);
            particleUtilsPS::setParticleSetTrailType(particleSet, 2, 2);
            scanType3SetTrails(); //handles setting up trail lengths 
                                  //(in own function b/c it needs to be repeated in setTrailLength())
            scannerInst->blend = true;  //need to turn on blend so that the two particles don't overwrite each other when they meet           
    }


    //can't have 0 length trails, so we need to change the trail type
    if( trailLength == 0 ) {
        particleUtilsPS::setParticleSetProp(particleSet, 4, 0, 0, 0);
    }

    scannerInst->setParticleSet(particleSet);
    reset();
}

//changes the trail length of the scan particle
void LarsonScannerSL::setTrailLength(uint8_t newTrailLength) {
    trailLength = newTrailLength;
    particleUtilsPS::setParticleSetProp(particleSet, 5, trailLength, 0, 0);

    //for scanType 3 we need halve the trail length for the double trail particle
    if( scanType == 3 ){
        scanType3SetTrails();
    }

    //can't have 0 length trails, so we need to change the trail type
    if( trailLength == 0 ) {
        particleUtilsPS::setParticleSetProp(particleSet, 4, 0, 0, 0);
    }
}

//Handles setting up trails for the center particle for scanType 3
//The particle needs double trails that are half the trailLength (so the all the particles match)
//If the trailLength is odd, we add 1 to the particle's body eyeSize to compensate
void LarsonScannerSL::scanType3SetTrails(){
    particleUtilsPS::setParticleSetTrailSize(particleSet, 2, trailLength/2, 0);

    if( mod8(trailLength, 2) != 0 ){
        particleUtilsPS::setParticleSetSize(particleSet, 2, eyeSize + 1, 0);
    }
}

//changes the size of the main body of the scan particle
void LarsonScannerSL::setEyeSize(uint16_t newEyeSize) {
    eyeSize = newEyeSize;
    particleUtilsPS::setParticleSetProp(particleSet, 3, eyeSize, 0, 0);
}

//updates the effect
//basically just calls the ParticlePS's instance update
//but also makes sure all of the scanner particles have the same rate as the effect update rate
//(since the effect's update rate is a pointer, so can be changed externally)
void LarsonScannerSL::update() {
    currentTime = millis();

    if( (currentTime - prevTime) >= *rate ) {
        prevTime = currentTime;

        //makes sure that the scan particles rate is the same as the effect's update rate
        if( prevRate != *rate ) {
            prevRate = *rate;
            particleUtilsPS::setParticleSetProp(particleSet, 2, *rate, 0, 0);
        }

        //Updating the particles instance
        scannerInst->update();
    }
}
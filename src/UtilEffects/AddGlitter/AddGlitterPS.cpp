#include "AddGlitterPS.h"

AddGlitterPS::AddGlitterPS(SegmentSetPS &SegSet, CRGB GlitterColor, uint16_t GlitterNum, uint8_t GlitterMode,
                           uint16_t GlitterRate, uint16_t Rate)
    : glitterNum(GlitterNum), glitterMode(GlitterMode)  //
{
    //bind the rate and segSet pointer vars since they are inherited from BaseEffectPS
    bindSegSetPtrPS();
    bindClassRatesPS();
    //Point the glitterColor to the passed in color
    glitterColorOrig = GlitterColor;
    glitterColor = &glitterColorOrig;

    //point the glitter update rate to the passed in rate
    glitterRateOrig = GlitterRate;
    glitterRate = &glitterRateOrig;

    //setup an inital set of glitter pixels
    setGlitterNum(glitterNum);
}

AddGlitterPS::~AddGlitterPS() {
    free(glitterLocs);
}

//creates an array of glitter locations of length newNum
void AddGlitterPS::setGlitterNum(uint16_t newGlitterNum) {
    glitterNum = newGlitterNum;

    //We only need to make a new glitter array if the current one isn't large enough
    //This helps prevent memory fragmentation by limiting the number of heap allocations
    //but this may use up more memory overall.
    if( alwaysResizeObj_PS || (newGlitterNum > glitterNumMax) ) {
        glitterNumMax = newGlitterNum;
        free(glitterLocs);
        glitterLocs = (uint16_t *)malloc(glitterNumMax * sizeof(uint16_t));
    }

    fillGlitterArr();
}

//Fills in the glitter array with random pixel locations from the strip
void AddGlitterPS::fillGlitterArr() {
    numLeds = segSet->numLeds;
    for( uint16_t i = 0; i < glitterNum; i++ ) {
        pixelNum = random16(numLeds);
        glitterLocs[i] = pixelNum;
    }
}

//Introduces one new glitter location into the glitter location array
//while shifting all the other locations forward one position
//(removing the final location in the array)
//So glitter particles will turn on and off one at a time
void AddGlitterPS::advanceGlitterArr() {
    numLeds = segSet->numLeds;
    for( int32_t i = glitterNum - 1; i >= 0; i-- ) {
        if( i == 0 ) {
            pixelNum = random16(numLeds);
            glitterLocs[i] = pixelNum;
        } else {
            glitterLocs[i] = glitterLocs[i - 1];
        }
    }
}

/* Updates the effect by placing more glitter
To allow it to run alongside faster effects, new glitter is only created
at the glitterRate, but glitter is re-drawn at the effect rate
Ie, if we wanted glitter to be set every 1000ms, but the effect we're applying glitter to 
updates every 100ms the glitter would need to be re-drawn every 100ms so that it wasn't over-written by the effect
but new glitter locations would only be set every 1000ms.
Note that the effect never removes any glitter, it assumes that it will be run along-side another effect,
which will remove the glitter for it.
It also assumes that the glitter rate is slower or equal to the other effect rates
if glitter is the fastest, then it will keep filling the segment up with glitter */
void AddGlitterPS::update() {
    currentTime = millis();
    
    if(active){
        //Every glitterRate period we get new glitter locations
        if((currentTime - prevGlitterTime) >= *glitterRate ) {
            prevGlitterTime = currentTime;
            if( glitterMode == 0 ) {
                fillGlitterArr();
            } else {
                advanceGlitterArr();
            }
        }

        //Drawn the glitter on the strip at the effect's rate
        if( (currentTime - prevTime) >= *rate ) {
            prevTime = currentTime;

            //Drawn the glitter on the strip
            for( uint16_t i = 0; i < glitterNum; i++ ) {
                segDrawUtils::setPixelColor(*segSet, glitterLocs[i], *glitterColor, colorMode);
            }

            showCheckPS();
        }
    }
}

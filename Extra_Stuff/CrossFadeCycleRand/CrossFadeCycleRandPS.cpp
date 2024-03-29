#include "CrossFadeCycleRandPS.h"

//same as CrossFadeCycle but it chooses a random color to fade to each time
CrossFadeCycleRandPS::CrossFadeCycleRandPS(SegmentSetPS &SegmentSetPS, uint16_t NumCycles, uint8_t Steps, uint16_t Rate):
    segmentSet(SegmentSetPS), numCycles(NumCycles), steps(Steps)
    {    
        //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
        bindSegPtrPS();
        bindClassRatesPS();
        init();
	}

void CrossFadeCycleRandPS::reset(){
    done = false;
    cycleCount = 0;
    currentStep = 0;
    if(numCycles == 0){
        infinite = true;
    }
    startColor = 0;
    nextColor = colorUtilsPS::randColor();
}


void CrossFadeCycleRandPS::update(){
    currentTime = millis();

    if(!infinite && (cycleCount == numCycles ) ){
        done = true;
    }
    if( !done && ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        newColor = colorUtilsPS::getCrossFadeColor(startColor, nextColor, currentStep, steps);
        segDrawUtils::fillSegSetColor(segmentSet, newColor, 0);

        currentStep++;

        if(currentStep == steps){
            currentStep = 0;
            startColor = nextColor;
            nextColor = colorUtilsPS::randColor();
            cycleCount++;
        }
        showCheckPS();
    }
}
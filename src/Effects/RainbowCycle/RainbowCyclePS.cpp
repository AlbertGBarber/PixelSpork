#include "RainbowCyclePS.h"

RainbowCyclePS::RainbowCyclePS(SegmentSet &SegmentSet, uint8_t Length, bool Direction, uint16_t Rate):
    length(Length), _direct(Direction), segmentSet(SegmentSet)
    {    
        //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
        bindSegPtrPS();
        bindClassRatesPS();
        init();
        //write out the first frame, but don't automatically show
        //then reset showOn to it's default
        showOn = false;
        update();
        showOn = true;
	}

//initializes/resets the core counting and direction vars for the effect
void RainbowCyclePS::init(){
    cycleCount = 0;
    numSegs = segmentSet.numSegs;
    numLeds = segmentSet.numLeds;
    //set the maximum cycle length 
    //ie the total amount of cycles before the effect is back at the start
    //we allow rainbows that are longer than the segmentSet
    maxCycleLength = numLeds - 1;
    if(length > numLeds){
        maxCycleLength = length;
    }
    setDirect(_direct);
}

//sets the step increment (1 or -1) and the start and end limits of the loop
//when the direction is reversed we draw the rainbow from the end of the segmentSet
void RainbowCyclePS::setDirect(bool newDirect){
    _direct = newDirect;
    stepDirect = 1;
    if (!_direct) {
        stepDirect = -1;
    }
}

//returns the direction of the effect
bool RainbowCyclePS::direct(){
    return _direct;
}

//core update cycle, draws the rainbows along the segmentSet every rate ms
void RainbowCyclePS::update(){
    currentTime = millis();

    //if it's time to update the effect, do so
    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;
        //prevents cycle count from overflowing by resetting it when the cycle loops
        //very important to keep the rainbow correct
        cycleCount = addMod16PS( cycleCount, 1, maxCycleLength); //(cycleCount + 1) % (maxCycleLength); 
        ledCount = 0; //reset the ledCount, the total number of leds we've set in the current cycle
        //for each segment, set each pixel in the segment to the appropriate rainbow color
        //we must call getSegmentPixel(segmentSet, i, j) to account for reversed segments
        for (uint8_t i = 0; i < numSegs; i++) {
            totSegLen = segmentSet.getTotalSegLength(i);
            for(uint16_t j = 0; j < totSegLen; j++){
                ledCount++;
                //we always need to make a rainbow of length # of steps
                //this is determined using (stepVal + ledCount) % length * (256 / length)
                //where stepVal = maxCycleLength + cycleCount * stepDirect; (see below for explanation)
                //and ledCount is the current led number
                //(stepVal + ledCount) % length keeps our postion counter between 0 and length
                //while * (256 / length) shifts the counter in (256/length) steps
                //so that we always finsh a rainbow after length # of steps
                
                //stepVal is set so that it increases/decreases across a cycle 
                //changing the led location where we take the color from
                //cycleCount is clamped the maxCycleLength above
                //note that maxCycleLength is added so that the value is never negative, 
                //because Adruino % is weird with neg values
                stepVal = maxCycleLength + cycleCount * stepDirect; 
                color = colorUtilsPS::wheel( ( addMod16PS(stepVal, ledCount, length ) * 256 / length ) & 255 , 0, satur, value);

                //get the actual pixel address, and set it
                //color mode is 0 because we are working out the rainbow color ourselves
                pixelNum = segDrawUtils::getSegmentPixel(segmentSet, i, j);
                //set the color, segNum and lineNum don't matter for this since we're always in colorMode 0
                segDrawUtils::setPixelColor(segmentSet, pixelNum, color, 0, 0, 0);
            }
        }
        showCheckPS();
    }
}
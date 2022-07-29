#include "RainbowCycleSLPS.h"

RainbowCycleSLPS::RainbowCycleSLPS(SegmentSet &SegmentSet, uint8_t Length, bool Direction, uint16_t Rate):
    length(Length), direct(Direction), segmentSet(SegmentSet)
    {    
        //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
        bindSegPtrPS();
        bindClassRatesPS();
        init();
	}

//initializes/resets the core counting and direction vars for the effect
void RainbowCycleSLPS::init(){
    cycleCount = 0;
    setLength(length);
    setDirect(direct);
}

//set the length of the rainbow, can be longer than the total number of segment lines
void RainbowCycleSLPS::setLength(uint8_t newLength){
    numLines = segmentSet.maxSegLength;
    //set the maximum cycle length 
    //ie the total amount of cycles before the effect is back at the start
    //we allow rainbows that are longer than the segmentSet lines
    maxCycleLength = numLines - 1;
    if(length > numLines){
        maxCycleLength = length;
    }
}

//sets the step increment (1 or -1) and the start and end limits of the loop
//when the direction is reversed we draw the rainbow from the end of the segmentSet
void RainbowCycleSLPS::setDirect(bool newDirect){
    direct = newDirect;
    stepDirect = 1;
    if (!direct) {
        stepDirect = -1;
    }
}

//core update cycle, draws the rainbows along the segmentSet every rate ms
void RainbowCycleSLPS::update(){
    currentTime = millis();

    //if it's time to update the effect, do so
    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        //we grab some segment info in case it's changed (It shouldn't have, but this is a safe guard)
        numLines = segmentSet.maxSegLength;

        //prevents cycle count from overflowing by resetting it when the cycle loops
        //very important to keep the rainbow correct
        cycleCount = addMod16PS(cycleCount, 1, maxCycleLength); 

        //stepVal is set so that it increases/decreases as a cycle progresses
        //offsetting the wheel input for each cycle
        //cycleCount is clamped the maxCycleLength above
        //note that maxCycleLength is added so that the value is never negative 
        //(we use addMod16 on with this value later, which needs positive inputs, 
        //adding the maximum value of the a mod before doing the mod doesn't change the result)
        stepVal = maxCycleLength + cycleCount * stepDirect; 

        //for each segment line, set each pixel in the line to the appropriate rainbow color
        for (uint8_t i = 0; i < numLines; i++) {
            //we always need to make a rainbow of length # of steps
            //this is determined using (stepVal + i) % length * (256 / length)
            //where stepVal = maxCycleLength + cycleCount * stepDirect; (see above for explination)
            //and i is the current line number
            //(stepVal + i) % length offsets our position, while keeping it between 0 and length
            //while * (256 / length) shifts the counter in (256/length) steps
            //so that we always finsh a rainbow after length # of steps
            color = colorUtilsPS::wheel( ( addMod16PS(stepVal, i, maxCycleLength) * 256 / length ), 0, satur, value);
            //fill the segment line at the line location with color
            //by default the rainbows would move counter-clockwise across the segmentSet, so we use numLines - i - 1
            //to reverse it
            segDrawUtils::drawSegLineSimple(segmentSet, numLines - i - 1, color, 0);
        }
        showCheckPS();
    }
}

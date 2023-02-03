#include "RainbowCycleSLSeg.h"

RainbowCycleSLSeg::RainbowCycleSLSeg(SegmentSet &SegmentSet, uint16_t Length, bool Direction, bool SegMode, uint16_t Rate):
    segmentSet(SegmentSet), length(Length), direct(Direction), segMode(SegMode)
    {    
        init(Rate);
	}

//Does a rainbow cycle of length 255
RainbowCycleSLSeg::RainbowCycleSLSeg(SegmentSet &SegmentSet, bool Direction, bool SegMode, uint16_t Rate):
    segmentSet(SegmentSet), direct(Direction), segMode(SegMode)
    {
        length = 255;
        init(Rate);
    }

//initializes/resets the core counting and direction vars for the effect
void RainbowCycleSLSeg::init(uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    cycleNum = 0;
    setLength(length);
}

//set the length of the rainbow, can be longer than the total number of segment lines
void RainbowCycleSLSeg::setLength(uint16_t newLength){
    uint16_t segLength;
    if(segMode){
        segLength = segmentSet.numSegs;
    } else {
        segLength = segmentSet.maxSegLength;
    }
    //set the maximum cycle length 
    //ie the total amount of cycles before the effect is back at the start
    //we allow rainbows that are longer than the segmentSet lines
    maxCycleLength = segLength - 1;
    if(length > segLength){
        maxCycleLength = length;
    }
}

//core update cycle, draws the rainbows along the segmentSet every rate ms
void RainbowCycleSLSeg::update(){
    currentTime = millis();

    //if it's time to update the effect, do so
    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        //get the step amount based on the direction
        //will be -1 if direct is false, 1 if direct is true
        //we do this live so you can change the direct directly
        stepDirect = direct - !direct;

        //prevents cycle count from overflowing by resetting it when the cycle loops
        //very important to keep the rainbow correct
        cycleNum = addMod16PS(cycleNum, 1, maxCycleLength); 

        //stepVal is set so that it increases/decreases as a cycle progresses
        //offsetting the wheel input for each cycle
        //cycleNum is clamped the maxCycleLength above
        //note that maxCycleLength is added so that the value is never negative 
        //(we use addMod16 on with this value later, which needs positive inputs, 
        //adding the maximum value of the a mod before doing the mod doesn't change the result)
        stepVal = maxCycleLength + cycleNum * stepDirect; 

        //either draw the rainbow along the segments or the segment lines
        if(segMode){
            numSegs = segmentSet.numSegs;
            //color each segment in a rainbow color
            for (uint16_t i = 0; i < numSegs; i++) {
                color = getRainbowColor(i);
                //Color the segment
                segDrawUtils::fillSegColor(segmentSet, i, color, 0);
            }
        } else {
            //we grab some segment info in case it's changed (It shouldn't have, but this is a safe guard)
            numLines = segmentSet.maxSegLength;
            //for each segment line, set each pixel in the line to the appropriate rainbow color
            for (uint16_t i = 0; i < numLines; i++) {
                color = getRainbowColor(i);
                //fill the segment line at the line location with color
                //by default the rainbows would move counter-clockwise across the segmentSet, so we use numLines - i - 1
                //to reverse it
                segDrawUtils::drawSegLineSimple(segmentSet, numLines - i - 1, color, 0);
            }
        }
        showCheckPS();
    }
}

CRGB RainbowCycleSLSeg::getRainbowColor(uint16_t index){
    //we always need to make a rainbow of length # of steps
    //this is determined using (stepVal + index) % length * (256 / length)
    //where stepVal = maxCycleLength + cycleNum * stepDirect; (see above for explanation)
    //and index is the current line number
    //(stepVal + index) % length offsets our position, while keeping it between 0 and length
    //while * (256 / length) shifts the counter in (256/length) steps
    //so that we always finsh a rainbow after length # of steps
    color = colorUtilsPS::wheel( ( addMod16PS(stepVal, index, maxCycleLength) * 256 / length ), 0, satur, value);
    return color;
}

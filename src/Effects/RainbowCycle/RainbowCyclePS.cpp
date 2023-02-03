#include "RainbowCyclePS.h"

RainbowCyclePS::RainbowCyclePS(SegmentSet &SegmentSet, uint16_t Length, bool Direction, uint16_t Rate):
    length(Length), direct(Direction), segmentSet(SegmentSet)
    {    
        init(Rate);
	}

//Does a rainbow cycle of length 255
RainbowCyclePS::RainbowCyclePS(SegmentSet &SegmentSet, bool Direction, uint16_t Rate):
    direct(Direction), segmentSet(SegmentSet)
    {    
        length = 255;
        init(Rate);
	}

//initializes/resets the core counting and direction vars for the effect
void RainbowCyclePS::init(uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    cycleNum = 0;
    setLength(length);
}

//set the length of the rainbow, can be longer than the total length of the segmentSet
void RainbowCyclePS::setLength(uint16_t newLength){
    numLeds = segmentSet.numLeds;
    numSegs = segmentSet.numSegs;
    //set the maximum cycle length 
    //ie the total amount of cycles before the effect is back at the start
    //we allow rainbows that are longer than the segmentSet
    maxCycleLength = numLeds - 1;
    if(length > numLeds){
        maxCycleLength = length;
    }
}

//core update cycle, draws the rainbows along the segmentSet every rate ms
void RainbowCyclePS::update(){
    currentTime = millis();

    //if it's time to update the effect, do so
    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;
        
        //get the step amount based on the direction
        //will be -1 if direct is false, 1 if direct is true
        //we do this live so you can change the direct directly
        stepDirect = direct - !direct;

        //we grab some segment info in case it's changed (It shouldn't have, but this is a safe guard)
        numLeds = segmentSet.numLeds;
        numSegs = segmentSet.numSegs;
        
        //prevents cycle count from overflowing by resetting it when the cycle loops
        //very important to keep the rainbow correct
        cycleNum = addMod16PS(cycleNum, 1, maxCycleLength); //(cycleNum + 1) % (maxCycleLength); 
        ledCount = 0; //reset the ledCount, the total number of leds we've set in the current cycle

        //stepVal is set so that it increases/decreases as a cycle progresses
        //offsetting the wheel input for each cycle
        //cycleNum is clamped the maxCycleLength above
        //note that maxCycleLength is added so that the value is never negative 
        //(we use addMod16 on with this value later, which needs positive inputs, 
        //adding the maximum value of the a mod before doing the mod doesn't change the result)
        stepVal = maxCycleLength + cycleNum * stepDirect; 

        //for each segment, set each pixel in the segment to the appropriate rainbow color
        //we must call getSegmentPixel(segmentSet, i, j) to account for reversed segments
        for (uint16_t i = 0; i < numSegs; i++) {
            //run along each segment and set its leds to rainbow colors,
            //counting how many leds we've set so fast (ledCount) to make sure the colors are correct
            totSegLen = segmentSet.getTotalSegLength(i);
            for(uint16_t j = 0; j < totSegLen; j++){
                ledCount++;
                //we always need to make a rainbow of length # of steps
                //this is determined using (stepVal + ledCount) % length * (256 / length)
                //where stepVal = maxCycleLength + cycleNum * stepDirect; (see above for explination)
                //and ledCount is the current led number
                //(stepVal + ledCount) % length offsets our position, while keeping it between 0 and length
                //while * (256 / length) shifts the counter in (256/length) steps
                //so that we always finsh a rainbow after length # of steps
                color = colorUtilsPS::wheel( ( addMod16PS(stepVal, ledCount, maxCycleLength) * 256 / length ), 0, satur, value);

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
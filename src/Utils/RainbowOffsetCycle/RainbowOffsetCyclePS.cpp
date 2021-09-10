#include "RainbowOffsetCyclePS.h"

//constructor for multiple segmentSets
RainbowOffsetCyclePS::RainbowOffsetCyclePS(SegmentSet** SegmentSetArr, uint8_t NumSegsSets, uint16_t Rate, bool offsetDirctInit):
    segGroup(SegmentSetArr), _dirct(offsetDirctInit), numSegSets(NumSegsSets)
    {    
        //bind the rate vars since they are inherited from BaseEffectPS
        bindClassRatesPS();
        setDirect(_dirct);
	}

//constructor for a single segmentSet
RainbowOffsetCyclePS::RainbowOffsetCyclePS(SegmentSet &segmentSet, uint16_t Rate, bool offsetDirctInit):
    _dirct(offsetDirctInit)
    {
        //bind the rate vars since they are inherited from BaseEffectPS
        bindClassRatesPS();
        setDirect(_dirct);
        setGroup(segmentSet);
    }

RainbowOffsetCyclePS::~RainbowOffsetCyclePS(){
    delete[] segGroupTemp;
}

//returns the direction of the offset
bool RainbowOffsetCyclePS::direct(void){
    return _dirct;
}

void RainbowOffsetCyclePS::setGroup(SegmentSet** SegmentSetArr, uint8_t NumSegsSets){
    segGroup = SegmentSetArr;
    numSegSets = NumSegsSets;
}

//sets up a group for a single segmentSet
//creates a SegmentSet array addressed to the groupTemp pointer
//and fills it with the address of the passed in segmentSet
//then assignes the array to group
//lets you delete the groupTemp when a new segmentSet is assigned
//without worring about deleting parts of any external arrays that may have been assigned to the group before
void RainbowOffsetCyclePS::setGroup(SegmentSet &segmentSet){
    delete[] segGroupTemp;
    segGroupTemp = new SegmentSet*[1];
    segGroupTemp[0] = &segmentSet;
    segGroup = segGroupTemp;
    numSegSets = 1;
}

// sets the rate (in ms) and direction of change of the wheel rainbow cycle
// lets you constantly shift the rainbow while an effect is active
// newDirct = true will set the rainbow to move positivly along the strip, false will set to reverse
//setting cycleRun to true will start the cycle
void RainbowOffsetCyclePS::setCycle(uint16_t newRate, bool newDirct, bool newCycleRun) {
    rate = newRate;
    setDirect(newDirct);
    cycleRun = newCycleRun;
}

//sets the direction of the rainbow cycle
// newDirct = true will set the rainbow to move positivly along the strip, false will set to reverse
void RainbowOffsetCyclePS::setDirect(bool newDirct){
    _dirct = newDirct;
    if (_dirct) {
        offsetStep = 1;
    } else {
        offsetStep = -1;
    }
}

//updates a cycle, if it's turned on, and enough time has passed
void RainbowOffsetCyclePS::update(){
	if (cycleRun) {
        currentTime = millis();
        globalRateCheckPS();
        if (currentTime - prevTime >= rate) {
            prevTime = currentTime;
            for(int i = 0; i < numSegSets; i++){ 
                //increment/decement the offset wrapping as it reaches 255
                segGroup[i]->rainbowOffset = (uint16_t)(255 + segGroup[i]->rainbowOffset + offsetStep) % 255;
            }
        }
    }
}
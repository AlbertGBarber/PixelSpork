#include "SegOffsetCyclerPS.h"

//constructor for multiple segmentSets
SegOffsetCyclerPS::SegOffsetCyclerPS(SegmentSetPS **SegmentSetArr, uint8_t NumSegSets, uint8_t ColorMode,
                                     bool direction, uint16_t Rate)
    : segGroup(SegmentSetArr), numSegSets(NumSegSets)  //
{
    init(direction, ColorMode, Rate);
}

//constructor for a single SegmentSetPS
SegOffsetCyclerPS::SegOffsetCyclerPS(SegmentSetPS &SegSet, uint8_t ColorMode, bool direction, uint16_t Rate)  //
{
    init(direction, ColorMode, Rate);
    setGroup(SegSet);
}

SegOffsetCyclerPS::~SegOffsetCyclerPS() {
    free(segGroupTemp);
}

//initialization
void SegOffsetCyclerPS::init(bool direction, uint8_t ColorMode, uint16_t rate) {
    setRate(rate);
    colorModeOrig = ColorMode;
    colorMode = &colorModeOrig;
    setCycle(direction, true);
}

//changes the segmentSet group
void SegOffsetCyclerPS::setGroup(SegmentSetPS **SegmentSetArr, uint8_t NumSegsSets) {
    segGroup = SegmentSetArr;
    numSegSets = NumSegsSets;
}

/* sets up a group for a single SegmentSetPS
creates a SegmentSetPS array addressed to the groupTemp pointer
and fills it with the address of the passed in SegmentSetPS
then assigns the array to group
lets you delete the groupTemp when a new SegmentSetPS is assigned
without worrying about deleting parts of any external arrays that may have been assigned to the group before */
void SegOffsetCyclerPS::setGroup(SegmentSetPS &SegSet) {
    free(segGroupTemp);
    segGroupTemp = (SegmentSetPS **)malloc(1 * sizeof(SegmentSetPS *));
    segGroupTemp[0] = &SegSet;
    segGroup = segGroupTemp;
    numSegSets = 1;
}

//ties all the SegmentSetPS's offsetRates to the Util rate
void SegOffsetCyclerPS::bindSegRates() {
    for( uint8_t i = 0; i < numSegSets; i++ ) {
        segGroup[i]->offsetRate = rate;
    }
}

//changes the update rate
//re-binds rateOrig to the actual update rate
void SegOffsetCyclerPS::setRate(uint16_t newRate) {
    rateOrig = newRate;
    rate = &rateOrig;
    bindSegRates();
}

//sets the direction and lets you activate/deactivate the offset cycle
void SegOffsetCyclerPS::setCycle(bool newDirect, bool newRunOffset) {
    setDirect(newDirect);
    setOffsetActive(newRunOffset);
}

//sets the direction of the offset cycle
//newDirect = true will set the gradient to move positively along the strip, false will set to reverse
void SegOffsetCyclerPS::setDirect(bool newDirect) {
    direct = newDirect;
    for( uint8_t i = 0; i < numSegSets; i++ ) {
        segGroup[i]->offsetDirect = newDirect;
    }
}

//sets the runOffset var for the segmentSets
void SegOffsetCyclerPS::setOffsetActive(bool newRunOffset) {
    runOffset = newRunOffset;
    for( uint8_t i = 0; i < numSegSets; i++ ) {
        segGroup[i]->runOffset = newRunOffset;
    }
}

//updates a cycle, if it's turned on, and enough time has passed
//The offsetMax values should match those in segDrawUtils::getPixelColor()
//basically 255 for all rainbow modes, then the SegmentSetPS gradOffsetMax for all others
void SegOffsetCyclerPS::update() {

    if( (currentTime - prevTime) >= *rate ) {
        prevTime = currentTime;

        for( uint8_t i = 0; i < numSegSets; i++ ) {
            if( *colorMode < 6 ) {
                offsetMax = 255;
            } else {
                offsetMax = segGroup[i]->gradOffsetMax;
            }
            segDrawUtils::setGradOffset(*segGroup[i], offsetMax);
        }
    }
}
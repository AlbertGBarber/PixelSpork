#ifndef RainbowOffsetCyclePS_h
#define RainbowOffsetCyclePS_h

#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
  #include "pins_arduino.h"
  #include "WConstants.h"
#endif

#include "SegmentFiles.h"
#include "Effects/EffectBasePS.h"

//Used to change the rainbow offset of a segment set (or group of segement sets) over time
//The offset is changed at the passed in rate (ms), in the specified direction (true is positive)
//this isn't an effect by itself (fixedLengthRainbowCycle is the effect version)
//but is more intended to be used to accent effects
//like shifting a rainbow background, or changing some rainbow bound particles
//this is all done internally in a segmentSet and segDrawUtils::wheel or setPixelColor

//Shifting the rainbow can be specified using the rate, setDirect(bool newDirct), and cycleRun vars
//or can be set using one of the class constructors (see class for details)
    //rate is the change rate in ms (may also be bound to an external variable as with all effects)
    //setDirect(bool newDirct) sets rainbow's direction of shift
    //cycleRun turns the cycle on or off, pausing or starting it (this is defaulted to true (on))
//you may also set these vars in one go using the setCycle() function (see class for details)

//calling update() updates the cycle (as long as enough time has passed)

//The class can be specified to work on a single segmentSet
//or an array of segmentSets, see class constructors below for details
//specifiy a segmentSet array like:
//SegmentSet *setArray[] = {&segmentSet1, &segmentSet2, etc}

//you may rebind the segmentSets using the setGroup() functions 

class RainbowOffsetCyclePS : public EffectBasePS {
    public:
        //two constructors, for single and multiple segmentSets
        RainbowOffsetCyclePS(SegmentSet &segmentSet, uint16_t Rate, bool offsetDirctInit),
        RainbowOffsetCyclePS(SegmentSet** SegmentSetArr, uint8_t NumSegsSets, uint16_t Rate, bool offsetDirctInit);

        ~RainbowOffsetCyclePS();
        
        bool 
            cycleRun = true, //turns the cycle on/off, default is on
            direct(void); //returns the current direction of the cycle
    
        void
            setCycle(uint16_t Rate, bool newDirct, bool cycleRun), //defines a new set of cycle parameters
            setDirect(bool newDirct), //sets the shift direction, 
            setGroup(SegmentSet** SegmentSetArr, uint8_t NumSegsSets), //sets a new group of segmentSets to act on
            setGroup(SegmentSet &segmentSet), //sets a new segmentSet to act on
            update();
        
    private:

        uint8_t
            numSegSets;

        int8_t
            offsetStep;
        
        unsigned long
            currentTime,
            prevTime;

        bool
            _dirct;

        SegmentSet
            **segGroupTemp, //temp pointer used for setting up single segmentSets, seperate from group to allow safe deletion
            **segGroup;

};

#endif
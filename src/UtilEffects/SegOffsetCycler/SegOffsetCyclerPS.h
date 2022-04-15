#ifndef SegOffsetCyclerPS_h
#define SegOffsetCyclerPS_h

#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
  #include "pins_arduino.h"
  #include "WConstants.h"
#endif

#include "SegmentFiles.h"
#include "Effects/EffectBasePS.h"
//*note Util == this class in comments below

//Used to change the rainbow/gradient offset of a segment set (or group of segment sets) over time
//The offset is changed at the passed in rate (ms), in the specified direction (true is positive)
//Note that by default effects update the offset by themselves, but the offset rate is limited to that of
//the effect (you can't update it faster than the effect updates)
//This utility is only needed if you need to update the offset at a faster rate.

//Note that because the offset is specific to a color mode ( see segDrawUtils::getPixelColor() )
//you need to specify a color mode for this Util. This should match the mode of whatever effect you want to run
//for this Util, colorMode is a pointer, so you can point it to your effect's color mode to keep them in sync
//ie colorMode = &yourEffectsColorMode;
//To change colorMode directly, use colorModeOrig --> ie colorModeOrig = yourColorMode;

//The Util can be specified to work on a single segmentSet
//or an array of segmentSets, see class constructors below for details
//specifiy a segmentSet array like:
//SegmentSet *setArray[] = {&segmentSet1, &segmentSet2, etc}
//you may rebind the segmentSets using the setGroup() functions

//By default all the segmentSet will have their offsetRates bound to the Util's 
//so changing the offsetRate of the Util will also change it for all the segments

//Make sure you use the Util's functions for adjusting the settings. They will set the settings for all the 
//segmentSets. The variables in this Util are for reference
//ie use setDirect(), setRate(), setOffsetActive() instead of just changing direct, runOffset, Rate
//(you can change colorMode dirctly since this is independent of the segmentSets)

//Example calls: 
    //SegOffsetCyclerPS(segmentSet, 1, true, 30);
    //Sets the offset using colorMode 1 in the forward direction at a rate of 30ms
    //for a single segmentset

    //SegmentSet *setArray[] = {&segmentSet1, &segmentSet2}
    //SegOffsetCyclerPS(setArray, SIZE(setArray), 4, false, 50);
    //Sets the offset using colorMode 4 in the backwards direction at a rate of 50ms
    //for an array of segmentSets (containing segmentSet1 and segmentSet2)

//Constructor Inputs:
    //segmentSet (optional, see constructors) -- A single segmentSet whose offset will be changed
    //SegmentSetArr (optional, see constructors) -- An array of segmentSets whose offsets will be changed
    //ColorMode -- The colorMode used for the offset
    //Direction -- The direction of the offset (true is forward)
    //Rate -- The update rate of the utility (ms)

//Functions:
    //setCycle() -- Sets the direction and can turn the offset on/off in one function
    //setDirect() -- Sets the offset direction
    //setOffsetActive() -- Turns the offset on or off
    //setRate() -- Changes the offset rate (also changes it for all the segmentSets)
    //setGroup(SegmentSet** SegmentSetArr, uint8_t NumSegSets) -- Sets the Util to act on the passed in segmentSet array
    //setGroup(SegmentSet &segmentSet) -- Sets the Util to act on the passed in segmentSet only
    //update() -- updates the effect

//Other Settings:
    //runOffset (default true) -- For reference, records if the offset is active or not
    //direct -- For reference, the offset's direction of motion
    //colorMode -- The colorMode used for the offset, is a pointer so it can be bound to an external variable
    //             By default it is bound to colorModeOrig

class SegOffsetCyclerPS : public EffectBasePS {
    public:
        //two constructors, for single and multiple segmentSets
        SegOffsetCyclerPS(SegmentSet &segmentSet, uint8_t ColorMode, bool direction, uint16_t Rate);

        SegOffsetCyclerPS(SegmentSet **SegmentSetArr, uint8_t NumSegSets, uint8_t ColorMode, bool direction, uint16_t Rate);

        ~SegOffsetCyclerPS();

        uint8_t 
            colorModeOrig, //local storage of the colorMode, by default colorMode points to this
            *colorMode;
        
        bool 
            runOffset = true, //turns the cycle on/off, default is on
            direct; //direction the offset moves
    
        void
            setCycle(bool newDirect, bool newRunOffset), //defines a new set of cycle parameters
            setDirect(bool newDirect), //sets the shift direction, 
            setRate(uint16_t newRate),
            setOffsetActive(bool newRunOffset),
            setGroup(SegmentSet** SegmentSetArr, uint8_t NumSegSets), //sets a new group of segmentSets to act on
            setGroup(SegmentSet &segmentSet), //sets a new segmentSet to act on
            update();
        
    private:

        void 
            bindSegRates();

        uint8_t
            numSegSets;
        
        uint16_t
            offsetMax;
        
        unsigned long
            currentTime,
            prevTime;

        SegmentSet
            **segGroupTemp, //temp pointer used for setting up single segmentSets, seperate from group to allow safe deletion
            **segGroup;
        
        void
            init(bool direction, uint8_t ColorMode, uint16_t rate);

};

#endif
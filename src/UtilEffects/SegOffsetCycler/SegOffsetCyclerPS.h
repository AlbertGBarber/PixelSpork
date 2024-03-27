#ifndef SegOffsetCyclerPS_h
#define SegOffsetCyclerPS_h

#if ARDUINO >= 100
    #include "Arduino.h"
#else
    #include "WProgram.h"
    #include "pins_arduino.h"
    #include "WConstants.h"
#endif

#include "Include_Lists/SegmentFiles.h"
#include "Effects/EffectBasePS.h"

/* 
A utility class that changes the rainbow/gradient offset of a segment set (or group of segment sets) 
over time for Color Modes (seeSegDrawUtils::setPixelColor()).
Note that for the utility to work, the segment set's `runOffset` setting must be true 
(otherwise the offsets will not be changed). 
Likewise, also note that effects will update their segment set's offset when they update, 
so this utility is _only_ needed if you need to update the offset faster, or want to update 
the offset for multiple segment sets at once.

The Color Mode used in this utility must match the Color Mode of the effect you're using 
(since the offset length is based on the Mode). 
Helpfully, the `colorMode` setting in the utility is a pointer, 
so you can bind it directly to the effect's Mode like: 

    yourUtilityInstance.colorMode = &yourEffect.colorMode;

This mean any changes made to the effect's Color Mode will also change the utility's.

Note that by default, the utility's `colorMode` is bound to its local variable, `colorModeOrig`. 
To change `colorMode` directly, use 
    
    yourUtilityInstance.colorModeOrig = aNewColorMode.

The utility can be specified to work on a single segment set or an array of multiple segment sets.

You can create an array of segment sets like:

    SegmentSetPS *setArray[] = {&segmentSet1, &segmentSet2, etc}

The utility will tie various offset settings for the segment sets together, 
so that they all share the same value for their offset, offset rate, etc.

Make sure you use the utility's functions for adjusting these settings, as they will set 
them for all the segment sets. 
The variables in this Util are for reference. ie use `setDirect()`, `setRate()`, and `setOffsetActive()` 
instead of just changing the set's `direct`, `offsetRate`, `runOffset`.

Example calls: 
    SegOffsetCyclerPS segOffsetCycler(mainSegments, 1, true, 30);
    Sets the offset for the single "mainSegments" segment set
    using colorMode 1 in the forward direction,
    changing at a rate of 30ms.

    SegmentSetPS *setArray[] = {&segmentSet1, &segmentSet2} 
    //You'll have to define segment sets 1 and 2 yourself

    SegOffsetCyclerPS SegOffsetCycler(setArray, SIZE(setArray), 4, false, 50);
    Sets the offset for the segment set array above, 
    using colorMode 4, in the backwards direction,
    changing at a rate of 50ms.

Constructor Inputs:
    segSet (optional, see constructors) -- A single segment set for the utility to use. 
                                           Note that you cannot access the segment set within the utility.
                                           (Use the utility's functions instead)
    segmentSetArr (optional, see constructors) -- An array of multiple segment sets for the utility to use.
                                                  Note that you cannot access the segment sets within the utility.
                                                  (Use the utility's functions instead)
    colorMode -- The Color Mode used by the utility (see more in into above).
    direct -- The direction of the offset (true is forward). Can be changed later using `setDirect()`.
    rate -- The update rate of the utility (ms).

Other Settings:
    active (default true) -- If false, the effect will be blocked from updating
                 
Functions:
    setDirect(newDirect) -- Sets the offset direction of motion (true is forward).
    setRunOffset(newRunOffset) -- Turns the offset motion on/off (false is off).
    setCycle(newDirect, newRunOffset) -- A combination of the above two functions. 
                                         Sets the direction and can turn the offset on/off in single function.
    setRate(newRate) -- Changes the offset rate for all the segment set's and the utility's update rate to match.
    setSegmentArray(SegmentSetPS** SegmentSetArr, NumSegSets) -- Sets the utility to use a new array of segment sets.
    setSingleSet(SegmentSetPS &SegSet) -- Sets the utility to use a new single segment set.
    update() -- updates the effect

Reference Vars:
    direct --The offset's direction of motion, set via setDirect() or setCycle().
    runOffset (default true) -- If false, the offset will **not** be changed over time, set using setRunOffset() or setCycle().

*/
class SegOffsetCyclerPS : public EffectBasePS {
    public:
        //Constructor for a single segment set
        SegOffsetCyclerPS(SegmentSetPS &SegSet, uint8_t ColorMode, bool Direct, uint16_t Rate);

        //Constructor for an array of segment sets
        SegOffsetCyclerPS(SegmentSetPS **SegmentSetArr, uint8_t NumSegSets, uint8_t ColorMode, bool Direct,
                          uint16_t Rate);

        ~SegOffsetCyclerPS();

        uint8_t
            colorModeOrig,  //local storage of the colorMode, by default colorMode points to this
            *colorMode = nullptr;

        bool
            runOffset = true,  //turns the cycle on/off, default is on, for reference
            direct;            //direction the offset moves, for reference

        void
            setCycle(bool newDirect, bool newRunOffset),  //defines a new set of cycle parameters
            setDirect(bool newDirect),                    //sets the shift direction,
            setRate(uint16_t newRate),
            setRunOffset(bool newRunOffset),
            setSegmentArray(SegmentSetPS **SegmentSetArr, uint8_t NumSegSets),  //sets a new array of segment sets to act on
            setSingleSet(SegmentSetPS &SegSet),                                 //sets a new segment set to act on
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

        SegmentSetPS
            **segGroupTemp = nullptr,  //temp pointer used for setting up single segmentSets, separate from group to allow safe deletion
            **segGroup = nullptr;

        void
            init(bool Direct, uint8_t ColorMode, uint16_t rate);
};

#endif
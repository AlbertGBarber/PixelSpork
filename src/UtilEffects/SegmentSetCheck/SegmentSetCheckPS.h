#ifndef SegmentSetCheckPS_h
#define SegmentSetCheckPS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
//#include "MathUtils/mathUtilsPS.h"

/*
An effect used to test and verify the configuration of the segments in a segment set
For a given segment set, two tests can be performed (0 and 1):
    0: Goes through each segment in the segment set
        Sets the first and last pixels to red and blue respectively for each segment
        While the rest of the segment is lit up in green, one pixel at a time, in order of the segment direction
    1: Colors the first and last segment line in red and blue respectively
        While the other lines are flashed in green, one by one
What modes are run is based on the testMode value:
    0: Only testing mode 0, repeating
    1: Only testing mode 1, repeating
    2+: Testing both modes, one after another, repeating

Example calls: 
    SegmentSetCheckPS(mainSegments, 2);
    Does both tests 0 and 1 on the mainSegments segment set 
    The default update rate of 500ms will be used

    SegmentSetCheckPS(mainSegments, 1, 1000);
    Does both test 1 on the mainSegments segment set 
    The update rate is set to 1000ms

Constructor Inputs:
    testMode -- The type of test (0 or 1, see above) to be done
                Passing in 2+ will do both tests
    rate (optional, default 500ms) -- The update rate of the utility (ms)

Functions:
    update() -- updates the effect

*/
class SegmentSetCheckPS : public EffectBasePS {
    public:
        SegmentSetCheckPS(SegmentSet &SegSet, uint8_t TestMode);  

        SegmentSetCheckPS(SegmentSet &SegSet, uint8_t TestMode, uint16_t Rate);  

        SegmentSet 
            &SegSet; 
        
        uint8_t
            testMode;
        
        void 
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        uint8_t
            mode;
        
        uint16_t
            numSegs,
            segNum = 0,
            numLines,
            pixelCount = 1,
            totSegLen;
        
        void 
            nextStage();
};

#endif
#ifndef SegmentSetCheckPS_h
#define SegmentSetCheckPS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
//#include "MathUtils/mathUtilsPS.h"

/*
A utility effect to verify the layout of a segment set. 
See https://github.com/AlbertGBarber/PixelSpork/wiki/Segment-Basics for more info on segment sets. 
The utility behaves like a regular effect, with updating, etc. For a pre-configured example, 
see the "Segment Set Check" example under "examples" in the Arduino IDE.

For a given segment set, two tests can be performed:
    * 0 -- For each _segment_ in the segment set, the first and last pixels will be set to red and blue respectively, 
           while the rest of the segment is lit up in green, one pixel at a time, in order of the segment direction. 
           The segments are lit matching their order in the segment set.

    * 1 -- Colors the first and last _segment lines_ in red and blue respectively, 
           while the other lines are flashed in green, one by one, matching their order in the segment set. 

What modes are run is based on the `testMode` value:
    * 0 -- Runs only testing mode 0, repeating.
    * 1 -- Runs only testing mode 1, repeating.
    * 2+ -- Runs both testing modes, one after another, repeating.

By default the effect's update rate is 500ms, or you can configure it as part of the constructor.

Example calls: 
    SegmentSetCheckPS segmentSetCheck(mainSegments, 2);
    Does tests 0 and 1 on the mainSegments segment set. 
    The default update rate of 500ms will be used.

    SegmentSetCheckPS(mainSegments, 1, 1000);
    Does test 1 on the mainSegments segment set.
    The update rate is set to 1000ms.

Constructor Inputs:
    *segSet -- The segment set you want to test.
    testMode -- The type of test to be done Passing in 2+ will do both tests. 
                (See intro above to test info).
    rate (optional, default 500ms) -- The update rate of the utility (ms)

Functions:
    update() -- updates the effect

Reference Vars:
    mode -- The current test mode being run.

*/
class SegmentSetCheckPS : public EffectBasePS {
    public:
        SegmentSetCheckPS(SegmentSetPS &SegSet, uint8_t TestMode, uint16_t Rate = 500);

        uint8_t
            testMode, //Sets what test to run (2+ runs all tests)
            mode = 0; //track what test mode we're running

        void
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0;

        uint16_t
            numSegs,
            segNum = 0,
            numLines,
            pixelCount = 1,
            totSegLen;

        bool
            testStart = true;

        void
            init(SegmentSetPS &SegSet, uint16_t Rate),
            nextTestMode();
};

#endif
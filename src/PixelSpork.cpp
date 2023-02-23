#include "PixelSpork.h"

/* 
Effects TODO:
    -- Draw pattern based on segments, (basically a static patternShifter) (not needed, just update patternShifter once?)
    -- Android (see WLED android effect)
    -- Synced streamer dots across segments -> one array tracking dot locations on each segment, use even spacing.
    -- Cyborg rings (each segment has a dot that moves along it, pausing and reversing direction randomly)
    -- Aurora (see WLED)
    -- Running dual or phased noise (see WLED)
    -- A multiple version of breath eye (like twinkle, but you do breath eyes in place of twinkles)
    -- Multipath snake. Snake runs across segments, when segments meet it chooses which segment to switch to.
       Needs way to define segment connection points and directions. Snake tracks all the pixels it's on in array. 
    -- A function that cycles through an array of patterns
    -- A stacking pattern I guess
    -- A colorWipe function that does random length wipes?

Utils TODO:
    -- Function that sets up a synced rainbow across multiple segement sets. 
       Basically it just goes through each segment set and sets various rainbow offsets and lengths.
    -- Copy one section to another?
    -- Segment x,y. Not a util, but a function added to segments sets. 
       Segment sets are already matrixes -> their dimensions are numSegs x numLines. 
       The function would just return the pixel at the passed in line and seg num.
*/


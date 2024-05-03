#ifndef Pixel_Spork_h
#define Pixel_Spork_h

/*Welcome to my library!
    Pixel Spork is a WS2812+ LED Effects library based on the FastLED library.
    Aims to streamline the use of the popular WS2812 (and other) addressable LEDs by providing a 
    wide variety of rich, class-based, effects and utilities. Also supports segment sets, allowing 
    users to organize and re-arrange their LED layout in code. These sets can be further used to 
    create 2D effects. The library lets users code at their own comfort level by only requiring a 
    few core tools to operate, but providing many more for power users. Start small with a simple 
    segment set and one effect, and scale up over time to larger segment sets, using multiple effects 
    that are created/destroyed on the fly, while managing multiple palettes, color modes, utilities and more!

    --Albert Barber

    Please See wiki: 
    For a complete guide and introduction to this library.
*/

#include "FastLED.h"
#pragma once

#if ARDUINO >= 100
    #include "Arduino.h"
#else
    #include "WConstants.h"
    #include "WProgram.h"
    #include "pins_arduino.h"
#endif

#include "./Include_Lists/GlobalVars/GlobalVars.h"

#include "./Include_Lists/SegmentFiles.h"

#include "./Include_Lists/PaletteFiles.h"

#include "./Include_Lists/PatternFiles.h"

#include "./Include_Lists/UtilsList.h"

#include "./Include_Lists/EffectsList.h"

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
       Snake tracks all the pixels it's on in its own in array.
       For each segment, have a connection points array (probs a struct w/ length), each array entry has 3 points:
       The pixel number of the connection, what segment it connects to, and what the connection number it is for the new segment (ie connection point 2)
       When moving, as long as you know what connection index you started at and your direction, you only have to check for the next index pixel.
       You might need a marker for dead ends in the connection array.
    -- A function that cycles through an array of patterns
    -- A stacking pattern I guess
    -- A colorWipe function that does random length wipes?

Utils TODO:
    -- Noise util that changes the color mode offset lengths of segments -> expanding and contracting rainbows
    -- Function that sets up a synced rainbow across multiple segment sets. 
       Basically it just goes through each segment set and sets various rainbow offsets and lengths.
    -- Copy one section to another?
    -- segment x,y. Not a util, but a function added to segments sets. 
       segment sets are already matrixes -> their dimensions are numSegs x numLines. 
       The function would just return the pixel at the passed in line and seg num.
*/

#endif
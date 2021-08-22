#include "PixelStrip2.h"
    
// improvments TODO:
//. write an exponential dimming function (and allow trail length setting on patternSweep) (done, but exp dimming might be off)
//. change dimming function to always use crossfade function, need to make funct to return bg color
//. allow switching pallets on the fly --> wont be needed with nodelay rewrite
//. BgColor setting code is repeated a few times --> make into funct? (done)
//. allow setting bg as a gradient? (done)
//. allow using bg gradient for the main color in patternSweep, colorSpin, and colorWipe functions (done)
//. check for repeates of single pixel BgColor setting code (done)
//. go through comments, make sure they outline all functionality
//. check variable types for consistency
//. see if drawPattern funct can be used more
//. rewrite splitColor to return array of rgb comps (done)
//. allow setting of zero point for fireV2 (done, along with changing the color)
//. look at fire, update it
//. rewrite crossfadeRandomPattern to take a pallet (need a search function to make pallets)
//. rewrite patterSweep to set last pixels off instead of redrawing whole BG each time (done)
//. rewrite patterSweep patterns to include -1, 1 instead of 1, 0 for direction
//. allow pattern sweep to contain different trail types in one pattern
//. check if palletLength is needed in color wipe functs (done)
//. check limits on color wipe functs
//. let you choose direction for pattern sweep rand
//. make dimming variables global, including the pattenSweep rainbow adjustments
//. remove patternSweep rainbow functs? not needed since b/c of color mode
//. re-write spinPattern, it is very restrictive about BG
//. add step rates to simple repeat, and wave functions?
//. patternSweepRepeat is memory with large patterns (sorta fixed with gradientCycle)
//. add radial rainbow to drawLineSimple (done, need to update/replace functions that call it) (done)
//. simpleRepeatPattern takes up to much memory, isnt 255 pixel compatible
//. add a direction to segments, which treats their pixels in reverse order (done) -----> fix segment functions to not clash with this
//. make RGB stored in PROGMEM, not sure how to work with dynamic ones (canceled, dont use RGB for variables in .ino)
//. fix end points on gradientCycle (done sorta, the dot doesn't always happen, depends on the lengths)
//. make shooter take an array of variable lengths --> write helper function for a fixed length {done} --> also made one that takes segments (done)
//. look at reworking patternSweep, it's slow with a lot of pixels, rework desaturate funct
//. for shooterSeg, let you split the segments into pieces?
//. rewrite do simple repeat pattern to not use crossfadeRandom, instead repeat the pattern down the strip
//. blend the colors in fireV2 for a more nuanced fire (done)
//. add the ability to shift a rainbow bg / anything that uses wheel by adding and offset parameter (you could also do this of the BgGradient) (done, not for BG)
//. adjust segGradientCycleSweep to account for dummy leds
//. merge sonarWave and sonarSpin into one funct b/c they share the same code mostly

// Patterns TODO:
//. a proper fireworks routine (trails fade over time), could be a mod of the patternSweep with exponentially dimming trails
//. Draw pattern based on segments, (basically a static colorSpin)
//. a proper rainbow mode for color spin, cycles through rainbow based on longest segment? (done)
//. a colorWipe version of colorSpin (done)
//. a "shooter" pattern, like patternSweep, but the pixels only cycle round once (done)
//. an automated section color wipe, wipes a series of different sized sections in colors (done)
//. a segmented version of PatternSweep
//. segmented version of firev2 (done)
//. a function that cycles through an array of patterns
//. function that crossfades between colors (done w/ random too)
//. a radial version of the color spinFunction (ie a waves function that takes patterns)
//. a stacking pattern I guess
//. a simple function to crossfade through a set of colors (done)
//. a less memory intensive streamer function (done)
//. low memory version of waves (done)
//. a colorWipe function that does random length wipes
//. radial color wipe funct (done)
//. a static waves function (ie a BG that doesn't change as waves move), write like a radial patternSweep (done? sonar?)
//. random color waves function (done)
//. color spin simple pallet rand (easier to gen a random pallet seperatly)
//. simple steamer where it takes a pallet, no pattern (like simpleWaves)
//. shooter that takes only one color (maybe use the palletLength for the color and a mode?) (right now, you need to make a one length pallet)
//. a gradient segment function that repeats the same gradient down each segment (done)
//. add a rainbow mode to shooter seg where the rainbow is destributed down the seg (done?)
//. gradientSweep rainbow (needs direction setting in segment definition) (not needed, colorSpin can do this?)


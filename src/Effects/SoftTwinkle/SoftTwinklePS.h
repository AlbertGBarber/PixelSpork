#ifndef SoftTwinklePS_h
#define SoftTwinklePS_h

//based on the softTwinkle effect by Mark Kriegsman
//https://gist.github.com/kriegsman/99082f66a726bdff7776

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

/* 
For this animation to work, the Red component of lightColor
MUST be nonzero, AND must be an EVEN number!!!!
Note that it is not compatible with color modes, 
and should not be used with other effects on the same segment set

If you are using FastLED setCorrection(TypicalLEDStrip), 
then (8,7,1) gives a very soft, warm yellowish white,
a little bit like incandescent fairy lights.  If you are
not using FastLED color correction, try (8,5,1) (default).

The basic idea behind this animation is that each pixel is 
black until it's 'bumped' with a small amount of light, at which 
point it will get brighter and brighter until it maxes out.  
Then it will switch direction and start getting dimmer and dimmer
until it's black again. That's basically the whole thing.
This gives a classic white holiday twinkling light look

So to implement this, each pixel needs to 'know' whether it should
be getting brighter or getting dimmer.  We could implement this
with a parallel array of direction indicators, but that would 
take up extra memory.

Instead, we use the lowest bit of the red component as a direction
indicator.  Stop and re-read that previous sentence again.  We're
using the least significant bit of one of the color components as
an actual data value.

If the lowest bit of the red component is ZERO (i.e.
pixel.red is EVEN), it means that this pixel should be BRIGHTENING.  
If the lowest bit of the red component is ONE (i.e. pixel.red is ODD),
it means that this pixel should be DIMMING.

In this way, the low bit of the red componetn IS the direction indicator:
If red is EVEN, it's going up.  If red is ODD, it's coming down.

Now as if that weren't complicated enough, we use the 'saturating'
properties of CRGB pixel math to avoid having to test for when
we've hit the maximum brightness, or for having to test for when
we've hit total black.  

The result is that we can do this whole thing in a few lines of code.

Here's how the code works: each pixel can be in one of three states:
  1. It can be total black, in which case we do nothing to it.
  2. It can have an EVEN red component, in which case we BRIGHTEN it
  3. It can have an ODD red component, in which case we DARKEN it
When we're brightening the red component (because it's EVEN), it will
saturate at a maximum of 255 -- which means it will automatically 
switch from EVEN to ODD when it hits the top of the range.  And 
because it's now an ODD number, we'll DARKEN this pixel each time;
the numbers start coming back down.  And when it dims all the way 
down, it will saturate out at total black automatically.

If this still all seems confusing, that's OK.  It took me a LONG 
time to slowly fit all these puzzle pieces together to make it work
the way I wanted it to.  And in the end I was actually very 
surprised at how little actual code was needed to make it work.
  - Mark Kriegsman, December 2014

Example Call:

    SoftTwinklePSminaSegments, 150, 20);
    soft twinkles with density of 150, updating at a rate of 20ms

Functions:
    reset() -- resets all the segment leds to black, restarting the effect
    update() -- updates the effect

Constructor Inputs:
    Density -- max 255, the threshold for turning on an led, higher will turn on more leds at once

Other Settings:
    lightcolor default( CRGB{8,5,1} ) -- The color used to increment the pixel colors
                                        as noted above the red value should always be an even number

Notes:
    This is a pretty basic and pre-packaged effect, so there's not a lot of inputs or adjustments allowed
    but it looks nice, and doesn't take up much memory 
*/
class SoftTwinklePS : public EffectBasePS {
    public:
        SoftTwinklePS(SegmentSet &SegmentSet, uint8_t Density, uint16_t Rate);  

        uint8_t
            density;

        SegmentSet 
            &segmentSet; 
        
        CRGB 
            lightcolor = CRGB{8,5,1};
        
        void
            reset(),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        uint16_t 
            pixelNum,
            numPixels;
        
        CRGB
            color;
};

#endif
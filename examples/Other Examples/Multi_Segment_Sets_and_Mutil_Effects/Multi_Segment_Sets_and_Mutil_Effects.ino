#define FASTLED_INTERNAL 1  //turns off the pragma messages from FastLED
#include <PixelSpork.h>

/*
This example demonstrates how to run two different effects on two different Segment Sets
Ie one effect on one Segment Set and one effect on the other. 

For this example we'll make two simple Segment Sets by splitting the strip in two.
(If you have an odd length strip, the last LED will be cut off)
We'll then create an rainbow cycle and twinkle effect and run them on the Segment Sets.

Even though the effects are running on the same strip, they will only be drawn in their Segment Sets.
So one half of the strip will have the twinkle effect and the other, a rainbow cycle.

Note that we'll use the same update rate for both effects, and we'll also set one of the effect's
"showNow" variable to false. This allows the effect to update, but not actually write the colors out to the strip.
Since both effects are updating at the same time, we only need to write out the colors using one effect.
(FastLED writes out the whole leds color buffer for every update, so both effects will be written out.)
You can read more about this at https://github.com/AlbertGBarber/PixelSpork/wiki/Common-Effect-Variables-and-Functions#common-variables
under the "showNow" entry.

Note that this example expects you to have read the starter examples,
so it won't go over effects, Segment Sets, etc. 
*/

//How many LEDs in your strip?
#define NUM_LEDS 60

//Define the data pin for your LEDs
//For SPI LEDs (4 wires) you'll need to define a clock pin as well,
//Check the FastLED examples for this
#define DATA_PIN LED_BUILTIN 
/* Most Micro-controllers have an on-board LED you can control.
   On the UNO, Nano, and MEGA it is attached to digital pin 13. 
   On the Wemos D1 Mini it is D4.
   For other boards, check a pin-out guide for the LED pin.
   LED_BUILTIN is set to the correct LED pin independent of which board is used. */

//Define the array of LEDs
CRGB leds[NUM_LEDS];

//=============================================================
//                  Segment Sets
//=============================================================

//The first Segment Set; the first half of the strip.
//Consists of one Segment with one Section starting at 0 and running NUM_LEDS/2.
const PROGMEM segmentSecCont half1Sec[] = { {0, NUM_LEDS/2} };
SegmentPS half1Seg = { half1Sec, SIZE(half1Sec), true };
SegmentPS *half1Seg_arr[] = { &half1Seg };
SegmentSetPS firstHalfSeg( leds, NUM_LEDS, half1Seg_arr, SIZE(half1Seg_arr) );

//-------------------------------------------------------------

//The second Segment Set; the second half of the strip.
//Consists of one Segment with one Section starting at NUM_LEDS/2 and running NUM_LEDS/2.
const PROGMEM segmentSecCont half2Sec[] = { {NUM_LEDS/2, NUM_LEDS/2} };
SegmentPS half2Seg = { half2Sec, SIZE(half2Sec), true };
SegmentPS *half2Seg_arr[] = { &half2Seg };
SegmentSetPS secondHalfSeg( leds, NUM_LEDS, half2Seg_arr, SIZE(half2Seg_arr) );

//=============================================================
//                  Effects
//=============================================================

//Setup a Rainbow Cycle effect, named "rainbowCycle" to draw on our firstHalfSeg Segment Set.
//The rainbows length is set to NUM_LEDS/2, so a full rainbow fits on the half strip.
//wiki page: https://github.com/AlbertGBarber/PixelSpork/wiki/Rainbow-Cycle
RainbowCyclePS rainbowCycle(firstHalfSeg, NUM_LEDS/2, true, 80);

//Setup a twinkle effect called "twinkle" to draw on our secondHalfSeg Segment Set.
//The effect will use the build in cybPnkPal_PS palette with a blank background
//wiki page: https://github.com/AlbertGBarber/PixelSpork/wiki/Twinkle-(Seg-Line)
TwinkleSL twinkle(secondHalfSeg, cybPnkPal_PS, 0, 1, 6, 6, 80);

//Note that the two effects are using the same update rate (80ms)
//This makes it easier to handle updating both the effects.
//(see the intro)

void setup() {

    //Setup your LED strip using FastLED
    //This setup assumes you're using WS2812 LEDs
    //For other LED type setups, see the FastLED Blink  example
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed

    //Set a strip brightness
    FastLED.setBrightness(40);

    //Set the twinkle effect to draw but not display
    //so that we only display once during an update cycle
    //(see the intro)
    twinkle.showNow = false;
}

void loop() {

    //To run both effects, we just need to update them.
    //Even though they're both on the same strip, they'll each only be drawn on their own Segment Sets
    //Note that we make sure to update rainbowCycle last, since it is our displaying effect
    //(see the intro)
    twinkle.update();
    rainbowCycle.update();
}

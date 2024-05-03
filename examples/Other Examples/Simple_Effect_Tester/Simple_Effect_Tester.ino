#define FASTLED_INTERNAL 1  //turns off the pragma messages from FastLED
#include <Pixel_Spork.h>

/*
This code is for quick effect testing.
It includes all the basic setup and a configurable 1 or 2D segment set
It is very similar to the Basic_Setup intro example, but omits much of the comments.
*/

// How many LEDs in your strip?
#define NUM_LEDS 60

// Define the data pin for your LEDs
#define DATA_PIN LED_BUILTIN
/* LED_BUILTIN is 13 on the UNO, Nano, and MEGA,
   It is D4 the Wemos D1 Mini.*/

// Define the array of LEDs
CRGB leds[NUM_LEDS];

//Segment Set
//configurable via uncommenting to either be the whole strip in 1D,
//or splitting the strip into two equal rows (a 2 x NUM_LEDS/2 matrix)
//(configured to 1D by default)
//==================================================
//Segment 1
//const PROGMEM segmentSecCont halfSec0[] = {{1, NUM_LEDS / 2}}; //For 2 row segment set
const PROGMEM segmentSecCont halfSec0[] = { {1, NUM_LEDS - 1 } }; //For 1D segment set
SegmentPS halfSeg0 = {halfSec0, SIZE(halfSec0), true};

//Segment 2 (for the 2D segment set's second row)
const PROGMEM segmentSecCont halfSec1[] = {{NUM_LEDS / 2 + 1, NUM_LEDS / 2}};
//Note that we specify the direction of the segment to be false, this mirrors the segment from halfSeg0 above
SegmentPS halfSeg1 = {halfSec1, SIZE(halfSec1), false};

//Create the segment set (mainSegments)
//SegmentPS *halfSegs_arr[] = {&halfSeg0, &halfSeg1}; //For 2 row segment set
SegmentPS *halfSegs_arr[] = { &halfSeg0 }; //For 1D segment set
SegmentSetPS mainSegments(leds, NUM_LEDS, halfSegs_arr, SIZE(halfSegs_arr));


//Effect
//=============================================================
StreamerSL streamer(mainSegments, cybPnkPal_PS, 0, 3, 3, 15, 60);

void setup() {

    //This setup assumes you're using WS2812 LEDs
    //For other LED type setups, see the FastLED Blink  example
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed

    FastLED.setBrightness(40);
}

void loop() {

    //update the effect
    streamer.update();
}

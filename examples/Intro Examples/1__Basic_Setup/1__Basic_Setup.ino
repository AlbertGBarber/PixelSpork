#define FASTLED_INTERNAL 1  //turns off the pragma messages from FastLED
#include <Pixel_Spork.h>

/*
This example lays out a basic Pixel Spork program.
We'll create a simple Segment Set and a RainbowCycle effect, and update the effect to display it.
For a complete guide to the code, see
https://github.com/AlbertGBarber/PixelSpork/wiki/Basic-Setup

Note that most of Pixel Spork is class based.
To use the library to its fullest, you'll want to
read/understand C++ classes and objects.
(Understanding pointers somewhat also isn't going to hurt :) )

Note that Pixel Spork uses the FastLED library to manage much of the nitty gritty of writing to the LEDs
So some functions in this example will be from FastLED.
(while helpful, a full understanding of FastLED is not required to use Pixel Spork)

Note: when compiling using the Arduino IDE, if you have your "compiler warnings" (found in "preferences") 
set to "More" or "All" you may get a few warnings when you first compile a sketch. 
These should mainly concern the possible non-usage of various static variables, and are expected. 
They will not prevent the code from running!
*/

// How many LEDs in your strip?
#define NUM_LEDS 60

// Define the data pin for your LEDs
// For SPI LEDs (4 wires) you'll need to define a clock pin as well,
// Check the FastLED examples for this
#define DATA_PIN LED_BUILTIN 
/* Most Micro-controllers have an on-board LED you can control.
   On the UNO, Nano, and MEGA it is attached to digital pin 13. 
   On the Wemos D1 Mini it is D4.
   For other boards, check a pin-out guide for the LED pin.
   LED_BUILTIN is set to the correct LED pin independent of which board is used. */

// Define the array of LEDs
CRGB leds[NUM_LEDS];

/*
Setup a simple Segment Set with one segment containing the whole strip.
Segment Sets allow you to virtually arrange you LEDs into whatever shape you wish. They are a core part of Pixel Spork.
Every effect requires a Segment Set -- they are the canvas's on which effects draw.
However, Segments Sets are too complicated to explain here, see the Segment Basics wiki page for more info.
https://github.com/AlbertGBarber/PixelSpork/wiki/Segment-Basics
or the "2D Segment Sets for 2D Effects" example
*/

//The code below sets up a segment set that replicates your LED strip as it is connected IRL.
const PROGMEM segmentSecCont mainSec[] = { {0, NUM_LEDS} };
SegmentPS mainSegment = { mainSec, SIZE(mainSec), true };
SegmentPS *main_arr[] = { &mainSegment };
SegmentSetPS mainSegments( leds, NUM_LEDS, main_arr, SIZE(main_arr) );

/*
Setup our Rainbow Cycle effect, named "rainbowCycle" to draw on our mainSegments Segment Set.
This effect cycles a rainbow across the Segment Set.
All effects in Pixel Spork are class based.
We create the effect using a constructor with a list of settings (arguments),
and give it a name so we can use it in our code.
A full list of effects is available in the Pixel Spork wiki under "Effect List" on the side bar.
Effect wiki page: https://github.com/AlbertGBarber/PixelSpork/wiki/Rainbow-Cycle
*/
RainbowCyclePS rainbowCycle(mainSegments, true, 80);

void setup() {

    //Setup your LED strip using FastLED
    //This setup assumes you're using WS2812 LEDs
    //For other LED type setups, see the FastLED Blink  example
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed

    //Set a strip brightness
    FastLED.setBrightness(40);
}

void loop() {

    //All of the code for our effect is included within the effect,
    //so to draw it, all we need to do is update() the effect.
    rainbowCycle.update();
}

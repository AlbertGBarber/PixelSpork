#define FASTLED_INTERNAL 1  //turns off the pragma messages from FastLED
#include <PixelSpork.h>

/*
This code can be used to check your Segment Set layout using 
the Pixel Spork SegmentSetCheck utility class.

Simply replace the "mainSegments" Segment Set below with your own Set and run the program.

SegmentSetCheck performs two tests:
    0: For each *segment* in the segment set, 
       the first and last pixels will be set to red and blue respectively, 
       while the rest of the segment is lit up in green, one pixel at a time, 
       in order of the segment direction. 
       The segments are lit matching their order in the segment set.

    1: Colors the first and last _segment lines_ in red and blue respectively, 
       while the other lines are flashed in green, one by one, 
       matching their order in the segment set. 
*/

// How many leds in your strip?
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

// Define the array of leds
CRGB leds[NUM_LEDS];

//=========================================================
//                 Test Segment Set
//=========================================================

//Replace the Segment Set setup below with your own.
//Re-name your set to "mainSegments".
const PROGMEM segmentSecCont mainSec[] = { {0, NUM_LEDS} };
SegmentPS mainSegment = { mainSec, SIZE(mainSec), true };
SegmentPS *main_arr[] = { &mainSegment };
SegmentSetPS mainSegments(leds, NUM_LEDS, main_arr, SIZE(main_arr));

//==========================================================

SegmentSetCheckPS segmentSetCheck(mainSegments, 2, 1000);
//Does both tests 0 and 1 on the mainSegment set
//(You can set the "2" to 0 or 1 to just do one test)
//The update rate is 1000ms

//Tracks when we change test modes, to output a new serial info string.
uint8_t testModePrev = 999;

void setup() {
    Serial.begin(9600);

    //Setup your LED strip using FastLED
    //This setup assumes you're using WS2812 LEDs
    //For other LED type setups, see the FastLED Blink  example
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed

    //Set a strip brightness
    FastLED.setBrightness(40);
}

void loop() {

    segmentSetCheck.update();

    //Output a serial string when we switch test modes to tell you what test is running
    if(testModePrev != segmentSetCheck.mode) {
        testModePrev = segmentSetCheck.mode;
        if(segmentSetCheck.mode == 0) {
            Serial.println("Test Mode 0: Sets the first and last pixels to red and blue for each segment,");
            Serial.println("             while the rest of the segment is lit up in green, one pixel at a time.");
            Serial.println("");
        } else {
            Serial.println("Test Mode 1: Colors the first and last segment lines in red and blue,");
            Serial.println("             while the other lines are flashed in green, one by one.");
            Serial.println("");
        }
    }
}

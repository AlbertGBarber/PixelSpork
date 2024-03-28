#define FASTLED_INTERNAL 1  //turns off the pragma messages from FastLED
#include <PixelSpork.h>

/*
Note that most of Pixel Spork is class based.
To use the library to its fullest, you'll want to
read/understand C++ classes and objects.
(Understanding pointers somewhat also isn't going to hurt :) )

This example lays out a how to use Color Modes in Pixel Spork
For a complete guide to the code, see
https://github.com/AlbertGBarber/PixelSpork/wiki/Color-Mode-Example
*/

/*
In this example, we'll create two effects, a ColorModeFillPS and a TheaterChaseSL.
ColorModeFillPS just fills in the segment set using a set Color Mode.
We'll use this to demonstrate each rainbow color mode (we won't do any custom gradient modes in this example).

TheaterChaseSL is a simple effect that moves bands of color along the segment set. We'll use this
effect to show how using Color Modes in an effect works

For both effects, we'll set the rainbow gradients to shift over time for extra fun.

Because some Color Modes only really work with 2D segment sets, we'll create a simple segment set
with 4 "rows" by splitting the strip into quarters.

We'll start by showing each Color Mode on the whole strip using ColorModeFillPS.
Once each Color Mode has been shown, we'll switch the the TheaterChaseSL effect , and leave that running
(while still switching the Color Mode periodically)
*/

/*
Color Modes:
    Color modes are a special setting included in many effects. 
    They replace the effect's colors with either a rainbow or custom gradient. 
    Allowing you to create rainbow effects without any extra work.

    Color modes are designed to interact directly with 2D segment sets,
    with each mode spreading the rainbow across the segment set in a different orientation.
    So how a rainbow looks, will depend on the effect's segment set (as well as a few extra settings in the segment set)

    You can also configure the rainbows to shift over time, adding an extra bit of spice to your effects.

    There are 4 main ways a rainbow gradient can be displayed:
        * Linearly across all the LEDs in the segment set, so that the rainbow gradient starts at the first LED and ends at the last. 
        * Radially, so that each whole segment is a single color from the rainbow gradient.
        * Linearly, but using segment lines, so that each line is a single color. This is used in the video above. 
        * With time (4D?), so that the whole segment set is a single color, but that color cycles through the rainbow over time.

    To read about all the details of Color Modes see
    https://github.com/AlbertGBarber/PixelSpork/wiki/Color-Modes
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

//Set up a simple 2D segment set, splitting the strip into 4 equal segments.
//If possible, you should arrange your LEDs into 4 equal rows to match the Segment Set.
//This will help make each Color Mode more obvious.
//Note that if you already have a 2D layout and Segment Set for your LEDs,
//I encourage you to use it in place of the Segment Set below. 
//For more info in segment sets, see
//https://github.com/AlbertGBarber/PixelSpork/wiki/Segment-Basics
//or the "2D Segment Sets for 2D Effects" example.

//Segment 0, starting at LED 0 with length for NUM_LEDS/4
const PROGMEM segmentSecCont sec0[] = { {0, NUM_LEDS / 4} };
SegmentPS segment0 = { sec0, SIZE(sec0), true };

//Segment 1, starting at LED (NUM_LEDS * 1/4) with length for NUM_LEDS/4
const PROGMEM segmentSecCont sec1[] = { {(NUM_LEDS * 1 / 4), NUM_LEDS / 4} };
SegmentPS segment1 = { sec1, SIZE(sec1), true };

//Segment 2, starting at LED (NUM_LEDS * 2/4) with length for NUM_LEDS/4
const PROGMEM segmentSecCont sec2[] = { {(NUM_LEDS * 2 / 4), NUM_LEDS / 4} };
SegmentPS segment2 = { sec2, SIZE(sec2), true };

//Segment 3, starting at LED (NUM_LEDS * 3/4) with length for NUM_LEDS/4
const PROGMEM segmentSecCont sec3[] = { {(NUM_LEDS * 3 / 4), NUM_LEDS / 4} };
SegmentPS segment3 = { sec3, SIZE(sec3), true };

//Create the segment set using the quarter segments above
SegmentPS *quatSegs_arr[] = { &segment0, &segment1, &segment2, &segment3 };
SegmentSetPS quarterSegs( leds, NUM_LEDS, quatSegs_arr, SIZE(quatSegs_arr) );

ColorModeFillPS colorModeFill(quarterSegs, 1, 70);
//Setup a ColorModeFillPS effect using our segment set and starting at Color Mode 1.
//(we'll cycle the color mode later)

TheaterChaseSL theaterChase(quarterSegs, CRGB::Red, 0, 3, 2, 100);
//Setup a theaterChaseSL effect using red as the running color on a blank background
//(since we're using color modes, the red will be overridden by the color mode)
//The chase will use bands of color of length 3, with 2 spaces in between, updating at 100ms
//wiki page: https://github.com/AlbertGBarber/PixelSpork/wiki/Theater-Chase-(Seg-Line)

//we'll use this to track the color mode
uint8_t colorMode = 1;

//Some time tracking variables for switching color modes and effects
unsigned long currentTime;
unsigned long modeSwitchTime = 6000; //how often we switch color modes (ms)
unsigned long effectChangeTime = 4 * modeSwitchTime; //what time we switch from the colorMode fill to theaterChase effect

void setup() {
    Serial.begin(9600);
    //Setup your LED strip using FastLED
    //This setup assumes you're using WS2812 LEDs
    //For other LED type setups, see the FastLED Blink  example
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed

    //Set a strip brightness
    FastLED.setBrightness(40);

    /*
    While a Color Mode is set in each effect, the look of the color modes actually depends on
    the settings in the effect's segment set. These determine how the color mode is drawn.
    By default, color modes gradients (ie the rainbows) are static, but you can configure them to 
    shift over time by setting the segment set's "runOffset", and adjusting the "offsetRateOrig" 
    and "offsetStep" to set the rainbow's speed.
    */
    quarterSegs.runOffset = true;     //tell the segment set to shift the color mode gradients over time
    quarterSegs.offsetRateOrig = 60;  //Set the rate that the gradient shifts at in ms

    /*
    In Pixel Spork, a rainbow has 256 unique colors. Each color mode "step" is a single color.
    256 colors can take a while to cycle through, but by setting the "offsetStep" you can tell the 
    segment set to "skip" forward by a set number of steps, increasing how fast the rainbow cycles.
    In practice this is useful, because it is not coupled to the offsetRate, so you can get faster speeds
    without needing to have a super high update rate.
    */
    quarterSegs.offsetStep = 3;

    /*
    You can also configure how "long" each rainbow gradient is by adjusting the values below, 
    each corresponds to a different set of color modes. Internally, they are defaulted such that a full rainbow
    fits on the segment set for each color mode.
    (These are just out of scope of this example, but you can un-comment the lines to try them out
    they each double how long the rainbow gradient is)
    */
    //quarterSegs.gradLenVal = NUM_LEDS * 2; //used for color modes 1 & 6
    //quarterSegs.gradSegVal = quarterSegs.numSegs * 2; //used for color modes 2 & 7
    //quarterSegs.gradLineVal = NUM_LEDS/4 * 2; //used for color modes 3 & 8
}

void loop() {
    //Finally we get to the actual code:

    //The code below switches color modes every "modeSwitchTime" ms, telling you which mode we're on and how it draws
    EVERY_N_MILLISECONDS(modeSwitchTime) {
        //Cycle through the color modes, but skip color mode 0 (since that just displays the effect as normal)
        colorMode = (colorMode + 1) % 5;
        if(colorMode == 0) { colorMode = 1; }

        //Set the color modes for both effects
        colorModeFill.colorMode = colorMode;
        theaterChase.colorMode = colorMode;

        //Output a message about what color mode we're using
        switch(colorMode) {
            case 1:
                Serial.println("Now using color mode 1, the rainbow is spread across all LEDs in the segment set.");
                break;
            case 2:
                Serial.println("Now using color mode 2, the rainbow is spread across all segments in the set.");
                break;
            case 3:
                Serial.println("Now using color mode 3, the rainbow is spread across all segment lines in the set.");
                break;
            case 4:
                Serial.println("Now using color mode 4, the segment set is a single color that cycles through the rainbow.");
                break;
        }
    }

    //Draw either the colorModeFill or theaterChase effects
    //We start by doing the colorModeFill until we hit effectChangeTime (after we're done each color mode)
    //And then we switch to just doing theaterChase forever.
    currentTime = millis();
    if(currentTime > effectChangeTime) {
        theaterChase.update();
    } else {
        colorModeFill.update();
    }
}

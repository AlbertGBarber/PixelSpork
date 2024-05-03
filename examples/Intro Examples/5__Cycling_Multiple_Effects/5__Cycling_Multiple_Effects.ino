#define FASTLED_INTERNAL 1  //turns off the pragma messages from FastLED
#include <Pixel_Spork.h>

/*
This example lays out basic program where you have a set of effects/utilities that you'd 
like to cycle through over time. 

In this example, we'll create a TwinkleSL and StreamerSL effect, and a PaletteBlenderPS utility.
We'll use the blend palette created by the PaletteBlender as the palette for the two effects 
(like in the 'Palette and Utility Classes" example).

To draw our effects, we'll use a switch statement to periodically swap which effect is being updated
(while also constantly updating the PaletteBlenderPS)

For a complete guide to the code, see
https://github.com/AlbertGBarber/PixelSpork/wiki/Cycling-Multiple-Effects
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

// Define the array of LEDs
CRGB leds[NUM_LEDS];

//Setup a simple Segment Set with one segment containing the whole strip.
//The code below sets up a segment set that replicates your LED strip as it is connected IRL.
const PROGMEM segmentSecCont mainSec[] = { {0, NUM_LEDS} };
SegmentPS mainSegment = { mainSec, SIZE(mainSec), true};
SegmentPS *main_arr[] = { &mainSegment };
SegmentSetPS mainSegments( leds, NUM_LEDS, main_arr, SIZE(main_arr) );

//Below we'll create two palettes to use with our PaletteBlender utility
//(see the "Palettes and Utility Classes" example for more about utilities)
//Create an array of random CRGB colors
CRGB palette0_arr[] = { colorUtilsPS::randColor(), colorUtilsPS::randColor(), colorUtilsPS::randColor() };
//Create the palette, "palette0" using the color array, and the array's length
//(automatically got using SIZE(), see https://github.com/AlbertGBarber/PixelSpork/wiki/Common-Effect-Variables-and-Functions#common-functions
palettePS palette0 = { palette0_arr, SIZE(palette0_arr) };

//Create a second palette, "palette1"
CRGB palette1_arr[] = { colorUtilsPS::randColor(), colorUtilsPS::randColor(), colorUtilsPS::randColor() };
palettePS palette1 = { palette1_arr, SIZE(palette1_arr) };

//Create two effects and a utility: myStreamer (a StreamerSL), myTwinkle (a TwinkleSL), and myPB (a PaletteBlenderPS).
PaletteBlenderPS myPB(palette0, palette1, true, 30, 100);
//wiki page: https://github.com/AlbertGBarber/PixelSpork/wiki/Palette-Blender

TwinkleSL myTwinkle(mainSegments, myPB.blendPalette, 0, 1, 6, 6, 70);
//wiki page: https://github.com/AlbertGBarber/PixelSpork/wiki/Twinkle-(Seg-Line)

StreamerSL myStreamer(mainSegments, myPB.blendPalette, 0, 4, 3, 8, 60);
//wiki page: https://github.com/AlbertGBarber/PixelSpork/wiki/Streamer-(Seg-Line)

//Variables we need to track the effect cycling
bool effectSetup = false;                        //Flag for if an effect has been configured for updating
uint8_t effectNum = 0;                           //Tracks what effect we're on; 0 or 1 for myTwinkle and myStreamer
uint16_t effectRunTime;                          //How long an effect should run for (set in the switch statements)
unsigned long currentTime, effectStartTime = 0;  //For tracking how long an effect has been running for.

void setup() {
    Serial.begin(9600);

    //Setup your LED strip using FastLED
    //This setup assumes you're using WS2812 LEDs
    //For other LED type setups, see the FastLED Blink  example
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed

    //Set a strip brightness
    FastLED.setBrightness(40);

    //Set the PaletteBlender to randomize the palettes when it loops
    //This gives us an every changing set of colors
    myPB.randomize = true;
}

/*
In the loop() cycle we're either updating or switching effects.
Overall the cycle looks like:   
    1. Start a loop cycle
    2. update() myPaletteBlend
    3. Use the switch() to go to the current effect
        3a. If we haven't setup the effect, do so by setting a run time (and anything else we might want to change about the effect).
        3b. If we have setup the effect, update it.
    4. Get out of the switch and check elapsed time.
        4a. If effectRunTime time has passed, swap to the next effect by incrementing the effectNum counter, 
            and flag the next effect to be setup.
    5. Loop back to the start for the next loop() cycle.
*/
void loop() {

    //Since we want the palette blender utility to always be running, we update it with every loop cycle
    myPB.update();

    //Use a switch statement to switch between what effect is being run
    switch(effectNum) {
        case 0: {  //effect 0, myStreamer
            //If we are just starting the effect, we need to do some setup, otherwise we update the effect
            if(!effectSetup) {
                effectRunTime = 10000;  //Set the effect to run for 10 sec
                effectSetup = true;
                Serial.println("Now running Streamer Effect");
            } else {
                myStreamer.update();
            }
        } break;
        case 1: {  //effect 1, myTwinkle
            //If we are just starting the effect, we need to do some setup, otherwise we update the effect
            if(!effectSetup) {
                effectRunTime = 10000;  //Set the effect to run for 10 sec
                effectSetup = true;
                Serial.println("Now running Twinkle Effect");
            } else {
                myTwinkle.update();
            }
        } break;
    }

    //Get the current time
    currentTime = millis();

    //Check if we need to move to the next effect based on the elapsed time and the effect's run time
    if(currentTime - effectStartTime > effectRunTime) {
        effectSetup = false;                        //flag the effect to run its setup if() in the next case iteration
        effectNum = (effectNum + 1) % 2;            //cycle to the next effect, using mod (%) to wrap back to 0
        effectStartTime = currentTime;              //Record the start time of the next effect
        segDrawUtils::turnSegSetOff(mainSegments);  //Clear the segment set to create a blank state for the next effect
                                                    //Note that we're using a helper function from the segDrawUtils namespace to do this
                                                    //https://github.com/AlbertGBarber/PixelSpork/wiki/Segment-Drawing-Functions
    }
}
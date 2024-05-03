#define FASTLED_INTERNAL 1  //turns off the pragma messages from FastLED
#include <Pixel_Spork.h>

/*
In this example, we'll use a Pixel Spork utility class, EffectSetsPS, to help switch between and manage
a pair of effects and a utility.

This example is more or less the same as the "Cycling Multiple Effects" example, but instead of managing the effects
manually, we'll use an EffectSetPS class. Effect sets are any alternative way of managing multiple effects. 
They allow you to update all your effects and utilities together, while automatically tracking the time elapsed.
For this simple example, they might seem like overkill, but as your code gets more complicated, such as
adding a large number of effects and utilities, running multiple effects on different Segment Sets, etc,
Effect Sets can really come in handy.
You can read about Effect Sets here: https://github.com/AlbertGBarber/PixelSpork/wiki/Effect-Set

In this example, we'll create a TwinkleSL and StreamerSL effect, and a PaletteBlenderPS utility.
We'll use the blend palette created by the PaletteBlender as the palette for the two effects 
(like in the 'Palette and Utility Classes" example). We'll also create an EffectSetPS to manage the effects.

For a complete guide to the code, see
https://github.com/AlbertGBarber/PixelSpork/wiki/Cycling-Using-EffectSets
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

//Setup a simple Segment Set with one segment containing the whole strip.
//The code below sets up a segment set that replicates your LED strip as it is connected IRL.
const PROGMEM segmentSecCont mainSec[] = { {0, NUM_LEDS} };
SegmentPS mainSegment = { mainSec, SIZE(mainSec), true };
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

//=====================================================================
//                  The Effect Set Setup
//=====================================================================
/*
Before we create our effects we'll create an Effect Set to manage them.

Effect sets use an array of effect pointers to control effects.
The length of the array should be the maximum number of effects and utilities 
you plan to have running at the same time.
In our case, that is 2 -- we have two effects and one utility, 
we plan on running the utility constantly, but only one effect at a time.
Note that we set all the pointers in the array to nullptr, which helps prevent crashes.
*/
EffectBasePS *effArray[2] = { nullptr, nullptr };  //Create an effect array for our effects

//Create the effect set using the effect array above, its length (2), and a run time (10000 ms)
EffectSetPS effectSet(effArray, SIZE(effArray), 10000);

//======================================================================================
//With the Effect Set created we can create our effects (same as the "Cycling Multiple Effects" )

//Create two effects and a utility: myStreamer (a StreamerSL), myTwinkle (a TwinkleSL), and myPB (a PaletteBlenderPS).
PaletteBlenderPS myPB(palette0, palette1, true, 30, 100);
//wiki page: https://github.com/AlbertGBarber/PixelSpork/wiki/Palette-Blender

TwinkleSL myTwinkle(mainSegments, myPB.blendPalette, 0, 1, 6, 6, 70);
//wiki page: https://github.com/AlbertGBarber/PixelSpork/wiki/Twinkle-(Seg-Line)

StreamerSL myStreamer(mainSegments, myPB.blendPalette, 0, 4, 3, 8, 60);
//wiki page: https://github.com/AlbertGBarber/PixelSpork/wiki/Streamer-(Seg-Line)

//Variables we need to track the effect cycling
bool effectSetup = false;  //Flag for if an effect has been configured for updating
uint8_t effectNum = 0;     //Tracks what effect we're on; 0 or 1 for myTwinkle and myStreamer

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

    /*
    Up to this point, our effect set is empty, we need to fill it with our streamer, twinkle, and palette blender.
    we'll be cycling the streamer and twinkle effects in and out of the effect set in out loop() below, so
    we'll add them to the effect set later. 
    However, the we always want to update the palette blender, so we can add it to the effect set now:

    To add `myPB` to the Effect Set, we use the `setEffect` function, 
    which takes a pointer to an effect (hence the "&"), and where in the effect array the new effect should go.
    In this case, we place `myPB` in the first index (0) of our effect array.
    */ 
    effectSet.setEffect(&myPB, 0);
}

/*
In the loop() cycle we're either updating or switching effects.
Overall the cycle looks like:   
    1. Start a loop cycle
    3. Use the switch() to go to the current effect
        3a. If we haven't setup the effect, do so by adding it to our effect set.
        3b. If we have setup the effect, do nothing
    4. Get out of the switch and update() our Effect Set, updating all the effects and utilities in the set
    5. Check if the effect is has finished its run time using the Effect Sets "done" flag.
       5a. If so, reset() the effect set and tell the code we need to setup the next effect.
    5. Loop back to the start for the next loop() cycle.
*/
void loop() {

    //Use a switch statement to switch between what effect is being run
    switch(effectNum) {
        case 0:
        default: {  //effect 0, myStreamer
            //If we are just starting the effect, we need to add it to the effect set, and set a run time
            if(!effectSetup) {
                effectSet.setEffect(&myStreamer, 1);  //add the effect to the effect set
                effectSet.runTime = 10000;            //set the effect to run for 10 sec
                Serial.println("Now running Streamer effect");
            } else {
                //We could add more code here that would run while the Streamer is running
            }
        } break;
        case 1: {  //effect 1, myTwinkle
            //If we are just starting the effect, we need to add it to the effect set, and set a run time
            if(!effectSetup) {
                effectSet.setEffect(&myTwinkle, 1);  //add the effect to the effect set
                effectSet.runTime = 10000;        //set the effect to run for 10 sec
                Serial.println("Now running Twinkle effect");
            } else {
                //We could add more code here that would run while the Twinkle is running
            }
        } break;
    }

    //In the switch statement above, we always setup an effect if needed (see the "default" case is shared with case 0).
    //So we can move the effectSetup flag handling code out of each switch case statement.
    //The code below is run once an effect is setup, so that we flag it not to setup again.
    if(!effectSetup) {
        effectSetup = true;
    }

    //Update all the effects and utilities in the effect set
    effectSet.update();

    /*
    The effect set will set its "done" flag to true once the "runTime" has passed, ie the effect has run for its set runtime
    At this point we want to switch to the next effect, so we increment the effectNum (wrapping back to 0 if needed),
    set the effectSetup flag, so we know to setup the next effect in the switch statement above,
    and reset the effect set, which tells the set to start tracking the time for the next effect. */
    if(effectSet.done) {
        effectNum = (effectNum + 1) % 2;
        effectSetup = false;
        effectSet.reset();
        //Clear the segment set to create a blank state for the next effect
        //Note that we're using a helper function from the segDrawUtils namespace to do this
        //https://github.com/AlbertGBarber/PixelSpork/wiki/Segment-Drawing-Functions
        segDrawUtils::turnSegSetOff(mainSegments); 
    }
}
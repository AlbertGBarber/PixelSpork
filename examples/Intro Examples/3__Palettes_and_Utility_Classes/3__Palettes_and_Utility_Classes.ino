#define FASTLED_INTERNAL 1  //turns off the pragma messages from FastLED
#include <Pixel_Spork.h>

/*
This example shows you how to use Pixel Spork's utility classes with effects
For a full explanation of the code, see
https://github.com/AlbertGBarber/PixelSpork/wiki/Palettes-and-Util-Classes

In the example, we'll create a pair of palettes, a PaletteBlender utility, and a Twinkle effect.
We'll use the palettes and the PaletteBlender to create a palette that constantly blends between colors.
We'll then use the palette for the Twinkle effect's colors.
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
//See the Segment Basics wiki page for more info:
//https://github.com/AlbertGBarber/PixelSpork/wiki/Segment-Basics
//or the "2D Segment Sets for 2D Effects" example

//The code below sets up a segment set that replicates your LED strip as it is connected IRL.
const PROGMEM segmentSecCont mainSec[] = { {0, NUM_LEDS} };
SegmentPS mainSegment = { mainSec, SIZE(mainSec), true };
SegmentPS *main_arr[] = { &mainSegment };
SegmentSetPS mainSegments( leds, NUM_LEDS, main_arr, SIZE(main_arr) );

/*
Next, we'll create a pair of palettes.
As their name implies, palettes are use to store and group colors.
Usually, effects use a palette to set the effect's colors.
Ie, if you use a palette with red, white, and blue, in an effect,
the effect will use red, white, and blue for its colors.
You can read more about palettes at:
https://github.com/AlbertGBarber/PixelSpork/wiki/Palette-Basics

Palettes are created using an array of FastLED CRGB colors, and the array's length.
Note that we are choosing the palette colors at random (you'll see why later)
*/

//Create an array of random CRGB colors
CRGB palette0_arr[] = { colorUtilsPS::randColor(), colorUtilsPS::randColor(), colorUtilsPS::randColor() };
//Create the palette, "palette0" using the color array, and the array's length
//(automatically set using SIZE(), see https://github.com/AlbertGBarber/PixelSpork/wiki/Common-Effect-Variables-and-Functions#common-functions)
palettePS palette0 = { palette0_arr, SIZE(palette0_arr) };

//Create a second palette, "palette1"
CRGB palette1_arr[] = { colorUtilsPS::randColor(), colorUtilsPS::randColor(), colorUtilsPS::randColor() };
palettePS palette1 = { palette1_arr, SIZE(palette1_arr) };

/*
Now well use our palettes to create a PaletteBlender utility.
In Pixel Spork, utility classes behave a lot like effects, but instead of drawing,
they usually produce or change something over time, such as a palette or variable.
Think of them as a companion to effects that you can use to enhance your effects in various ways.
There are a lot of different utilities in Pixel Spork, you can read about them here:
https://github.com/AlbertGBarber/PixelSpork/wiki/Utility-Classes-Basics

The PaletteBlender utility takes a start and an end palette, and transitions smoothly between them over a set time period.
The utility doesn't modify the input palettes, instead, it has its own internal "blendPalette" that holds the
transitional palette. We can then use the blendPalette in our effects.
For more fun, the PaletteBlender can be configured to loop and randomize the start and end palettes,
so we get a constantly shifting set of colors.
*/

PaletteBlenderPS PB(palette0, palette1, true, 30, 100);
/* The PaletteBlender above, "PB", is configured to blend between palette0 and palette1, taking blend 30 steps
with a 100ms step rate. It has been set to loop, so that once the blend is finished, it will restart.
Note that the output palette, PB.blendPalette, will have the same number of colors as the longest input palette
(any mapping between unequal length input palettes is worked out internally).

we also want to configure our PaletteBlender to randomize the palette colors when it loops.
This is not a constructor option, so we'll set it in the Arduino Setup() function below.

wiki page: https://github.com/AlbertGBarber/PixelSpork/wiki/Palette-Blender
*/

TwinkleSL twinkle(mainSegments, PB.blendPalette, 0, 1, 6, 6, 70);
/* The Twinkle effect above is setup using the PaletteBlender's output blendPalette for the colors.
The effect is configured to create 1 twinkle at a time, with 6 fade-in and fade-out steps,
with a blank background.
(1 twinkle is created every update cycle, taking 12 (fade-in + fade-out steps) to decay)
(For an effect with more control, see Twinkle2SLSeg)
The effect updates at 70ms
wiki page: https://github.com/AlbertGBarber/PixelSpork/wiki/Twinkle-(Seg-Line)
*/

//An alternative, Streamer effect you may want to try out!
//----------------------------
//StreamerSL strem(mainSegments, PB.blendPalette, 0, 4, 3, 8, 60);
//-----------------------------
//Setup a Streamer effect using the PaletteBlender's output blendPalette.
//The effect will have streamers of length 4, with 3 blank spaces in between
//It'll use 8 blend steps to shift the color bands, with 60ms between each step
//wiki page: https://github.com/AlbertGBarber/PixelSpork/wiki/Streamer-(Seg-Line)

void setup() {

    //Setup your LED strip using FastLED
    //This setup assumes you're using WS2812 LEDs
    //For other LED type setups, see the FastLED Blink  example
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed

    //Set the PaletteBlender to randomize the palettes when it loops
    //This gives us an every changing set of colors
    PB.randomize = true;

    //Set a strip brightness
    FastLED.setBrightness(40);
}

void loop() {

    //Update both the PaletteBlender and Twinkle
    //The result should be twinkles that change colors over time (but always use 3 colors total)
    PB.update();
    twinkle.update();

    //update for the unused streamer effect (make sure you comment out the twinkle.update())
    //The result should be bands of 3 colors that change over time
    //strem.update();
}

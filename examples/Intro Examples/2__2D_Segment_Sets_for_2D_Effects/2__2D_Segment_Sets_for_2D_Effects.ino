#define FASTLED_INTERNAL 1  //turns off the pragma messages from FastLED
#include <Pixel_Spork.h>

/*
This example lays out how to use Segment Sets to create 2D effects.
Much of the code in this example will be the same as in the Basic Setup example, and won't be re-explained.
For a full explanation of this example see:
https://github.com/AlbertGBarber/PixelSpork/wiki/2D-Segment-Sets-for-2D-Effects

In the example, we'll create two different segment sets by virtually splitting the strip into sections.
We'll re-use the rainbowCycle effect from the "Basic Setup" example, but we'll swap between the two segment sets
periodically. This will change how the effect is drawn on the strip.

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

//Below we'll setup two Segment Sets:

/*
A brief intro to Segment Sets:
Segment Sets allow you to virtually arrange you LEDs into whatever shape you wish. They are a core part of Pixel Spork.
Every effect requires a Segment Set -- they are the canvases on which effects draw.
They also allow you to expand your effects to be 2D by creating 2D segment sets.
Segment Sets are built out of segments, which are lengths of LEDs.
By joining mutiple segments together into one set, we create a sudo 2D matrix, where the segments are the matrix rows.
Most effects are configured to automatically expand into 2D when given a 2D segment set,
usually by copying colors along the sudo matrix columns (so that each column is one color).
Importantly, segments do not all need to be the same length,
the code automatically maps effects onto any shaped segment set, so that its colors are evenly distributed.

You can read more about Segment Sets at
https://github.com/AlbertGBarber/PixelSpork/wiki/Segment-Basics
Note that Segment Sets are class based.
To understand them (and the rest of Pixel Spork)
You'll want to read up on C++ classes and objects.
*/

/*
The first Segment Set below splits the strip into two equal halves. (ie a matrix with two equal rows)
We'll use this segment set to mirror a rainbow cycle effect at the middle of the strip.
Note that for both segment sets below, if your strip has an odd number of LEDs, the final LED will be left out of the segment sets
*/

/*
To split the strip in half, we'll create two segments, each containing half the strip.
Segment sets are made up of sections, which are lengths of pixels.
When multiple sections are formed into a segment, those sections are treated as a continuous line of LEDs.
In this case each of our segments only needs a single section.
Sections are available in two types: continuous and mixed.
We'll use continuous sections for our segments. (See the wiki for info on mixed sections)
Continuous sections are used to specify lengths of unbroken pixels.
They contain an starting pixel and a length.
*/
//The first half of the strip, setup using a continuous section that starts at 0, and is NUM_LEDS/2 long
const PROGMEM segmentSecCont halfSec0[] = { {0, NUM_LEDS / 2} };
SegmentPS halfSeg0 = { halfSec0, SIZE(halfSec0), true };

//The second half of the strip, setup using a continuous section that starts at NUM_LEDS/2 and is NUM_LEDS/2 long
const PROGMEM segmentSecCont halfSec1[] = { {NUM_LEDS / 2, NUM_LEDS / 2} };
//Note that we specify the direction of the segment to be false, this mirrors the segment from halfSeg0 above
SegmentPS halfSeg1 = { halfSec1, SIZE(halfSec1), false };

//Create the segment set using both segments (named halfSegSet)
SegmentPS *halfSegs_arr[] = { &halfSeg0, &halfSeg1 };
SegmentSetPS halfSegsSet( leds, NUM_LEDS, halfSegs_arr, SIZE(halfSegs_arr) );

//=================================================================================
//Let's get a bit more creative, and create a second segment set that splits the strip into 3 segments.
//However, we'll split the strip unevenly, with one segment being half the strip, and the other two being quarters.
//We'll use this to show how effects automatically distribute themselves for segment sets with different segment lengths.

//We can re-use the first half segment from our other segment set,
//so we only have to create the two quarter segments:

//Create the first quarter segment using a continuous section, starting at NUM_LEDS/2, with length NUM_LEDS/4
const PROGMEM segmentSecCont quartSec0[] = { {NUM_LEDS / 2, NUM_LEDS / 4} };
SegmentPS quartSeg0 = {quartSec0, SIZE(quartSec0), true};

//Create the second quarter segment using a continuous section, starting at NUM_LEDS * 3/4, with length NUM_LEDS/4
const PROGMEM segmentSecCont quartSec1[] = { {NUM_LEDS * 3 / 4, NUM_LEDS / 4} };
//Note that we specify the direction of the segment to be false, this mirrors the segment from quartSeg0 above
SegmentPS quartSeg1 = { quartSec1, SIZE(quartSec1), false };

//Create the segment set using the three segments, using halfSeg0 from the first segment set
SegmentPS *quartSegs_arr[] = { &halfSeg0, &quartSeg0, &quartSeg1 };
SegmentSetPS quartSegsSet( leds, NUM_LEDS, quartSegs_arr, SIZE(quartSegs_arr) );

//================================================================================

RainbowCycleSLSeg rainbowCycle(halfSegsSet, NUM_LEDS / 2, true, false, 80);
/*
Setup our Rainbow Cycle (SLSeg) effect, named "rainbowCycle" to draw on our mainSegments Segment Set.
(Note that SLSeg means the effect is configured for 2D use, you can read more about that at
https://github.com/AlbertGBarber/PixelSpork/wiki/Effect-Basics#effect-naming-conventions)
This effect cycles a rainbow across the Segment Set.
we'll set the length of the rainbow to be NUM_LEDS/2 (our maximum segment length),
so that a whole rainbow is shown across our segment sets.
Effect wiki page: https://github.com/AlbertGBarber/PixelSpork/wiki/Rainbow-Cycle-(Seg-Line-or-Seg)
*/

//We'll use this to track what segment set we're using
bool flipFlop = true;

void setup() {
    Serial.begin(9600);
    //Setup your LED strip using FastLED
    //This setup assumes you're using WS2812 LEDs
    //For other LED type setups, see the FastLED Blink  example
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed

    //Set a strip brightness
    FastLED.setBrightness(40);
}

/*
The code below will run our rainbowCycle effect.
Every 10 sec we'll change what segment set we're using, flipping between halfSegsSet and quartSegsSet.
This will change how the effect is drawn:
For halfSegsSet:
  The rainbow will appear mirrored at the center of the strip.
For quartSegsSet:
  The will be three rainbows one taking up the first half of the strip, and the other two taking up the last quarters.
  The quarter rainbows should look like a mini version of halfSegsSet,
  with mirrored rainbows meeting at the center of the two segments.
  Because they are all in the same segment set, the rainbows should all be in sync with one another (ending/starting at the same time)
  despite the segments being different lengths.
*/
void loop() {

    //every 10 sec, switch the rainbowCycle's segment set between quartSegsSet and halfSegsSet.
    //(note that the rainbowCycle effect allows us to change the segment set freely,
    //but for some effects, you may need reset() them when doing so. The effect's wiki page will explain)
    EVERY_N_MILLISECONDS(10000) {
        if( flipFlop ) {
            //Change the effect's segment set, note the use of &, this is because the effect uses a pointer to the segment set.
            rainbowCycle.segSet = &quartSegsSet;
            Serial.println("Using quartSegsSet now");
        } else {
            //Change the effect's segment set, note the use of &, this is because the effect uses a pointer to the segment set.
            rainbowCycle.segSet = &halfSegsSet;
            Serial.println("Using halfSegsSet now");
        }
        flipFlop = !flipFlop;
    }

    //update the rainbow cycle to draw it
    rainbowCycle.update();
}

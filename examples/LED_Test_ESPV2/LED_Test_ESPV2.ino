//#define NO_EFADE_PS
#define FASTLED_INTERNAL 1
#include <PixelStrip2.h>

#define NUM_LEDs 61
#define DATA_PIN D8

// Define the array of leds
CRGB leds[NUM_LEDs];

const CRGB white =    CRGB(255, 255, 255);
const CRGB UCLAGold = CRGB(254, 187, 54);
const CRGB UCLABlue = CRGB(83, 104, 149);
const CRGB red =      CRGB(255, 0, 0);
const CRGB orange =   CRGB(255, 43, 0);
const CRGB ltOrange = CRGB(255, 143, 0);
const CRGB yellow =   CRGB(255, 255, 0);
const CRGB ltYellow = CRGB(255, 255, 100);
const CRGB green =    CRGB(0, 128, 0);
const CRGB blue =     CRGB(0, 0, 255);
const CRGB indigo =   CRGB( 75, 0, 130);
const CRGB violet =   CRGB(238, 130, 238);
const CRGB purple =   CRGB(123, 7, 197);
const CRGB pink =     CRGB(225, 0, 127);

CRGB pallet1_arr[5] = { red, blue, purple, green, yellow };
CRGB pallet2_arr[2] = { blue, green };

palletPS pallet1 = {pallet1_arr, SIZE(pallet1_arr)};

palletPS pallet2 = {pallet2_arr, SIZE(pallet2_arr)};

RainbowOffsetPS rainbowOffset(0, 40, true, false);

segmentSection mainSec[] = {{0, 61}}; //outer ring 0, 70 pixels
Segment mainSegment = { SIZE(mainSec), mainSec, true }; //numSections, section array pointer
Segment *main_arr[] = { &mainSegment };
SegmentSet mainSegments(leds, NUM_LEDs, main_arr, SIZE(main_arr), rainbowOffset );

CRGB color1 = segDrawUtils::randColor();
CRGB color2 = segDrawUtils::randColor();

segmentSection ringSec0[] = {{0, 24}}; //outer ring 1, 24 pixels
Segment ringSegment0 = { SIZE(ringSec0), ringSec0, true }; //numSections, section array pointer

segmentSection ringSec1[] = {{24, 16}}; //outer ring 2, 16 pixels
Segment ringSegment1 = { SIZE(ringSec1), ringSec1, true }; //numSections, section array pointer

segmentSection ringSec2[] = {{40, 12}}; //outer ring 3, 12 pixels
Segment ringSegment2 = { SIZE(ringSec2), ringSec2, true }; //numSections, section array pointer

segmentSection ringSec3[] = {{52, 8}}; //outer ring 4, 8 pixels
Segment ringSegment3 = { SIZE(ringSec3), ringSec3, true }; //numSections, section array pointer

segmentSection ringSec4[] = {{60, 1}}; //outer ring 5, 1 pixel
Segment ringSegment4 = { SIZE(ringSec4), ringSec4, true }; //numSections, section array pointer

Segment *rings_arr[] = { &ringSegment0 , &ringSegment1, &ringSegment2, &ringSegment3, &ringSegment4 };
SegmentSet ringSegments = {leds, NUM_LEDs, rings_arr, SIZE(rings_arr), rainbowOffset };

//palletBlenderPS PB(pallet1, pallet2, 100, 80);

//FixedLengthRainbowCyclePS FLRC(ringSegments, 15, true, 80);

//FixedLengthRainbowCyclePS FLRC2(ringSegments, 15, false, 80);

EffectBasePS *effArray[1];

EffectGroupPS effectGroup(effArray, SIZE(effArray), 10000, 5000);

//RateCtrlPS rateCtrl(90, 20, false, true, 100);

//EffectFaderPS effectFade(effArray, SIZE(effArray), true, 5000);

int normalBrightness = 20;
uint8_t count = 0;
bool def = false;
void setup() {
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDs);
  FastLED.setBrightness(normalBrightness);
  //Serial.begin(9600);
  //randomSeed(ESP.getCycleCount());
  //PB.looped = true;
  //PB.randomize = false;
  //FLRC.setGlobalRate(rateCont.rate);
  //rateCtrl.changeHold = 3000;
  //effArray[0] = &FLRC;
}

void loop() {
  yield();

  switch (count) {
    case 0: {
        if (!def) {
          FixedLengthRainbowCyclePS *FLRC = new FixedLengthRainbowCyclePS(ringSegments, 15, true, 80);
          effArray[0] = FLRC;
          //def = true;
         // effectFade.reset();
        }
      }
      break;
    case 1: {
        if (!def) {
          FixedLengthRainbowCyclePS *FLRC2 = new FixedLengthRainbowCyclePS(ringSegments, 61, false, 80);
          effArray[0] = FLRC2;
          //def = true;
          //effectFade.reset();
        }
      }
      break;
  }

  if(!def){
        def = true;
        effectGroup.reset();
  }
//  EVERY_N_MILLISECONDS( 7000 ) {
//    count = (count + 1) % 2;
//    def = false;
//  }
  //rateCtrl.update();
  //PB.update();
  //effectFade.update();
  effectGroup.update();
  
  if(effectGroup.done){
    count = ( count + 1) % 2;
    def = false;
    //effectFade.reset();
  }
  //FLRC.update();
  // Turn the LED on, then pause
  //segDrawUtils::setPixelColor(mainSegments, 0, CRGB::Green, 4, 0);
  //segDrawUtils::fillSegSetColor(ringSegments, CRGB::Green, 1);

  //segDrawUtils::fillSegSetColor(ringSegments, CRGB::Green, 1);
  //segDrawUtils::setPixelColor(mainSegments, 0, 0, CRGB::Green, 0);

  //  for(int i = 0; i < NUM_LEDs; i++){
  //    //fract8 ratio;
  //    //ratio = i /NUM_LEDs;
  //    leds[i] = segDrawUtils::getCrossFadeColor(color1, color2, i, NUM_LEDs);
  //  }
  //   PB.update();
  //    for(int i = 0; i < ringSegments.numSegs; i++){
  //      segDrawUtils::fillSegColor(ringSegments, i, palletUtils::getPalletColor(PB.blendPallet, i), 0);
  //    }
  //FastLED.show();
  //delay(1000);

  //palletUtils::randomize(pallet2);
  //  segDrawUtils::fillSegSetlengthColor(ringSegments, CRGB::Green, 0, 3, 15);
  //  FastLED.show();
  //  delay(1000);
  //  segDrawUtils::fillSegSetlengthColor(ringSegments, CRGB::Red, 0, 10, 32);
  //  FastLED.show();
  //  delay(1000);
  //  segDrawUtils::fillSegSetlengthColor(ringSegments, CRGB::Blue, 0, 30, 57);
  //  FastLED.show();
  //  delay(1000);
  // Now turn the LED off, then pause
  //segDrawUtils::fillSegSetColor(mainSegments, CRGB::Black, 0);
  //FastLED.show();
  //delay(1000);
}

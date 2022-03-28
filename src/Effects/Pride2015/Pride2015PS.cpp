#include "Pride2015PS.h"

Pride2015PS::Pride2015PS(SegmentSet& SegmentSet): 
    segmentSet(SegmentSet) 
    {
        // bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
        uint16_t Rate = 1; //rate is hard coded to satisfy the BaseEffect, but isn't used in the update function
        bindSegPtrPS();
        bindClassRatesPS();
    }

void Pride2015PS::update() {
    currentTime = millis();
    deltams = currentTime - prevTime;
    prevTime = currentTime;

    sat8 = beatsin88(87, 220, 250);
    brightdepth = beatsin88(341, 96, 224);
    brightnessthetainc16 = beatsin88(203, (25 * 256), (40 * 256));
    msmultiplier = beatsin88(147, 23, 60);

    hue16 = sHue16; // gHue * 256;
    hueinc16 = beatsin88(113, 1, 3000);

    sPseudotime += deltams * msmultiplier;
    sHue16 += deltams * beatsin88(400, 5, 9);
    brightnesstheta16 = sPseudotime;

    numActiveLeds = segmentSet.numActiveSegLeds;

    for (uint16_t i = 0; i < numActiveLeds; i++) {
        hue16 += hueinc16;
        hue8 = hue16 / 256;

        brightnesstheta16 += brightnessthetainc16;
        b16 = sin16(brightnesstheta16) + 32768;

        bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
        bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
        bri8 += (255 - brightdepth);

        newColor = CHSV(hue8, sat8, bri8);

        pixelnumber = segDrawUtils::getSegmentPixel(segmentSet, (numActiveLeds - 1) - i);

        nblend(segmentSet.leds[pixelnumber], newColor, 64);
    }
    showCheckPS();
}
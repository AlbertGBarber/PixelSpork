#include "colorUtilsPS.h"

using namespace colorUtilsPS;

//Returns a random color using a random hue and a limited random saturation
//Seems to produce a good set of random colors, tending towards more pastel type colors
//We don't randomize the value because doing so mostly just dims the output colors without significantly changing them
CRGB colorUtilsPS::randColor() {
    return CHSV(random8(), random8(100, 255), 255);
    //return CRGB( random8(), random8(), random8() );
}

//Returns a complimentary color to the passed in base hue
//numColors is the type of split, ie complementary is 2, triad is 3, tetrad is 4, etc (can't do split complementary)
//num is the number of the color in the split you want, ie for a triad you'd ask for colors num 0, 1, 2
//satur and value are the saturation and value for the color wheel
//NOTE: you can't extract a hue from a CRGB color, you need to be using CHSV
CRGB colorUtilsPS::getCompColor(uint8_t baseHue, uint8_t numColors, uint8_t num, uint8_t satur, uint8_t value) {
    //To get a complementary color we get the percent of the color wheel we need to move by
    //ie (255 / numColors * num) where 255 is the color wheel's total length
    //Then we add this to the baseHue to shift the color to the correct hue
    return CRGB(CHSV(baseHue + 255 * num / numColors, satur, value));
}

//Input a value 0 to 255 to get a rainbow color value.
//The colours are a transition r - g - b - back to r.
//satur and value set saturation and value of the FastLed CHSV function
//Note that the input is a uint16_t for flexibility, but values over 255 will be modded back into range
CRGB colorUtilsPS::wheel(uint16_t wheelPos, uint16_t offset, uint8_t satur, uint8_t value) {
    wheelPos = 255 - addMod16PS(wheelPos, offset, 256);
    //wheelPos = (uint16_t)(255 - wheelPos + offset) % 256;
    return CHSV(wheelPos, satur, value);
}

//same as wheel, but the sat and value vars are set to 255
CRGB colorUtilsPS::wheel(uint16_t wheelPos, uint16_t rainbowOffset) {
    return wheel(wheelPos, rainbowOffset, 255, 255);
}

//returns a color that is dimmed by the ratio
//the ratio is between 0 and 255, 255 being the total black
CRGB colorUtilsPS::dimColor(const CRGB &startColor, uint8_t ratio) {
    return getCrossFadeColor(startColor, 0, ratio);
}

//returns a color that is blended/cross-faded between a start and end color according to the ratio of step/totalSteps
//maximum value of totalSteps is 255 (since the color components are 0-255 uint8_t's)
CRGB colorUtilsPS::getCrossFadeColor(const CRGB &startColor, const CRGB &endColor, uint8_t blendStep, uint8_t totalSteps) {
    ratio = (uint16_t)blendStep * 255 / totalSteps;
    return getCrossFadeColor(startColor, endColor, ratio);
}

//returns a color that is blended/cross-faded between a start and end color according to the ratio
//the ratio is between 0 and 255, 255 being the total conversion to the end color
CRGB colorUtilsPS::getCrossFadeColor(const CRGB &startColor, const CRGB &endColor, uint8_t ratio) {
    //am using the built in fastLed function for blending
    //the commented code after is an alternative way to do it, but yields the same result in most cases
    return blend(startColor, endColor, ratio);

    /*
    if( ratio == 0 ) {
        return startColor;
    } else if( ratio == 255 ) {
        return endColor;
    } else {
       colorFinal.r = startColor.r + ( (int16_t)( endColor.r - startColor.r ) * ratio) / 255;
       colorFinal.g = startColor.g + ( (int16_t)( endColor.g - startColor.g ) * ratio) / 255;
       colorFinal.g = startColor.b + ( (int16_t)( endColor.b - startColor.b ) * ratio) / 255;
       return colorFinal;
    }
    */
}

//returns an interpolated value between two 8 bit uint's according to the ratio
//the ratio is between 0 and 255, 255 being the total conversion to the end int
//useful for blending of individual RGB color components
uint8_t colorUtilsPS::getCrossFadeColorComp(uint8_t startColor, uint8_t endColor, uint8_t ratio) {
    if( startColor == endColor ) return startColor;

    if( startColor > endColor ) {
        return startColor - (int16_t)(startColor - endColor) * ratio / 255;
    } else {
        return startColor + (int16_t)(endColor - startColor) * ratio / 255;
    }
}

#include "colorUtilsPS.h"


using namespace colorUtilsPS;

//returns a random color
CRGB colorUtilsPS::randColor(){
    return CHSV( random8(), 255, 255 );
    //return CRGB( random(256), random(256), random(256) );
    //return CRGB( random8(), random8(), random8() );
}

//Input a value 0 to 255 to get a color value.
//The colours are a transition r - g - b - back to r.
//satur and value set saturation and value of the FastLed CHSV function
//Note that the input is a uint16_t for flexbility, but values over 255 will be modded back into range
CRGB colorUtilsPS::wheel( uint16_t wheelPos, uint16_t offset, uint8_t satur, uint8_t value ){
    wheelPos = 255 - addMod16PS(wheelPos, offset, 255);
    //wheelPos = (uint16_t)(255 - wheelPos + offset) % 255;
    hsv2rgb_rainbow( CHSV(wheelPos, satur, value), colorFinal );
    return colorFinal;
}

//same as wheel, but the sat and value vars are set to 255
CRGB colorUtilsPS::wheel( uint16_t wheelPos, uint16_t rainbowOffset) { 
    return wheel( wheelPos, rainbowOffset, 255, 255);
}

//returns a color that is blended/cross-faded between a start and end color according to the ratio of step/totalSteps
//maximum value of totalSteps is 255 (since the color components are 0-255 uint8_t's)
CRGB colorUtilsPS::getCrossFadeColor(CRGB startColor, CRGB endColor, uint8_t blendStep, uint8_t totalSteps){
    ratio = (uint16_t)blendStep * 255 / totalSteps;
    return getCrossFadeColor(startColor, endColor, ratio);
}

//returns a color that is blended/cross-faded between a start and end color according to the ratio
//the ratio is between 0 and 255, 255 being the total conversion to the end color
CRGB colorUtilsPS::getCrossFadeColor(CRGB startColor, CRGB endColor, uint8_t ratio){
    //am using the built in fastLed function for blending
    //the commented code after is an alternative way to do it, but yeilds the same result in most cases
    return blend( startColor, endColor, ratio );

    // CRGB result;
    // result.r = startColor.r + ( (int16_t)( endColor.r - startColor.r ) * ratio) ) / 255;
    // result.g = startColor.g + ( (int16_t)( endColor.g - startColor.g ) * ratio) ) / 255;
    // result.b = startColor.b + ( (int16_t)( endColor.b - startColor.b ) * ratio) ) / 255;

    // return CRGB(result.r, result.g, result.b);
}

//returns an interpolated value between two 8 bit uint's according to the ratio
//the ratio is between 0 and 255, 255 being the total conversion to the end int
//useful for blending of individual RGB color components
uint8_t colorUtilsPS::getCrossFadeColorComp(uint8_t startColor, uint8_t endColor, uint8_t ratio){
    if(startColor == endColor) return startColor;

    if(startColor > endColor){
        return startColor - (int16_t)(startColor - endColor) * ratio / 255;
    } else{
        return startColor + (int16_t)(endColor - startColor) * ratio / 255;
    }
}

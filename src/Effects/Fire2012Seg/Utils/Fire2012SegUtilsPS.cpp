#include "Fire2012SegUtilsPS.h"

using namespace Fire2012SegUtilsPS;

//Returns a heat output color using a palette based in the input temperature
//The colors are blended between palette colors for smoothness
//To work out what color matches the temperature we scale it to match the palette length
//so that it gets paired with a palette index (ie a color)
//To blend, we divide the palette into temperature sections ( see paletteSecLen in update() )
//to get the minimum temperature of each color index
//We then subtract this from the temperature to work out how far the temperature is in the section
//We then blend between towards the next palette index based using this number
//so we blend further to the next color with higher temps
//this produces a more nuanced fire
//Palettes scale from coldest to hottest, so the last color in a palette will match the highest temperature
//We do additional adjustments to include the background color, which is for the coldest temperature (0th palette index)
//(The background is usually blank, which palettes don't usually include, so 
//seperating it as a seperate color makes it easier to use pre-made palettes)
//Since blending takes a bit of processing power
//it can be turned off using the blend flag to speed up the effect
//this creates a more blocky fire
CRGB Fire2012SegUtilsPS::getPixelHeatColorPalette(palettePS *palette, uint8_t paletteLength, uint8_t paletteSecLen, CRGB *bgColor, uint8_t temperature,  bool blend) {

    //scale the temperature to match it to a palette index
    colorIndex = scale8(temperature, paletteLength);

    //for blending
    //work out the minimum temperature of the section of the palette we're in
    secHeatLimit = colorIndex * paletteSecLen;

    //if the we're in the background index, we need to set a set of flags
    //the background is always the coldest color, ie the 0th index zone
    //since the background color is not in the palette and needs to be handled seperately
    if(colorIndex == 0){
        doBg = true;
    } else {
        doBg = false;
        //we do colorIndex-- to adjust it to match the palette color index
        colorIndex--; 
    }
    
    //if we're not blending, or we're at the last color in the palette
    //There's nothing to blend towards, so just set the color
    if(!blend || colorIndex == (paletteLength - 1) ){
        if(doBg){
            //use the background color if we need to set the background
            targetColor = *bgColor;
        } else {
            targetColor = paletteUtilsPS::getPaletteColor( *palette, colorIndex );
        }
    } else {
        //if we are blending, we need to get the starting and ending colors
        if(doBg){
            //for the background, we're blending from the background to the first color in the palette
            startColor = *bgColor;
            targetColor = paletteUtilsPS::getPaletteColor( *palette, 0 );
        } else {
            //in general we're blending from the current color to the next in the palette
            startColor = paletteUtilsPS::getPaletteColor( *palette, colorIndex );
            targetColor = paletteUtilsPS::getPaletteColor( *palette, colorIndex + 1 );
        }
        //get the blended color
        //The blend amount is the temperature - secHeatLimit
        //ie the difference between the minimum temperature of the palette section, and the current temperature
        //so we blend further to the next color with higher temps
        targetColor = colorUtilsPS::getCrossFadeColor(startColor, targetColor, temperature - secHeatLimit, paletteSecLen);
    }

    return targetColor;
}


/* 
//Original implementation of the setPixelHeatColorPalette()
//Works exactly the same, but the scaling is done manually
void Fire2012SegUtilsPS::setPixelHeatColorPaletteOrig(uint16_t pixelLoc, uint8_t temperature) {

    // determine which color the heat belongs to and blend it
    for (uint8_t i = 0; i <= paletteLength; i++) {
        // if we find the right section, we don't need to loop any more, so we break
        secHeatLimit = i * paletteSecLen;
        if (temperature <= ( secHeatLimit + paletteSecLen ) ) {

            if(i == 0){
                doBg = true;
                colorIndex = i;
            } else {
                doBg = false;
                colorIndex = i - 1; 
            }

            if(blend && (colorIndex != paletteLength - 1) ){
                // if blend is true we'll try to blend between colors to get a cleaner fire
                // this uses more processing power, so turn it off if the animation is too slow (or you're having color bugs)
                // if we're at the end last color, we don't need to blend, prevent us from running off the palette end
                if(doBg){
                    targetColor = paletteUtilsPS::getPaletteColor( palette, 0 );
                    startColor = *bgColor;
                } else{
                    targetColor = paletteUtilsPS::getPaletteColor( palette, colorIndex + 1 );
                    startColor = paletteUtilsPS::getPaletteColor( palette, colorIndex );
                }
                // blend between the current color an the next based on the what temperature the pixel is at
                colorOut = segDrawUtils::getCrossFadeColor(startColor, targetColor, temperature - secHeatLimit, paletteSecLen);
            } else {
                if(doBg){
                    colorOut = *bgColor;
                } else {
                    colorOut = paletteUtilsPS::getPaletteColor( palette, colorIndex );
                }
            }
            break;
        }
    }
    segDrawUtils::setPixelColor(segmentSet, pixelLoc, colorOut, 0, 0, 0);
} */
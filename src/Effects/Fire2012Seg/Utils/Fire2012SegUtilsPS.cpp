#include "Fire2012SegUtilsPS.h"

using namespace Fire2012SegUtilsPS;

//Returns a heat output color using a pallet based in the input temperature
//The colors are blended between pallet colors for smoothness
//To work out what color matches the temperature we scale it to match the pallet length
//so that it gets paired with a pallet index (ie a color)
//To blend, we divide the pallet into temperature sections ( see palletSecLen in update() )
//to get the minimum temperature of each color index
//We then subtract this from the temperature to work out how far the temperature is in the section
//We then blend between towards the next pallet index based using this number
//so we blend further to the next color with higher temps
//this produces a more nuanced fire
//Pallets scale from coldest to hottest, so the last color in a pallet will match the highest temperature
//We do additional adjustments to include the background color, which is for the coldest temperature (0th pallet index)
//(The background is usually blank, which pallets don't usually include, so 
//seperating it as a seperate color makes it easier to use pre-made pallets)
//Since blending takes a bit of processing power
//it can be turned off using the blend flag to speed up the effect
//this creates a more blocky fire
CRGB Fire2012SegUtilsPS::getPixelHeatColorPallet(palletPS *pallet, uint8_t palletLength, uint8_t palletSecLen, CRGB *bgColor, uint8_t temperature,  bool blend) {

    //scale the temperature to match it to a pallet index
    colorIndex = scale8(temperature, palletLength);

    //for blending
    //work out the minimum temperature of the section of the pallet we're in
    secHeatLimit = colorIndex * palletSecLen;

    //if the we're in the background index, we need to set a set of flags
    //the background is always the coldest color, ie the 0th index zone
    //since the background color is not in the pallet and needs to be handled seperately
    if(colorIndex == 0){
        doBg = true;
    } else {
        doBg = false;
        //we do colorIndex-- to adjust it to match the pallet color index
        colorIndex--; 
    }
    
    //if we're not blending, or we're at the last color in the pallet
    //There's nothing to blend towards, so just set the color
    if(!blend || colorIndex == (palletLength - 1) ){
        if(doBg){
            //use the background color if we need to set the background
            targetColor = *bgColor;
        } else {
            targetColor = palletUtilsPS::getPalletColor( pallet, colorIndex );
        }
    } else {
        //if we are blending, we need to get the starting and ending colors
        if(doBg){
            //for the background, we're blending from the background to the first color in the pallet
            startColor = *bgColor;
            targetColor = palletUtilsPS::getPalletColor( pallet, 0 );
        } else {
            //in general we're blending from the current color to the next in the pallet
            startColor = palletUtilsPS::getPalletColor( pallet, colorIndex );
            targetColor = palletUtilsPS::getPalletColor( pallet, colorIndex + 1 );
        }
        //get the blended color
        //The blend amount is the temperature - secHeatLimit
        //ie the difference between the minimum temperature of the pallet section, and the current temperature
        //so we blend further to the next color with higher temps
        targetColor = colorUtilsPS::getCrossFadeColor(startColor, targetColor, temperature - secHeatLimit, palletSecLen);
    }

    return targetColor;
}


/* 
//Original implementation of the setPixelHeatColorPallet()
//Works exactly the same, but the scaling is done manually
void Fire2012SegUtilsPS::setPixelHeatColorPalletOrig(uint16_t pixelLoc, uint8_t temperature) {

    // determine which color the heat belongs to and blend it
    for (uint8_t i = 0; i <= palletLength; i++) {
        // if we find the right section, we don't need to loop any more, so we break
        secHeatLimit = i * palletSecLen;
        if (temperature <= ( secHeatLimit + palletSecLen ) ) {

            if(i == 0){
                doBg = true;
                colorIndex = i;
            } else {
                doBg = false;
                colorIndex = i - 1; 
            }

            if(blend && (colorIndex != palletLength - 1) ){
                // if blend is true we'll try to blend between colors to get a cleaner fire
                // this uses more processing power, so turn it off if the animation is too slow (or you're having color bugs)
                // if we're at the end last color, we don't need to blend, prevent us from running off the pallet end
                if(doBg){
                    targetColor = palletUtilsPS::getPalletColor( pallet, 0 );
                    startColor = *bgColor;
                } else{
                    targetColor = palletUtilsPS::getPalletColor( pallet, colorIndex + 1 );
                    startColor = palletUtilsPS::getPalletColor( pallet, colorIndex );
                }
                // blend between the current color an the next based on the what temperature the pixel is at
                colorOut = segDrawUtils::getCrossFadeColor(startColor, targetColor, temperature - secHeatLimit, palletSecLen);
            } else {
                if(doBg){
                    colorOut = *bgColor;
                } else {
                    colorOut = palletUtilsPS::getPalletColor( pallet, colorIndex );
                }
            }
            break;
        }
    }
    segDrawUtils::setPixelColor(segmentSet, pixelLoc, colorOut, 0, 0, 0);
} */
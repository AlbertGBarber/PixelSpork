#include "Fire2012PS.h"

Fire2012PS::Fire2012PS(SegmentSet &SegmentSet, palletPS *Pallet, CRGB BgColor, uint8_t Cooling, uint8_t Sparking, bool Blend, uint16_t Rate):
    segmentSet(SegmentSet), pallet(Pallet), cooling(Cooling), sparking(Sparking), blend(Blend)
    {    
        //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
        bindSegPtrPS();
        bindClassRatesPS();
        //bind background color pointer (if needed)
        bindBGColorPS();
        reset();
	}

Fire2012PS::~Fire2012PS(){
    delete[] heat;
    delete[] heatSegStarts;
}

//resets the effect and creates new heat arrays
//call this if you change segment sets or sections
void Fire2012PS::reset(){
    delete[] heat;
    delete[] heatSegStarts;

    numSegs = segmentSet.numSegs;
    numLeds = segmentSet.numLeds;
    
    //create the heat array to store temperatures
    heat = new uint8_t[numLeds];
    for(uint16_t i = 0; i < numLeds; i++){
        heat[i] = 0;
    }

    //The heat array works by storing heat values at points on the strip
    //For speed, the heat values for the whole segmentSet
    //are stored in one array, using offsets to ensure we only work on the section that
    //corrosponds to the current segment
    //the offsets are stored in the heatSegStarts in order of the segments
    uint16_t segmentSetLength = 0;
    heatSegStarts = new uint16_t[numSegs];
    // get the total segmentSet length, and set the starting offsets
    for (uint8_t i = 0; i < numSegs; i++) {
        heatSegStarts[i] = segmentSetLength; // the start offsets are just the lengths of each segment
        segmentSetLength += segmentSet.getTotalSegLength(i);
    }
}

//Updates the effect
//This code has been adapted from https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/#fire
//to work with segments and pallets
//Overall the effect manages a heat array of uint8_t's, with each pixel in the segment set being having it's own 
//heat (temperature) value
//These heat values are heated and cooled with each update cycle, producing the flame effect
//Each segment has it's own seperate fire simulation
//We use a single array heat to manage multiple segments by splitting it into sections of lengths equal to the segment lengths
//We store the start indexes of the segments in the heatSegStarts array (created in reset())
//Within a fire colors taken from a pallet and blended together to create a smooth flame (see setPixelHeatColorPallet() for info)
void Fire2012PS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;
        
        //work out some pallet vars to be used in setPixelHeatColorPallet
        //setting them here is more efficent since they only need to be set once per update cycle
        //(we check them each update incase the pallet has changed)
        palletLength = pallet->length;
        palletLimit = palletLength - 1;
        //For color blending, we need to divide the temperature range (0 - 255) into sections of the pallet
        //we divide by palletLength + 1 because we need to include a section for the background
        palletSecLen = 255 / (palletLength + 1); 

        //For each segment do the following:
        for (uint8_t j = 0; j < numSegs; j++) {
            //get the length of the segment we're workging on
            segLength = segmentSet.getTotalSegLength(j);

            heatSecStart = heatSegStarts[j]; // current segment's start index in the heat array

            //Step 1. Cool down every cell a little
            for (uint16_t i = 0; i < segLength; i++) {

                heatIndex = i + heatSecStart; // adjusted index for heat array
                //subtract a random cooling factor from the current heat (qsub8 prevents running past 0)
                heat[heatIndex] = qsub8( heat[heatIndex], random8(0, ((cooling * 10) / segLength) + 2) );
                //cooldown = random(0, ((cooling * 10) / segLength) + 2) ;

                // if cooldown would turn off a pixel, turn if off,
                // otherwise just cool it down
                //if (cooldown > heat[heatIndex]) {
                    //heat[heatIndex] = 0;
                //} else {
                    //heat[heatIndex] = heat[heatIndex] - cooldown;
                //}
            }

            //Step 2. Heat from each cell drifts 'up' and diffuses a little
            for (uint16_t k = (segLength - 1); k >= 2; k--) {
                heatIndex = k + heatSecStart; // adjusted index for heat array
                heat[heatIndex] = (heat[heatIndex - 1] + heat[heatIndex - 2] + heat[heatIndex - 2]) / 3;
            }

            //Step 3. Randomly ignite new 'sparks' near the bottom
            if (random8(255) < sparking) {
                // pick a random pixel near the start of the strip
                //default is 7, but we'll lower this for shorter segments
                sparkPoint = 7;
                if(segLength < sparkPoint){
                    sparkPoint = 2;
                }
                heatIndex = random8(sparkPoint) + heatSecStart; // adjusted index for heat array
                // add a random bit of heat (qadd8 keeps within 255)
                heat[heatIndex] = qadd8( heat[heatIndex], random8(160, 255) );
                //heat[heatIndex] = heat[heatIndex] + random8(160, 255);
            }
            
            //Step 4. For each flame, convert heat to pallet colors colors and output
            for (uint16_t i = 0; i < segLength; i++) {
                ledLoc = segDrawUtils::getSegmentPixel(segmentSet, j, i); //the physical location of the led
                setPixelHeatColorPallet(ledLoc, heat[i + heatSecStart]);       
            }
        }
        showCheckPS();
    }
}


//Sets heat output color using a pallet based in the input temperature
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
void Fire2012PS::setPixelHeatColorPallet(uint16_t pixelLoc, uint8_t temperature) {

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
    if(!blend || colorIndex == palletLimit){ //palletLimit is palletLength - 1
        if(doBg){
            //use the background color if we need to set the background
            colorOut = *bgColor;
        } else {
            colorOut = palletUtilsPS::getPalletColor( pallet, colorIndex );
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
        colorOut = segDrawUtils::getCrossFadeColor(startColor, targetColor, temperature - secHeatLimit, palletSecLen);
    }
    segDrawUtils::setPixelColor(segmentSet, pixelLoc, colorOut, 0, 0, 0);
}


/* 
//Original implementation of the setPixelHeatColorPallet()
//Works exactly the same, but the scaling is done manually
void Fire2012PS::setPixelHeatColorPalletOrig(uint16_t pixelLoc, uint8_t temperature) {

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
#include "FairyLightsPS.h"

//see update() for how the effect works

//pallet based constructor
FairyLightsPS::FairyLightsPS(SegmentSet &SegmentSet, palletPS *Pallet, uint8_t NumTwinkles, CRGB BGColor, uint8_t Tmode, uint16_t Rate):
    segmentSet(SegmentSet), pallet(Pallet), numTwinkles(NumTwinkles), tmode(Tmode)
    {    
        init(BGColor, Rate);
	}

//single color constructor
FairyLightsPS::FairyLightsPS(SegmentSet &SegmentSet, CRGB Color, uint8_t NumTwinkles, CRGB BGColor, uint8_t Tmode, uint16_t Rate):
    segmentSet(SegmentSet), numTwinkles(NumTwinkles), tmode(Tmode)
    {    
        init(BGColor, Rate);
        setSingleColor(Color);
	}

//random colors constructor
FairyLightsPS::FairyLightsPS(SegmentSet &SegmentSet, uint8_t NumTwinkles, CRGB BGColor, uint8_t Tmode, uint16_t Rate):
    segmentSet(SegmentSet), numTwinkles(NumTwinkles), tmode(Tmode)
    {    
        init(BGColor, Rate);
        //we make a random pallet of one color so that 
        //if we switch to randMode 0 then we have a pallet to use
        setSingleColor(colorUtilsPS::randColor()); 
        //since we're choosing colors at random, set the randMode
        randMode = 1;
	}

//destructor
FairyLightsPS::~FairyLightsPS(){
    delete[] palletTemp.palletArr;
    delete[] pixelSet;
    delete[] colorSet;
}

void FairyLightsPS::init(CRGB BgColor, uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    bindBGColorPS();
    genPixelSet();
}

//makes new pixelSet and colorSet arrays to store the pixel locations and colors
void FairyLightsPS::genPixelSet(){
    if(numTwinkles < 1){
        numTwinkles = 1;
    }
    uint16_t numLEDS = segmentSet.numActiveSegLeds;
    delete[] pixelSet;
    pixelSet = new uint16_t[numTwinkles];
    cycleLimit = numTwinkles - 1; //the number of cycles to go through the whole array
    //pick locations for the pixels (local to the segment set)
    for (uint8_t i = 0; i < numTwinkles; i++) {
        pixelSet[i] = random16(numLEDS);
    }

    delete[] colorSet;
    colorSet = new CRGB[numTwinkles];
}

//changes the number of twinkles, also resets the pixelSet
void FairyLightsPS::setNumTwinkles(uint8_t newNumTwinkles){
    numTwinkles = newNumTwinkles;
    genPixelSet();
}

//creates an pallet of length 1 containing the passed in color
//binds it to the temp pallet to keep it in scope
void FairyLightsPS::setSingleColor(CRGB Color){
    delete[] palletTemp.palletArr;
    palletTemp = palletUtilsPS::makeSingleColorPallet(Color);
    pallet = &palletTemp;
}

//binds the pallet to a new one
void FairyLightsPS::setPallet(palletPS *newPallet){
    pallet = newPallet;
}

//Updates the effect
//The pixel locations (local to segment set) and colors are stored in the pixelSet and colorSet arrays (the colorSet is only used if we're re-drawing)
//Each time we update, we advance the cycleNum counter
//This corrosponds to the pixel we're working with (ie it's the index of the pixel and color arrays)
//cycleNum wraps at numTwinkles, so we know when we've touched all the pixels
//Other than that, we call whatever mode function to draw the pixels each cycle
//see the individual functions for how they work
void FairyLightsPS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;
        palletLength = pallet->length;
        //by default, we only touch the pixel at the current cycleNum
        //but for rainbow or gradient backgrounds that a cycling
        //you want to redraw the whole thing
        if(fillBG){
            reDrawAll = true;
            segDrawUtils::fillSegSetColor(segmentSet, *bgColor, bgColorMode);
        }
        switch (tmode) {
            case 0: 
                modeZeroSet();
                break;
            case 1: 
                modeOneSet();
                break;
            case 2: 
                modeTwoSet();
                break;
            default:
                modeZeroSet();
                break;
        }
        cycleNum = addmod8( cycleNum, 1, numTwinkles );//(cycleNum + 1) % (numTwinkles);
        showCheckPS();
    }
}

//set a color based on the size of the pallet
void FairyLightsPS::pickColor(uint16_t pixelNum){
    switch (randMode) {
        case 0: // we're picking from a set of colors 
            color = palletUtilsPS::getPalletColor(pallet, random8(palletLength));
            break;
        default: //(mode 1) set colors at random
            color = colorUtilsPS::randColor();
            break;
    }
    //get the pixel color to account for any color modes
    //this also fills in pixelInfo for the pixel location
    segDrawUtils::getPixelColor(segmentSet, &pixelInfo, color, colorMode, pixelNum);
    color = pixelInfo.color;
}

//Mode 0: turns one pixel on after another and then resets all at once
//Overview:
    //The function basically has two seperate modes, one where we're turning the pixels on, and one where they're turing off
    //While turning on, we switch one pixel on each cycle, using the pixelSet and choosing a color each time, storing it in the colorSet
    //If we are re-drawing the background each time, we have to re-draw all the pixel's we've turned on so far
    //We do this by looping from the first pixel to the one at the most current cycle
    //and setting the color based on the colorSet
    //If we've finished turning all the pixel's on, then we need to turn them all off on the next cycle
    //(At this point we decrement the cycleNum by 1, so that we don't skip the first pixel when we next turn on)
    //Once the pixel's are off, we generate a new set of pixel locations, and start over
void FairyLightsPS::modeZeroSet(){

    if(!turnOff){
        //to save space, we always loop to set the pixels
        //but if we're not re-drawing all, then we only need to draw the pixel at the current cycleNum
        loopStart = cycleNum;
        if(reDrawAll){
           loopStart = 0;
        } 
        for(uint8_t i = loopStart; i <= cycleNum; i++){
            //pick a pixel color, this also gives us the pixel location info
            pickColor(pixelSet[i]);
            //if we're re-drawing, then we need to get the color from the colorSet
            //(unless we're setting the pixel at the current cycleNum, then we need choose a new color)
            if(reDrawAll && (i != cycleNum) ){
                color = colorSet[i];
            } else {
                colorSet[i] = color;
            }
            segDrawUtils::setPixelColor(segmentSet, pixelInfo.pixelLoc, color, 0, pixelInfo.segNum, pixelInfo.lineNum);
        }
        //if we've reached the cycle limit, we need to turn off all the pixels on the next cycle
        //we don't want the turn off step to count as a cycle (since turning on the first pixel would be skipped)
        //so we decrement the cycleNum
        if(cycleNum == cycleLimit ){
            turnOff = true;
            cycleNum--;
        }
    } else {
        //turn off all the pixels, and make a new pixel group to turn on
        for(uint8_t i = 0; i <= cycleLimit; i++){
            segDrawUtils::setPixelColor(segmentSet, pixelSet[i], *bgColor, bgColorMode);
        }
        genPixelSet();
        turnOff = false;
    }

}

//Mode 1: Turns on each pixel one at a time, then off one at a time
//Overview:
    //The function basically has two seperate modes, one where we're turning the pixels on, and one where they're turing off
    //While turning on, we switch one pixel on each cycle, using the pixelSet and choosing a color each time, storing it in the colorSet
    //If we are re-drawing the background each time, we have to re-draw all the pixel's we've turned on so far
    //We do this by looping from the first pixel to the one at the most current cycle
    //and setting the color based on the colorSet
    //If we've finished turning all the pixel's on, then we need to start turning them off
    //This is basically the reverse of turning them on
    //If we need to re-draw them, we loop from the current pixel to the last
void FairyLightsPS::modeOneSet(){
    //sort out the re-drawing
    //If we're turning the pixels on, we need to loop through all the pixels up to the cycleNum pixel
    //If we're turning them off, we need to loop through all the pixels starting at the cycleNum pixel 
    //(since all the one's before will already be off)
    if(reDrawAll){
        loopStart = 0;
        loopEnd = cycleNum;
        if(turnOff){
            loopStart = cycleNum;
            loopEnd = cycleLimit;
        }

        for(uint8_t i = loopStart; i <= loopEnd; i++ ){
            pickColor(pixelSet[i]); //we don't actually need to pick a color, but calling the function fills in the pixelInfo
            color = colorSet[i]; //since we're re-drawing, we get the color from the colorSet
            segDrawUtils::setPixelColor(segmentSet, pixelInfo.pixelLoc, color, 0, pixelInfo.segNum, pixelInfo.lineNum);
        }
    }
    if(!turnOff){
        //we're turning the pixels on, so we pick and record a color
        //then set the pixel
        pickColor(pixelSet[cycleNum]);
        colorSet[cycleNum] = color;
        segDrawUtils::setPixelColor(segmentSet, pixelInfo.pixelLoc, color, 0, pixelInfo.segNum, pixelInfo.lineNum);
        //once we've finished drawing all the pixels, we need to start turning them off
        if(cycleNum == cycleLimit ){
           turnOff = true;
        }
    } else {
        //turn the pixels off one at a time
        segDrawUtils::setPixelColor(segmentSet, pixelSet[cycleNum], *bgColor, bgColorMode);
        //once we've finished turning all the pixels off, we need make a new pixelSet and start again
        if(cycleNum == cycleLimit ){
            genPixelSet();
            turnOff = false;
        }
    }
}

//Mode 2: Each cycle, a new pixel is turned on while an old is turned off (first on first off)
//Overview: 
    //Each cycle we first get the pixel info for the current pixel (the cycleNum)
    //Then we turn it off, and pick a new random location for the pixel
    //We pick a color and draw it
    //So each cycle we exchange the current pixel with a new one
    //Since the cycle's are in order, we always turn the pixels on and off in order 
    //If we're re-drawing, then we loop through all the pixels each cycle to set them
void FairyLightsPS::modeTwoSet(){
    
    //turn the current pixel off
    pickColor(pixelSet[cycleNum]); 
    segDrawUtils::setPixelColor(segmentSet, pixelInfo.pixelLoc, *bgColor, bgColorMode, pixelInfo.segNum, pixelInfo.lineNum);
    
    //pick a new pixel to turn on
    pixelSet[cycleNum] = random16(segmentSet.numActiveSegLeds); // set a new pixel to turn on and put it in the array
    
    //to save space, we always use a loop to draw the pixels
    //we we're not redrawing, we set the limits to only do one cycle at the current cycleNum
    loopStart = cycleNum;
    loopEnd = cycleNum;
    if(reDrawAll){
        loopStart = 0;
        loopEnd = cycleLimit;
    } 

    for(uint8_t i = loopStart; i <= loopEnd; i++){
        //pick a pixel color, this also gives us the pixel location info
        pickColor(pixelSet[i]);
        //if we're re-drawing, then we need to get the color from the colorSet
        //(unless we're setting the pixel at the current cycleNum, then we need choose a new color)
        if(reDrawAll && (i != cycleNum) ){
            color = colorSet[i];
        } else {
           colorSet[i] = color; 
        }
        segDrawUtils::setPixelColor(segmentSet, pixelInfo.pixelLoc, color, 0, pixelInfo.segNum, pixelInfo.lineNum);
    }

}
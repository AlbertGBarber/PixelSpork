#include "FairyLightsSLPS.h"

//see update() for how the effect works

//palette based constructor
FairyLightsSLPS::FairyLightsSLPS(SegmentSet &SegmentSet, palettePS *Palette, uint8_t NumTwinkles, CRGB BGColor, uint8_t Tmode, uint16_t Rate):
    segmentSet(SegmentSet), palette(Palette), numTwinkles(NumTwinkles), tmode(Tmode)
    {    
        init(BGColor, Rate);
	}

//single color constructor
FairyLightsSLPS::FairyLightsSLPS(SegmentSet &SegmentSet, CRGB Color, uint8_t NumTwinkles, CRGB BGColor, uint8_t Tmode, uint16_t Rate):
    segmentSet(SegmentSet), numTwinkles(NumTwinkles), tmode(Tmode)
    {    
        init(BGColor, Rate);
        setSingleColor(Color);
	}

//random colors constructor
FairyLightsSLPS::FairyLightsSLPS(SegmentSet &SegmentSet, uint8_t NumTwinkles, CRGB BGColor, uint8_t Tmode, uint16_t Rate):
    segmentSet(SegmentSet), numTwinkles(NumTwinkles), tmode(Tmode)
    {    
        init(BGColor, Rate);
        //we make a random palette of one color so that 
        //if we switch to randMode 0 then we have a palette to use
        setSingleColor(colorUtilsPS::randColor()); 
        //since we're choosing colors at random, set the randMode
        randMode = 1;
	}

//destructor
FairyLightsSLPS::~FairyLightsSLPS(){
    delete[] paletteTemp.paletteArr;
    delete[] twinkleSet;
    delete[] colorSet;
}

void FairyLightsSLPS::init(CRGB BgColor, uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    bindBGColorPS();
    genPixelSet();
}

//makes new twinkleSet and colorSet arrays to store the twinkle locations and colors
void FairyLightsSLPS::genPixelSet(){
    if(numTwinkles < 1){
        numTwinkles = 1;
    }
    //fetch some core vars
    numLines = segmentSet.maxSegLength;
    delete[] twinkleSet;
    twinkleSet = new uint16_t[numTwinkles];
    cycleLimit = numTwinkles - 1; //the number of cycles to go through the whole array
    //pick locations for the twinkles (local to the segment set)
    //We use segment lines because the effect is set to draw twinkles across all segment lines
    for (uint8_t i = 0; i < numTwinkles; i++) {
        twinkleSet[i] = random16(numLines);
    }

    delete[] colorSet;
    colorSet = new CRGB[numTwinkles];
}

//changes the number of twinkles, also resets the twinkleSet
void FairyLightsSLPS::setNumTwinkles(uint8_t newNumTwinkles){
    numTwinkles = newNumTwinkles;
    genPixelSet();
}

//creates an palette of length 1 containing the passed in color
//binds it to the temp palette to keep it in scope
void FairyLightsSLPS::setSingleColor(CRGB Color){
    delete[] paletteTemp.paletteArr;
    paletteTemp = paletteUtilsPS::makeSingleColorPalette(Color);
    palette = &paletteTemp;
}

//Updates the effect
//The twinkle locations (local to segment set) and colors are stored in the twinkleSet and colorSet arrays (the colorSet is only used if we're re-drawing)
//Each time we update, we advance the cycleNum counter
//This corrosponds to the twinkle we're working with (ie it's the index of the twinkle and color arrays)
//cycleNum wraps at numTwinkles, so we know when we've touched all the twinkles
//Other than that, we call whatever mode function to draw the twinkles each cycle
//see the individual functions for how they work
//Note that twinkles are drawn along segment lines, so each twinkle will light up a whole segment line
void FairyLightsSLPS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;
        paletteLength = palette->length;
        //by default, we only touch the twinkle at the current cycleNum
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
        cycleNum = addmod8(cycleNum, 1, numTwinkles);
        showCheckPS();
    }
}

//Pick a color based on the size of the palette and random modes
CRGB FairyLightsSLPS::pickColor(){
    switch (randMode) {
        case 0: // we're picking from a set of colors 
            color = paletteUtilsPS::getPaletteColor(palette, random8(paletteLength));
            break;
        default: //(mode 1) set colors at random
            color = colorUtilsPS::randColor();
            break;
    }
    return color;
}

//Mode 0: turns one twinkle on after another and then resets all at once
//Overview:
    //The function basically has two seperate modes, one where we're turning the twinkles on, and one where they're turing off
    //While turning on, we switch one twinkle on each cycle, using the twinkleSet and choosing a color each time, storing it in the colorSet
    //If we are re-drawing the background each time, we have to re-draw all the twinkle's we've turned on so far
    //We do this by looping from the first twinkle to the one at the most current cycle
    //and setting the color based on the colorSet
    //If we've finished turning all the twinkle's on, then we need to turn them all off on the next cycle
    //(At this point we decrement the cycleNum by 1, so that we don't skip the first twinkle when we next turn on)
    //Once the twinkle's are off, we generate a new set of twinkle locations, and start over
void FairyLightsSLPS::modeZeroSet(){

    if(!turnOff){
        //to save space, we always loop to set the twinkles
        //but if we're not re-drawing all, then we only need to draw the twinkle at the current cycleNum
        loopStart = cycleNum;
        if(reDrawAll){
           loopStart = 0;
        } 
        for(uint8_t i = loopStart; i <= cycleNum; i++){
            //pick a twinkle color
            color = pickColor();
            //if we're re-drawing, then we need to get the color from the colorSet
            //(unless we're setting the twinkle at the current cycleNum, then we need choose a new color)
            if(reDrawAll && (i != cycleNum) ){
                color = colorSet[i];
            } else {
                colorSet[i] = color;
            }
            //fill the segment line at the twinkle location with color
            segDrawUtils::drawSegLineSimple(segmentSet, twinkleSet[i], color, colorMode);
        }
        //if we've reached the cycle limit, we need to turn off all the twinkles on the next cycle
        //we don't want the turn off step to count as a cycle (since turning on the first twinkle would be skipped)
        //so we decrement the cycleNum
        if(cycleNum == cycleLimit){
            turnOff = true;
            cycleNum--;
        }
    } else {
        //turn off all the twinkles, and make a new twinkle group to turn on
        for(uint8_t i = 0; i <= cycleLimit; i++){
            segDrawUtils::drawSegLineSimple(segmentSet, twinkleSet[i], *bgColor, bgColorMode);
        }
        genPixelSet();
        turnOff = false;
    }

}

//Mode 1: Turns on each twinkle one at a time, then off one at a time
//Overview:
    //The function basically has two seperate modes, one where we're turning the twinkles on, and one where they're turing off
    //While turning on, we switch one twinkle on each cycle, using the twinkleSet and choosing a color each time, storing it in the colorSet
    //If we are re-drawing the background each time, we have to re-draw all the twinkle's we've turned on so far
    //We do this by looping from the first twinkle to the one at the most current cycle
    //and setting the color based on the colorSet
    //If we've finished turning all the twinkle's on, then we need to start turning them off
    //This is basically the reverse of turning them on
    //If we need to re-draw them, we loop from the current twinkle to the last
void FairyLightsSLPS::modeOneSet(){
    //sort out the re-drawing
    //If we're turning the twinkles on, we need to loop through all the twinkles up to the cycleNum twinkle
    //If we're turning them off, we need to loop through all the twinkles starting at the cycleNum twinkle 
    //(since all the one's before will already be off)
    if(reDrawAll){
        loopStart = 0;
        loopEnd = cycleNum;
        if(turnOff){
            loopStart = cycleNum;
            loopEnd = cycleLimit;
        }

        for(uint8_t i = loopStart; i <= loopEnd; i++){
            color = colorSet[i]; //since we're re-drawing, we get the color from the colorSet
            //fill the segment line at the twinkle location with color
            segDrawUtils::drawSegLineSimple(segmentSet, twinkleSet[i], color, colorMode);
        }
    }
    if(!turnOff){
        //we're turning the twinkles on, so we pick and record a color
        //then set the twinkle
        color = pickColor();
        colorSet[cycleNum] = color;
        //fill the segment line at the twinkle location with color
        segDrawUtils::drawSegLineSimple(segmentSet, twinkleSet[cycleNum], color, colorMode);
        //once we've finished drawing all the twinkles, we need to start turning them off
        if(cycleNum == cycleLimit){
           turnOff = true;
        }
    } else {
        //turn the twinkles off one at a time
        segDrawUtils::drawSegLineSimple(segmentSet, twinkleSet[cycleNum], *bgColor, bgColorMode);
        //segDrawUtils::setPixelColor(segmentSet, twinkleSet[cycleNum], *bgColor, bgColorMode);
        //once we've finished turning all the twinkles off, we need make a new twinkleSet and start again
        if(cycleNum == cycleLimit){
            genPixelSet();
            turnOff = false;
        }
    }
}

//Mode 2: Each cycle, a new twinkle is turned on while an old is turned off (first on first off)
//Overview: 
    //Each cycle we first get the twinkle info for the current twinkle (the cycleNum)
    //Then we turn it off, and pick a new random location for the twinkle
    //We pick a color and draw it
    //So each cycle we exchange the current twinkle with a new one
    //Since the cycle's are in order, we always turn the twinkles on and off in order 
    //If we're re-drawing, then we loop through all the twinkles each cycle to set them
void FairyLightsSLPS::modeTwoSet(){
    
    //turn the current twinkle off
    segDrawUtils::drawSegLineSimple(segmentSet, twinkleSet[cycleNum], *bgColor, bgColorMode);
    
    //pick a new twinkle to turn on
    twinkleSet[cycleNum] = random16(numLines); // set a new twinkle to turn on and put it in the array
    
    //to save space, we always use a loop to draw the twinkles
    //we we're not redrawing, we set the limits to only do one cycle at the current cycleNum
    loopStart = cycleNum;
    loopEnd = cycleNum;
    if(reDrawAll){
        loopStart = 0;
        loopEnd = cycleLimit;
    } 

    for(uint8_t i = loopStart; i <= loopEnd; i++){
        //pick a twinkle color, this also gives us the twinkle location info
        color = pickColor();
        //if we're re-drawing, then we need to get the color from the colorSet
        //(unless we're setting the twinkle at the current cycleNum, then we need choose a new color)
        if( reDrawAll && (i != cycleNum) ){
            color = colorSet[i];
        } else {
            colorSet[i] = color; 
        }
        //fill the segment line at the twinkle location with color
        segDrawUtils::drawSegLineSimple(segmentSet, twinkleSet[i], color, colorMode);
    }

}
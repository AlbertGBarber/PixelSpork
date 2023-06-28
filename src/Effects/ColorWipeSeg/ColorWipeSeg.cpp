#include "ColorWipeSeg.h"

//Constructor using pattern and palette
ColorWipeSeg::ColorWipeSeg(SegmentSet &SegSet, palettePS &Palette, patternPS &Pattern, uint8_t Style,
                          bool Alternate, bool WipeDirect, bool SegWipeDir, uint16_t Rate):
    SegSet(SegSet), palette(&Palette), pattern(&Pattern), style(Style), alternate(Alternate), wipeDirect(WipeDirect), segWipeDir(SegWipeDir)
    {    
        init(Rate);
	}

//Constructor using palette alone 
ColorWipeSeg::ColorWipeSeg(SegmentSet &SegSet, palettePS &Palette, uint8_t Style, bool Alternate, bool WipeDirect, bool SegWipeDir, uint16_t Rate):
    SegSet(SegSet), palette(&Palette), style(Style), alternate(Alternate), wipeDirect(WipeDirect), segWipeDir(SegWipeDir)
    {    
        setPaletteAsPattern();
        init(Rate);
	}

//Constructor for a single color wipe
ColorWipeSeg::ColorWipeSeg(SegmentSet &SegSet, CRGB WipeColor, uint8_t Style, bool Alternate, bool WipeDirect, bool SegWipeDir, uint16_t Rate):
    SegSet(SegSet), style(Style), alternate(Alternate), wipeDirect(WipeDirect), segWipeDir(SegWipeDir)
    {    
        paletteTemp = paletteUtilsPS::makeSingleColorPalette(WipeColor);
        palette = &paletteTemp;
        setPaletteAsPattern();
        init(Rate);
	}

//destructor
ColorWipeSeg::~ColorWipeSeg(){
    free(paletteTemp.paletteArr);
    free(patternTemp.patternArr);
}

//Setup core variables for the effect
void ColorWipeSeg::init(uint16_t Rate){
    //bind the rate and SegSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    startingDirect = wipeDirect;
    reset();
}

//sets the pattern to match the current palette
//ie for a palette length 5, the pattern would be 
//{0, 1, 2, 3, 4}
void ColorWipeSeg::setPaletteAsPattern(){
    patternTemp = generalUtilsPS::setPaletteAsPattern(*palette);
    pattern = &patternTemp;
}

//Resets the core wipe variables, restarting the wipe
void ColorWipeSeg::reset(){
    segWipeCount = 0;
    pixelCount = 0;
    bgWipe = false;
    wipeDirect = startingDirect;
    loopCount = 0;

    setUpWipeOrder();
}

//Resets variables to loop the color wipe
//The various looping variables are explained in setUpLoop()
void ColorWipeSeg::resetLoop(){
    //set the basic wipe variables so we start a new wipe
    done = false;
    segWipeCount = 0;
    pixelCount = 0;

    loopCount++;

    //Set the effect to do a background color wipe every other loop
    if(bgLoop && !bgWipe ){
        bgWipe = true;
    } else{
        bgWipe = false;
    }
    
    //Flip the segment wipe direction
    //This is different than flipping the wipe direction, since it makes the first wipe start at the opposite end of the segment set
    //Rather than having the wipe just move in the opposite direction
    if(altSegWipeDirLoop){
        segWipeDir = !segWipeDir;
    }

    //Determine if we need to flip the wipe direction
    //The loop can be set to flip whenever we do a background wipe (using bgAltLoop), 
    //or to skip over background wipes (ie only flipping when we are drawing the wipe pattern -> every other loop)
    //So if bgAltLoop is true, then we only want to flip if we're not background wiping (bgWipe is false)
    //Otherwise (bgAltLoop is false), we flip whenever we start a new loop
    if(altWipeDirLoop && ( !bgAltLoop || (bgAltLoop && !bgWipe) ) ){
        startingDirect = !startingDirect;
    }
    wipeDirect = startingDirect;

    //If we're moving the pattern colors, we shift them using patOffset
    //We only shift them when we're not doing a background wipe
    if(shiftPatLoop && !bgWipe){
        patternStep = patShiftDir - !patShiftDir;
        patOffset = addMod16PS(patOffset, patternStep + pattern->length, pattern->length);
    }
    
    //Setup the next wipe
    setUpWipeOrder();
}

//A quick way of changing all of the looping variables at once.
//There should be enough variables to cover most looping effect variations.
//An explanation of each of the variables (ignore the n in the arg names):
//  looped -- Sets if the wipes loop or not. Looping wipes automatically restart every time a wipe is finished.
//            The other variables are only relevant if the wipe is looping, because they modify subsequent loops.
//  bgLoop -- If true, then the background color (default 0) will be used as the color wipe every other loop
//            Ie, we wipe a color and then wipe off, looping
//  shiftPatLoop -- If true, the pattern will be shifted by 1 with every loop, 
//                  changing the colors of each segment / line with each wipe
//                  (this is done with an offset, it does not change the existing pattern)
//  patShiftDir -- Sets the direction the pattern shifts in, forwards (true) or backwards (false)
//  altWipeDirLoop -- If true, then the wipe direction will be flipped every loop
//                    (note that this modifies the original startingDirect value)
//  bgAltLoop -- Only used if altWipeDirLoop is true. 
//               If true, the the wipe direction will only swap on colored wipes, not on background ones.
//               Ie colored wipe->background wipe->(wipe direction flips)->colored wipe->background wipe->(flip)->etc
//               If false, then the wipe direction will flip every loop
//               ie colored wipe->(wipe direction flips)->background wipe->(wipe direction flips)->colored wipe->etc
//  altSegWipeDirLoop -- If true, then the segment set direction will be flipped every loop
//                       This is different than flipping the wipe direction, since it makes the first wipe 
//                       start at the opposite end of the segment set, rather than having the wipe just move in the opposite direction    
void ColorWipeSeg::setUpLoop(bool nLooped, bool nBgLoop, bool nShiftPatLoop, bool nPatShiftDir, bool nAltWipeDirLoop, 
                             bool nBgAltLoop, bool nAltSegWipeDirLoop){
    looped = nLooped;
    altSegWipeDirLoop = nAltSegWipeDirLoop;
    altWipeDirLoop = nAltWipeDirLoop,
    bgLoop = nBgLoop;
    bgAltLoop = nBgAltLoop; //only matters if altWipeDirLoop is true
    shiftPatLoop = nShiftPatLoop;
    patShiftDir = nPatShiftDir;
}

//Sets the order the segment are wiped in, either starting with the first of last segment depending on segWipeDir
//Also sets up the first wipe for the set
void ColorWipeSeg::setUpWipeOrder(){
    numSegs = SegSet.numSegs;

    //Get the step we increment the currentSeg by, either 1 or -1
    segStep = segWipeDir - !segWipeDir;

    //Set the first and last segment for the wipe
    if (segWipeDir) { //going forward, start at the first segment, go to the last
        currentSeg = 0;
        lastSeg = numSegs - 1;
    } else{ //going backwards, start at the last segment and go to the first
        currentSeg = numSegs - 1;
        lastSeg = 0;
    }

    //Setup the first segment wipe limits
    setupSegWipe();
}

//Sets the start and end pixels for wiping the current segment, depending on the wipeDirect
void ColorWipeSeg::setupSegWipe(){
    //Get the step we increment the currentPixel by, either 1 or -1
    wipeStep = wipeDirect - !wipeDirect;
    
    //Set the wipe start and end pixels
    segLength = SegSet.getTotalSegLength(currentSeg);
    if (wipeDirect) { // positive: start from zeroth segment pixel and run to last
        currentPixel = 0;
        endPixel = segLength - 1;
    } else { // negative: start from end of segment and run till beginning
        currentPixel = segLength - 1;
        endPixel = 0;
    }
}

//Updates the color wipe
//The effect is quite simple; we color in each segment in the segment set one pixel at a time until all are colored
//To do this, we have to have a starting segment and pixel (set by setUpWipeOrder() and setupSegWipe())
//The wipes can either start at the first or last segment, and can run from the first or last pixel in each segment.
//Each update we color in one pixel, and then choose the next to be colored in based on the direction.
//This is tracked by currentPixel, while the segment is tracked by currentSeg
//The color we choose for the pixel depends on the style (see effect intro in .h file)
//If we reached the last pixel in the segment, we advance to the next segment, calling setupSegWipe() to set the new pixel start/end.
//If we're at the last segment and last pixel, then we're "done", and we either stop or loop and reset.
//Note that when choosing colors, we use a count of the pixels and segments (pixelCount and segWipeCount) rather than using the pixel or segment number directly
//This prevents the effect from looking the same in both wipe directions.
void ColorWipeSeg::update(){
    currentTime = millis();

    if( !done && ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        //get the location of the next pixel and its line
        pixelNum = segDrawUtils::getSegmentPixel(SegSet, currentSeg, currentPixel);
        lineNum = segDrawUtils::getLineNumFromPixelNum(SegSet, currentPixel, currentSeg);

        if(bgWipe){ //if we're doing a background wipe, only relevant when looping
            modeOut = bgColorMode;
            colorOut = *bgColor;
        } else {
            //set the pixel color according to the style
            //Note that when choosing colors, we use a count of the pixels and segments (pixelCount and segWipeCount) rather than using the pixel or segment number directly
            //This prevents the effect from looking the same in both wipe directions.
            switch (style) {
                case 0: default: // alternates colors for each segment
                    palIndex = patternUtilsPS::getPatternVal( *pattern, segWipeCount + patOffset);
                    break;

                case 1: // alternates colors for each pixel
                    palIndex = patternUtilsPS::getPatternVal( *pattern, pixelCount + patOffset );
                    break;
                
                case 2: // alternates colors for each line num
                    palIndex = patternUtilsPS::getPatternVal( *pattern, lineNum + patOffset );
                    break;
            }
            //Get the final color
            modeOut = colorMode;
            colorOut = paletteUtilsPS::getPaletteColor( *palette, palIndex );
        }

        //Set the pixel color
        segDrawUtils::setPixelColor(SegSet, pixelNum, colorOut, modeOut, currentSeg, lineNum);

        //Advance the currentPixel to the next pixel
        //If were at the last pixel in the segment, we need to either setup the next segment wipe
        //of, if we're also at the last segment, finish the wipe
        if(currentPixel == endPixel){
            segWipeCount++; //count how many wipes we've done
            if(currentSeg == lastSeg){ //If we're at the last segment, end the wipe, or loop it
                done = true;
                if(looped){
                    resetLoop();
                }
            } else { //If we're not a the last segment, advance to the next one
                // if we're alternating, we need to switch the direction of the next wipe
                if (alternate) {
                    wipeDirect = !wipeDirect;
                }
                currentSeg += segStep;
                setupSegWipe();
            }
        } else {
            currentPixel += wipeStep;
            pixelCount++; //count how many pixels we've colored
        }

        showCheckPS();
    }
}
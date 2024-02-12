#include "XmasLightsSLSeg.h"

//Constructor with just a palette
XmasLightsSLSeg::XmasLightsSLSeg(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, uint16_t ColorLength,
                           uint16_t OnChance, uint16_t OffChance, uint8_t SegMode, uint16_t Rate)              //
    : palette(&Palette), colorLength(ColorLength), onChance(OnChance), offChance(OffChance), segMode(SegMode)  //
{
    setPaletteAsPattern();
    init(BgColor, SegSet, Rate);
}

//Constructor with pattern and palette
XmasLightsSLSeg::XmasLightsSLSeg(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, CRGB BgColor, uint16_t ColorLength,
                           uint16_t OnChance, uint16_t OffChance, uint8_t SegMode, uint16_t Rate)                                 //
    : pattern(&Pattern), palette(&Palette), colorLength(ColorLength), onChance(OnChance), offChance(OffChance), segMode(SegMode)  //
{
    init(BgColor, SegSet, Rate);
}

//Destructor
XmasLightsSLSeg::~XmasLightsSLSeg() {
    free(patternTemp.patternArr);
    free(twinkleArr);
}

//Initializes core effect variables and setup the effect
void XmasLightsSLSeg::init(CRGB BgColor, SegmentSetPS &SegSet, uint16_t Rate) {
    //bind the rate and segSet pointer vars since they are inherited from BaseEffectPS
    bindSegSetPtrPS();
    bindClassRatesPS();
    //bind background color pointer (if needed)
    bindBGColorPS();
    setSegMode(segMode);
};

//sets the pattern to match the current palette
//ie for a palette length 5, the pattern would be
//{0, 1, 2, 3, 4}
void XmasLightsSLSeg::setPaletteAsPattern() {
    generalUtilsPS::setPaletteAsPattern(patternTemp, *palette);
    pattern = &patternTemp;
}

//Restarts the effect
void XmasLightsSLSeg::reset(){
    //Treat the effect as if it's doing the first update, 
    //which will set the inital state of the twinkles
    firstUpdate = true;
}

//Changes if the segMode for the effect. Also acts as a reset() for the effect.
//You must call this if you change the segment set at all!
//segModes:
//    0 -- The pattern and twinkles will be drawn using segment lines (each line will be a single color)
//    1 -- The pattern and twinkles will be drawn using whole segments (each segment will be a single color)
//    2 -- The pattern and twinkles will be drawn linearly along the segment set (1D).
//Will also recalculate the size of the twinkle bit array and reset it to it's inital state,
//with either the pattern or background being filled depending on the bgPrefill.
void XmasLightsSLSeg::setSegMode(uint8_t newSegMode) {
    segMode = newSegMode;

    switch( segMode ) {
        case 0:
        default:
            numTwinkles = segSet->numLines;
            break;
        case 1:
            numTwinkles = segSet->numSegs;
            break;
        case 2:
            numTwinkles = segSet->numLeds;
            break;
    }

    setupTwinkleArray();
}

//Creates the "compacted bit array" for storing the pixel's states
//We use single bits in uint8_t's to track the on/off state for each pixel, 
//with the uint8_t's arranged in an array, twinkleArr.
//The length of the array is numTwinkles/8 (numTwinkles is numLines/segs/pixels depending on segMode)
//Note that to prevent memory fragmentation, the array is only re-sized if the current array is too small.
//Note that the function also "resets" the effect by setting firstUpdate = true, thus triggering 
//a reset of the pixel's states on the next update.
void XmasLightsSLSeg::setupTwinkleArray() {

    uint16_t twinkArrLenTemp = ceil((float)numTwinkles / 8);

    //We only need to make a new offsets array if the current one isn't large enough
    //This helps prevent memory fragmentation by limiting the number of heap allocations
    //but this may use up more memory overall.
    if( alwaysResizeObj_PS || (twinkArrLenTemp > twinkArrLenMax) ) {
        twinkArrLenMax = twinkArrLenTemp;
        twinkArrLen = twinkArrLenTemp;
        free(twinkleArr);
        twinkleArr = (uint8_t *)malloc(twinkArrLen * sizeof(uint8_t));
    }
    twinkArrLen = twinkArrLenTemp;

    //We treat this function as a "reset", 
    //so we want to reset their states and re-draw all the pixels on the next update()
    reset();
}

//Draws the color pattern or background for the whole segment set
//bgPrefill is true, the segment set is initially filled with the background color (and all the pixel states are 0)
//Otherwise, the set is filled with the color pattern (and all the pixel states are 1)
void XmasLightsSLSeg::initialFill() {

    //colorLength must be 1 or more
    //we do this here b/c it's called every reset(), but not otherwise
    if(colorLength < 1){
        colorLength = 1;
    }

    //Set the initial state for all the twinkles based on bgPrefill
    bitState = !bgPrefill;

    //Bit write the initial state to the pixels in the "compacted bit array"
    for( uint16_t i = 0; i < twinkArrLen; i++ ) {
        for( uint8_t j = 0; j < 8; j++ ) {
            bitWrite(twinkleArr[i], j, bitState);
        }
    }

    //Draw all the pixels
    for( uint16_t i = 0; i < numTwinkles; i++ ) {
        draw(i);
    }
}

//This effect is pretty simple in concept, with a lot of extra bits to handle different segModes, patterns, etc
//The main idea is that with each update we roll a dice for each LED/line/whatever.
//If we roll above a certain value we switch the state of the LED between on/off (background).
//To help vary the effect, we allow different probability thresholds for turning on and off,
//so we can make it more likely an off LED will turn on, etc.
//To track the state of the LEDs we use a "compacted bit array" where we use the individual bits of
//a set of uint8_t's represent individual LEDs. (1 is on, 0 is off)
//The rest of the code is just picking the color to draw and how to draw it.
//Note that by default the effect only draws an LED if it changed, and it pre-fills the pattern on the first update
//(these settings are controlled with fillBg and bgPrefill)
void XmasLightsSLSeg::update() {
    currentTime = millis();

    if( (currentTime - prevTime) >= *rate ) {
        prevTime = currentTime;

        //If it's the first update we need to pre-fill the segment set with the pattern or background (based on bgPrefill)
        //because we only draw pixels when their state changes, and we don't want to start with an empty segment set
        if( firstUpdate ) {
            initialFill();
            firstUpdate = false;
        }

        //Cycle across all the segment lines/segments/pixels depending on segMode
        //For each pixel, check the probability of it turning on/off
        //If it triggers, toggle its bit state and color
        for( uint16_t i = 0; i < numTwinkles; i++ ) {

            //We only want to draw pixels that change states,
            //drawNow flags if it changes
            drawNow = false;

            //Find what number our current line/segment/pixel is in in the bit array
            //and then find what bit it's on
            twinkArrPos = i / 8;  //(integer math rounds down so this works)
            bitPos = mod16PS(i, 8);

            //Get the on/off state for the current pixel
            bitState = bitRead(twinkleArr[twinkArrPos], bitPos);

            //We allow different probability for turning on/off
            chance = bitState ? offChance : onChance;

            //Roll the dice for turning on/off the pixel
            //If we hit it, we swap the pixel's bit state and tell the effect to draw it (drawNow = true)
            if( random16(chanceBasis) <= chance ) {
                drawNow = true;

                bitState = !bitState;
                bitWrite(twinkleArr[twinkArrPos], bitPos, bitState);
            }

            //If the pixel has changed state (or we're set to always re-draw the pixels),
            //draw it!
            if( drawNow || fillBg ) {
                draw(i);
            }
        }

        showCheckPS();
    }
}

//Draws the specific line/segment/pixel, "i" according to what bit state it's in (on/off)
//(bitState is expected to be set for the current pixel already)
void XmasLightsSLSeg::draw(uint16_t i) {
    //Pick the color to draw from the pattern and palette or the background based on the pixel's state
    if( bitState ) {
        patternIndex = mod16PS(i / colorLength, pattern->length); //note the i/colorLength to manage the lengths of the pattern colors
        paletteIndex = patternUtilsPS::getPatternVal(*pattern, patternIndex);
        colorOut = paletteUtilsPS::getPaletteColor(*palette, paletteIndex);
        modeOut = colorMode;
    } else {
        colorOut = *bgColor;
        modeOut = bgColorMode;
    }

    //Draw the next line/segment/LED depending on the segMode
    switch( segMode ) {
        case 0: default:
            segDrawUtils::drawSegLine(*segSet, i, colorOut, modeOut);
            break;
        case 1:
            segDrawUtils::fillSegColor(*segSet, i, colorOut, modeOut);
            break;
        case 2:
            segDrawUtils::setPixelColor(*segSet, i, colorOut, modeOut);
            break;
    }
}

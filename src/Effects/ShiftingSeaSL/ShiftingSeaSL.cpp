#include "ShiftingSeaSL.h"

/* Overview:
We start by initializing the offset array using shiftingSeaUtilsPS::genOffsetArray(); (in separate file b/c it used to be shared with another effect)
Each index of the offset array holds the offset for its corresponding pixel (max value of palette.length * gradLength) to cover all
the color values the pixel can be
Then, with each offset cycle, the pixel's color is calculated using the current cycle number and it's offset
So all pixels are following the same pattern through the palette, but their individual positions in the pattern are all different
creating the effect
If the randomShift is on, then with each cycle we do a random check to see if we should change the pixel's offset
if so, then we increment it by a random amount up to shiftStep.

Note that in rainbow mode, we get our colors from the rainbow, blending through them over time.
Much of the notes above still apply, but our cycle length is capped to 255 steps and we use the wheel() function
to get our colors.
 */
//Constructor for effect with pattern and palette
ShiftingSeaSL::ShiftingSeaSL(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, uint8_t GradLength,
                             uint8_t ShiftMode, uint8_t Grouping, uint16_t Rate)
    : pattern(&Pattern), palette(&Palette), gradLength(GradLength), shiftMode(ShiftMode), grouping(Grouping)  //
{
    init(SegSet, Rate);
}

//Constructor for effect with palette
ShiftingSeaSL::ShiftingSeaSL(SegmentSetPS &SegSet, palettePS &Palette, uint8_t GradLength, uint8_t ShiftMode,
                             uint8_t Grouping, uint8_t BgMode, uint16_t Rate)
    : palette(&Palette), gradLength(GradLength), shiftMode(ShiftMode), grouping(Grouping), bgMode(BgMode)  //
{
    setPaletteAsPattern();
    init(SegSet, Rate);
}

//Constructor for effect with randomly created palette
ShiftingSeaSL::ShiftingSeaSL(SegmentSetPS &SegSet, uint8_t NumColors, uint8_t GradLength, uint8_t ShiftMode,
                             uint8_t Grouping, uint8_t BgMode, uint16_t Rate)
    : gradLength(GradLength), shiftMode(ShiftMode), grouping(Grouping), bgMode(BgMode)  //
{
    paletteTemp = paletteUtilsPS::makeRandomPalette(NumColors);
    palette = &paletteTemp;
    setPaletteAsPattern();
    init(SegSet, Rate);
}

//Constructor for rainbow mode
ShiftingSeaSL::ShiftingSeaSL(SegmentSetPS &SegSet, uint8_t GradLength, uint8_t ShiftMode, uint8_t Grouping, uint16_t Rate)
    : shiftMode(ShiftMode), gradLength(GradLength), grouping(Grouping)  //
{
    rainbowMode = true;

    //In rainbow mode we won't use a palette, but to prevent crashes should
    //the user turn off rainbow mode, we'll create a quick 3 color palette and pattern.
    bgMode = 0;
    paletteTemp = paletteUtilsPS::makeRandomPalette(3);
    palette = &paletteTemp;
    setPaletteAsPattern();

    init(SegSet, Rate);
}

ShiftingSeaSL::~ShiftingSeaSL() {
    free(offsets);
    free(paletteTemp.paletteArr);
    free(patternTemp.patternArr);
}

//initializes core variables
void ShiftingSeaSL::init(SegmentSetPS &SegSet, uint16_t Rate) {
    //bind the rate and segSet pointer vars since they are inherited from BaseEffectPS
    bindSegSetPtrPS();
    bindClassRatesPS();

    resetOffsets();
}

//sets the pattern to match the current palette
//ie for a palette length 5, the pattern would be {0, 1, 2, 3, 4}
//Also injects "blank" spaces in the pattern depending on the bgMode setting.
//The spaces are set to 255 in the pattern, which is recognized as the bgColor for the rest of the effect.
//See setBgMode() below for the bgModes list
void ShiftingSeaSL::setPaletteAsPattern() {
    uint8_t spacing = 0;

    //Manage the bgMode
    if( bgMode == 1 ) {
        //We're going to build the pattern to match the palette automatically below,
        //but for bgMode 1, we need an extra space in the pattern
        //Unfortunately, this means we need to "trick" the code into thinking the is palette longer by adjusting its length
        //So we increment the palette length here, and then decrement it once the pattern is made
        //!!This is only ok because it's all being done in one function call. You shouldn't be manipulating palette lengths usually!
        palette->length = palette->length + 1;
    } else if( bgMode == 2 ) {
        //For bgMode 2, we're adding a space between each palette color, we can do this by setting a spacing
        //value of 1 for when we call setPaletteAsPattern() below
        spacing = 1;
    }

    //Set patternTemp to match the palette, possibly with a single space in-between each color
    generalUtilsPS::setPaletteAsPattern(patternTemp, *palette, 1, spacing);

    //For bgMode 1, once the pattern is created, we need to revert the palette back to its original length
    //and also set the final pattern value to 255 (so it's recognized as the background color)
    if( bgMode == 1 ) {
        palette->length = palette->length - 1;
        patternUtilsPS::setVal(patternTemp, 255, patternTemp.length - 1);
    }

    pattern = &patternTemp;

    setTotalCycleLen();
}

//changes the mode, also resets the offset array
//since that's where the mode is expressed
void ShiftingSeaSL::setShiftMode(uint8_t newMode) {
    shiftMode = newMode;
    resetOffsets();
}

//Changes the bgMode to add background spaces to the shift pattern
//bgModes:
//  0 -- No spaces (ex: {0, 1, 2, 3, 4}, where the values are palette indexes)
//  1 -- One space added to the end of the pattern (ex: {0, 1, 2, 3, 4, 255})
//  2 -- A space is added after each color (ex: {0, 255, 1, 255, 2, 255, 3, 255, 4, 255})
//(background spaces are denoted by 255 in the patterns)
//Note that changing the bgMode also changes the shift pattern to use patternTemp
//and re-writes patternTemp to a new pattern for the bgMode.
void ShiftingSeaSL::setBgMode(uint8_t newBgMode) {
    if( newBgMode != bgMode ) {
        bgMode = newBgMode;
        setPaletteAsPattern();
    }
}

//changes the grouping, also resets the offset array
//since that's where the grouping is expressed
void ShiftingSeaSL::setGrouping(uint16_t newGrouping) {
    grouping = newGrouping;
    resetOffsets();
}

//Re-builds the offset array with new values
//Will only re-size the array if it needs more room,
//so the can stay larger than needed if you switch to a smaller segment set
void ShiftingSeaSL::resetOffsets() {
    numLines = segSet->numLines;

    //We only need to make a new offsets array if the current one isn't large enough
    //This helps prevent memory fragmentation by limiting the number of heap allocations
    //but this may use up more memory overall.
    if( alwaysResizeObj_PS || (numLines > numLinesMax) ) {
        numLinesMax = numLines;
        free(offsets);
        offsets = (uint16_t *)malloc(numLines * sizeof(uint16_t));
    }

    setTotalCycleLen();
    shiftingSeaUtilsPS::genOffsetArray(offsets, numLines, gradLength, grouping, totalCycleLength, shiftMode);
}

//calculates the totalCycleLength, which represents the total number of possible offsets a pixel can have
void ShiftingSeaSL::setTotalCycleLen() {
    if( rainbowMode ) {  //in rainbow mode the cycle length is 256, the total number of rainbow colors + 1 (b/c it's a limit)
        totalCycleLength = 256;
    } else {
        patternLen = pattern->length;
        totalCycleLength = gradLength * patternLen;
    }
}

//Updates the effect
//Runs through each pixel, calculates it's color based on the cycle number and its offset
//and determines which colors from the palette it's in between
//increments the offset based on the randomShift values
//then writes it out.
//Note that the shift pattern allows for background colors between palette colors (bgModes)
//These are marked in the pattern as 255.
void ShiftingSeaSL::update() {
    currentTime = millis();

    if( (currentTime - prevTime) >= *rate ) {
        prevTime = currentTime;

        //calculates the totalCycleLength, which represents the total number of possible offsets a pixel can have
        //we do this on the fly so you can change gradLength and the palette freely
        //If we're adding a blank color to the cycle, we add an extra gradLength to the totalCycleLength
        //to account for the extra blank color cycle steps
        setTotalCycleLen();

        for( uint16_t i = 0; i < numLines; i++ ) {
            //where we are in the cycle of all the colors based on the current pixel's offset
            step = addMod16PS(cycleNum, offsets[i], totalCycleLength);

            if( rainbowMode ) {
                //in rainbow mode the color is taken from the rainbow wheel
                color = colorUtilsPS::wheel(step, 0, sat, val);

            } else {

                //what step we're on between the current and next color
                gradStep = addMod16PS(cycleNum, offsets[i], gradLength);

                //what pattern index we've started from (integers always round down)
                curPatIndex = step / gradLength;

                //Get the palette index from the pattern then the color from the palette
                curColorIndex = patternUtilsPS::getPatternVal(*pattern, curPatIndex);

                //Get the current color based on the pattern value. If the value is 255, then we use the bgColor as a "space"
                //otherwise we get the color from the palette
                if( curColorIndex == 255 ) {
                    currentColor = *bgColor;
                } else {
                    currentColor = paletteUtilsPS::getPaletteColor(*palette, curColorIndex);
                }

                //Get the next pattern index, wrapping to the start of the pattern as needed, then the color from the palette
                nextColorIndex = patternUtilsPS::getPatternVal(*pattern, curPatIndex + 1);

                //Get the next color based on the pattern value. If the value is 255, then we use the bgColor as a "space"
                //otherwise we get the color from the palette
                if( nextColorIndex == 255 ) {
                    nextColor = *bgColor;
                } else {
                    nextColor = paletteUtilsPS::getPaletteColor(*palette, nextColorIndex);
                }

                //get the cross faded color and write it out
                color = colorUtilsPS::getCrossFadeColor(currentColor, nextColor, gradStep, gradLength);
            }

            //Write out the color
            segDrawUtils::drawSegLine(*segSet, i, color, 0);

            //randomly increment the offset (keeps the effect varied)
            if( randomShift ) {
                if( random16(shiftBasis) <= shiftThreshold ) {
                    offsets[i] = addMod16PS(offsets[i], random8(1, shiftStep), totalCycleLength);
                }
            }
        }

        //increment the cycle, clamping it's max value to prevent any overflow
        cycleNum = addMod16PS(cycleNum, 1, totalCycleLength);

        showCheckPS();
    }
}

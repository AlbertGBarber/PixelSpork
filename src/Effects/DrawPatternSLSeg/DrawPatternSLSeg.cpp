#include "DrawPatternSLSeg.h"

//Constructor for using the passed in pattern and palette
DrawPatternSLSeg::DrawPatternSLSeg(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, CRGB BgColor,
                                   uint8_t SegMode, uint16_t Rate)
    : pattern(&Pattern), palette(&Palette), segMode(SegMode)  //
{
    init(BgColor, SegSet, Rate);
}

//Constructor for building the pattern from the passed in pattern and the palette,
//using the passed in colorLength and spacing
DrawPatternSLSeg::DrawPatternSLSeg(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, uint16_t ColorLength,
                                   uint16_t Spacing, CRGB BgColor, uint8_t SegMode, uint16_t Rate)
    : palette(&Palette), segMode(SegMode)  //
{
    init(BgColor, SegSet, Rate);
    setPatternAsPattern(Pattern, ColorLength, Spacing);
}

//Constructor for building the pattern using all the colors in the passed in palette,
//using the colorLength and spacing for each color
DrawPatternSLSeg::DrawPatternSLSeg(SegmentSetPS &SegSet, palettePS &Palette, uint16_t ColorLength, uint16_t Spacing,
                                   CRGB BgColor, uint8_t SegMode, uint16_t Rate)
    : palette(&Palette), segMode(SegMode)  //
{
    setPaletteAsPattern(ColorLength, Spacing);
    init(BgColor, SegSet, Rate);
}

//Constructor for doing a single colored pattern, using colorLength and spacing
DrawPatternSLSeg::DrawPatternSLSeg(SegmentSetPS &SegSet, CRGB Color, uint16_t ColorLength, uint16_t Spacing, CRGB BgColor,
                                   uint8_t SegMode, uint16_t Rate)
    : segMode(SegMode)  //
{
    paletteTemp = paletteUtilsPS::makeSingleColorPalette(Color);
    palette = &paletteTemp;
    setPaletteAsPattern(ColorLength, Spacing);
    init(BgColor, SegSet, Rate);
}

//destructor
DrawPatternSLSeg::~DrawPatternSLSeg() {
    free(paletteTemp.paletteArr);
    free(patternTemp.patternArr);
}

//initialization of core variables and pointers
void DrawPatternSLSeg::init(CRGB BgColor, SegmentSetPS &SegSet, uint16_t Rate) {
    //bind the rate and segSet pointer vars since they are inherited from BaseEffectPS
    bindSegSetPtrPS();
    bindClassRatesPS();
    //bind background color pointer
    bindBGColorPS();
}

//takes the passed in pattern and creates a pattern for the streamer
//using the passed in color length and spacing
//then sets this pattern to be the streamer pattern
//ex : inputPattern is {1, 2, 4} with color length 2, and 1 spacing
//the streamer pattern would be: {1, 1, 255, 2, 2, 255, 4, 4, 255}
//(255 will be set to the background color)
void DrawPatternSLSeg::setPatternAsPattern(patternPS &inputPattern, uint16_t colorLength, uint16_t spacing) {
    generalUtilsPS::setPatternAsPattern(patternTemp, inputPattern, colorLength, spacing);
    pattern = &patternTemp;
}

//sets the current palette to be the streamer pattern (using all colors in the palette)
//using the passed in colorLength and spacing
//ex: for palette of length 3, and a colorLength of 2, and spacing of 1
//the final streamer pattern would be : {0, 0, 255, 1, 1, 255, 2, 2, 255}
void DrawPatternSLSeg::setPaletteAsPattern(uint16_t colorLength, uint16_t spacing) {
    generalUtilsPS::setPaletteAsPattern(patternTemp, *palette, colorLength, spacing);
    pattern = &patternTemp;
}

//Update the effect, just re-draws the pattern again by running over all the lines/segments/LEDs and filling in a color
//while also tracking where we are in the pattern.
//segModes:
//    0 -- The pattern will be drawn using segment lines (each line will be a single color)
//    2 -- The pattern will be drawn using whole segment (each segment will be a single color)
//    3 -- The pattern will be drawn linearly along the segment set (1D).
void DrawPatternSLSeg::update() {
    currentTime = millis();

    if( (currentTime - prevTime) >= *rate ) {
        prevTime = currentTime;

        patIndex = 0;
        patLen = pattern->length;
        
        //To draw the pattern, we just run across each LED/line/segment and fill in a pattern color
        //But we need to know when to stop, so we set the limit based on the segMode
        switch( segMode ) {
            case 0:
            default:
                patIndexesToDraw = segSet->numLines;
                break;
            case 1:
                patIndexesToDraw = segSet->numSegs;
                break;
            case 2:
                patIndexesToDraw = segSet->numLeds;
                break;
        }

        //Draws the pattern according to the segMode by setting the color of the next line/segment/LED
        //Ends once all the lines/segments/LEDs have been colored.
        for( uint16_t i = 0; i < patIndexesToDraw; i++ ) {
            //sets colorOut to be the pattern color we need to draw
            //also advances our pattern counter.
            getNextPatColor();

            //Draw the next line/segment/LED depending on the segMode
            switch( segMode ) {
                case 0:
                default:
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

        showCheckPS();
    }
}

//Sets colorOut to be the next color in the pattern
//if the pattern value is 255, the background color will be used.
//Also advances the patIndex tracker to the next pattern index.
void DrawPatternSLSeg::getNextPatColor() {

    //get the next pattern value and color (using the background color if the value is 255)
    patVal = patternUtilsPS::getPatternVal(*pattern, patIndex);
    if( patVal == 255 ) {
        colorOut = *bgColor;
        modeOut = bgColorMode;
    } else {
        colorOut = paletteUtilsPS::getPaletteColor(*palette, patVal);
        modeOut = colorMode;
    }

    //increment the pattern index tracker, wrapping if needed
    patIndex = addMod16PS(patIndex, 1, patLen);
}

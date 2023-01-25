 #include "StreamerSL.h"

//constructor for using the passed in pattern and palette for the streamer
StreamerSL::StreamerSL(SegmentSet &SegmentSet, patternPS *Pattern, palettePS *Palette, CRGB BgColor, uint8_t FadeSteps, uint16_t Rate):
    segmentSet(SegmentSet), pattern(Pattern), palette(Palette), fadeSteps(FadeSteps)
    {    
        init(BgColor, Rate);
	}

//constructor for building the streamer pattern from the passed in pattern and the palette, using the passed in colorLength and spacing
StreamerSL::StreamerSL(SegmentSet &SegmentSet, patternPS *Pattern, palettePS *Palette, uint8_t ColorLength, uint8_t Spacing, CRGB BgColor, uint8_t FadeSteps, uint16_t Rate):
    segmentSet(SegmentSet), palette(Palette), fadeSteps(FadeSteps)
    {    
        setPatternAsPattern(Pattern, ColorLength, Spacing);
        init(BgColor, Rate);
	}

//constructor for building a streamer using all the colors in the passed in palette, using the colorLength and spacing for each color
StreamerSL::StreamerSL(SegmentSet &SegmentSet, palettePS *Palette, uint8_t ColorLength, uint8_t Spacing, CRGB BgColor, uint8_t FadeSteps, uint16_t Rate):
    segmentSet(SegmentSet), palette(Palette), fadeSteps(FadeSteps)
    {    
        setPaletteAsPattern(ColorLength, Spacing);
        init(BgColor, Rate);
	}

//constructor for doing a single colored streamer, using colorLength and spacing
StreamerSL::StreamerSL(SegmentSet &SegmentSet, CRGB Color, uint8_t ColorLength, uint8_t Spacing, CRGB BgColor, uint8_t FadeSteps, uint16_t Rate):
    segmentSet(SegmentSet), fadeSteps(FadeSteps)
    {    
        paletteTemp = paletteUtilsPS::makeSingleColorPalette(Color);
        palette = &paletteTemp;
        setPaletteAsPattern(ColorLength, Spacing);
        init(BgColor, Rate);
	}

//destructor
StreamerSL::~StreamerSL(){
    free(paletteTemp.paletteArr);
    free(pattern_arr);
    free(prevLineColors);
}

//intilization of core variables and pointers
void StreamerSL::init(CRGB BgColor, uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    //bind background color pointer
    bindBGColorPS();
    reset();
    //the minimum value for fade steps is 1, this does an instant transition between colors
    //so we set the fadeOn, since this will produce a faster effect execution
    if(fadeSteps <= 1){
        fadeOn = false;
    }
}

//takes the passed in pattern and creates a pattern for the streamer
//using the passed in color length and spacing
//then sets this pattern to be the streamer pattern
//ex : inputPattern is {1, 2, 4} with color length 2, and 1 spacing
//the streamer pattern would be: {1, 1, 255, 2, 2, 255, 4, 4, 255}
void StreamerSL::setPatternAsPattern(patternPS *inputPattern, uint8_t colorLength, uint8_t spacing){
    uint8_t patternIndex;
    uint8_t repeatLength = (colorLength + spacing); //the total length taken up by a single color and spacing
    uint16_t patternLength = inputPattern->length;
    uint16_t totalPatternLength = patternLength * repeatLength; 
    //create new storage for the pattern array
    free(pattern_arr);
    pattern_arr = (uint8_t*) malloc(totalPatternLength*sizeof(uint8_t));

    //for each color in the inputPattern, we fill in the color and spacing for the output pattern
    for(uint16_t i = 0; i < patternLength; i++){
        patternIndex = patternUtilsPS::getPatternVal(inputPattern, i);
        //for each color in the pattern we run over the length of the color and spacing
        //for the indexes up to color length, we set them as the current patternIndex
        //after that we set them as spacing (255)
        for(uint8_t j = 0; j < repeatLength; j++){
            if(j < colorLength){
                //we do i*repeatLength to account for how many color sections we've 
                //filled in already
                pattern_arr[i * repeatLength + j] = patternIndex;
            } else {
                pattern_arr[i * repeatLength + j] = 255;
            }
        }
    }

    patternTemp = {pattern_arr, totalPatternLength};
    pattern = &patternTemp;
}

//sets the current palette to be the streamer pattern (using all colors in the palette)
//using the passed in colorLength and spacing
//ex: for palette of lenth 3, and a colorLength of 2, and spacing of 1
//the final streamer pattern would be : {0, 0, 255, 1, 1, 255, 2, 2, 255}
void StreamerSL::setPaletteAsPattern(uint8_t colorLength, uint8_t spacing){
    uint8_t repeatLength = (colorLength + spacing);
    uint8_t palettelength = palette->length;
    uint16_t totalPatternLength = palettelength * repeatLength; //the total length taken up by a single color and spacing
    //create new storage for the pattern array
    free(pattern_arr);
    pattern_arr = (uint8_t*) malloc(totalPatternLength*sizeof(uint8_t));

    //for each color in the palette, we fill in the color and spacing for the output pattern
    for(uint16_t i = 0; i < palettelength; i++){
        //for each color in the palette we run over the length of the color and spacing
        //for the indexes up to color length, we set them as the current palette index
        //after that we set them as spacing (255)
        for(uint8_t j = 0; j < repeatLength; j++){
            if(j < colorLength){
                //we do i*repeatLength to account for how many color sections we've 
                //filled in already
                pattern_arr[i * repeatLength + j] = i;
            } else {
                pattern_arr[i * repeatLength + j] = 255;
            }
        }
    }

    patternTemp = {pattern_arr, totalPatternLength};
    pattern = &patternTemp;
}

//resets the streamer to it's original starting point
//Also re-creates the prevLineColors array
void StreamerSL::reset(){
    blendStep = 0;
    cycleCount = 0;

    numSegs = segmentSet.numSegs;

    free(prevLineColors);
    prevLineColors = (CRGB*) malloc(numSegs*sizeof(CRGB));
}

//the main update function
//either calls updateFade, or updateNoFade depending on if fadeOn is true
void StreamerSL::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        //both updateFade() and updateNoFade() need the current segment and pattern lengths,
        //so we'll do them here to reduce repetition
        numLines = segmentSet.maxSegLength;
        numLinesLim = numLines - 1; //used for setting the line colors
        longestSeg = segmentSet.segNumMaxSegLength;
        patternLength = pattern->length;

        if(fadeOn){
            updateFade();
        } else {
            updateNoFade();
        }
        
        showCheckPS();
    }
}

//returns the next output color for the streamer, taking into account color modes
//also gets/sets the pixelNum for the lineNum & segNum
//note that a spacing pixel is indicated by a pattern value of 255, these pixels will be filled in with the bgColor
CRGB StreamerSL::getNextColor(uint16_t lineNum, uint16_t segNum){

    //get the current pixel's location in the segment set
    pixelNum = segDrawUtils::getPixelNumFromLineNum(segmentSet, numLines, segNum, lineNum);
    if(nextPattern == 255){
        return segDrawUtils::getPixelColor(segmentSet, pixelNum, *bgColor, bgColorMode, segNum, lineNum);
    } else {
        nextColor = paletteUtilsPS::getPaletteColor(palette, nextPattern);
        return segDrawUtils::getPixelColor(segmentSet, pixelNum, nextColor, colorMode, segNum, lineNum);
    }
}

//Returns the blended color based on the nextColor in line
//Also records the color in the prevLineColors array for the next loop iteration
CRGB StreamerSL::getBlendedColor(CRGB nextColor, uint16_t segNum){
    //if the next pixel is the same color as the current one, we don't need to do any blending
    //otherwise, we need to get the blended color between the current and next pixels
    if(nextColor == prevLineColors[segNum]){
        colorOut = nextColor;
    } else {
        colorOut = colorUtilsPS::getCrossFadeColor(prevLineColors[segNum], nextColor, blendStep, fadeSteps);
    }
    //Store the color in the prevLineColors array since
    //the color of the current pixel is the target color of the next one
    prevLineColors[segNum] = nextColor; 
    return colorOut;
}

//Updates the effect, fading each line forward one step
//Runs accross all the lines, works out what their current and target colors are, 
//and fades them one step towards the target
//In the case of color modes where each pixel in the line is a different color, 
//each pixel's color is fetched and then faded
//To speed this up, since the pixels are fading towards the color of the next pixel in line
//we make the color of the next pixel the target of the current one
//so we only need to find one color for each pixel each cycle
//The previous colors are stored in the prevLineColors[] array, with one entry for each segment in the set
//Once the fade is finished, the whole cycle advances by one, and then the fades begin again
//The colors are re-fetched each cycle to account for color modes and palette blending
void StreamerSL::updateFade(){
    
    //We need the target color for the pixels in the first line before the beginning of the loop 
    //But prevLineColors[] only set at the end of each loop, so we need to pre-fill it for the first loop iteration
    //(note that we use numLinesLim as the 0th line, this is because to get the effect moving in the same
    //direction as the segments, we actually need to draw the lines in reverse, hence we start at the last line)
    nextPattern = patternUtilsPS::getPatternVal(pattern, cycleCount); //cycleCount is the next index of the pattern
    for(uint16_t j = 0; j < numSegs; j++){
        prevLineColors[j] = getNextColor(numLinesLim, j);
    }

    //For each line, we get its color and then blend it using the previous line's color
    for(uint16_t i = 0; i < numLines; i++){

        //the next color is the color of the next line
        nextPatternIndex = (i + cycleCount + 1);
        nextPattern = patternUtilsPS::getPatternVal(pattern, nextPatternIndex);

        //Based on the colorMode, lines will either be a soild color or not
        //For color modes where each line pixel is a different color, we walk over the line, blending each pixel in order
        //Otherwise, the lines are solid colors, so we only fetch and blend the line color once, then draw the whole line
        switch(colorMode){
            case 0: case 3: case 4: case 5: case 8: case 9: case 10:
            default: //For these modes the line is solid, so we get the color using the longest segment
                nextColor = getNextColor(i, longestSeg);
                colorOut = getBlendedColor(nextColor, longestSeg);
                //write the color out to all the leds in the segment line
                //(we used numLinesLim - i in place of just i to make the default line motion positive)
                segDrawUtils::drawSegLineSimple(segmentSet, numLinesLim - i, colorOut, 0);
                break;
            case 1: case 2: case 6: case 7:
                //For these modes each pixel may be different, so we need to blend each of them individually
                for(uint16_t j = 0; j < numSegs; j++){
                    //get the current pixel's color and location
                    //(we used numLinesLim - i in place of just i to make the default line motion positive)
                    //Note that getNextColor() also gets the pixelNum for the lineNum & segNum
                    nextColor = getNextColor(numLinesLim - i, j); 
                    colorOut = getBlendedColor(nextColor, j);
                    segDrawUtils::setPixelColor(segmentSet, pixelNum, colorOut, 0, j, i);
                }
                break;
        }

    }

    //each cycle we advance the blend step
    //if the blend step wraps back to zero, then a blend is finished
    //and we need to advance the cycleCount, so that all the streamer colors shift forwards
    blendStep = addmod8(blendStep, 1, fadeSteps); 
    if(blendStep == 0){
        cycleCount = addMod16PS( cycleCount, 1, patternLength );
    }

}

//update function without using fades
//For each line, we basically just get the color it's meant to be and write it out
//then advance the cycle by one so all the colors shift forwards
//(Note that unlike with updateFade() we get the colors for all the pixels in each line regardless of colorMode.
//This keeps the code more compact, and isn't a big deal b/c we aren't blending, so we don't need to do any 
//more caculations for each pixel than what the system would already be doing)
void StreamerSL::updateNoFade(){
    //For each line, set the pixel colors in it
    for(uint16_t i = 0; i < numLines; i++){

        //the next color is the color of the next line
        nextPatternIndex = i + cycleCount;
        nextPattern = patternUtilsPS::getPatternVal(pattern, nextPatternIndex);

        //For each segment pixel in the line, get its color (accouting for colorModes)
        //and write it out
        for(uint16_t j = 0; j < numSegs; j++){
            //get the current pixel's color and location
            //(we used numLinesLim - i in place of just i to make the default line motion positive)
            //Note that getNextColor() also gets the pixelNum for the lineNum & segNum
            nextColor = getNextColor(numLinesLim - i, j);
            segDrawUtils::setPixelColor(segmentSet, pixelNum, nextColor, 0, j, i); 
        }   
    }
    //No blending, so the color change after each step
    cycleCount = addMod16PS( cycleCount, 1, patternLength );
}
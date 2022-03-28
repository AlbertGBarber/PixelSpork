 #include "StreamerPS.h"

//constructor for using the passed in pattern and pallet for the streamer
StreamerPS::StreamerPS(SegmentSet &SegmentSet, patternPS *Pattern, palletPS *Pallet, CRGB BgColor, uint8_t FadeSteps, uint16_t Rate):
    segmentSet(SegmentSet), pattern(Pattern), pallet(Pallet), fadeSteps(FadeSteps)
    {    
        init(BgColor, Rate);
	}

//constructor for building the streamer pattern from the passed in pattern and the pallet, using the passed in colorLength and spacing
StreamerPS::StreamerPS(SegmentSet &SegmentSet, patternPS *Pattern, palletPS *Pallet, uint8_t ColorLength, uint8_t Spacing, CRGB BgColor, uint8_t FadeSteps, uint16_t Rate):
    segmentSet(SegmentSet), pallet(Pallet), fadeSteps(FadeSteps)
    {    
        setPatternAsPattern(Pattern, ColorLength, Spacing);
        init(BgColor, Rate);
	}

//constructor for building a streamer using all the colors in the passed in pallet, using the colorLength and spacing for each color
StreamerPS::StreamerPS(SegmentSet &SegmentSet, palletPS *Pallet, uint8_t ColorLength, uint8_t Spacing, CRGB BgColor, uint8_t FadeSteps, uint16_t Rate):
    segmentSet(SegmentSet), pallet(Pallet), fadeSteps(FadeSteps)
    {    
        setPalletAsPattern(ColorLength, Spacing);
        init(BgColor, Rate);
	}

//constructor for doing a single colored streamer, using colorLength and spacing
StreamerPS::StreamerPS(SegmentSet &SegmentSet, CRGB Color, uint8_t ColorLength, uint8_t Spacing, CRGB BgColor, uint8_t FadeSteps, uint16_t Rate):
    segmentSet(SegmentSet), fadeSteps(FadeSteps)
    {    
        palletTemp = palletUtilsPS::makeSingleColorpallet(Color);
        pallet = &palletTemp;
        setPalletAsPattern(ColorLength, Spacing);
        init(BgColor, Rate);
	}

//destructor
StreamerPS::~StreamerPS(){
    delete[] palletTemp.palletArr;
    delete[] patternTemp.patternArr;
}

//takes the passed in pattern and creates a pattern for the streamer
//using the passed in color length and spacing
//then sets this pattern to be the streamer pattern
//ex : inputPattern is {1, 2, 4} with color length 2, and 1 spacing
//the streamer pattern would be: {1, 1, 255, 2, 2, 255, 4, 4, 255}
void StreamerPS::setPatternAsPattern(patternPS *inputPattern, uint8_t colorLength, uint8_t spacing){
    uint8_t patternIndex;
    uint8_t repeatLength = (colorLength + spacing); //the total length taken up by a single color and spacing
    uint16_t patternLength = inputPattern->length;
    uint16_t totalPatternLength = patternLength * repeatLength; 
    //create new storage for the pattern array
    delete[] patternTemp.patternArr;
    uint8_t *pattern_arr = new uint8_t[totalPatternLength];

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

//sets the current pallet to be the streamer pattern (using all colors in the pallet)
//using the passed in colorLength and spacing
//ex: for pallet of lenth 3, and a colorLength of 2, and spacing of 1
//the final streamer pattern would be : {0, 0, 255, 1, 1, 255, 2, 2, 255}
void StreamerPS::setPalletAsPattern(uint8_t colorLength, uint8_t spacing){
    uint8_t repeatLength = (colorLength + spacing);
    uint8_t palletlength = pallet->length;
    uint16_t totalPatternLength = palletlength * repeatLength; //the total length taken up by a single color and spacing
    //create new storage for the pattern array
    delete[] patternTemp.patternArr;
    uint8_t *pattern_arr = new uint8_t[totalPatternLength];

    //for each color in the pallet, we fill in the color and spacing for the output pattern
    for(uint16_t i = 0; i < palletlength; i++){
        //for each color in the pallet we run over the length of the color and spacing
        //for the indexes up to color length, we set them as the current pallet index
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
void StreamerPS::reset(){
    blendStep = 0;
    cycleCount = 0;
}

//intilization of core variables and pointers
void StreamerPS::init(CRGB BgColor, uint16_t Rate){
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

//the main update function
//either calls updateFade, or updateNoFade depending on if fadeOn is true
void StreamerPS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        //both updateFade() and updateNoFade() need the current segment and pattern lengths,
        //so we'll do them here to reduce repetition
        numPixels = segmentSet.numActiveSegLeds;
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
//also fills in the pixelInfo struct to get the pixel's location
//note that a spacing pixel is indicated by a pattern value of 255, these pixels will be filled in with the bgColor
CRGB StreamerPS::getNextColor(uint16_t pixelNum){
    //nextPatternIndex is set before the call
    nextPattern = patternUtilsPS::getPatternVal(pattern, nextPatternIndex);
    if(nextPattern == 255){
        pixelInfo.color = *bgColor;
        segDrawUtils::getPixelColor(segmentSet, &pixelInfo, pixelInfo.color, bgColorMode, pixelNum);
    } else {
        pixelInfo.color = palletUtilsPS::getPalletColor(pallet, nextPattern);
        segDrawUtils::getPixelColor(segmentSet, &pixelInfo, pixelInfo.color, colorMode, pixelNum);
    }
    return pixelInfo.color;
}

//updates the effect, fading each pixel forward one step
//Runs accross all the pixels, works out what their current and target colors are, 
//and fades them one step towards the target
//To speed this up, since the pixels are fading towards the color of the next pixel in line
//we make the color of the next pixel the target of the current one
//so we only need to find one color for each pixel each cycle
//Once the fade is finished, the whole cycle advances by one, and then the fades begin again
//The colors are re-fetched each cycle to account for color modes and pallet blending
void StreamerPS::updateFade(){

    nextPattern = patternUtilsPS::getPatternVal(pattern, cycleCount);
    //we need the target color for the first pixel.
    //before be beginning of the loop since currentColor is only set at the end of each loop step
    currentColor = getNextColor(0);

    for(uint16_t i = 0; i < numPixels; i++){
        
        //the next color is the color of the next pixel in line
        nextPatternIndex = (i + cycleCount + 1);
        nextColor = getNextColor(i);

        //if the next pixel is the same color as the current one, we don't need to do any blending
        //otherwise, we need to get the blended color between the current and next pixels
        if(nextColor == currentColor){
            colorOut = nextColor;
        } else {
            colorOut = colorUtilsPS::getCrossFadeColor(currentColor, nextColor, blendStep, fadeSteps);
        }
        segDrawUtils::setPixelColor(segmentSet, pixelInfo.pixelLoc, colorOut, 0, pixelInfo.segNum, pixelInfo.lineNum);
        currentColor = nextColor; //the start color of the current pixel is the target color of the next one
    }

    //each cycle we advance the blend step
    //if the blend step wraps back to zero, then a blend is finished
    //and we need to advance the cycleCount, so that all the streamer colors shift forwards
    blendStep = addmod8(blendStep, 1, fadeSteps); //(blendStep + 1) % fadeSteps;
    if(blendStep == 0){
        cycleCount = addMod16PS( cycleCount, 1, patternLength ); //(cycleCount + 1) % patternLength;
    }

}

//update function without using fades
//For each pixel, we basically just get the color it's meant to be and write it out
//then advance the cycle by one so all the colors shift forwards
void StreamerPS::updateNoFade(){
    for(uint16_t i = 0; i < numPixels; i++){

        nextPatternIndex = i + cycleCount;
        nextColor = getNextColor(i);

        segDrawUtils::setPixelColor(segmentSet, pixelInfo.pixelLoc, nextColor, 0, pixelInfo.segNum, pixelInfo.lineNum);
    }

    cycleCount = addMod16PS( cycleCount, 1, patternLength );//(cycleCount + 1) % patternLength;

}
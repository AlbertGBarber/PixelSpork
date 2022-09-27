#include "SegWaves.h"

//constructor for using the passed in pattern and palette for the wave
SegWaves::SegWaves(SegmentSet &SegmentSet, patternPS *Pattern, palettePS *Palette, CRGB BgColor, uint8_t FadeSteps, bool Direct, uint16_t Rate):
    segmentSet(SegmentSet), pattern(Pattern), palette(Palette), fadeSteps(FadeSteps), direct(Direct)
    {    
        init(BgColor, Rate);
	}

//constructor for building the wave pattern from the passed in pattern and the palette, using the passed in waveThickness and spacing
//Passing a color length of 0 will set the wave thickness to 1 and the spacing such that there's only one wave on the segment set at once
SegWaves::SegWaves(SegmentSet &SegmentSet, patternPS *Pattern, palettePS *Palette, uint8_t WaveThickness, uint8_t Spacing, CRGB BgColor, uint8_t FadeSteps, bool Direct, uint16_t Rate):
    segmentSet(SegmentSet), palette(Palette), fadeSteps(FadeSteps), direct(Direct)
    {    
        //short cut for creating a single segment wave
        if(WaveThickness == 0){ 
            WaveThickness = 1;
            Spacing = segmentSet.numSegs;
        }
        setPatternAsPattern(Pattern, WaveThickness, Spacing);
        init(BgColor, Rate);
	}

//constructor for building a wave using all the colors in the passed in palette, using the waveThickness and spacing for each color
//Passing a color length of 0 will set the wave thickness to 1 and the spacing such that there's only one wave on the segment set at once
SegWaves::SegWaves(SegmentSet &SegmentSet, palettePS *Palette, uint8_t WaveThickness, uint8_t Spacing, CRGB BgColor, uint8_t FadeSteps, bool Direct, uint16_t Rate):
    segmentSet(SegmentSet), palette(Palette), fadeSteps(FadeSteps), direct(Direct)
    {    
        //short cut for creating a single segment wave
        if(WaveThickness == 0){
            WaveThickness = 1;
            Spacing = segmentSet.numSegs;
        }
        setPaletteAsPattern(WaveThickness, Spacing);
        init(BgColor, Rate);
	}

//constructor for doing a single colored wave, using waveThickness and spacing
//Passing a color length of 0 will set the wave thickness to 1 and the spacing such that there's only one wave on the segment set at once
SegWaves::SegWaves(SegmentSet &SegmentSet, CRGB Color, uint8_t WaveThickness, uint8_t Spacing, CRGB BgColor, uint8_t FadeSteps, bool Direct, uint16_t Rate):
    segmentSet(SegmentSet), fadeSteps(FadeSteps), direct(Direct)
    {    
        paletteTemp = paletteUtilsPS::makeSingleColorPalette(Color);
        palette = &paletteTemp;
        //short cut for creating a single segment wave
        if(WaveThickness == 0){
            WaveThickness = 1;
            Spacing = segmentSet.numSegs;
        }
        setPaletteAsPattern(WaveThickness, Spacing);
        init(BgColor, Rate);
	}

//constructor doing a rainbow based on the number of segments, there's no spacing for this mode
SegWaves::SegWaves(SegmentSet &SegmentSet,uint8_t FadeSteps, bool Direct, uint16_t Rate):
    segmentSet(SegmentSet), fadeSteps(FadeSteps), direct(Direct)
    {    
        //For the rainbow we create a pattern and palette of length equal to the number of segments
        //The pattern is just each segment number in order,
        //while the palette is filled with a rainbow spread across all the segments (one color for each segment)
        numSegs = segmentSet.numSegs;
        uint8_t *patternArr = new uint8_t[numSegs];
        CRGB *paletteArr = new CRGB[numSegs];
        for (uint16_t i = 0; i < numSegs; i++) {
            patternArr[i] = i;
            paletteArr[i] = colorUtilsPS::wheel( (i * 255 / numSegs), 0);
        }
        
        paletteTemp = {paletteArr, numSegs};
        palette = &paletteTemp;

        patternTemp = {patternArr, numSegs};
        pattern = &patternTemp;

        init(0, Rate);
	}

//destructor
SegWaves::~SegWaves(){
    delete[] paletteTemp.paletteArr;
    delete[] patternTemp.patternArr;
    delete[] segColors; 
}

//intilization of core variables and pointers
void SegWaves::init(CRGB BgColor, uint16_t Rate){
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

//resets the wave to it's original starting point
//Also recreates the random wave color tracking array
void SegWaves::reset(){
    blendStep = 0;
    cycleCount = 0;
    resetSegColors();
    initFillDone = false;
}

//Sets up the segColors array used for random colors
//Note that this deletes the existing array
//Only needs to be used if you change the number of segments in your segment set
void SegWaves::resetSegColors(){
    numSegs = segmentSet.numSegs;
    delete[] segColors; 
    segColors = new CRGB[numSegs + 1];
}

//creates a pattern so that there's only a single segment wave on the segment set at one time
//ie a wave of thickness 1, and a spacing equal to the number of segments, so there's only one 
//wave on the segment at once
void SegWaves::makeSingleWave(){
    setPaletteAsPattern(1, segmentSet.numSegs);
}

//takes the passed in pattern and creates a pattern for the wave
//using the passed in color length and spacing
//then sets this pattern to be the wave pattern
//ex : inputPattern is {1, 2, 4} with color length 2, and 1 spacing
//the wave pattern would be: {1, 1, 255, 2, 2, 255, 4, 4, 255}
void SegWaves::setPatternAsPattern(patternPS *inputPattern, uint8_t waveThickness, uint8_t spacing){
    uint8_t patternIndex;
    uint8_t repeatLength = (waveThickness + spacing); //the total length taken up by a single color and spacing
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
            if(j < waveThickness){
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

//sets the current palette to be the wave pattern (using all colors in the palette)
//using the passed in waveThickness and spacing
//ex: for palette of lenth 3, and a waveThickness of 2, and spacing of 1
//the final wave pattern would be : {0, 0, 255, 1, 1, 255, 2, 2, 255}
void SegWaves::setPaletteAsPattern(uint8_t waveThickness, uint8_t spacing){
    uint8_t repeatLength = (waveThickness + spacing);
    uint8_t palettelength = palette->length;
    uint16_t totalPatternLength = palettelength * repeatLength; //the total length taken up by a single color and spacing
    //create new storage for the pattern array
    delete[] patternTemp.patternArr;
    uint8_t *pattern_arr = new uint8_t[totalPatternLength];

    //for each color in the palette, we fill in the color and spacing for the output pattern
    for(uint16_t i = 0; i < palettelength; i++){
        //for each color in the palette we run over the length of the color and spacing
        //for the indexes up to color length, we set them as the current palette index
        //after that we set them as spacing (255)
        for(uint8_t j = 0; j < repeatLength; j++){
            if(j < waveThickness){
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

//the main update function
//either calls updateFade, or updateNoFade depending on if fadeOn is true
void SegWaves::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        //both updateFade() and updateNoFade() need the current segment and pattern lengths,
        //so we'll do them here to reduce repetition
        numSegs = segmentSet.numSegs;
        patternLength = pattern->length;

        //For the random modes we need to fill out the segColors array during the first update cycle
        //(so that it's not just blank)
        if(!initFillDone){
            initFill();
            initFillDone = true;
        }

        if(fadeOn){
            updateFade();
        } else {
            updateNoFade();
        }
        
        showCheckPS();
    }
}

//OVERALL NOTES:
//So this effect is set to work with random colors and also a pattern of colors
//These two concepts end up being opposed, so the effect is configured to either be doing
//random colors, or a fixed pattern of colors
//For the fixed pattern:
    //Whenever we write out a color we actively work out what it should be
    //We do this by checking what the next pattern value is, working out what color that is 
    //(accounting for color modes and if it should be a background or palette color)
    //and then finding the blended value between out current color and the next 
    //(We know our current color because it was the next color for the previous segment)
    //Whenever we finish a blend, we advance the cycle count, which advances what pattern step each segment is on
//For the random colors:
    //Random colors are introduced from the first segment and shift towards the last
    //They still use the pattern from the fixed pattern case, but the colors are set at random
    //So we can't find the colors on the fly, and instead we have to track what color each segment is
    //So we store an array of size numSegs+1 (segColors[]) to store the colors of each array
    //We use the array to get the current and target colors for blending
    //Whenever a blend is finished, we shift the array forward by one, copying the colors from the previous segment
    //While at the last segment we introduce a new random color
    //Note that the array is has one extra entry off the end of the segment set
    //This provides a color target for the final segment, so that as a wave enters it transistions onto
    //The segment rather than just appearing
//I haven't been able to get these two modes to play well together (especially when dealing with colorModes)
//So they work seperately within the effect
//Switching between them can cause jumps in colors
//Note that you can change the numSegs for fixed patterns, but not in random mode
//For random mode you need to reset so that the segColors array can be resized.

//Doesn't support color mode 1, 3, 6, 8
//returns the next output color for the wave, taking into account color modes
//note that a spacing segment is indicated by a pattern value of 255, these segments will be filled in with the bgColor
//For random modes, the colors are pulled from the segColors array
CRGB SegWaves::getNextColor(uint16_t segNumTemp){
    //nextPatternIndex is set before the call
    nextPattern = patternUtilsPS::getPatternVal(pattern, nextPatternIndex);
    pixelNum = segDrawUtils::getSegmentPixel(segmentSet, segNumTemp, 0);
    //lineNum = segDrawUtils::getLineNumFromPixelNum(segmentSet, 0, segNum);

    //If we're in random mode, the next color is taken from the segColors array
    //otherwise we work it out on the fly based on the pattern, palette, and colorMode
    if(randMode != 0){
        return segColors[segNumTemp];
    } else {
        if(nextPattern == 255){ //255 in the pattern marks a background color
            return segDrawUtils::getPixelColor(segmentSet, pixelNum, *bgColor, bgColorMode, segNumTemp, 0);
        } else {
            colorOut = paletteUtilsPS::getPaletteColor(palette, nextPattern);
            return segDrawUtils::getPixelColor(segmentSet, pixelNum, colorOut, colorMode, segNumTemp, 0);
        }
    }
}

//Only needed for random colors
//Fills out the segColors array so that it has something to start with
//Does this by simulating full blend cycles up to numSegs
//So that the segColors array is filled in
void SegWaves::initFill(){
    for(uint16_t i = 0; i < numSegs; i++){
        cycleCount = i;
        handleRandColors();
    }
    //if we're not doing random colors, we want to reset the 
    //cycleCount back to 0 so as to not disrupt the fixed pattern cycling
    if(randMode == 0){
        cycleCount = 0;
    }
}

//To handle fading between random colors, we need to track each color as it moves accross the segments 
//We store each segment's color in the segColors array, each time handleRandColors() is called
//We advance the color array forward by one, copying each color from the next segment in line
//(each color is shifted back one segment)
//We then pick a new color to enter the segment set, which is stored in segColors[numSegs]
//(one off the end of the segment set) This allows the color to transition onto the segments.
//Random colors use the following modes:
//    0: Colors will be choosen in order from the pattern (not random)
//    1: Colors will be choosen completely at random
//    2: Colors will be choosen randomly from the palette (allowing repeats)
//    3: Colors will be choosen at random from the palette,
//    but the same color won't be repeated in a row
//Color modes are supported, but the color is only set when it enters the segment set
//So they won't change as they shift across the segments
//If you want then to do that, don't use a random mode
void SegWaves::handleRandColors(){

    //Copy the colors from one segment to the next
    //Note that segColors is length numSegs + 1
    for(uint16_t i = 0; i < numSegs; i++){
        segColors[i] = segColors[i + 1];
    }

    //We need to pick a new color to transition onto the segments
    //To do this we first get some info about the final segment (where the color will enter)
    //and the next pattern value
    segNum = numSegs - 1;
    nextPattern = patternUtilsPS::getPatternVal(pattern, cycleCount);
    pixelNum = segDrawUtils::getSegmentPixel(segmentSet, segNum, 0);

    //Pick the new color
    //If it's background (indicated by a pattern value of 255), we pick a background color
    //Otherwise, pick a random color based on the randMode
    if(nextPattern == 255){
        segColors[numSegs] = segDrawUtils::getPixelColor(segmentSet, pixelNum, *bgColor, bgColorMode, segNum, 0);
        prevPattern = 255;
    } else if(prevPattern != nextPattern) {
        //If we're doing random colors, we still want to stick to the wave lengths in the pattern
        //but replace the next color with a random one 
        //So we only pick a new random color each time the next pattern is different from the previous one
        //since that indicates a new wave length
        //(we don't need to worry about the 255 spacing values, since they're caught further up in the if)
        prevPattern = nextPattern;
        if(randMode == 0){
            //Choose from the palette, this is mainly to keep the transition from random to fixed colors
            //smooth
            colorOut = paletteUtilsPS::getPaletteColor(palette, nextPattern);
        } else if(randMode == 1) {
            //choose a completely random color
            colorOut = colorUtilsPS::randColor();
        } else if(randMode == 2) {
            //choose a color randomly from the palette (can repeat)
            colorOut = paletteUtilsPS::getPaletteColor( palette, random8(palette->length) );
        } else {
            //choose a color randomly from the palette (making sure it's not the same as the current random color)
            colorOut = paletteUtilsPS::getShuffleIndex(palette, randColor);
        }

        //set the new random color into the segColors array
        segColors[numSegs] = segDrawUtils::getPixelColor(segmentSet, pixelNum, colorOut, colorMode, segNum, 0);
        randColor = segColors[numSegs];
            
    } else {
        //If the pattern is the same as the last one, then we want to keep using the same random color
        segColors[numSegs] = randColor;
    }  
}

//updates the effect, fading each segment forward one step
//Runs accross all the segments, works out what their current and target colors are, 
//and fades them one step towards the target
//To speed this up, since the segments are fading towards the color of the next segment in line
//we make the color of the next segment the target of the current one
//so we only need to find one color for each segment each cycle
//Once the fade is finished, the whole cycle advances by one, and then the fades begin again
//The colors are re-fetched each cycle to account for color modes and palette blending
void SegWaves::updateFade(){

    nextPatternIndex = cycleCount;
    //we need the target color for the first segment.
    //before be beginning of the loop since currentColor is only set at the end of each loop step
    currentColor = getNextColor(0);

    for(uint16_t i = 0; i < numSegs; i++){
        
        //the next color is the color of the next segment in line
        nextPatternIndex = (i + cycleCount + 1);
        nextColor = getNextColor( addMod16PS(i, 1, numSegs) );

        //In random modes, we need to provide the correct color target
        //which is stored in segColors[numSegs]
        //We need to do this because getNextColor() must be given a real segment value 
        //and won't accept numSegs
        if( randMode != 0 && i == (numSegs - 1) ){
           nextColor = segColors[numSegs]; 
        }

        //if the next segment is the same color as the current one, we don't need to do any blending
        //otherwise, we need to get the blended color between the current and next segments
        if(nextColor == currentColor){
            colorOut = nextColor;
        } else {
            colorOut = colorUtilsPS::getCrossFadeColor(currentColor, nextColor, blendStep, fadeSteps);
        }
        
        //depending on the direction, we reverse the output segment
        if(direct){
            segNum = i;
        } else {
            segNum = numSegs - i - 1;
        }

        segDrawUtils::fillSegColor(segmentSet, segNum, colorOut, 0);
        currentColor = nextColor; //the start color of the current segment is the target color of the next one
    }

    //each cycle we advance the blend step
    //if the blend step wraps back to zero, then a blend is finished
    //and we need to advance the cycleCount, so that all the wave colors shift forwards
    blendStep = addmod8(blendStep, 1, fadeSteps); 
    if(blendStep == 0){
        cycleCount = addMod16PS( cycleCount, 1, patternLength );
        //If the blend has finished, we need to advance the segColors array
        //and pick a new entering color 
        //(only used for random modes)
        handleRandColors();
    }
}

//update function without using fades
//For each segment, we basically just get the color it's meant to be and write it out
//then advance the cycle by one so all the colors shift forwards
void SegWaves::updateNoFade(){
    for(uint16_t i = 0; i < numSegs; i++){

        nextPatternIndex = i + cycleCount;
        nextColor = getNextColor( addMod16PS(i, 1, numSegs) );


        //In random modes, we need to provide the correct color target
        //which is stored in segColors[numSegs]
        //We need to do this because getNextColor() must be given a real segment value 
        //and won't accept numSegs
        if( randMode != 0 && i == (numSegs - 1) ){
           nextColor = segColors[numSegs]; 
        }

        //depending on the direction, we reverse the output segment
        if(direct){
            segNum = i;
        } else {
            segNum = numSegs - i - 1;
        }

        segDrawUtils::fillSegColor(segmentSet, segNum, nextColor, 0);
    }

    cycleCount = addMod16PS( cycleCount, 1, patternLength );
    //Pick new colors for random modes
    handleRandColors();
}
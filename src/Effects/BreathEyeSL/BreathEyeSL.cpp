#include "BreathEyeSL.h"

//consturctor for using a pattern and palette
BreathEyeSL::BreathEyeSL(SegmentSet &SegmentSet, patternPS *Pattern, palettePS *Palette, CRGB BgColor, uint16_t EyeHalfSize, bool Wrap, bool RandEyePos, uint8_t BreathFreq, uint16_t Rate):
    segmentSet(SegmentSet), pattern(Pattern), palette(Palette), eyeHalfSize(EyeHalfSize), wrap(Wrap), randEyePos(RandEyePos)
    {    
        init(BgColor, BreathFreq, Rate);
	}

//constructor for using palette as pattern
BreathEyeSL::BreathEyeSL(SegmentSet &SegmentSet, palettePS *Palette, CRGB BgColor, uint16_t EyeHalfSize, bool Wrap, bool RandEyePos, uint8_t BreathFreq, uint16_t Rate):
    segmentSet(SegmentSet), palette(Palette), eyeHalfSize(EyeHalfSize), wrap(Wrap), randEyePos(RandEyePos)
    {    
        setPaletteAsPattern();
        init(BgColor, BreathFreq, Rate);
	}

//constructor for a single color breath (pass in 0 as the color to trigger randMode 1, fully random)
//Note that maxBreath only included as a input for this constructor because doing partial fades 
//really only makes sense when using a single color
//With multiple colors in a pattern, the colors will jump if you don't do full fades
//Passing in maxBreath also forces the compiler to differenciate this constructor from the rainbow one below
//(CRGB's look like uint8_t's to the compiler)
BreathEyeSL::BreathEyeSL(SegmentSet &SegmentSet, CRGB color, CRGB BgColor, uint8_t MaxBreath, uint16_t EyeHalfSize, bool Wrap, bool RandEyePos, uint8_t BreathFreq, uint16_t Rate):
    segmentSet(SegmentSet), maxBreath(MaxBreath), eyeHalfSize(EyeHalfSize), wrap(Wrap), randEyePos(RandEyePos)
    {    
        if(color == CRGB{0,0,0}){
            randMode = 1; //set mode to 3 since we are doing a full random set of colors
        }

        paletteTemp = paletteUtilsPS::makeSingleColorPalette(color);
        palette = &paletteTemp;
        setPaletteAsPattern();
        init(BgColor, BreathFreq, Rate);
    }

//constructor for rainbow mode
BreathEyeSL::BreathEyeSL(SegmentSet &SegmentSet, CRGB BgColor, uint8_t RainbowRate, uint16_t EyeHalfSize, bool Wrap, bool RandEyePos, uint8_t BreathFreq, uint16_t Rate):
    segmentSet(SegmentSet), rainbowRate(RainbowRate), eyeHalfSize(EyeHalfSize), wrap(Wrap), randEyePos(RandEyePos)
    {    
        randMode = 4; //set mode to 4 for rainbow mode
       
        //we still make a palette with a single random color in case we switch randMode away from the rainbow mode
        paletteTemp = paletteUtilsPS::makeSingleColorPalette(colorUtilsPS::randColor());
        palette = &paletteTemp;
        setPaletteAsPattern();
        init(BgColor, BreathFreq, Rate);
	}

BreathEyeSL::~BreathEyeSL(){
    free(paletteTemp.paletteArr);
    free(patternTemp.patternArr);
}

//bind core class vars
void BreathEyeSL::init(CRGB BgColor, uint8_t BreathFreq, uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    //bind background color pointer
    bindBGColorPS();

    //bind the breath frequency
    breathFreqOrig = BreathFreq;
    breathFreq = &breathFreqOrig;
    
    //set a default center eye size
    //This helps make sure the core of the eye stays bright
    //The end effect this has depends on your setup and other settings
    //But 1/8 the eye size seemed to work well in my tests
    eyeCenterSize = eyeHalfSize / 8;
    
    eyeHalfSizeMax = eyeHalfSize;
    eyeHalf = eyeHalfSize;

    reset();
}

//resets all loop and color variables to start the effect from scratch
void BreathEyeSL::reset(){
    patternIndex = 0;
    palIndex = 0;
    hue = 0;

    //Set the eye center position
    //If it's not being choosen randonmly we default to the center of the segment set
    //Because it makes the most sense, and is probably what most people are looking for
    numLines = segmentSet.maxSegLength;
    if(randEyePos){
        eyePos = random16(numLines);
    } else {
        eyePos = numLines/2;
    }

    getNextColor();

    //blank out the segment set for before the first update
    segDrawUtils::fillSegSetColor(segmentSet, *bgColor, 0);
}

//sets the pattern to match the current palette
//ie for a palette length 5, the pattern would be 
//{0, 1, 2, 3, 4}
void BreathEyeSL::setPaletteAsPattern(){
    patternTemp = generalUtilsPS::setPaletteAsPattern(palette);
    pattern = &patternTemp;
}

//Gets the next breath color based on the pattern, palette, and randMode
//Also advances the patternIndex to track where we are in the pattern
//randModes are:
//  0: Colors will be choosen in order from the pattern (not random)
//  1: Colors will be choosen completely at random
//  2: Colors will be choosen randomly from the palette (not allowing repeats)
//  3: Colors will be choosen randomly from the palette (allowing repeats)
//  4: Colors will be from the rainbow (the hue is offset by rainbowRate each time a color is choosen)
void BreathEyeSL::getNextColor(){   
    //set the next color depending on the mode
    switch (randMode) {
        case 0: //Colors will be choosen in order from the pattern (not random)
        default:
            palIndex = patternUtilsPS::getPatternVal( pattern, patternIndex );
            breathColor = paletteUtilsPS::getPaletteColor( palette, palIndex );
            break;
        case 1: //Colors will be choosen completely at random
            breathColor = colorUtilsPS::randColor();
            break;
        case 2: //Colors will be choosen randomly from the palette (not allowing repeats)
            //Note that we use the palIndex to keep track of what palette color we're doing
            //So that we don't choose it twice
            palIndex = patternUtilsPS::getShuffleIndex( pattern, palIndex );
            breathColor = paletteUtilsPS::getPaletteColor( palette, palIndex );
            break;
        case 3://Colors will be choosen randomly from the palette (allowing repeats)
            patternIndex = random16(pattern->length);
            palIndex = patternUtilsPS::getPatternVal( pattern, patternIndex );
            breathColor = paletteUtilsPS::getPaletteColor( palette, palIndex );
            break;
        case 4: //Colors will be from the rainbow (the hue is offset by rainbowRate each time a color is choosen)
            hue += rainbowRate;
            breathColor = CHSV(hue, rainbowSat, rainbowVal);
            break;
    }

    //Advance the patternIndex for when we pick the next color
    patternIndex = addMod16PS(patternIndex, 1, pattern->length);
}

//Updates the effect.
//Each update we get a breath (brightness) value based on some math (taken from Irdkir's original code)
//We then apply the brightness to our current breath color and output it to the whole segment set
//To switch to the next color we need to catch when a full cycle has finished (faded in and out fully)
//To do this we check the brighness value, if it passes a threshold, we know it has faded, and can set the next color
void BreathEyeSL::update(){
    currentTime = millis();

    if( (currentTime - prevTime) >= *rate ) {
        prevTime = currentTime;

        numLines = segmentSet.maxSegLength;

        bWave = triwave8(currentTime / *breathFreq); 
        breath = ease8InOutCubic(bWave); 
        breath = map8(breath, minBreath, maxBreath); //map from 0, 255 to min and maxBreath;

        colorOut = colorUtilsPS::getCrossFadeColor(breathColor, *bgColor, breath);

        //We only need to walk over half the eye, mirroring it as we do so
        //To do this we start at the end of the eye and work towards the center
        //caculating the brightness of each pair of pixels, producing a smooth gradient to the center of the eye
        //We walk backwards so that we set the center eye pixels last so they are not overwritten if you have unequal segments
        //The center eye pixels use the colorOut directly, and are not dimmed (except as part of the breath)
        //This helps give the eye a better look imo by keeping the center bright
        for(int32_t i = eyeHalf - 1; i >= 0; i--){
            
            //get the color for the eye pixels
            //The center pixels are only dimmed according to the breath, which keeps them bright
            //and looks better imo
            if(i <= eyeCenterSize){
                dimColor = colorOut;
            } else {
                dimColor = getEyeColor(i);
            }

            //Get line number for the pair of eye pixels we're setting, accounting for wrapping
            //we have a forward and reverse pixel, both of which are offset by i from the eye center position, 
            //but in opposite directions
            //(note that for the reverse, we add numLines so it's never negative, this doesn't change the mod result)
            lineNumRev = mod16PS( eyePos - i + numLines, numLines);
            lineNumFor = mod16PS( eyePos + i, numLines);

            //If we're not wrapping we only want to draw the eye up to each end of the segment set
            //The reverse line position is always less than the eyePos, unless it's wrapped back to the start
            //So if we're not wrapping we ignore any lines that are greater than eyePos
            //The opposite is true for the forward line number
            if( wrap || (lineNumRev < eyePos) ){
                segDrawUtils::drawSegLineSimple(segmentSet, lineNumRev, dimColor, 0);
            }

            if( wrap || (lineNumFor > eyePos) ){
                segDrawUtils::drawSegLineSimple(segmentSet, lineNumFor, dimColor, 0);
            }

        }

        //If we've reached the end of the current fade we need to choose the next color to fade to
        //To do this we check the fade brightness. Since we are sampling the brightness from a wave
        //output, we don't know the exact top value of the wave (it will skip values depending on freqency)
        //So we check that it's past a certain point, pick a color and then prevent any new colors from being choosen
        //(we want the point to be close to the peak, so that we choose a new color when the fade is almost finished)
        //From experimentation a difference of 5 from the peak (maxBreath) seems to work well. The highest possible peak is 255.
        //We lock the color because the wave may stay above the difference for multiple cycles, but we only want to set the color once.
        //After the color is set, we wait until the brightness is below the limit to unlock the color again, ready for the next cycle.
        breathEndVal = maxBreath - breathEndOffset; 
        if(!lockColor && breath >= breathEndVal){
            lockColor = true;
            getNextColor();

            //get a new random eye location
            if(randEyePos){
                eyePos = random16(numLines);
            }

            if(randEyeSize){
                eyeHalf = random16(eyeHalfSize, eyeHalfSizeMax);
            } else {
                eyeHalf = eyeHalfSize;
            }

            //fill in the segement set with background if needed
            if(fillBG){
                segDrawUtils::fillSegSetColor(segmentSet, *bgColor, 0);
            }

        } else if(lockColor && breath < breathEndVal){
            lockColor = false;
        }

        showCheckPS();
    }
}

//Returns a non-linearly dimmed eye color based on the lineNum and the total eye size
//Uses the same formula as ParticlesPS.h
//Helps create a sharper fade than just using getCrossFadeColor()
//The maximum brightness is scaled by dimPow
//dimPow 0 will produce a normal linear gradient, but for a sharper dimming we can dial the bightness down
//dimpow of 80 gives a good effect
CRGB BreathEyeSL::getEyeColor(uint16_t lineNum){
    
    //alternate dimming formula for more aggressive dimming (set dimPow between -127 and 127)
    //basically subtracts a term from the step value to simiulate an increase in dimming
    //the subtraction term decreases as we get closer to totalSteps, so we don't bug out and over run
    ratio = ( lineNum * (255 - dimPow) ) / (eyeHalf + 1) + dimPow; 

    return colorUtilsPS::getCrossFadeColor(colorOut, *bgColor, ratio);
}
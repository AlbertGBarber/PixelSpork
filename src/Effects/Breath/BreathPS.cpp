#include "BreathPS.h"

//constructor for using a pattern and palette
BreathPS::BreathPS(SegmentSet &SegSet, patternPS &Pattern, palettePS &Palette, CRGB BgColor, uint8_t BreathFreq, uint16_t Rate):
    SegSet(SegSet), pattern(&Pattern), palette(&Palette)
    {    
        init(BgColor, BreathFreq, Rate);
	}

//constructor for using palette as pattern
BreathPS::BreathPS(SegmentSet &SegSet, palettePS &Palette, CRGB BgColor, uint8_t BreathFreq, uint16_t Rate):
    SegSet(SegSet), palette(&Palette)
    {    
        setPaletteAsPattern();
        init(BgColor, BreathFreq, Rate);
	}

//constructor for a single color breath (pass in 0 as the color to trigger randMode 1, fully random)
//Note that maxBreath only included as a input for this constructor because doing partial fades 
//really only makes sense when using a single color
//With multiple colors in a pattern, the colors will jump if you don't do full fades
//Passing in maxBreath also forces the compiler to differentiate this constructor from the rainbow one below
//(CRGB's look like uint8_t's to the compiler)
BreathPS::BreathPS(SegmentSet &SegSet, CRGB color, CRGB BgColor, uint8_t MaxBreath, uint8_t BreathFreq, uint16_t Rate):
    SegSet(SegSet), maxBreath(MaxBreath)
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
BreathPS::BreathPS(SegmentSet &SegSet, CRGB BgColor, uint8_t RainbowRate, uint8_t BreathFreq, uint16_t Rate):
    SegSet(SegSet), rainbowRate(RainbowRate)
    {    
        randMode = 4; //set mode to 4 for rainbow mode
       
        //we still make a palette with a single random color in case we switch randMode away from the rainbow mode
        paletteTemp = paletteUtilsPS::makeSingleColorPalette(colorUtilsPS::randColor());
        palette = &paletteTemp;
        setPaletteAsPattern();
        init(BgColor, BreathFreq, Rate);
	}

BreathPS::~BreathPS(){
    free(paletteTemp.paletteArr);
    free(patternTemp.patternArr);
}

//bind core class vars
void BreathPS::init(CRGB BgColor, uint8_t BreathFreq, uint16_t Rate){
    //bind the rate and SegmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    //bind background color pointer
    bindBGColorPS();

    //bind the breath frequency
    breathFreqOrig = BreathFreq;
    breathFreq = &breathFreqOrig;

    reset();
}

//resets all loop and color variables to start the effect from scratch
void BreathPS::reset(){
    patternIndex = 0;
    palIndex = 0;
    hue = 0;
    breathCount = 0;
    getNextColor();
}

//sets the pattern to match the current palette
//ie for a palette length 5, the pattern would be 
//{0, 1, 2, 3, 4}
void BreathPS::setPaletteAsPattern(){
    patternTemp = generalUtilsPS::setPaletteAsPattern(*palette);
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
void BreathPS::getNextColor(){   
    //set the next color depending on the mode
    switch (randMode) {
        case 0: //Colors will be choosen in order from the pattern (not random)
        default:
            palIndex = patternUtilsPS::getPatternVal( *pattern, patternIndex );
            breathColor = paletteUtilsPS::getPaletteColor( *palette, palIndex );
            break;
        case 1: //Colors will be choosen completely at random
            breathColor = colorUtilsPS::randColor();
            break;
        case 2: //Colors will be choosen randomly from the palette (not allowing repeats)
            //Note that we use the palIndex to keep track of what palette color we're doing
            //So that we don't choose it twice
            palIndex = patternUtilsPS::getShuffleVal( *pattern, palIndex );
            breathColor = paletteUtilsPS::getPaletteColor( *palette, palIndex );
            break;
        case 3://Colors will be choosen randomly from the palette (allowing repeats)
            patternIndex = random16(pattern->length);
            palIndex = patternUtilsPS::getPatternVal( *pattern, patternIndex );
            breathColor = paletteUtilsPS::getPaletteColor( *palette, palIndex );
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
//To do this we check the brightness value, if it passes a threshold, we know it has faded, and can set the next color
void BreathPS::update(){
    currentTime = millis();

    if( (currentTime - prevTime) >= *rate ) {
        prevTime = currentTime;

        bWave = triwave8(currentTime / *breathFreq); 
        breath = ease8InOutCubic(bWave); 
        breath = map8(breath, minBreath, maxBreath); //map from 0, 255 to min and maxBreath;

        colorOut = colorUtilsPS::getCrossFadeColor(breathColor, *bgColor, breath);
        segDrawUtils::fillSegSetColor(SegSet, colorOut, 0);

        //If we've reached the end of the current fade we need to choose the next color to fade to
        //To do this we check the fade brightness. Since we are sampling the brightness from a wave
        //output, we don't know the exact top value of the wave (it will skip values depending on frequency)
        //So we check that it's past a certain point, pick a color and then prevent any new colors from being choosen
        //(we want the point to be close to the peak, so that we choose a new color when the fade is almost finished)
        //From experimentation a difference of 5 from the peak (maxBreath) seems to work well. The highest possible peak is 255.
        //We lock the color because the wave may stay above the difference for multiple cycles, but we only want to set the color once.
        //After the color is set, we wait until the brightness is below the limit to unlock the color again, ready for the next cycle.
        breathEndVal = maxBreath - breathEndOffset; 
        if(!lockColor && breath >= breathEndVal){
            breathCount++;
            getNextColor();
            lockColor = true;
        } else if(lockColor && breath < breathEndVal){
            lockColor = false;
        }

        showCheckPS();
    }
}
#include "PrideWPalSL.h"

//constructor for rainbow mode
PrideWPalSL::PrideWPalSL(SegmentSet &SegmentSet, bool BriDirect, bool RandomBriInc, uint16_t Rate):
    segmentSet(SegmentSet), briDirect(BriDirect)
    {   
        prideMode = true; 
        //we make a random palette so we get gradSteps correctly (needed for program, but not used for rainbow)
        //and just in case rainbow mode is turned off
        paletteTemp = paletteUtilsPS::makeRandomPalette(4);
        palette = &paletteTemp;
        init(RandomBriInc, Rate);
	}

//constructor for palette input
PrideWPalSL::PrideWPalSL(SegmentSet &SegmentSet, palettePS &Palette, bool BriDirect, bool RandomBriInc, uint16_t Rate):
    segmentSet(SegmentSet), palette(&Palette), briDirect(BriDirect)
    {    
        prideMode = false; 
        init(RandomBriInc, Rate);
	}

//constructor for making a random palette
PrideWPalSL::PrideWPalSL(SegmentSet &SegmentSet, uint8_t numColors, bool BriDirect, bool RandomBriInc, uint16_t Rate):
    segmentSet(SegmentSet), briDirect(BriDirect)
    {    
        prideMode = false; 
        paletteTemp = paletteUtilsPS::makeRandomPalette(numColors);
        palette = &paletteTemp;
        init(RandomBriInc, Rate);
	}

//constructor with inputs for all main variables
PrideWPalSL::PrideWPalSL(SegmentSet &SegmentSet, palettePS &Palette, bool BriDirect, uint8_t GradLength, 
                                            uint8_t BrightDepthMin, uint8_t BrightDepthMax, uint16_t BriThetaFreq, 
                                            uint8_t BriThetaInc16Min, uint8_t BriThetaInc16Max, uint8_t HueChangeMin, 
                                            uint8_t HueChangeMax, uint16_t Rate):
    segmentSet(SegmentSet), palette(&Palette), briDirect(BriDirect), gradLength(GradLength), brightDepthMin(BrightDepthMin), 
    brightDepthMax(BrightDepthMax), briThetaFreq(BriThetaFreq), briThetaInc16Min(BriThetaInc16Min), 
    briThetaInc16Max(BriThetaInc16Max), hueChangeMin(HueChangeMin), hueChangeMax(HueChangeMax)
    {
        prideMode = false; 
        init(false, Rate);
    }

PrideWPalSL::~PrideWPalSL(){
    free(paletteTemp.paletteArr);
}

//Initializes core variables and also picks random values for briThetaInc16 and briThetaFreq if randomBriInc is true
//The random values are picked from ranges hand picked by me to offer a good variation in the effect
//while not being too extreme
void PrideWPalSL::init(bool RandomBriInc, uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    if(RandomBriInc){
        randomizeBriInc( 15, 25, 30, 40 );
        randomizeBriFreq( 200, 500 );
    }
}

//Sets briThetaInc16Min and briThetaInc16Max to be a random value between the passed in ranges
//!!Make sure that briThetaMinMax < briThetaMaxMin
//See Inputs Guide for info on briThetaInc16 ranges
void PrideWPalSL::randomizeBriInc(uint8_t briThetaMinMin, uint8_t briThetaMinMax, 
                                            uint8_t briThetaMaxMin, uint8_t briThetaMaxMax ){
    briThetaInc16Min = random8(briThetaMinMin, briThetaMinMax);
    briThetaInc16Max = random8(briThetaMaxMin, briThetaMaxMax);
}

//Sets the briThetaFreq to be a random value between the passed in ranges
//See Inputs Guide for info on briThetaFreq
void PrideWPalSL::randomizeBriFreq( uint16_t briFreqMin, uint16_t briFreqMax ){
    briThetaFreq = random16(briFreqMin, briFreqMax);
}

//Updates the effect
//The effect consists of two parts: A set of brightness waves and a set of color waves
//These are both take from the original Pride2015 by Mark Kriegsman
//In the original code both the waves were applied in the same direction along the strip
//In this effect I've done the same, but expanded the waves to be along whole segment lines
//So the segment set has a set of shifting color waves that run across it
//I've also made it so you can reverse the direction of the brightness waves
//I've also combined the code from Mark's colorwaves and pride2015 into one function
//since they are so similar.
//I don't know exactly how all the waves work in the effect, but overall:
//Each Update cycle we:
    //Update various brightnes and hue values from their waves
    //Then, for each line, we fetch a color using the brightness and hue
    //(while also incrementing brightness and color as we go)
    //We then color all the pixels on the line
void PrideWPalSL::update(){
    currentTime = millis();
    deltams = currentTime - prevTime;
    if( ( deltams ) >= *rate ) {
        prevTime = currentTime;

        //update various wave values
        //for those with input variables, their purpose is described in the Inputs Guide in the .h file
        sat8 = beatsin88(87, 220, 250);    
        brightdepth = beatsin88(341, brightDepthMin, brightDepthMax); 
        brightnessthetainc16 = beatsin88(briThetaFreq, (briThetaInc16Min * 256), (briThetaInc16Max * 256));
        msmultiplier = beatsin88(147, 23, 60);

        hue16 = sHue16;
        hueinc16 = beatsin88(113, 1, 3000);
            
        sPseudotime += deltams * msmultiplier;
        sHue16 += deltams * beatsin88(400, hueChangeMin, hueChangeMax);
        brightnesstheta16 = sPseudotime;

        //sets the brightness waves to either move forward or backward across the segments
        briDirectMult = briDirect - !briDirect; //1 or -1
        
        //fetch some core vars
        //we re-fetch these in case the segment set or palette has changed
        numSegs = segmentSet.numSegs;
        numLines = segmentSet.maxSegLength;
        numSteps = gradLength * palette->length;

        //For each segment line do the following:
        for (uint16_t i = 0; i < numLines; i++) {
            
            //update the brightness wave for each line
            brightnesstheta16 += briDirectMult * brightnessthetainc16;
            b16 = sin16(brightnesstheta16) + 32768;

            bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
            bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
            bri8 += (255 - brightdepth);

            //get the next color hue
            hue16 += hueinc16;
            hue8 = hue16 / 256;

            //If we're not drawing rainbows we need to get a color from the palette
            //other wise the hue is constrained to 256
            if(!prideMode){
                h16_128 = hue16 >> 7;
                if(h16_128 & 0x100) {
                    hue8 = 255 - (h16_128 >> 1);
                } else {
                    hue8 = h16_128 >> 1;
                }
                //get the blended color from the palette mapped into numSteps based on the hue
                index = scale16by8( numSteps, hue8 );
                newColor = paletteUtilsPS::getPaletteGradColor(*palette, index, 0, numSteps, gradLength);
                nscale8x3(newColor.r, newColor.g, newColor.b, bri8);
            } else{
                newColor = CHSV(hue8, sat8, bri8);
            }
            
            //reverse the line number so that the effect moves positivly along the strip
            lineNum = numLines - i - 1;

            for (uint16_t j = 0; j < numSegs; j++) {
                //get the physical pixel location based on the line and seg numbers
                pixelNum = segDrawUtils::getPixelNumFromLineNum(segmentSet, numLines, j, lineNum);
                nblend(segmentSet.leds[pixelNum], newColor, 128);
                
                //Need to check to dim the pixel color manually
                //b/c we're not calling setPixelColor directly
                segDrawUtils::handleBri(segmentSet, pixelNum);
            }
        }          
        showCheckPS();
    } 
}

//Oringinal code for non-seg line version
/*
//Updates the effect
//Basically we shift the colors and brightness based on some wave values
//adjusting the values for each pixel
//I am not totall clear on how everything works
//If you really need to know you'll have to track down Mark Kriegsman
void PrideWPalPS::update() {
    currentTime = millis();
    deltams = currentTime - prevTime;

    if( ( deltams ) >= *rate ) {
        prevTime = currentTime;

        //Adjust the blend ratio 
        //(based on Marks original code values from Pride2015 and colorwaves + my own testing)
        //64 looks better with rainbows, but 128 looks better with most palettes
        if(prideMode) {
            blendRatio = 64;
        } else {
            blendRatio = 128;
        }

        //update various wave values
        sat8 = beatsin88(87, 220, 250);
        brightdepth = beatsin88(341, 96, 224);
        brightnessthetainc16 = beatsin88(203, (25 * 256), (40 * 256));
        msmultiplier = beatsin88(147, 23, 60);

        hue16 = sHue16;
        hueinc16 = beatsin88(113, 1, 3000);

        sPseudotime += deltams * msmultiplier;
        sHue16 += deltams * beatsin88(400, 5, 9);
        brightnesstheta16 = sPseudotime;

        //sets the brightness waves to either move forward or backward across the segments
        briDirectMult = briDirect - !briDirect; //1 or -1

        //do the subtraction here so we don't need to do it each loop
        //@getSegmentPixel
        //The loop limit is adjusted up by one
        numActiveLeds = segmentSet.numActiveSegLeds - 1;
        //re-caculate the gradLength incase the palette changed
        numSteps = gradLength * palette->length;

        for (uint16_t i = 0; i <= numActiveLeds; i++) {
            
            //update the brightness wave for each pixel
            brightnesstheta16 += briDirectMult * brightnessthetainc16;
            b16 = sin16(brightnesstheta16) + 32768;

            bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
            bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
            bri8 += (255 - brightdepth);

            //get the next color hue
            hue16 += hueinc16;
            hue8 = hue16 / 256;

            //If we're not drawing rainbows we need to get a color from the palette
            //other wise the hue is constrained to 256
            if(!prideMode){
                h16_128 = hue16 >> 7;
                if(h16_128 & 0x100) {
                    hue8 = 255 - (h16_128 >> 1);
                } else {
                    hue8 = h16_128 >> 1;
                }
                //get the blended color from the palette mapped into numSteps based on the hue
                index = scale16by8( numSteps, hue8 );
                newColor = paletteUtilsPS::getPaletteGradColor(palette, index, 0, numSteps, gradLength);
                nscale8x3(newColor.r, newColor.g, newColor.b, bri8);
            } else{
                newColor = CHSV(hue8, sat8, bri8);
            }

            //get the physical pixel location based on the line and seg numbers
            pixelnumber = segDrawUtils::getSegmentPixel(segmentSet, numActiveLeds - i);

            nblend(segmentSet.leds[pixelnumber], newColor, blendRatio);
        }
        showCheckPS();
    }
}
*/
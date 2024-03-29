#include "PrideWPalSL2.h"

//constructor for rainbow mode
PrideWPalSL2::PrideWPalSL2(SegmentSetPS &SegSet, bool Direct, bool RandomBriInc, uint16_t Rate)
    : direct(Direct)  //
{
    prideMode = true;
    //we make a random palette so we get gradSteps correctly (needed for program, but not used for rainbow)
    //and just in case rainbow mode is turned off
    paletteTemp = paletteUtilsPS::makeRandomPalette(4);
    palette = &paletteTemp;
    init(RandomBriInc, SegSet, Rate);
}

//constructor for palette input
PrideWPalSL2::PrideWPalSL2(SegmentSetPS &SegSet, palettePS &Palette, bool Direct, bool RandomBriInc, uint16_t Rate)
    : palette(&Palette), direct(Direct)  //
{
    prideMode = false;
    init(RandomBriInc, SegSet, Rate);
}

//constructor for making a random palette
PrideWPalSL2::PrideWPalSL2(SegmentSetPS &SegSet, uint8_t numColors, bool Direct, bool RandomBriInc, uint16_t Rate)
    : direct(Direct)  //
{
    prideMode = false;
    paletteTemp = paletteUtilsPS::makeRandomPalette(numColors);
    palette = &paletteTemp;
    init(RandomBriInc, SegSet, Rate);
}

//constructor with inputs for all main variables
PrideWPalSL2::PrideWPalSL2(SegmentSetPS &SegSet, palettePS &Palette, bool Direct, uint8_t GradLength,
                           uint8_t BrightDepthMin, uint8_t BrightDepthMax, uint16_t BriThetaFreq,
                           uint8_t BriThetaInc16Min, uint8_t BriThetaInc16Max, uint8_t HueChangeMin,
                           uint8_t HueChangeMax, uint16_t Rate)
    : palette(&Palette), direct(Direct), gradLength(GradLength), brightDepthMin(BrightDepthMin),
      brightDepthMax(BrightDepthMax), briThetaFreq(BriThetaFreq), briThetaInc16Min(BriThetaInc16Min),
      briThetaInc16Max(BriThetaInc16Max), hueChangeMin(HueChangeMin), hueChangeMax(HueChangeMax)  //
{
    prideMode = false;
    init(false, SegSet, Rate);
}

PrideWPalSL2::~PrideWPalSL2() {
    free(paletteTemp.paletteArr);
}

//Initializes core variables and also picks random values for briThetaInc16 and briThetaFreq if randomBriInc is true
//The random values are picked from ranges hand picked by me to offer a good variation in the effect
//while not being too extreme
void PrideWPalSL2::init(bool RandomBriInc, SegmentSetPS &SegSet, uint16_t Rate) {
    //bind the rate and segSet pointer vars since they are inherited from BaseEffectPS
    bindSegSetPtrPS();
    bindClassRatesPS();

    if( RandomBriInc ) {
        randomizeBriInc(15, 25, 35, 60);
        randomizeBriFreq(250, 600);
    }
}

//Sets briThetaInc16Min and briThetaInc16Max to be a random value between the passed in ranges
//!!Make sure that briThetaMinMax < briThetaMaxMin
//See Inputs Guide for info on briThetaInc16 ranges
void PrideWPalSL2::randomizeBriInc(uint8_t briThetaMinMin, uint8_t briThetaMinMax,
                                   uint8_t briThetaMaxMin, uint8_t briThetaMaxMax) {
    briThetaInc16Min = random8(briThetaMinMin, briThetaMinMax);
    briThetaInc16Max = random8(briThetaMaxMin, briThetaMaxMax);
}

//Sets the briThetaFreq to be a random value between the passed in ranges
//See Inputs Guide for info on briThetaFreq
void PrideWPalSL2::randomizeBriFreq(uint16_t briFreqMin, uint16_t briFreqMax) {
    briThetaFreq = random16(briFreqMin, briFreqMax);
}

/* Updates the effect
The effect consists of two parts: A set of brightness waves and a set of color waves
These are both take from the original Pride2015 by Mark Kriegsman
In the original sketch both the waves were applied in the same direction along the strip
However, for segment sets I've applied the color waves radially and the brightness waves along segment lines
So the brightness varies along each segment, while the color radiates out segment by segment
This closely mimics Jason Coons Pride code running on his Fibonacci spirals
(although I think this is a coincidence since the codes work differently)
I've also combined the code from Mark's colorWaves and pride2015 into one function
since they are so similar.
I don't know exactly how all the waves work in the effect, but overall:
Each Update cycle we:
    Update various brightness and hue values from their waves
    Then for each segment we fetch a color based on the segment number and hue
    This color will be applied to the whole segment, but have it's brightness modified
        For each segment we walk across each of the segment lines (pixels)
        and vary the brightness according to various wave values, 
        modifying the segment color with the brightness
Each cycle the colors are shifted outwards (or inwards) while 
the brightness wave shifts clockwise round the segment set */
void PrideWPalSL2::update() {
    currentTime = millis();
    
    deltaTime = currentTime - prevTime;
    if( (deltaTime) >= *rate ) {
        prevTime = currentTime;

        //update various wave values
        //for those with input variables, their purpose is described in the Inputs Guide in the .h file
        sat8 = beatsin88(87, 220, 250);
        brightDepth = beatsin88(342, brightDepthMin, brightDepthMax);
        brightnessThetaInc16 = beatsin88(briThetaFreq, (briThetaInc16Min * 256), (briThetaInc16Max * 256));
        msMultiplier = beatsin88(147, 23, 60);

        hue16 = sHue16;
        hueInc16 = beatsin88(113, 1, 3000);

        sPseudoTime += deltaTime * msMultiplier;
        sHue16 += deltaTime * beatsin88(400, hueChangeMin, hueChangeMax);
        brightnessTheta16 = sPseudoTime;

        //fetch some core vars
        //we re-fetch these in case the segment set or palette has changed
        numSegs = segSet->numSegs;
        numLines = segSet->numLines;
        numSteps = gradLength * palette->length;

        //For each segment we first get a color for the whole segment
        //then for each segment line in the segment we modify the color by the brightness
        //to create the brightness waves
        for( uint16_t i = 0; i < numSegs; i++ ) {
            //get the next color hue
            hue16 += hueInc16;
            hue8 = hue16 / 256;

            //if we're not drawing rainbows we need to get a color from the palette
            //other wise the hue is constrained to 256
            if( !prideMode ) {
                //dunno exactly what this code is doing
                h16_128 = hue16 >> 7;
                if( h16_128 & 0x100 ) {
                    hue8 = 255 - (h16_128 >> 1);
                } else {
                    hue8 = h16_128 >> 1;
                }
                //returns the blended color from the palette mapped into numSteps
                index = scale16by8(numSteps, hue8);
                newColor = paletteUtilsPS::getPaletteGradColor(*palette, index, 0, numSteps, gradLength);
            }

            //for each of the lines in the segment, get a new brightness value
            //and use it to modify the segment color
            for( uint16_t j = 0; j < numLines; j++ ) {

                //update the brightness wave for each line
                brightnessTheta16 += brightnessThetaInc16;
                b16 = sin16(brightnessTheta16) + 32768;

                bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
                bri8 = (uint32_t)(((uint32_t)bri16) * brightDepth) / 65536;
                bri8 += (255 - brightDepth);

                //if we're in pride mode the brightness is a direct input into the CHSV
                //otherwise we need to modify our existing color
                if( prideMode ) {
                    colorOut = CHSV(hue8, sat8, bri8);
                } else {
                    //the segment color is copied into colorOut
                    //because nscale8x3 directly modifies the color
                    colorOut = newColor;
                    nscale8x3(colorOut.r, colorOut.g, colorOut.b, bri8);
                }

                //for shifting the colors either inwards or outwards
                if( direct ) {
                    segOut = i;
                } else {
                    segOut = numSegs - i - 1;
                }

                //get the physical pixel location based on the line and seg numbers
                pixelNum = segDrawUtils::getPixelNumFromLineNum(*segSet, segOut, numLines - j - 1);

                //output a the color
                nblend(segSet->leds[pixelNum], colorOut, 128);

                //Need to check to dim the pixel color manually
                //b/c we're not calling setPixelColor directly
                segDrawUtils::handleBri(*segSet, pixelNum);
            }
        }
        showCheckPS();
    }
}
#include "PrideWPalPS.h"

//constructor for rainbow mode
PrideWPalPS::PrideWPalPS(SegmentSet& SegmentSet, bool BriDirect, uint16_t Rate): 
    segmentSet(SegmentSet), briDirect(BriDirect)
    {
        prideMode = true;
        //we make a random pallet so we get gradSteps correctly (needed for program, but not used for rainbow)
        //and just in case rainbow mode is turned off
        palletTemp = palletUtilsPS::makeRandomPallet(4);
        pallet = &palletTemp;
        init(Rate);
    }

//constructor for pallet input
PrideWPalPS::PrideWPalPS(SegmentSet &SegmentSet, palletPS *Pallet, bool BriDirect, uint16_t Rate):
    segmentSet(SegmentSet), pallet(Pallet), briDirect(BriDirect)
    {    
        prideMode = false; 
        init(Rate);
	}

//constructor for making a random pallet
PrideWPalPS::PrideWPalPS(SegmentSet &SegmentSet, uint8_t numColors, bool BriDirect, uint16_t Rate):
    segmentSet(SegmentSet), briDirect(BriDirect)
    {    
        prideMode = false;
        init(Rate);
        palletTemp = palletUtilsPS::makeRandomPallet(numColors);
        pallet = &palletTemp;
	}

PrideWPalPS::~PrideWPalPS(){
    delete[] palletTemp.palletArr;
}

//initializes core variables
void PrideWPalPS::init(uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
}

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
        //64 looks better with rainbows, but 128 looks better with most pallets
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
        //re-caculate the gradLength incase the pallet changed
        numSteps = gradLength * pallet->length;

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

            //If we're not drawing rainbows we need to get a color from the pallet
            //other wise the hue is constrained to 256
            if(!prideMode){
                h16_128 = hue16 >> 7;
                if(h16_128 & 0x100) {
                    hue8 = 255 - (h16_128 >> 1);
                } else {
                    hue8 = h16_128 >> 1;
                }
                //get the blended color from the pallet mapped into numSteps based on the hue
                index = scale16by8( numSteps, hue8 );
                newColor = palletUtilsPS::getPalletGradColor(pallet, index, 0, numSteps, gradLength);
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
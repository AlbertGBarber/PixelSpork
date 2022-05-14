#ifndef PacificaPS_h
#define PacificaPS_h

//TODO: -- Add lava pallet as an option?

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

//
//  "Pacifica"
//  Gentle, blue-green ocean waves.
//  By Mark Kriegsman and Mary Corey March.
//  For Dan.
//

//An effect based on the pacific code here: https://github.com/FastLED/FastLED/blob/master/examples/Pacifica/Pacifica.ino
//A gentle effect of blue-green ocean waves rolling across the strip
//The effect runs as is; there's no customization
//Almost all of the code is directly copied from the original linked above
//If you have any questions about how it works, please direct them to Mark Kriegsman

//recommend brightness > 120 and rate > 10ms
//For low brightness values I recommend turing off dithering using
//FastLED.setDither(0);

//The effect is a bit computationally heavy

//Example calls: 
    //PacificaPS(mainSegments, 30);
    //That's it

//Functions:
    //update() -- updates the effect 
class PacificaPS : public EffectBasePS {
    public:
        PacificaPS(SegmentSet &SegmentSet, uint16_t Rate);  

        SegmentSet 
            &segmentSet; 
        
        uint8_t 
            //Produces a total blend length of 240 for the whole pallet, matches the original code
            numSteps = 240 / SIZE(pallet1_arr);
        
        CRGB 
            bgColorOrig,
           *bgColor; //bgColor is a pointer so it can be tied to an external variable if needed (such as a pallet color)
        
        //I've translated the original Pacifica pallets into my pallets
        //These produce a very slightly different blend of colors, but it still looks good
        //I've left the original pallets as comments below, should you want to use them
        CRGB 
            pallet1_arr[6] = { CRGB(0, 5, 7), CRGB(0, 3, 11), CRGB(0, 2, 16), CRGB(0, 0, 25), CRGB(20, 85, 75), CRGB(40, 170, 80) },
            //pallet1_arr[16] =  { CRGB(0, 5, 7),  CRGB(0, 4, 9),  CRGB(0, 3, 11),   CRGB(0, 3, 13), 
            //                     CRGB(0, 2, 16), CRGB(0, 2, 18), CRGB(0, 1, 20),   CRGB(0, 1, 23), 
            //                     CRGB(0, 0, 25), CRGB(0, 0, 28), CRGB(0, 0, 38),   CRGB(0, 0, 49), 
            //                     CRGB(0, 0, 59), CRGB(0, 0, 70), CRGB(20, 85, 75), CRGB(40, 170, 80) },

            pallet2_arr[6] = { CRGB(0, 5, 7), CRGB(0, 3, 11), CRGB(0, 2, 16), CRGB(0, 0, 25), CRGB(12, 95, 82), CRGB(25, 190, 95) },
            //pallet2_arr[16] =  { CRGB(0, 5, 7),  CRGB(0, 4, 9),  CRGB(0, 3, 11),   CRGB(0, 3, 13), 
            //                     CRGB(0, 2, 16), CRGB(0, 2, 18), CRGB(0, 1, 20),   CRGB(0, 1, 23), 
            //                     CRGB(0, 0, 25), CRGB(0, 0, 28), CRGB(0, 0, 38),   CRGB(0, 0, 49), 
            //                     CRGB(0, 0, 59), CRGB(0, 0, 70), CRGB(12, 95, 82), CRGB(25, 190, 95) },
                  
            pallet3_arr[6] = { CRGB(0, 2, 8), CRGB(0, 5, 20), CRGB(0, 8, 32), CRGB(0, 14, 57), CRGB(16, 64, 191), CRGB(32, 96, 255) };
            //pallet3_arr[16] = { CRGB(0, 2, 8),    CRGB(0, 3, 14),   CRGB(0, 5, 20),    CRGB(0, 6, 26), 
            //                    CRGB(0, 8, 32),   CRGB(0, 9, 39),   CRGB(0, 11, 45),   CRGB(0, 12, 51), 
            //                    CRGB(0, 14, 57),  CRGB(0, 16, 64),  CRGB(0, 20, 80),   CRGB(0, 24, 96), 
            //                    CRGB(0, 28, 112), CRGB(0, 32, 128), CRGB(16, 64, 191), CRGB(32, 96, 255) };
        
        //For lava colors
        //To use these you also need to adjust the background and the deepenColors function
        //CRGB 
            //pallet1_arr[6] = { CRGB(7, 0, 0), CRGB(11, 0, 0), CRGB(16, 0, 0), CRGB(25, 0, 0), CRGB(80, 6, 3), CRGB(120, 15, 5) },
            //pallet2_arr[6] = { CRGB(7, 0, 0), CRGB(11, 0, 0), CRGB(16, 0, 0), CRGB(25, 0, 0), CRGB(110, 15, 6), CRGB(160, 30, 10) },
            //pallet3_arr[6] = { CRGB(8, 0, 0), CRGB(20, 0, 0), CRGB(32, 0, 0), CRGB(57, 0, 0), CRGB(191, 95, 11), CRGB(184, 122, 24) };

        palletPS
            pacificaPallet1 = {pallet1_arr, SIZE(pallet1_arr)},
            pacificaPallet2 = {pallet2_arr, SIZE(pallet2_arr)},
            pacificaPallet3 = {pallet3_arr, SIZE(pallet3_arr)};
        
        void 
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0,
            deltams,
            deltams1,
            deltams2,
            deltams21;
        
        uint8_t
            numSegs,
            basethreshold,
            wave,
            threshold,
            lightLvl,
            overage,
            overage2;

        uint16_t 
            totBlendLength,
            totSegLen,
            pixelNum,
            sCIStart1,
            sCIStart2,
            sCIStart3, 
            sCIStart4,
            speedfactor1,
            speedfactor2,
            ci,
            waveangle,
            wavescale_half,
            s16,
            cs,
            index,
            sindex16;
        
        CRGB
            colorOut;
        
        void 
            doOneLayer( palletPS *pallet, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff),
            addWhitecaps(),
            deepenColors();
};

#endif
#ifndef RandomColorsPS_h
#define RandomColorsPS_h

#include "SegmentFiles.h"
#include "palletFiles.h"

#include "Effects/EffectBasePS.h"

//an empty effect, copy it to use as a base to bulid effects
class RandomColorsPS : public EffectBasePS {
    public:
        RandomColorsPS(SegmentSet &SegmentSet,  palletPS Pallet, uint16_t NumPixels, CRGB BgColor, uint8_t totalSteps, uint16_t Rate);  
        
        uint16_t 
            numPixels;

        CRGB 
            bgColor;

        bool 
            fillBG = false;
        
        uint8_t 
            colorMode = 0,
            bgColorMode = 0;

        uint8_t 
            totalSteps;

        SegmentSet 
            &segmentSet; 
        
        palletPS
            pallet;
        
        void 
            setTotalSteps(uint8_t newTotalSteps),
            setPallet(palletPS newPallet),
            reset(),
            setNumPixels(uint16_t newNumPixels),
            initPixelArrays(),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        uint8_t 
            totalSteps_ = 0,
            arraySteps;

        uint16_t
            **ledArray = nullptr;

        bool
            startUpDone = false;
        
        CRGB
            **colorIndexArr = nullptr,
            color;

        pixelInfoPS
            pixelInfo{0, 0, 0, 0};
        
        void 
            incrementPixelArrays(),
            pickColor(uint16_t pixelNum);
};

#endif
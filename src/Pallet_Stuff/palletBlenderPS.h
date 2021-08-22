#ifndef palletBlenderPS_h
#define palletBlenderPS_h

#include "SegmentFiles.h"
#include "palletPS.h"
#include "palletUtilsPS.h"
#include "Effects/EffectBasePS.h"

#if !defined(max)
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

//takes two pallets, a start and and end one, and gives you a pallet that is blended from the start towards the end
//the pallet is blended at the passed in rate (ms) in the passed in totalSteps steps
//update() updates the pallet (assuming enought time has passed)
//the resulting pallet can be accessed as "blendPallet", it will be the length of the longest of the passed in pallets
//once the end pallet is reached blendEnd is flagged as true
//the passed in start and end pallets are not modified by this process
//You may additionally set the looped or randomize options:
    //looped will reset the blend once it has ended, switching the start and end pallets
    //so that we blend back to the original start pallet, looping forever

    //randomize will randomize the end pallet
    //note that this will modify the end pallet, so make sure you aren't using it elsewhere!
//by using both looped and randomize, you can constantly blend to randomized pallets

//you may reuse class instances by calling one of the reset functions, and passing in new colors
class palletBlenderPS : public EffectBasePS {
    public:
        palletBlenderPS(palletPS &StartPallet, palletPS &EndPallet, uint8_t TotalSteps, uint16_t Rate);
        
        void
            reset(palletPS &StartPallet, palletPS &EndPallet), //resets just the colors (also starts the blend again)
            reset(palletPS &StartPallet, palletPS &EndPallet, uint8_t TotalSteps, uint16_t Rate), //resets all vars
            reset(), //resets the loop vars, restarting the blend from the beginning
            update();

        uint8_t
            step,
            totalSteps;
        
        bool
            looped = false,
            randomize = false,
            blendEnd;
        
        palletPS
            blendPallet;
    
    private:
        
        CRGB
           *blendPallet_arr;
        
        unsigned long
            currentTime,
            prevTime = 0;

        palletPS
            &startPallet,
            &endPallet;
};

#endif
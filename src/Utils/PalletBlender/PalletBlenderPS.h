#ifndef PalletBlenderPS_h
#define PalletBlenderPS_h

#include "PalletFiles.h"
#include "Effects/EffectBasePS.h"
#include "Utils/MathUtilsPS.h"

//TODO:
//-- Add new blend version that lets you exclude some indexes from blending 
//   (for randomizing when you want a const color), forced same length pallets, excluding colors at end?

//Takes two pallets, a start and and end one, and gives you a pallet that is blended over time from the start towards the end
//the pallet is blended at the passed in rate (ms) in the passed in totalSteps steps
//the resulting pallet can be accessed as "blendPallet", it will be the length of the longest of the passed in pallets
//The blend can be looped (so it repeats), or just set to happen once
//A hold time can be set, that will pause the loop once a blend is finished
//While looping, the pallets can be set to be randomized, which will blend to a random pallet each time

//pass blendPallet to your effects to use the blended pallet 

//The PalletBlenderPS update rate is a pointer, and can be bound externally, like in other effects

//Example calls: 
    //PalletBlenderPS(startPallet, endPallet, false, 50, 100);
    //Blends from the start pallet to the end pallet once (no looping), using 50 steps, with 100ms between each step

//Constructor Inputs:
    //startPallet -- The pallet the blendPallet will start as
    //endPallet -- The pallet that the blendPallet will end as
    //Looped -- set true, it will reset the blend once it has ended, switching the start and end pallets
    //TotalSteps (max 255) -- The total number of steps taken to blend between the pallets
    //Rate -- The update rate of the blend (ms)

//Functions:
    //reset(StartPallet, EndPallet) -- Restarts the blend with a new start/end pallet, with the same steps and update rate
    //reset(StartPallet, EndPallet, TotalSteps, Rate) -- Restarts the blend with all new vars
    //reset() -- restarts the blend (all settings and pallets stay the same)
    //setupBlendPallet(blendPalletLength) -- //creates a blend pallet, used only by PalletCycle, not for general use!
    //update() -- updates the effect

//Other Settings:
    //holdTime (default 0) -- Sets a time (ms) that the blendPallet will be pause at after finishing a transiton before starting the next
    //                        Only relevant if looped is true
    //randomize (default false) -- //randomize will randomize the end pallet note that this will permanently modify the end pallet
    //                               so make sure you aren't using it elsewhere!
    //                               Combine this with looped, to produce constantly changing pallets

//Flags:
    //blendEnd -- Set when the blend has ended, causes the hold to start
    //holdActive -- Set when the class is in the holding time after transitioning between pallets
    //              Is reset to false once the hold time has passed

//Notes:
//The passed in start and end pallets are not modified by the blend unless randomize is active

//If you have set an external variable pointing to one of the colors in the blendPallet (such as a bgColor for an effect)
//be warned that the blandPallet is allocated on the fly using new
//It is only re-allocated if it's size needs to change for a new blend 
//(ie new start/end pallets are set that don't have the same max size as the previous pair)
//So if you do set new pallets, either make sure they have the same max size a the previous pair,
//or re-bind you variable's pointer

class PalletBlenderPS : public EffectBasePS {
    public:
        PalletBlenderPS(palletPS *StartPallet, palletPS *EndPallet, bool looped, uint8_t TotalSteps, uint16_t Rate);
        
        ~PalletBlenderPS();
        
        void
            reset(palletPS *StartPallet, palletPS *EndPallet), //resets just the colors (also starts the blend again)
            reset(palletPS *StartPallet, palletPS *EndPallet, uint8_t TotalSteps, uint16_t Rate), //resets all vars
            reset(), //resets the loop vars, restarting the blend from the beginning
            setupBlendPallet(uint8_t blendPalletLength), //used only by PalletCycle, not for general use!
            update();

        uint8_t
            step, //the current step, mainly for refrence, not intended to be manually set
            totalSteps; //The total number of steps taken to blend between the pallets, you can change this on the fly
        
        uint16_t 
            holdTime = 0;
        
        bool
            looped,
            randomize = false,
            holdActive = false,
            blendEnd = false;
        
        palletPS
            blendPallet; //the output pallet from the blend
        
        palletPS
            *startPallet,
            *endPallet;
    
    private:
        unsigned long
            currentTime,
            holdStartTime = 0,
            prevTime = 0;

        CRGB
           *blendPallet_arr;
        
        uint8_t 
            blendPalletLength = 0;
};

#endif
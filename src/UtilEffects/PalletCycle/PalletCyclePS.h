#ifndef PalletCyclePS_h
#define PalletCyclePS_h

#include "Effects/EffectBasePS.h"
#include "UtilEffects/PalletBlender/PalletBlenderPS.h"

//NEEDS TESTING!!!!

//TODO:
//-- Add a shuffle function, so the next pallet is choosen at random
//-- Add direction controls?

//struct used to hold a pallet array and it's length in one spot
//also provides code for getting pallet objects
struct palletSetPS {
    palletPS **palletArr;
    uint8_t length;

    palletPS *getPallet(uint8_t index){
        return palletArr[index % length];
    };
};

//Takes an array of pallets and blends from one pallet to the next
//the pallets are blended at the passed in rate (ms) in the passed in totalSteps steps
//the resulting pallet can be accessed as "cyclePallet", it will be the length of the longest of the pallets in the array
//The blend can be looped (so it repeats), or just set to happen once
//A hold time can be set, that will pause the the cycle once a blend has finished before starting the next
//While looping, the pallets can be set to be randomized, which will blend to a random pallet each time

//pass cyclePallet to your effects to use the blended pallet 

//The PalletCyclePS update rate is a pointer, and can be bound externally, like in other effects

//Uses an instance of palletBlenderPS to do the blends. 
//The instance is public and can be accessed as "PB"

//The hold time and totalSteps are variables of the PalletBlenderPS instance
//so they use setter functions as shown below

//Example calls: 
    //to declare pallet array, use palletSet struct (above):
    //palletPS *palletArr[] = { &pallet1, &pallet2, etc};
    //palletSetPS palletSet = {palletArr, SIZE(palletArr)};

    //to declare a palletCycle
    //PalletCyclePS(*palletSet, true, 50, 80);
    //Blends between each pallet in the array in order, looping back to the first pallet at the end
    //each blend takes 50 steps, with 80ms between each step

//Constructor Inputs:
    //*palletSet -- The pointer to the set of pallets used for the blends
    //Looped -- set true, it will reset the blend once it has ended, cycling back to the start pallet
    //TotalSteps (max 255) -- The total number of steps taken to blend between the pallets
    //Rate -- The update rate of the blend (ms)

//Functions:
    //reset(*newPalletSet) -- Restarts the blend with a new pallet set, with the same steps and update rate
    //reset() -- Restarts the blend (all settings and pallets stay the same)
    //setTotalSteps(newTotalSteps) -- changes the total number of steps used in the blends (set in the PB instance, see PalletBlenderPS)
    //getTotalSteps() -- Returns the number of steps being used for the blend (set in the PB instance, see PalletBlenderPS)
    //setHoldTime(newHoldTime) -- Changes the pause between blends (set in the PB instance, see PalletBlenderPS)
    //update() -- updates the effect

//Other Settings:
    //randomize (default false) -- //Will randomize the each pallet in the cycle, every cycle
    //                               note that this will permanently modify pallets
    //                               so make sure you aren't using them elsewhere!
    //                               Combine this with looped, to produce constantly changing pallets

//Flags:
    //cycleEnd -- Set when we've blended through all the pallets in the pallet array 

//Notes:
//To make sure this works well with various effects, we manually set the length of the blendPallet to be the same each cycle
//by pre-determining the longest pallet in the pallet set and setting the blendPallet's length to match
//(the length usually would change depending on the lengths of the start and end pallets. which might change each cycle)
//This should be ok because pallets wrap. So the blendPallet might have some repeated colors, but will still
//be the correct blend overall. 

//This produces a pallet with a consistent length, that should play nice will all effects

//The downside is that you're always doing the maximum number of blend calculations even if your pallets are much smaller than the max
//for example, we have a 5, 2, and 3 length pallet in the set
//The maximum length is 5, so our cycle/blendPallet will be length 5, even when we're blending between the 2 and 3 length pallets
//The caculations for the 3 and 2 blends will be done 5 times, instead of the 3 if we setup the blend normally

//Most pallets aren't very long, so it shouldn't be a huge issue, but something to be aware of!

//the PalletBlendPS instance  (PB) is public, but don't mess with it unless you know what you're doing
class PalletCyclePS : public EffectBasePS {
    public:
        PalletCyclePS(palletSetPS *PalletSet, bool Looped, uint8_t TotalSteps, uint16_t Rate);  

        ~PalletCyclePS();
        
        uint8_t
            nextIndex,
            currentIndex = 0,
            maxPalletLength = 0,
            getTotalSteps();

        bool 
            cycleEnd = false,
            randomize = false,
            looped = false;
        
        palletSetPS
            *palletSet;
                                 
        palletPS
            *cyclePallet; //ouput pallet
        
        PalletBlenderPS
            *PB; //PalletBlenderPS instance

        void 
            reset(),
            reset(palletSetPS *newPalletSet),
            setTotalSteps(uint8_t newTotalSteps),
            setHoldTime(uint16_t newHoldTime),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        uint8_t
            palletLength;
};

#endif
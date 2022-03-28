#ifndef SinglePalletCyclePS_h
#define SinglePalletCyclePS_h

//NOT TESTED!!!

//TODO
// -- Add cycle limit

#include "Effects/EffectBasePS.h"
#include "UtilEffects/PalletBlender/PalletBlenderPS.h"

//Takes a single pallet and blends between it's own colors depending on the blend mode
//for example, you could shift all of the pallet's colors forward by one
//the pallets are blended at the passed in rate (ms) in the passed in totalSteps steps
//the resulting pallet can be accessed as "cyclePallet", it's length will vary depending on the mode
//The blend is always looped, so it will repeat indefinitly 
//A hold time can be set, that will pause the the cycle once a blend has finished before starting the next
//Modes can be changed mid-cycle
//Some modes have a diection setting (see below)

//The input pallet is never modified by the blends, and can be changed at will

//pass cyclePallet to your effects to use the blended pallet 

//The PalletCyclePS update rate is a pointer, and can be bound externally, like in other effects

//Uses an instance of palletBlenderPS to do the blends. 
//The instance is public and can be accessed as "PB"

//The hold time and totalSteps are variables of the PalletBlenderPS instance
//so they use setter functions as shown below

//Examples below use a pallet of  {blue, red, green}
//Blend Modes:
    //0 -- Cycles the entire pallet by foward/backward by one step each cycle. 
    //     ie for direct = true, {blue, red, green} => {green, blue, red} => {red, green, blue} , etc
    //1 -- Like mode 0, but a random color is inserted at the beginning (end if direct is false) each cycle, (initially starts as pallet)
    //     ie for direct = true, {rand1, red, green} => {rand4, rand1, red} => {rand5, rand4, rand1}, etc
    //2 -- shuffles the pallet each cycle
    //     ie {blue, red, green} could go to {red, blue, green}, or {blue, green, red}, etc 
    //     direct has no effect
    //3 -- Makes the pallet length 1, cycles through each color of the pallet
    //     ie for direct = true and pallet {blue, red, green} it will be {blue} => {red} => {green} => {blue}, etc
    //4 -- Same as mode 3, but chooses the next color randomly from the pallet (will not be the current color)
    //5 -- Same as mode 3, but the next color is choosen completely randomly (not from the pallet)

//Example calls: 
    //SinglePalletCyclePS(pallet, 0, 50, 80);
    //Blends the pallet using blend mode 0
    //each blend takes 50 steps, with 80ms between each step

//Constructor Inputs:
    //*pallet -- The pointer to the set of pallet used in the blend
    //BlendMode -- The mode used to blend the pallet (see blend modes above). You should be able to change this on the fly
    //TotalSteps (max 255) -- The total number of steps taken to blend between the pallets
    //Rate -- The update rate of the blend (ms)

//Functions:
    //switchPallet() -- Adjusts the current start/end pallets used for the current blend. Should only be called if you manually changed the pallet
    //setPallet(*pallet) -- Restarts the blend with a new pallet, with the same steps and update rate, and mode
    //reset() -- Restarts the blend (all settings and pallets stay the same)
    //setTotalSteps(newTotalSteps) -- changes the total number of steps used in the blends (set in the PB instance, see PalletBlenderPS)
    //getTotalSteps() -- Returns the number of steps being used for the blend (set in the PB instance, see PalletBlenderPS)
    //setHoldTime(newHoldTime) -- Changes the pause between blends (set in the PB instance, see PalletBlenderPS)
    //update() -- updates the effect

//Other Settings:
    //direct (default true)-- direction setting for some blendModes, will either cycle the pallet backwards or forwards

//Public Vars:
   //*palletOrig -- The pointer to the input pallet (as supplied when you created the class instance)
   //               You can change it by calling setPallet(), but this resets the cycle
   //               To avoid reseting, you can change it manually, but you may need to call switchPallet()
   //cyclePallet -- The output pallet for the blend, length is set depending on mode (see notes below for more)
   //current/nextPallet -- //starting/end pallet for the current blend
   //*PB -- The PalletBlendPS instance, public for reference 

//Notes:
//If you switch modes, the output pallet may change length, either from 1 to the input pallet length or vise versa
//Be aware that this may break some effects, if they don't adjust for the pallet length mid-cycle. 
//This is also true if you change the input pallet, depending on the mode

//the PalletBlendPS instance  (PB) is public, but don't mess with it unless you know what you're doing

class SinglePalletCyclePS : public EffectBasePS {
    public:
        SinglePalletCyclePS(palletPS *Pallet, uint8_t BlendMode, uint8_t TotalSteps, uint16_t Rate);

        ~SinglePalletCyclePS();
        
        uint8_t
            blendMode,
            cycleNum = 0, //the current cycle of the blend we're on (max value of pallet length), for reference only
            getTotalSteps();

        CRGB 
            *palletColorArr1, //storage for the start/end pallet colors, for reference
            *palletColorArr2;
        
        bool 
            direct = true;
                                 
        palletPS
            *palletOrig,
            cyclePallet; //ouput pallet
        
        palletPS
            currentPallet, //starting pallet for the current blend
            nextPallet; //ending pallet for the current blend
        
        PalletBlenderPS
            *PB; //PalletBlenderPS instance

        void 
            switchPallet(),
            setPallet(palletPS *pallet),
            setTotalSteps(uint8_t newTotalSteps),
            setHoldTime(uint16_t newHoldTime),
            reset(),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        int8_t 
            stepDirect;
        
        uint8_t 
            palletLength = 0,
            prevMode = 0;
        
        uint16_t
            currentIndex;
};

#endif
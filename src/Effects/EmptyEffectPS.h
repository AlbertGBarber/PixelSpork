#ifndef EmptyEffectPS_h
#define EmptyEffectPS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
//#include "MathUtils/mathUtilsPS.h"

/*
An empty effect, copy it to use as a base to build effects

<<Effect Comment Sections>>
Effect Description:
Inputs Guide / Notes:
Example calls: 
Constructor Inputs:
Functions:
Other Settings:
Reference Vars:
Flags:

*/
class EmptyEffectPS : public EffectBasePS {
    public:
        EmptyEffectPS(SegmentSet &SegSet, uint8_t AnArg, uint16_t Rate);  
        
        uint8_t anArg; //sample effect variable
        //use for background color (if needed)
        //use BgColor in constructor
        //CRGB 
            //bgColorOrig,
            //*bgColor = nullptr; //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)
        
        //It is common to have structs containing an array pointer and the length of the array
        //This makes passing the array around easier.
        //Palettes are an example of this kind of struct, a typical effect definition is below:
            //*palette is the pointer to the palette used in the effect (either passed in, or created randomly)
            //Many effects need to be able to generate a palette manually (to support randomly palettes,
            //or creating one for a single color)
            //To store generated palettes we use paletteTemp (which *palette is then pointed to in a constructor)
            //These palettes will be free()'d when the effect is destructed.
            //To prevent destruct errors, we need to initialize any pointers in the struct to nullptr, as is done below.
        /*palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0}; //Must init structs w/ pointers set to null for safety
        */

        void 
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
};

#endif
#ifndef CrossFadeCyclePS_h
#define CrossFadeCyclePS_h

#include "FastLED.h"
#include "SegmentFiles.h"
#include "palletFiles.h"

#include "Effects/EffectBasePS.h"

//Cross fades the entire segmentSet from one solid color to the next
//following either a pattern and pallet, a pallet alone, or using random colors
//each fade is done in a specified number of steps, at a specified rate
//fades continue until the speficied number of fades is reached, or indefinitly if the flag is set
//(passing 0 for the number of fades sets the infinite flag)
//once the fade number is reached, the done flag will be set
//see constructors below for inputs

//Constructor options
    //Using a Pattern and Pallet:
    //A pattern is a 1-d array of pallet indexes ie {0, 1, 3, 6, 7} 
    //the pattern is faded through in order, wrapping at the end
    //so the above example would fade from the first(0) color in the pallet to the second, to the fourth, etc

    //Using just a Pallet
    //Like the previous option, but the pallet is also used as the pattern
    //so the colors will fade in the order they are in the pallet array

    //Random
    //Fade colors will be choosen at random

//Modes ( not set in constructor, set using mode variable ):
    //Mode 0: (default), cycles through the pattern in order
    //Mode 1: Chooses the next color randomly from the current pattern (like it shuffles)
    //Mode 2: Chooses colors completely randomly (is set by the corrosponding constuctor)

//setPattern(), setPallet(), and setPalletAsPattern() to adjust patterns/pallets after initialization
//reset() restarts the pattern

class CrossFadeCyclePS : public EffectBasePS {
    public:
        //Constructor for pattern and pallet
        CrossFadeCyclePS(SegmentSet &SegmentSet, uint8_t *Pattern, uint8_t PatternLength, palletPS pallet, uint16_t NumFades, uint8_t Steps, uint16_t Rate);
        //Constructor for pallet as the pattern
        CrossFadeCyclePS(SegmentSet &SegmentSet, palletPS Pallet, uint16_t NumFades, uint8_t Steps, uint16_t Rate);  
        //Constructor for random colors
        CrossFadeCyclePS(SegmentSet &SegmentSet, uint16_t NumFades, uint8_t Steps, uint16_t Rate);
        
        uint8_t
            steps, //total steps per fade
            mode = 0, //see description above
            patternLength, //length of the pattern array, use SIZE(pattern)
            *pattern;
        
        uint16_t 
            numFades; //total number of fades to do
        
        bool
            infinite = false, //for ignoring the done flag
            done = false; //flag for if we've done the total number of fades

        SegmentSet 
            &segmentSet; 

        palletPS
            pallet;
        
        void 
            setPattern(uint8_t *newPattern, uint8_t newPatternLength),
            setPallet(palletPS newPallet),
            setPalletAsPattern(palletPS newPallet),
            reset(void),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        CRGB 
            startColor, //the color we are fading from
            nextColor; //the color we are fading to
        
        uint8_t
            currentIndex = 0, //the index of the pattern that we are currently fading away from, mainly used for shuffle()
            currentStep = 0, //current step of the current fade
            shuffleIndex();
        
        uint16_t 
            fadeCount = 0; //how many total fades we've done
};

#endif
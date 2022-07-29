#ifndef CrossFadeCyclePS_h
#define CrossFadeCyclePS_h

//TODO:
    //Add direction setting?
    //Add hang time?

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

/* 
Cross fades the entire segmentSet from one solid color to the next
following either a pattern and pallet, a pallet alone, or using random colors
each fade is done in a specified number of steps, at a specified rate
fades continue until the speficied number of fades is reached, or indefinitly if the flag is set
(passing 0 for the number of fades sets the infinite flag)
once the fade number is reached, the done flag will be set
see constructors below for inputs

Example calls: 
    uint8_t pattern_arr = {0, 1, 4};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr)};
    CrossFadeCyclePS(mainSegments, &pattern, pallet, 0, 40, 30);
    Will fade from color 0, to color 1, to color 4 of the pallet, infinitly, taking 40 steps for each fade, with 30ms between steps

    CrossFadeCyclePS(mainSegments, pallet, 5, 40, 30);
    Will fade through the colors of the pallet in order until 5 fades have been completed, taking 40 steps for each fade, with 30ms between steps

    CrossFadeCyclePS(mainSegments, 0, 40, 30);
    Will fade from one random color to the next infinitly, taking 40 steps for each fade, with 30ms between steps

Modes ( not set in constructor, set using mode variable ):
    Mode 0: (default), cycles through the pattern in order
    Mode 1: Chooses the next color randomly from the current pattern (it shuffles)
    Mode 2: Chooses colors completely randomly (is set by the corrosponding constuctor)

Doe not accept color modes from segDrawUtils::setPixelColor();

Constructor Inputs
    Pattern(optional, see constructors) -- A pattern is struct made from a 1-d array of pallet indexes ie {0, 1, 3, 6, 7} 
                                          and the length of the array 
                                          (see patternPS.h)                                     
    Pallet(optional, see constructors) -- The repository of colors used in the pattern, or can be used as the pattern itself
    NumFades -- The total number of crossfades(colors) the effect will go through, setting this to 0 will flag for infinite fades
    Steps -- How many steps for each fade
    Rate -- update rate (ms)

Functions:
    setPattern(*newPattern) -- Sets the passed in pattern to be the effect pattern
    setPallet(*newPallet) -- Sets the pallet to the passed in pallet
    setPalletAsPattern() -- Sets the effect pattern to match the current pallet
    reset() -- restarts the effect
    update() -- updates the effect

Flags:
    infinite (default false) -- Sets the effect to run though an contiuous cycle of fades with no stop point
    done (starts false) -- set to true flag for if we've done the total number of fades

Other Settings:
    Mode -- see mode note above
*/
class CrossFadeCyclePS : public EffectBasePS {
    public:
        //Constructor for pattern and pallet
        CrossFadeCyclePS(SegmentSet &SegmentSet, patternPS *Pattern, palletPS *Pallet, uint16_t NumFades, uint8_t Steps, uint16_t Rate);
        
        //Constructor for pallet as the pattern
        CrossFadeCyclePS(SegmentSet &SegmentSet, palletPS *Pallet, uint16_t NumFades, uint8_t Steps, uint16_t Rate);  
        
        //Constructor for random colors
        CrossFadeCyclePS(SegmentSet &SegmentSet, uint16_t NumFades, uint8_t Steps, uint16_t Rate);

        ~CrossFadeCyclePS();

        uint8_t
            steps, //total steps per fade
            fMode = 0; //see description above

        patternPS
            *pattern;
        
        uint16_t 
            numFades; //total number of fades to do
        
        bool
            infinite = false, //for ignoring the done flag
            done = false; //flag for if we've done the total number of fades

        SegmentSet 
            &segmentSet; 

        palletPS
            *pallet;
        
        void 
            setPattern(patternPS *newPattern),
            setPallet(palletPS *newPallet),
            setPalletAsPattern(),
            reset(void),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        palletPS
            palletTemp;
        
        patternPS
            patternTemp;
        
        CRGB 
            startColor, //the color we are fading from
            newColor,
            nextColor; //the color we are fading to
        
        uint8_t
            currentIndex = 0, //the index of the pattern that we are currently fading away from, mainly used for shuffle()
            currentStep = 0; //current step of the current fade
        
        uint16_t 
            fadeCount = 0; //how many total fades we've done
        
        void
            init(uint16_t Rate);
};

#endif
#ifndef StreamerFastPS_h
#define StreamerFastPS_h

//TODO -- add constructor for random options?
//     -- When picking random colors, use getShuffleIndex instead of just randoming

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

//Similar to StreamerPS, but runs faster with some restrictions
//Repeats a set of lines down the strip according to the passed in pallet, shifting them forward over time
//Like a theater chase, but a pattern is used instead of just dots

//The restrictions when compared to StreamerPS are listed below:
//1: There is no fading to shift the streamers along the strip
//2: Changing the pallet on the fly will have a delayed effect on the colors
//   The exisiting colors will shift off the strip before new ones shift on
//   This prevents this effect from playing well with palletBlend functions
//3: The same restrictions as (2) apply to changing the pattern
//4: Changing the direction of the segments or segment set mid-effect may break it temporarily
//5: This effect is not compatible with colorModes for either the streamers or the background

//Basically the effect works by setting the color of the first pixel, then for each subsequent pixel,
//it copies the color of the next pixel in line
//So any changes you make to colors will only show up at the first pixel, and will be shifted along the strip

//However, as a bonus, this effect supports random colored streamer
//where the colors for the streamers are choosen at random as the enter the strip
//This is controlled by the randMode setting

//Otherwise, all the settings are/functions are the same as StreamerPS

//Patterns work the same as with other effects, they are a pattern of pallet array indexes
//ie a pattern of {0, 1, 2} would be the first three colors of a pallet
//to indicate a background pixel (ie set to the BgColor) we use 255 in the pattern
//This does mean if your pallet has 255 colors, you'll lose the final color, but you shouldn't have pallets that large

//For example, lets say we wanted to do the first two colors of our pallet,
//each as length 4 streamers, with 3 background leds inbetween each
//we would make a pattern as : {0, 0, 0, 0, 255, 255, 255, 1, 1, 1, 1, 255, 255, 255}

//For simple patterns like the previous example, I have writtern a few constructors for the effect
//that automate the pattern creation, so you don't have to do it yourself (see constructor notes below)

//Note that while each entry in the pattern is a uint8_t,
//if you have a lot of colors, with long streamers, your patterns may be quite large
//so watch your memory usage

//Example calls: 
    //uint8_t pattern_arr = {0, 255, 255, 255, 1, 1, 255, 255};
    //patternPS pattern = {pattern_arr, SIZE(pattern_arr)};
    //StreamerPS(mainSegments, &pattern, &pallet3, 0, 120);
    //Will do a set of streamers using the first two colors in the pallet
    //The streamer will begin with 1 pixel of color 0, with three spaces after, followed by 2 pixels of color 1, followed by 2 spaces
    //The bgColor is zero (off)
    //The streamers will update at a 120ms rate

    //uint8_t pattern_arr = {1, 2, 3};
    //patternPS pattern = {pattern_arr, SIZE(pattern_arr)};
    //StreamerPS(mainSegments, &pattern, &pallet3, 3, 4, 0, 120);
    //Will do a streamer using the first three colors of the pallet (taken from the pattern)
    //Each streamer will be length 3, followed by 4 spaces, bgColor is 0 (off)
    //The effect updates at a rate of 120ms

    //StreamerPS(mainSegments, &pallet3, 3, 4, CRGB::Red, 120);
    //Will do a streamer using all the colors in pallet3, each streamer will be length 3, with 4 spaces inbetween
    //The bgColor is red
    //The streamers will update at a 120ms rate

    //StreamerPS(mainSegments, CRGB::Blue, 2, 2, CRGB::Red, 0, 140);
    //Will do a blue streamers with length 2 and 2 spaces inbetween
    //The bgColor is red
    //The effect updates at a rate of 140ms
 
//Constructor Inputs:
    //Pattern(optional, see constructors) -- The pattern used for the streamers, made up of pallet indexes 
    //Pallet(optional, see constructors) -- The repository of colors used in the pattern
    //Color(optional, see constructors) -- Used for making a single color streamer
    //ColorLength (optional, see constructors, max 255) -- The number pixels a streamer color is. Used for automated pattern creation.
    //Spacing (optional, see constructors, max 255) -- The number of pixels between each streamer color (will be set to bgColor).  Used for automated pattern creation.
    //BgColor -- The color of the spacing pixels. It is a pointer, so it can be tied to an external variable
    //Rate -- The update rate (ms)

//Functions:
    //reset() -- Restarts the streamer pattern
    //setPatternAsPattern(*inputPattern, colorLength, spacing) -- Takes an input pattern and creates a streamer pattern from it using the current pallet
    //                                                            Ex: uint8_t pattern_arr = {1, 2, 3};
    //                                                                patternPS pattern = {pattern_arr, SIZE(pattern_arr)};
    //                                                                setPatternAsPattern(&pattern, 3, 4) 
    //                                                                Will do a streamer using the first three colors of the pallet (taken from the pattern)
    //                                                                Each streamer will be length 3, followed by 4 spaces
    //setPalletAsPattern(uint8_t colorLength, uint8_t spacing) -- Like the previous function, but all of the current pallet will be used for the pattern                                                       
    //update() -- updates the effect

//Other Settings:
    //fadeOn (default true) -- If false, the streamer will jump directly to the next color instead of fading
    //                         Note that if 1 or 0 are passed in as the FadeSteps in the constructor, 
    //                         fadeOn will be set to false automatically
    //randMode (default 0) -- Sets the type of how colors are choosen:
    //                     -- 0: Colors will be choosen in order from the pattern (not random)
    //                     -- 1: Colors will be choosen completely at random
    //                     -- 2: Colors will be choosen randomly from the pallet

//Flags:
    //preFillDone -- Indicates if the strip has been pre-filled with the effect's color outputs 
    //               This needs to happen before running the first update cycle
    //               If false, preFill() will be called when first updating
    //               Set true once the first update cycle has been finished

//Notes:
    //If the constructor made your pattern, it will be stored in patternTemp
    //same goes for the pallet

class StreamerFastPS : public EffectBasePS {
    public:
        //constructor for using the passed in pattern and pallet for the streamer
        StreamerFastPS(SegmentSet &SegmentSet, patternPS *Pattern, palletPS *Pallet, CRGB BgColor, uint16_t Rate);

        //constructor for building the streamer pattern from the passed in pattern and the pallet, using the passed in colorLength and spacing
        StreamerFastPS(SegmentSet &SegmentSet, patternPS *Pattern, palletPS *Pallet, uint8_t ColorLength, uint8_t Spacing, CRGB BgColor, uint16_t Rate);
        
        //constructor for building a streamer using all the colors in the passed in pallet, using the colorLength and spacing for each color
        StreamerFastPS(SegmentSet &SegmentSet, palletPS *Pallet, uint8_t ColorLength, uint8_t Spacing, CRGB BgColor, uint16_t Rate);

        //constructor for doing a single colored streamer, using colorLength and spacing
        StreamerFastPS(SegmentSet &SegmentSet, CRGB Color, uint8_t ColorLength, uint8_t Spacing, CRGB BgColor, uint16_t Rate);  
    
        ~StreamerFastPS();

        uint8_t
            nextPattern,
            prevPattern,
            randMode = 0,
            cycleCount = 0;
            
        bool
            preFillDone = false;

        CRGB 
            nextColor,
            bgColorOrig,
           *bgColor; //bgColor is a pointer so it can be tied to an external variable if needed (such as a pallet color)

        SegmentSet 
            &segmentSet; 
        
        patternPS
            patternTemp,
            *pattern;
        
        palletPS
            palletTemp,
            *pallet;
        
        void 
            init(CRGB BgColor, uint16_t Rate),
            setPatternAsPattern(patternPS *inputPattern, uint8_t colorLength, uint8_t spacing),
            setPalletAsPattern(uint8_t colorLength, uint8_t spacing),
            reset(),
            preFill(),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        uint16_t
            numPixels,
            nextPixelNumber,
            pixelNumber;

        CRGB 
            pickStreamerColor(uint8_t patternIndex);
};  

#endif
#ifndef StreamerPS_h
#define StreamerPS_h

#include "Effects/EffectBasePS.h"
#include "Effects/EffectUtils/EffectUtilsPS.h"

//Repeats a set of lines down the strip according to the passed in pallet, shifting them forward over time
//Like a theater chase, but a pattern is used instead of just dots
//Each streamer blends as it moves forwards, for a smoother effect
//Since the blending takes a bit of processing power, you can disable it with fadeOn
//For an even faster version of this effect, see StreamerFastPS

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

//This effect is fully compatible with color modes, and the bgColor is a pointer, so you can bind it
//to an external color variable

//Example calls: 
    //uint8_t pattern_arr = {0, 255, 255, 255, 1, 1, 255, 255};
    //patternPS pattern = {pattern_arr, SIZE(pattern_arr)};
    //StreamerPS(mainSegments, &pattern, &pallet3, 0, 30, 20);
    //Will do a set of streamers using the first two colors in the pallet
    //The streamer will begin with 1 pixel of color 0, with three spaces after, followed by 2 pixels of color 1, followed by 2 spaces
    //The bgColor is zero (off)
    //The streamers will blend forward, taking 30 steps, with 20ms between each step

    //uint8_t pattern_arr = {1, 2, 3};
    //patternPS pattern = {pattern_arr, SIZE(pattern_arr)};
    //StreamerPS(mainSegments, &pattern, &pallet3, 3, 4, 0, 0, 120);
    //Will do a streamer using the first three colors of the pallet (taken from the pattern)
    //Each streamer will be length 3, followed by 4 spaces, bgColor is 0 (off)
    //The fade steps are set to zero, so there is no blending, 
    //The effect updates at a rate of 120ms

    //StreamerPS(mainSegments, &pallet3, 3, 4, CRGB::Red, 10, 40);
    //Will do a streamer using all the colors in pallet3, each streamer will be length 3, with 4 spaces inbetween
    //The bgColor is red
    //The streamer will blend forward, taking 10 steps, with 40ms between each step

    //StreamerPS(mainSegments, CRGB::Blue, 2, 2, CRGB::Red, 0, 140);
    //Will do a blue streamers with length 2 and 2 spaces inbetween
    //The bgColor is red
    //The fade steps are set to zero, so there is no blending
    //The effect updates at a rate of 140ms
 
//Constructor Inputs:
    //Pattern(optional, see constructors) -- The pattern used for the streamers, made up of pallet indexes 
    //Pallet(optional, see constructors) -- The repository of colors used in the pattern
    //Color(optional, see constructors) -- Used for making a single color streamer
    //ColorLength (optional, see constructors, max 255) -- The number pixels a streamer color is. Used for automated pattern creation.
    //Spacing (optional, see constructors, max 255) -- The number of pixels between each streamer color (will be set to bgColor).  Used for automated pattern creation.
    //BgColor -- The color of the spacing pixels. It is a pointer, so it can be tied to an external variable
    //FadeSteps -- The number of steps to transition from one color to the next as the streamers move down the strip
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
    //colorMode (default 0) -- sets the color mode for the streamer pixels (see segDrawUtils::setPixelColor)
    //bgColorMode (default 0) -- sets the color mode for the spacing pixels (see segDrawUtils::setPixelColor)
    //fadeOn (default true) -- If false, the streamer will jump directly to the next color instead of fading
    //                         Note that if 1 or 0 are passed in as the FadeSteps in the constructor, 
    //                         fadeOn will be set to false automatically

//Notes:
    //You can change the pallet, and patterns on the fly, but there's no way to smoothly transition
    //between patterns, so there will possibly be a jump

    //If the constructor made your pattern, it will be stored in patternTemp
    //same goes for the pallet

    //Turning off fading using fadeOn should improve performance. The effect will not be as smooth, but should still look
    //pretty good. Note that you'll need to set your new update rate to (fadeSteps * your current rate) to keep the same
    //overall update rate

class StreamerPS : public EffectBasePS {
    public:
        //constructor for using the passed in pattern and pallet for the streamer
        StreamerPS(SegmentSet &SegmentSet, patternPS *Pattern, palletPS *Pallet, CRGB BgColor, uint8_t FadeSteps, uint16_t Rate);  

        //constructor for building the streamer pattern from the passed in pattern and the pallet, using the passed in colorLength and spacing
        StreamerPS(SegmentSet &SegmentSet, patternPS *Pattern, palletPS *Pallet, uint8_t ColorLength, uint8_t Spacing, CRGB BgColor, uint8_t FadeSteps, uint16_t Rate);
        
        //constructor for building a streamer using all the colors in the passed in pallet, using the colorLength and spacing for each color
        StreamerPS(SegmentSet &SegmentSet, palletPS *Pallet, uint8_t ColorLength, uint8_t Spacing, CRGB BgColor, uint8_t FadeSteps, uint16_t Rate);

        //constructor for doing a single colored streamer, using colorLength and spacing
        StreamerPS(SegmentSet &SegmentSet, CRGB Color, uint8_t ColorLength, uint8_t Spacing, CRGB BgColor, uint8_t FadeSteps, uint16_t Rate);

        ~StreamerPS();

        uint8_t
            colorMode = 0,
            bgColorMode = 0,
            fadeSteps;

        CRGB 
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
        
        bool 
            fadeOn = true;
        
        void 
            init(CRGB BgColor, uint16_t Rate),
            setPatternAsPattern(patternPS *inputPattern, uint8_t colorLength, uint8_t spacing),
            setPalletAsPattern(uint8_t colorLength, uint8_t spacing),
            reset(),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        uint8_t
            nextPattern,
            blendStep = 0,
            cycleCount = 0;
        
        uint16_t
            patternLength,
            nextPatternIndex,
            numPixels;
        
        pixelInfoPS
            pixelInfo{0, 0, 0, 0};
        
        CRGB 
            nextColor,
            currentColor,
            colorOut,
            getNextColor(uint16_t pixelNum);
        
        void
            updateFade(),
            updateNoFade();
};

#endif
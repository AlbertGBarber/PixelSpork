#ifndef Noise16PS_h
#define Noise16PS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

/*
An effect based on the noise_16 effects created by Andrew Tuline here: https://github.com/atuline/FastLED-Demos/blob/master/noise16_3/noise16_3.ino
See more about FastLED noise here: https://github.com/s-marley/FastLED-basics/tree/main/6.%20Noise
This effect is basically a sort of Perlin noise playground
It tends to produce long stretches of color that shift around the strip.
Colors are based on an input palette.

You have a number of different dials you can turn to produce a range of results
The values you use will depend on how long your strip is, but I'll provide some guidance below
You can also try Andrew's inputs (see his noise16_1, noise16_2, and noise16_3 via the links above)
But these seemed like they moved to fast for me.

I don't fully understand how everything in this effect works, but based on testing I've written the guide below.

You can think of Perlin noise as a sort of randomly generated 3D landscape,
which smoothly transitions from point to point (no sudden jumps like cliffs, spikes, etc). 
When we ask for a noise value, we are getting back a coordinate from the landscape. 
To change the noise value, we move through the landscape.
Because it is smooth, our values transition nicely between one another, 
which is great for blending colors, moving pixels, etc. 
Keep in mind that the noise is only sudo-random, if we put in the same input, we'll get the same output every time.  

The noise has four main inputs: an overall scale, and individual scaling values for x, y, and z; 
the three coordinates of our noise field. 

The overall scale determines how "zoomed-in" the noise is, sort of controlling how long lengths of color are. 
I found that setting it between 500 - 1000 worked well.

For x, y, and z you are given two options of how each are set:
These are set by x_mode, y_mode, and z_mode, and x_val, y_val, and z_val:
* 0 -- Set to a constant value. 
* 1 -- Set to a value that varies in time by millis() / val. 30 - 100+ works well.
* 2 -- Set to the output of beatsin8(val) (a sin function with frequency "val"). 1 - 10 works well.
* 3 -- Set to a value that varies in time by millis() * val. Only recommend using this with the z_val.

You want at least one input to vary with time, so that we move through the noise field.

With one value varying in time, you can either vary the others as well, or leave them static. 
Varying them in time can produce a more complex effect, but for some reason, 
certain combinations of x and y values tend to produce some flicker. 
I've not been able to track down why this happens sadly.
(using z with x or y seems to be okay tho). 
It's something to do with the oscillations of the two timing effects lining up I think.

Other than that, I can only recommend you experiment. 
Keep in mind that there are other noise effects, which may be easier to use, 
but are usually more limited to a certain type of "look". 
These include any effect with "noise" in the name, and the Lava effect.

Finally, this effect is 1D, so it always treats your Segment Set as a single line.

Example calls: 
    Noise16PS noise16(mainSegments, 4, 40, 500, 2, 1, 0, 2, 100, 0, 40);
    Will produce a noise effect with a palette of 4 randomly chosen colors
    There are 40 blend steps between each color
    An overall scaling value of 500 is applied to the noise
    The noise has the following inputs:
        x is mode 2, and follows a beatsin with 3 bpm
        y is mode 1, and scales with time as millis() / 100;
        z is mode 0, so is static at 0;
    The effect updates at 40ms
    
    Noise16PS noise16(mainSegments, cybPnkPal_PS, 20, 600, 1, 0, 2, 200, 10, 5, 60);
    Will produce a noise effect using colors from the cybPnkPal_PS palette
    There are 20 blend steps between each color
    An overall scaling value of 600 is applied to the noise
    The noise has the following inputs:
        x is mode 1, and scales with time as millis() / 200;
        y is mode 0, so is static at 10;
        z is mode 2, and follows a beatsin with 5 bpm
    The effect updates at 60ms

Constructor inputs: 
    palette (optional, see constructors) -- A custom palette passed to the effect, the default is the 
                                          lava colors palette encoded below
    numColors (optional, see constructors) -- How many colors will be in the randomly created palette
    blendSteps -- Sets how many steps are used to blend between each color
                 Basically changes how fast the colors blend
                 Between 20 - 100 looks good
    blendScale -- Sets how "zoomed-in" the noise is
                 (test it out yourself to see what I mean)
                 Between 500 - 1000 looks good
    x_mode -- Sets how the x noise input will vary with time (see Inputs Guide above)
    y_mode -- Sets how the y noise input will vary with time (see Inputs Guide above)
    z_mode -- Sets how the z noise input  will vary with time (see Inputs Guide above)
    x_val -- The scaling factor for the x noise input (see Inputs Guide above)
    y_val -- The scaling factor for the y noise input (see Inputs Guide above)
    z_val -- The scaling factor for the z noise input (see Inputs Guide above)
    rate -- The update rate (ms) note that this is synced with all the particles.

Functions:
    update() -- updates the effect  
*/
class Noise16PS : public EffectBasePS {
    public:
        //Constructor with palette
        Noise16PS(SegmentSetPS &SegSet, palettePS &Palette, uint8_t BlendSteps, uint16_t BlendScale,
                  uint8_t X_mode, uint8_t Y_mode, uint8_t Z_mode, uint16_t X_val, uint16_t Y_val, uint16_t Z_val, uint16_t Rate);

        //Constructor with randomly generated palette
        Noise16PS(SegmentSetPS &SegSet, uint8_t numColors, uint8_t BlendSteps, uint16_t BlendScale,
                  uint8_t X_mode, uint8_t Y_mode, uint8_t Z_mode, uint16_t X_val, uint16_t Y_val, uint16_t Z_val, uint16_t Rate);

        ~Noise16PS();

        uint8_t
            x_mode,
            y_mode,
            z_mode,
            blendSteps;

        uint16_t
            x_val,
            y_val,
            z_val,
            blendScale;  // the "zoom factor" for the noise

        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0};  //Must init structs w/ pointers set to null for safety

        void
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0;

        uint8_t
            bri,
            noise;

        uint16_t
            pixelCount = 0,
            index,
            shift_x,
            shift_y,
            totBlendLength,
            totSegLen,
            pixelNum,
            numSegs;

        uint32_t
            real_x,
            real_y,
            real_z,
            getShiftVal(uint8_t shiftMode, uint16_t scale);

        CRGB
            colorOut;

        void
            init(SegmentSetPS &SegSet, uint16_t Rate);
};

#endif
#ifndef PacificaHuePalPS_h
#define PacificaHuePalPS_h

#include "PaletteFiles.h"

//A small class for storing a pacifica palette with an adjustable hue
//To keep the "pacifica" look, the palette colors are offset from the base pfHue value
//Whenever the hue is changed, the palette colors need to be re-caculated.

//Code based on code by u/christian_suryanto at 
//https://old.reddit.com/r/FastLED/comments/wo7n9u/pacifica_with_hue_settings_codes_in_comments/

//Call setHue(uint8_t newHue) to set a new hue value
//Call incHue() to increment the current hue value by one
class PacificaHuePalPS{
    public:
        //Constructor for setting an inital hue
        PacificaHuePalPS(uint8_t hueInit){
            setHue(hueInit);
        };

        //Constructor for using the default pacific hue setting
        PacificaHuePalPS(){
            setHue(pfHue);
        };
        
        //The base hue value, the default of 130 produces the original pacifica look
        //for reference only, call setHue() the change
        uint8_t
            pfHue = 130;

        //I've translated the original Pacifica palettes into my palettes
        //These produce a very slightly different blend of colors, but it still looks good
        //The orignal palettes can be found in Pacifica->pacificaPalette.h
        CRGB 
            bgColor, //background color for the pacifica effect
            paPal1_arr[5],
            paPal2_arr[5],    
            paPal3_arr[5];

        palettePS
            pacificaPal1PS = {paPal1_arr, SIZE(paPal1_arr)},
            pacificaPal2PS = {paPal2_arr, SIZE(paPal2_arr)},
            pacificaPal3PS = {paPal3_arr, SIZE(paPal3_arr)};

        void    
            //Sets the hue for the palette by adjusting the palette colors
            //We use CHSV() so we can set a hue directly, rather than trying to change RGB values,
            //which would be almost very tedious. 
            //The hue offsets were mostly worked out through trial and error.
            //(converting from the original pacifica colors isn't easy b/c FastLED's HSV spectrum is different from online color picker tools)
            //Hopefully these look good for most hues (except yellow, b/c the hue range is very small)
            setHue(uint8_t newHue){
                pfHue = newHue;

                bgColor = CRGB( CHSV(pfHue+21, 255, 30) ); //10

                //palette 1 colors
                paPal1_arr[0] = CRGB(CHSV(pfHue+10, 255, 80)); 
                paPal1_arr[1] = CRGB(CHSV(pfHue+20, 255, 80)); 
                paPal1_arr[2] = CRGB(CHSV(pfHue+27, 255, 100)); 
                paPal1_arr[3] = CRGB(CHSV(pfHue+8, 255, 130));
                paPal1_arr[4] = CRGB(CHSV(pfHue, 255, 171)); 

                //palette 2 colors
                paPal2_arr[0] = CRGB(CHSV(pfHue+10, 255, 80));
                paPal2_arr[1] = CRGB(CHSV(pfHue+20, 255, 80));
                paPal2_arr[2] = CRGB(CHSV(pfHue+27, 255, 100));
                paPal2_arr[3] = CRGB(CHSV(pfHue+3, 255, 130));
                paPal2_arr[4] = CRGB(CHSV(pfHue+6, 255, 230));

                //palette 3 colors
                paPal3_arr[0] = CRGB(CHSV(pfHue+12, 255, 80)); 
                paPal3_arr[1] = CRGB(CHSV(pfHue+22, 255, 80)); 
                paPal3_arr[2] = CRGB(CHSV(pfHue+18, 255, 100)); 
                paPal3_arr[3] = CRGB(CHSV(pfHue+18, 255, 171)); 
                paPal3_arr[4] = CRGB(CHSV(pfHue+17, 255, 255)); 

            };
        
        void
            //Increments the current hue value by 1 and caculates new palette colors
            incHue(){
                pfHue++; //wraps automatically at 255 b/c it's uint8_t
                setHue(pfHue);
            };
};

#endif
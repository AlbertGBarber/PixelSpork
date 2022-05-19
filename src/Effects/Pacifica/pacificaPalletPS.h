
#ifndef pacificaPalletPS_h
#define pacificaPalletPS_h
#include "PalletFiles.h"

//I've translated the original Pacifica pallets into my pallets
//These produce a very slightly different blend of colors, but it still looks good
//I've left the original pallets as comments below, should you want to use them
static CRGB 
    paPal1_arr[5] = { CRGB(0, 5, 7), CRGB(0, 3, 11), CRGB(0, 3, 25), CRGB(20, 85, 75), CRGB(40, 170, 80) },
    //paPal1_arr[16] =  { CRGB(0, 5, 7),  CRGB(0, 4, 9),  CRGB(0, 3, 11),   CRGB(0, 3, 13), 
    //                     CRGB(0, 2, 16), CRGB(0, 2, 18), CRGB(0, 1, 20),   CRGB(0, 1, 23), 
    //                     CRGB(0, 0, 25), CRGB(0, 0, 28), CRGB(0, 0, 38),   CRGB(0, 0, 49), 
    //                     CRGB(0, 0, 59), CRGB(0, 0, 70), CRGB(20, 85, 75), CRGB(40, 170, 80) },

    paPal2_arr[5] = { CRGB(0, 5, 7), CRGB(0, 3, 11), CRGB(0, 3, 25), CRGB(12, 95, 82), CRGB(25, 190, 95) },
    //paPal2_arr[16] =  { CRGB(0, 5, 7),  CRGB(0, 4, 9),  CRGB(0, 3, 11),   CRGB(0, 3, 13), 
    //                     CRGB(0, 2, 16), CRGB(0, 2, 18), CRGB(0, 1, 20),   CRGB(0, 1, 23), 
    //                     CRGB(0, 0, 25), CRGB(0, 0, 28), CRGB(0, 0, 38),   CRGB(0, 0, 49), 
    //                     CRGB(0, 0, 59), CRGB(0, 0, 70), CRGB(12, 95, 82), CRGB(25, 190, 95) },
                  
    paPal3_arr[5] = { CRGB(0, 2, 8), CRGB(0, 5, 20), CRGB(0, 14, 57), CRGB(16, 64, 191), CRGB(32, 96, 255) };
    //paPal3_arr[16] = { CRGB(0, 2, 8),    CRGB(0, 3, 14),   CRGB(0, 5, 20),    CRGB(0, 6, 26), 
    //                    CRGB(0, 8, 32),   CRGB(0, 9, 39),   CRGB(0, 11, 45),   CRGB(0, 12, 51), 
    //                    CRGB(0, 14, 57),  CRGB(0, 16, 64),  CRGB(0, 20, 80),   CRGB(0, 24, 96), 
    //                    CRGB(0, 28, 112), CRGB(0, 32, 128), CRGB(16, 64, 191), CRGB(32, 96, 255) };
        
//For lava colors
//To use these you also need to adjust the background and the deepenColors function
//static CRGB 
    //paPal1_arr[5] = { CRGB(7, 0, 0), CRGB(11, 0, 0), CRGB(25, 0, 0), CRGB(80, 6, 3), CRGB(120, 15, 5) },
    //paPal2_arr[5] = { CRGB(7, 0, 0), CRGB(11, 0, 0), CRGB(25, 0, 0), CRGB(110, 15, 6), CRGB(160, 30, 10) },
    //paPal3_arr[5] = { CRGB(8, 0, 0), CRGB(20, 0, 0), CRGB(57, 0, 0), CRGB(191, 95, 11), CRGB(184, 122, 24) };

static palletPS
    pacificaPal1PS = {paPal1_arr, SIZE(paPal1_arr)},
    pacificaPal2PS = {paPal2_arr, SIZE(paPal2_arr)},
    pacificaPal3PS = {paPal3_arr, SIZE(paPal3_arr)};

#endif
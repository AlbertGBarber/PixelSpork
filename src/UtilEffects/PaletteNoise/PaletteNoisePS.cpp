#include "PaletteNoisePS.h"

//Constructor with hueRate of 0
PaletteNoisePS::PaletteNoisePS(uint8_t numColors, uint8_t HueBase, uint8_t HueMax, bool Compliment, uint16_t BlendSpeed, uint16_t Rate):
    hueBase(HueBase), hueMax(HueMax), blendSpeed(BlendSpeed), compliment(Compliment)
    {    
        init(numColors, 0, Rate);
	}

//Constructor with hueRate setting
PaletteNoisePS::PaletteNoisePS(uint8_t numColors, uint8_t HueBase, uint8_t HueMax, bool Compliment, uint16_t BlendSpeed, uint16_t HueRate, uint16_t Rate):
    hueBase(HueBase), hueMax(HueMax), blendSpeed(BlendSpeed), compliment(Compliment)
    {
        init(numColors, HueRate, Rate);  
    }   

//Constructor with saturation and value min settings and  hueRate setting
PaletteNoisePS::PaletteNoisePS(uint8_t numColors, uint8_t HueBase, uint8_t HueMax, uint8_t SatMin, uint8_t ValMin, 
                              bool Compliment, uint16_t BlendSpeed, uint16_t HueRate, uint16_t Rate):
    hueBase(HueBase), hueMax(HueMax), satMin(SatMin), valMin(ValMin), blendSpeed(BlendSpeed), compliment(Compliment)
    {
        init(numColors, HueRate, Rate);  
    }   
    
PaletteNoisePS::~PaletteNoisePS(){
    free(noisePalette.paletteArr);
}

//initialize various core variables
void PaletteNoisePS::init(uint16_t numColors, uint16_t HueRate, uint16_t Rate){
    //bind the rate and SegmentSet pointer vars since they are inherited from BaseEffectPS
    bindClassRatesPS();
    //Create the noisePalette
    setupPalette(numColors);

    //bind the hue rate to it's pointer
    hueRateOrig = HueRate;
    hueRate = &hueRateOrig;
}

//Allocates memory for the noisePalette
//will also call update() to fill in the noise palette initially
void PaletteNoisePS::setupPalette(uint8_t numColors){
    free(noisePalette.paletteArr);
    noisePalette = paletteUtilsPS::makeRandomPalette(numColors);
    prevTime = 0;
    update();
}

//Updates the noisePalette colors
//Each update we get a new color for each palette entry by generating some noise and mapping it to a color based on the hue range
//We also use a separate noise function to adjust the saturation and value of the color (the colors are set using hsv)
//This helps add more variation to the colors
//The noise varies as a function of time (millis())
void PaletteNoisePS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        //increment the hue if needed
        if(*hueRate > 0 && ( currentTime - prevHueTime ) >= *hueRate ){
            prevHueTime = currentTime; 
            hueBase++;
        }
         
        //The offset for each palette color if we're doing complimentary colors
        //255 is the circumference of our color wheel
        compStep = 255 / noisePalette.length;

        //Set a noise based color for each palette index
        for(uint8_t i = 0; i < noisePalette.length; i++){

            //Generate some noise data for a color and saturation/value
            //varying the noise using the currentTime seems to work well, 
            //although if your updates were slow, you could use a fixed counter system (see the NoiseSL effect)
            //We use a separate noise value for the saturation and value to add more variation
            noiseData = inoise8( i * colorScale, currentTime / blendSpeed);
            noiseData2 = inoise8( i * briScale, currentTime / briSpeed);

            //The range of the inoise8 function is roughly 16-238.
            //These two operations expand those values out to roughly 0..255
            //You can comment them out if you want the raw noise data.
            noiseData = qsub8( noiseData, 16);
            noiseData = qadd8( noiseData, scale8( noiseData, 39 ) );

            //map the noise data to a hue offset between 0 and hueMax
            hueAdj = map8(noiseData, 0, hueMax);

            //If we're getting complimentary colors, we offset each hue by x degrees on the color wheel
            if(compliment){
                hueAdj += compStep * i;
            }

            //map the second noise data for the saturation and value
            val = map8(noiseData2, valMin, 255);
            sat = map8(noiseData, satMin, 255);

            //Set the palette color
            //We use hsv to set the color, adjusting from the hueBase
            paletteUtilsPS::setColor(noisePalette, CRGB(CHSV(hueBase + hueAdj, sat, val)), i);
        }
    }
}
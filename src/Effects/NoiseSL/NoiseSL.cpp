#include "NoiseSL.h"

//Constuctor for randomly generated palette
NoiseSL::NoiseSL(SegmentSet &SegmentSet, uint8_t numColors, uint16_t BlendSteps, uint16_t ScaleBase, uint16_t ScaleRange, uint16_t Speed, uint8_t CMode, uint16_t Rate):
    segmentSet(SegmentSet), blendSteps(BlendSteps), scaleBase(ScaleBase), scaleRange(ScaleRange), speed(Speed), cMode(CMode)
    {    
        init(Rate);    
        paletteTemp = paletteUtilsPS::makeRandomPalette(numColors);
        palette = &paletteTemp; 
	}

//Constructor using palette
NoiseSL::NoiseSL(SegmentSet &SegmentSet, palettePS &Palette, uint16_t BlendSteps, uint16_t ScaleBase, uint16_t ScaleRange, uint16_t Speed, uint8_t CMode, uint16_t Rate):
    segmentSet(SegmentSet), palette(&Palette), blendSteps(BlendSteps), scaleBase(ScaleBase), scaleRange(ScaleRange), speed(Speed), cMode(CMode)
    {
        init(Rate);
    }

NoiseSL::~NoiseSL(){
    free(noise);
    free(paletteTemp.paletteArr);
}

//Sets up the initial effect values and other key variables
void NoiseSL::init(uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();

    //create the noise array
    setupNoiseArray();

    // Initialize our coordinates to some random values
    x = random16();
    y = random16();
    z = random16();
    
    scale = scaleBase;
    scaleTarget = scale;
    //get the first scale target
    //(we set scaelEnd = scale to force setShiftScale to trigger)
    setShiftScale();
}

//If you ever change the effect's segmentSet call this function
//Creates a noise array for the segmentSet lines
//Ideally this would be a 2D array, with each row being a line on the segmentSet
//But it's much easier in C++ to create a 1D array and then offset out starting points
//when reading/writing values
//So each segment line is stored consecutively in the array
//ie if we had 4 segments and 10 segment lines
//then array index's 0, 1, 2, 3 would be for line 0
//4, 5, 6, 7 for line 1, etc
void NoiseSL::setupNoiseArray(){
    //fetch some core vars
    numSegs = segmentSet.numSegs;
    numLines = segmentSet.numLines;
    uint16_t numPoints = numLines * numSegs;
    
    free(noise);
    //create the noise array to store noise value of each line point
    noise = (uint8_t*) malloc(numPoints * sizeof(uint8_t));
}

//updates the effect
//The basic setup here is that for each update, we generate a new array of 
//'noise' data, and then map it onto the segment set lines through a color palette or rainbow.
//We also shift the scale value around to vary the output
void NoiseSL::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        //recalculate the blend length in case the palette changed
        totBlendLength = blendSteps * palette->length;
        
        //shift towards the target scale value, or get a new target
        setShiftScale();

        //make some noise!
        fillnoise8();

        //map the noise to colors and output it
        mapNoiseSegsWithPalette();

        showCheckPS();
    }
}

//Fill the noise array with 8-bit noise values using the inoise8 function.
//In addition, it includes some fast automatic 'data smoothing' at 
//lower noise speeds to help produce smoother animations in those cases.
void NoiseSL::fillnoise8() {
    //If we're runing at a low "speed", some 8-bit artifacts become visible
    //from frame-to-frame.  In order to reduce this, we can do some fast data-smoothing.
    //The amount of data smoothing we're doing depends on "speed".
    dataSmoothing = 0;
    if( speed < 50 ) {
        dataSmoothing = 200 - (speed * 4);
    }
  
    //For each segment line do the following:
    for (uint16_t i = 0; i < numLines; i++) {
        ioffset = scale * i;
        //current segment line's start index in the noise array
        noiseStart = i * numSegs;
        for (uint16_t j = 0; j < numSegs; j++) {
            joffset = scale * j;
            //The location of the noise data in the noise array
            noiseIndex = noiseStart + j;

            noiseData = inoise8( x + ioffset, y + joffset, z );

            // The range of the inoise8 function is roughly 16-238.
            // These two operations expand those values out to roughly 0..255
            // You can comment them out if you want the raw noise data.
            noiseData = qsub8( noiseData, 16);
            noiseData = qadd8( noiseData, scale8( noiseData, 39 ) );

            if( dataSmoothing ) {
                oldData = noise[noiseIndex];
                newData = scale8( oldData, dataSmoothing ) + scale8( noiseData, 256 - dataSmoothing );
                noiseData = newData;
            }
            
            noise[noiseIndex] = noiseData;
        }
    }
  
    z += speed;
    // apply slow drift to X and Y, just for visual variation.
    x += ceil( float(speed) / 8 );
    y -= ceil( float(speed) / 16 );
}

//Maps the noise into colors and writes them out to the segment set
//The noise array is split segment line sections, we write each line out one at a time
void NoiseSL::mapNoiseSegsWithPalette(){
  
    //For each segment line we run over each segment, getting the noise value for the segment pixel from the noise array
    //We take the noise and map it into a palette/rainbow color and a brightness. 
    for (uint16_t i = 0; i < numLines; i++) {
        //current segment line's start index in the noise array
        noiseStart = i * numSegs; 
        for (uint16_t j = 0; j < numSegs; j++) {
            //The location of the noise data in the noise array
            noiseIndex = noiseStart + j;

            colorIndex = noise[noiseIndex];
            bri = noise[noiseIndex];

            //brighten up, as the color palette itself often contains the 
            //light/dark dynamic range desired
            if( bri > 127 ) {
                bri = 255;
            } else {
                bri = dim8_raw( bri * 2 );
            }

            //get the physical pixel location based on the line and seg numbers
            pixelNum = segDrawUtils::getPixelNumFromLineNum(segmentSet, numLines, j, i);

            //Get the output color based on the noise value
            switch (cMode){
                case 0:
                default:
                    //For mode 0 we set the color from the palette
                    //Scale color index to be somewhere between 0 and totBlendLength to put it somewhere in the blended palette
                    colorIndex = scale16by8( totBlendLength, colorIndex + ihue ); //colorIndex * totBlendLength /255;   
                    //get the resulting blended color and dim it by bri
                    colorOut = paletteUtilsPS::getPaletteGradColor(*palette, colorIndex, 0, totBlendLength, blendSteps);
                    //get background color info for the current pixel
                    colorTarget = segDrawUtils::getPixelColor(segmentSet, pixelNum, *bgColor, bgColorMode, j, i);
                    //get the color blended towards the background and output it
                    colorOut = colorUtilsPS::getCrossFadeColor(colorOut, colorTarget, 255 - bri);
                    //nscale8x3(colorOut.r, colorOut.g, colorOut.b, bri);
                    break;
                case 1:
                    //For mode 0 we set the color from the palette, but want to mainly pick from one color at a time
                    //so we map the colorIndex in the blendSteps, then use ihue to offset what color we're on
                    colorIndex = scale16by8( blendSteps, colorIndex ); //colorIndex * blendSteps /255;   
                    //get the resulting blended color and dim it by bri
                    colorOut = paletteUtilsPS::getPaletteGradColor(*palette, colorIndex, ihue, totBlendLength, blendSteps);
                    //get background color info for the current pixel
                    colorTarget = segDrawUtils::getPixelColor(segmentSet, pixelNum, *bgColor, bgColorMode, j, i);
                    //get the color blended towards the background and output it
                    colorOut = colorUtilsPS::getCrossFadeColor(colorOut, colorTarget, 255 - bri);
                    //nscale8x3(colorOut.r, colorOut.g, colorOut.b, bri);
                    break;
                case 2:
                    //Produces colors of the rainbow mapped to the noise val
                    //Noise tends to "clump up" around the middle of the range
                    //so we slowly offset the rainbow with ihue to produce all the colors more regularly
                    colorOut = CHSV( colorIndex + ihue, 255, bri );
                    break;
                case 3: 
                    //An alternative rainbow function that cycles through each color one at a time
                    //So most of the strip will be one color fading towards the next
                    //!!Make sure rotateHue is true!!
                    //Taken from https://github.com/FastLED/FastLED/blob/master/examples/Noise/Noise.ino
                    //We still use the colorIndex for the brightness here b/c I thought it looked better
                    colorOut = CHSV(ihue + (colorIndex>>2), 255, colorIndex );
                    break;
            }
            segDrawUtils::setPixelColor(segmentSet, pixelNum, colorOut, 0, 0, 0); 
        }
    }

    //shift the hue offset
    //helps deal with the fact that most noise tends to fall
    //in the middle of the range, so we have a shifting offset to help 
    //expose all the colors in a palette/rainbow
    if(rotateHue){
        switch (cMode){
            //For everything but case 1, we wrap the iHue at 255 (max possible noise value)
            //For case 1, we're using iHue to offset what palette color we're on, 
            //so we want to wrap it once we've gone through all the blended palette colors (totBlendLength)
            case 0: case 2: case 3: default:
                ihue = addmod8(ihue, 1, 255);
                break;
            case 1:
                ihue = addMod16PS(ihue, 1, totBlendLength);
                break;
        }
    }
}

//Shifts the scale towards the targetScale by one step (this keeps things smooth)
//If it's reached the target scale, pick a new target to shift to
//!!Do NOT set the scale directly after turning on shiftScale
//if you do, be sure to adjust scaleTarget = scale
void NoiseSL::setShiftScale(){
    if(scale == scaleTarget){
        //get a new target scale
        scaleTarget = scaleBase + random16(scaleRange);

        //set the scale step (+1 or -1)
        //if we happen to have re-rolled the current scale, then 
        //we want to re-roll. The easist way to do this is to set scaleStep to 0
        //so scaleTarget will stay equal to scale, triggering a re-roll when setShiftScale is called again
        if(scaleTarget == scale){
            scaleStep = 0;
        } else if (scaleTarget > scale){
            scaleStep = 1; 
        } else {
            scaleStep = -1;
        }
    }
    scale += scaleStep;  
}
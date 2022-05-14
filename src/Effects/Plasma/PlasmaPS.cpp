#include "PlasmaPS.h"

PlasmaPS::PlasmaPS(SegmentSet &SegmentSet, uint8_t numColors, uint16_t BlendSteps, bool Randomize, uint16_t Rate):
    segmentSet(SegmentSet), blendSteps(BlendSteps), randomize(Randomize)
    {    
        palletTemp = palletUtilsPS::makeRandomPallet(numColors);
        pallet = &palletTemp;
        init(Rate);
	}

PlasmaPS::PlasmaPS(SegmentSet &SegmentSet, palletPS *Pallet, uint16_t BlendSteps, bool Randomize, uint16_t Rate):
    segmentSet(SegmentSet), pallet(Pallet), blendSteps(BlendSteps), randomize(Randomize)
    {    
        init(Rate);
	}

PlasmaPS::~PlasmaPS(){
    delete[] palletTemp.palletArr;
}

//sets up variables for the effect
//if randomize is true, the frequencies and phase beat times will be randomized
void PlasmaPS::init(uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();

    if(randomize){
        //randomize the phases
        pAdj1 = random8(2);
        pAdj2 = random8(2);
        //set the inital frequencies equal to their targets so a new target is picked by shiftFreq();
        freq1 = 10;
        freq1Target = freq1;
        freq2 = 10;
        freq2Target = freq1;
        shiftFreq(&freq1, &freq1Target, &freq1Step, freq1Base, freq1Range);
        shiftFreq(&freq2, &freq2Target, &freq2Step, freq2Base, freq2Range);
    }
}

//This demonstrates 2D sinusoids in 1D using 16 bit math.
void PlasmaPS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        if(randomize){
            shiftFreq(&freq1, &freq1Target, &freq1Step, freq1Base, freq1Range);
            shiftFreq(&freq2, &freq2Target, &freq2Step, freq2Base, freq2Range);
        }
        //Setting phase change for a couple of waves.
        //(with the random salt added)
        phase1 = beatsin8(6 + pAdj1, -64, 64);
        phase2 = beatsin8(7 + pAdj2, -64, 64);

        numSegs = segmentSet.numSegs;
        totBlendLength = blendSteps * pallet->length;
        //run over each of the leds in the segment set and set a color value
        for (uint8_t i = 0; i < numSegs; i++) {
            totSegLen = segmentSet.getTotalSegLength(i);
            for(uint16_t j = 0; j < totSegLen; j++){ 

                //get the current pixel's location in the segment set
                pixelNum = segDrawUtils::getSegmentPixel(segmentSet, i, j);

                //For each of the LED's in the strand, set a brightness based on a wave as follows:
                //Create a wave and add a phase change and add another wave with its own phase change.
                colorIndex = cubicwave8( (pixelNum * freq1) + phase1 )/2 + cos8( (pixelNum * freq2) + phase2 )/2;          
                //qsub gives it a bit of 'black' dead space by setting sets a minimum value. 
                //If colorIndex < current value of beatsin8(), then bright = 0. Otherwise, bright = colorIndex.
                brightness = qsuba( colorIndex, beatsin8(briFreq, 0, briRange) );
                
                //scale color index to be somewhere between 0 and totBlendLength to put it somewhere in the blended pallet
                colorIndex = scale16by8( totBlendLength, colorIndex ); //colorIndex * totBlendLength /255;                        

                //get the blended color from the pallet and set it's brightness
                colorOut = palletUtilsPS::getPalletGradColor(pallet, colorIndex, 0, totBlendLength, blendSteps);
                //colorOut = colorUtilsPS::getCrossFadeColor(colorOut, 0, 255 - brightness);
                nscale8x3(colorOut.r, colorOut.g, colorOut.b, brightness);
                segmentSet.leds[pixelNum] = colorOut;
            }
        }
        showCheckPS();
    }
}

//Shifts the freq towards the targetFreq by one step (this keeps things smooth)
//If it's reached the target freq, pick a new target to shift to
//Some values are passed in as pointers because the function needs to modify them directly
//!!Do NOT set the freq directly after turning on randomize
//if you do, be sure to adjust freq == targetFreq
void PlasmaPS::shiftFreq(uint8_t *freq, uint8_t *freqTarget, int8_t *freqStep, uint8_t freqBase, uint8_t freqRange){
    if(*freq == *freqTarget){
        //get a new target scale
        *freqTarget = freqBase + random8(freqRange);

        //set the scale step (+1 or -1)
        //if we happen to have re-rolled the current scale, then 
        //we want to re-roll. The easist way to do this is to set scaleStep to 0
        //so scaleTarget will stay equal to scale, triggering a re-roll when setShiftScale is called again
        if(*freqTarget == *freq){
            *freqStep = 0;
        } else if (*freqTarget > *freq){
            *freqStep = 1; 
        } else {
            *freqStep = -1;
        }
    }
    *freq += *freqStep;  
}
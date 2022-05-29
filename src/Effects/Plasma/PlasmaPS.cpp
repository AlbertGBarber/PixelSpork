#include "PlasmaPS.h"

//Constructor for effect with pallet
PlasmaPS::PlasmaPS(SegmentSet &SegmentSet, palletPS *Pallet, uint16_t BlendSteps, bool Randomize, uint16_t Rate):
    segmentSet(SegmentSet), pallet(Pallet), blendSteps(BlendSteps), randomize(Randomize)
    {    
        init(Rate);
	}

//Constructor for effect with randomly chosen pallet
PlasmaPS::PlasmaPS(SegmentSet &SegmentSet, uint8_t numColors, uint16_t BlendSteps, bool Randomize, uint16_t Rate):
    segmentSet(SegmentSet), blendSteps(BlendSteps), randomize(Randomize)
    {    
        palletTemp = palletUtilsPS::makeRandomPallet(numColors);
        pallet = &palletTemp;
        init(Rate);
	}

PlasmaPS::~PlasmaPS(){
    delete[] palletTemp.palletArr;
}

//sets up variables for the effect
//if randomize is true, the phases, phaseBase beat times, and frequencies will be randomized
void PlasmaPS::init(uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();

    if(randomize){
        //randomize the phase bases
        pAdj1 = random8(2);
        pAdj2 = random8(2);
        randomizeFreq(10, 30);
        //set the inital phases equal to their targets so a new target is picked by shiftPhase();
        phase1 = 0;
        phase1Target = freq1;
        phase2 = 0;
        phase2Target = freq1;
        shiftPhase(&freq1, &phase1Target, &phase1Step, phase1Base, phase1Range);
        shiftPhase(&phase2, &phase2Target, &phase2Step, phase2Base, phase2Range);
    }
}

//Sets the wave frequencies to a random value between the passed in range
//Note that doing this after the effect has started will cause a jump in colors
void PlasmaPS::randomizeFreq(uint8_t freqMin, uint8_t freqMax){
    freq1 = random8(freqMin, freqMax);
    freq2 = random8(freqMin, freqMax);
}

//This demonstrates 2D sinusoids in 1D using 16 bit math.
//Basically we combine two waves use the result for bightness and color
//To keep the effect varied we use two phase values for each wave. 
//One varies using beatsin, while the other shifts up and down to random values over time
//Combined, these phases help give the effect a unique look and prevent it from repeating itself too often
void PlasmaPS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        pixelCount = 0;

        //shift towards the target phases for phase 1 and 2
        if(randomize){
            shiftPhase(&phase1, &phase1Target, &phase1Step, phase1Base, phase1Range);
            shiftPhase(&phase2, &phase2Target, &phase2Step, phase2Base, phase2Range);
        }

        //Setting phaseBase change for a couple of waves.
        //(with the random salt added)
        phaseWave1 = beatsin8(6 + pAdj1, -64, 64);
        phaseWave2 = beatsin8(7 + pAdj2, -64, 64);

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
                colorIndex = cubicwave8( (pixelCount * freq1) + phaseWave1 + phase1)/2 + cos8( (pixelCount * freq2) + phaseWave2 + phase2)/2;          
                //qsub gives it a bit of 'black' dead space by setting sets a minimum value. 
                //If colorIndex < current value of beatsin8(), then bright = 0. Otherwise, bright = colorIndex.
                brightness = qsuba( colorIndex, beatsin8(briFreq, 0, briRange) );
                
                //scale color index to be somewhere between 0 and totBlendLength to put it somewhere in the blended pallet
                colorIndex = scale16by8( totBlendLength, colorIndex ); //colorIndex * totBlendLength /255;                        

                //get the blended color from the pallet and set it's brightness
                colorOut = palletUtilsPS::getPalletGradColor(pallet, colorIndex, 0, totBlendLength, blendSteps);
                //colorOut = colorUtilsPS::getCrossFadeColor(colorOut, 0, 255 - brightness);
                nscale8x3(colorOut.r, colorOut.g, colorOut.b, brightness);
                segDrawUtils::setPixelColor(segmentSet, pixelNum, colorOut, 0, 0, 0); 
                pixelCount++;
            }
        }
        showCheckPS();
    }
}

//Shifts the phase towards the phaseFreq by one step (this keeps things smooth)
//If it's reached the target phase, pick a new target to shift to
//Some values are passed in as pointers because the function needs to modify them directly
//!!Do NOT set the phase directly after turning on randomize
//if you do, be sure to adjust phase == phaseFreq
void PlasmaPS::shiftPhase(uint8_t *phase, uint8_t *phaseTarget, int8_t *phaseStep, uint8_t phaseBase, uint8_t phaseRange){
    if(*phase == *phaseTarget){
        //get a new target scale
        *phaseTarget = phaseBase + random8(phaseRange);

        //set the scale step (+1 or -1)
        //if we happen to have re-rolled the current scale, then 
        //we want to re-roll. The easist way to do this is to set scaleStep to 0
        //so scaleTarget will stay equal to scale, triggering a re-roll when setShiftScale is called again
        if(*phaseTarget == *phase){
            *phaseStep = 0;
        } else if (*phaseTarget > *phase){
            *phaseStep = 1; 
        } else {
            *phaseStep = -1;
        }
    }
    *phase += *phaseStep;  
}

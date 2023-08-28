#include "PlasmaSL.h"

//Constructor for effect with palette
PlasmaSL::PlasmaSL(SegmentSetPS &SegSet, palettePS &Palette, uint16_t BlendSteps, bool Randomize, uint16_t Rate)
    : palette(&Palette), blendSteps(BlendSteps), randomize(Randomize)  //
{
    init(SegSet, Rate);
}

//Constructor for effect with randomly chosen palette
PlasmaSL::PlasmaSL(SegmentSetPS &SegSet, uint8_t numColors, uint16_t BlendSteps, bool Randomize, uint16_t Rate)
    : blendSteps(BlendSteps), randomize(Randomize)  //
{
    paletteTemp = paletteUtilsPS::makeRandomPalette(numColors);
    palette = &paletteTemp;
    init(SegSet, Rate);
}

PlasmaSL::~PlasmaSL() {
    free(paletteTemp.paletteArr);
}

//sets up variables for the effect
//if randomize is true, the phases, phaseBase beat times, and frequencies will be randomized
void PlasmaSL::init(SegmentSetPS &SegSet, uint16_t Rate) {
    //bind the rate and segSet pointer vars since they are inherited from BaseEffectPS
    bindSegSetPtrPS();
    bindClassRatesPS();

    if( randomize ) {
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
void PlasmaSL::randomizeFreq(uint8_t freqMin, uint8_t freqMax) {
    freq1 = random8(freqMin, freqMax);
    freq2 = random8(freqMin, freqMax);
}

//This demonstrates 2D sinusoids in 1D using 16 bit math.
//Basically we combine two waves use the result for brightness and color
//To keep the effect varied we use two phase values for each wave.
//One varies using beatsin, while the other shifts up and down to random values over time
//Combined, these phases help give the effect a unique look and prevent it from repeating itself too often
void PlasmaSL::update() {
    currentTime = millis();

    if( (currentTime - prevTime) >= *rate ) {
        prevTime = currentTime;

        //shift towards the target phases for phase 1 and 2
        if( randomize ) {
            shiftPhase(&phase1, &phase1Target, &phase1Step, phase1Base, phase1Range);
            shiftPhase(&phase2, &phase2Target, &phase2Step, phase2Base, phase2Range);
        }

        //Setting phaseBase change for a couple of waves.
        //(with the random salt added)
        phaseWave1 = beatsin8(6 + pAdj1, -64, 64);
        phaseWave2 = beatsin8(7 + pAdj2, -64, 64);

        numLines = segSet->numLines;
        totBlendLength = blendSteps * palette->length;

        //run over each of the lines in the segment set and set a color value
        for( uint16_t i = 0; i < numLines; i++ ) {
            //For each of the LED's in the strand, set a brightness based on a wave as follows:
            //Create a wave and add a phase change and add another wave with its own phase change.
            colorIndex = cubicwave8((i * freq1) + phaseWave1 + phase1) / 2 + cos8((i * freq2) + phaseWave2 + phase2) / 2;
            //qsub gives it a bit of 'black' dead space by setting sets a minimum value.
            //If colorIndex < current value of beatsin8(), then bright = 0. Otherwise, bright = colorIndex.
            brightness = qsuba(colorIndex, beatsin8(briFreq, 0, briRange));

            //scale color index to be somewhere between 0 and totBlendLength to put it somewhere in the blended palette
            colorIndex = scale16by8(totBlendLength, colorIndex);  //colorIndex * totBlendLength /255;

            //get the blended color from the palette and set it's brightness
            colorOut = paletteUtilsPS::getPaletteGradColor(*palette, colorIndex, 0, totBlendLength, blendSteps);
            //colorOut = colorUtilsPS::getCrossFadeColor(colorOut, 0, 255 - brightness);
            nscale8x3(colorOut.r, colorOut.g, colorOut.b, brightness);

            //reverse the line number so that the effect moves positively along the strip
            lineNum = numLines - i - 1;

            //write the color out to all the leds in the segment line
            segDrawUtils::drawSegLine(*segSet, lineNum, colorOut, 0);
        }
        showCheckPS();
    }
}

//Shifts the phase towards the phaseFreq by one step (this keeps things smooth)
//If it's reached the target phase, pick a new target to shift to
//Some values are passed in as pointers because the function needs to modify them directly
//!!Do NOT set the phase directly after turning on randomize
//if you do, be sure to adjust phase == phaseFreq
void PlasmaSL::shiftPhase(uint8_t *phase, uint8_t *phaseTarget, int8_t *phaseStep, uint8_t phaseBase, uint8_t phaseRange) {
    if( *phase == *phaseTarget ) {
        //get a new target scale
        *phaseTarget = phaseBase + random8(phaseRange);

        //set the scale step (+1 or -1)
        //if we happen to have re-rolled the current scale, then
        //we want to re-roll. The easiest way to do this is to set scaleStep to 0
        //so scaleTarget will stay equal to scale, triggering a re-roll when setShiftScale is called again
        if( *phaseTarget == *phase ) {
            *phaseStep = 0;
        } else if( *phaseTarget > *phase ) {
            *phaseStep = 1;
        } else {
            *phaseStep = -1;
        }
    }
    *phase += *phaseStep;
}

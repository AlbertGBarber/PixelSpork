#include "ScannerSL.h"

//Constructor for using a single color
ScannerSL::ScannerSL(SegmentSet &SegSet, CRGB Color, CRGB BGColor, uint16_t numWaves, uint8_t TrailType, uint16_t TrailSize, 
                     uint16_t Size, bool direction, bool alternate, bool makeEndWave, bool Bounce,
                     bool Blend, uint16_t Rate):
    trailSize(TrailSize), trailType(TrailType), size(Size), bounce(Bounce), blend(Blend)
    {    
        bounceChange = false;
        init(BGColor, SegSet, Rate);
        paletteTemp = paletteUtilsPS::makeSingleColorPalette(Color);
        palette = &paletteTemp;
        setPaletteAsPattern();
        makeWaveSet(numWaves, direction, alternate, makeEndWave);
	}

//Constructor for using a pattern with a custom set of repeating waves
ScannerSL::ScannerSL(SegmentSet &SegSet, patternPS &Pattern, palettePS &Palette, CRGB BGColor, uint16_t numWaves,
                     uint8_t TrailType, uint16_t TrailSize, uint16_t Size, bool direction, bool alternate, bool makeEndWave, 
                     bool Bounce, bool BounceChange, bool Blend, uint16_t Rate):
    pattern(&Pattern), palette(&Palette), trailSize(TrailSize), trailType(TrailType), size(Size), bounce(Bounce), bounceChange(BounceChange), blend(Blend)
    {    
        init(BGColor, SegSet, Rate);
        makeWaveSet(numWaves, direction, alternate, makeEndWave);
	}

//Constructor for using the palette as the pattern with a custom set of repeating waves
ScannerSL::ScannerSL(SegmentSet &SegSet, palettePS &Palette, CRGB BGColor, uint16_t numWaves, uint8_t TrailType,   
                     uint16_t TrailSize, uint16_t Size,  bool direction, bool alternate, bool makeEndWave, bool Bounce, 
                     bool BounceChange, bool Blend, uint16_t Rate):
    palette(&Palette), trailSize(TrailSize), trailType(TrailType), size(Size), bounce(Bounce), bounceChange(BounceChange), blend(Blend)
    {    
        init(BGColor, SegSet, Rate);
        setPaletteAsPattern();
        makeWaveSet(numWaves, direction, alternate, makeEndWave);
	}

//destructor
//since particleSetTemp is only ever set using particleUtilsPS::buildParticleSet() (which makes everything using new)
//we need to clean up it's memory. This includes the particles themselves.
//However if particleSetTemp has not been set, then trying to clean it up will probably cause an error
//by default, the particleArr pointer is NULL, so we can check for that to confirm if particleSetTemp has been used or not
ScannerSL::~ScannerSL(){
    free(trailEndColors);
    free(paletteTemp.paletteArr);
    free(patternTemp.patternArr);
    //clear the memory of the existing particles (to prevent a memory leak)
    particleUtilsPS::freeParticleSet(particleSet);
}

//initializes the core variables of the effect
void ScannerSL::init(CRGB BgColor, SegmentSet &SegSet, uint16_t Rate){
    //bind the rate and segSet pointer vars since they are inherited from BaseEffectPS
    bindSegSetPtrPS();
    bindClassRatesPS();

    //bind background color pointer
    bindBGColorPS();
    
    //initial fill to clear out any previous effects
    segDrawUtils::fillSegSetColor(*segSet, *bgColor, bgColorMode);
}

void ScannerSL::setPaletteAsPattern(){
    generalUtilsPS::setPaletteAsPattern(patternTemp, *palette);
    pattern = &patternTemp;
}

//creates a set of particles of numWaves, with the particles evenly spaced across the segment set
//also sets their initial directions, and will alternate the directions of each particle if alternate is true
//if makeEndWave is true, then, as long a you have 2 or more waves, 
//then a wave will always start at the end of the segment set lines
//Otherwise they will be evenly spaced.
//This lets you create traditional scanner patterns like the classic Kitt scanner (see Larson Scanner effect)
//Note that if makeEndWave is true, you should also set alternate to true, or the start/end waves will overlap
//(see inputs guide for better explanation)
//NOTE that we use each particle's "bounce" var to store its initial direction
//(all particles use the same bounce value, so we don't need to store it)
void ScannerSL::makeWaveSet(uint16_t numWaves, bool direction, bool alternate, bool makeEndWave){
    numLines = segSet->numLines;
    
    //We only need to make a new particle set if the current one isn't large enough
    //This helps prevent memory fragmentation by limiting the number of heap allocations
    //but this may use up more memory overall.
    if(numWaves > particleSet.maxLength){
        //clear the memory of the existing particles (to prevent a memory leak)
        particleUtilsPS::freeParticleSet(particleSet);
        particleSet = particleUtilsPS::buildParticleSet(numWaves, numLines, direction, *rate, 0, size, 0, trailType, trailSize, 0, bounce, 0, false);

        free(trailEndColors);
        trailEndColors = (CRGB*) malloc( numWaves * sizeof(CRGB));
    }
    //Set the length of the particle set to the number of waves
    //The particle set may be longer (due to numWavesMax above)
    //but any particles off the end of the set's length won't be drawn
    particleSet.length = numWaves;

    uint16_t spacing;

    //If we makeEndWave is true, we always want to place a wave at the end of the segment lines, 
    //otherwise we want the waves to be evenly spaced
    //We use spacing to determine the separation between each wave
    //With makeEndWave false, the waves a spaced by splitting the segment set into a length for each wave
    //If makeEndWave is true, then we increase the spacing so that one wave is always placed at the end of the segment set
    if( makeEndWave && numWaves > 1){ //numWaves - 1 must be greater than 0
        spacing = numLines / (numWaves - 1); 
    } else {
        spacing = numLines / numWaves; 
    }

    //set each particle's starting location and direction according to the spacing and size of the particle
    for(uint16_t i = 0; i < numWaves; i++){

        //Set the starting particle direction (stored in the bounce property)
        //This is used for determining when to change the particle's color
        particleSet.particleArr[i]->bounce = direction;
        
        //Alternate every other particle's direction if desired
        if(alternate && mod16PS(i, 2) != 0){
            particleUtilsPS::setParticleSetDirection(particleSet, i, !direction);
            particleSet.particleArr[i]->bounce = !direction;
        }

        position = i * spacing + size - 1; //offset -1 so we start at 0 for a 1 size particle
        //handle the position of the wave if it ends up being off the strip to prevent crashes
        //if we're bouncing, we place the wave back from the end, as if it already bounced, and adjust the starting direction,
        //otherwise, we just wrap it to the start
        if(position >= numLines){
            if(bounce){
                position = numLines - 1 - (size - 1); //offset -1 so we start at numLines - 1 for a 1 size particle
                
                //Since the particle is as the end of the segment set, it's "bounced" already
                //so we want its direction to flip from the inital direction
                //This prevents waves with size > 1 from starting in the wrong location due to the offset position from above
                if(direction == particleSet.particleArr[i]->direction){
                    particleSet.particleArr[i]->direction = !direction;
                    particleSet.particleArr[i]->bounce = !direction;
                }
            } else{
                position = mod16PS(position, numLines);
            }
        }
        particleUtilsPS::setParticleSetPosition(particleSet, i, position, false);
    }

    reset();
}

//resets all the particles to their start locations and sets their starting colors
void ScannerSL::reset(){
    //reset particles to starting locations
    particleUtilsPS::resetParticleSet(particleSet);

    //set the particle's starting colors
    //we need to run over them twice to set their initial and next colors
    patternIndexCount = 0;
    numParticles = particleSet.length;
    for(uint8_t i = 0; i < numParticles * 2; i++){
        setPartColor(particleSet.particleArr[ mod16PS(i, numParticles) ]);
    }

    //can't have 0 length trails, so we need to change the trail type
    if(trailSize == 0){
        particleUtilsPS::setParticleSetProp(particleSet, 4, 0, 0, 0);
    }

    segDrawUtils::fillSegSetColor(*segSet, *bgColor, bgColorMode);
}

///--------------------------------------------------------------------------------------
/*
This effect is basically a copy of ParticlesSL, but it includes code for switching the particle colors as they move.
So if you want to know how the particles move it would be better to look at ParticlesSL.
For the color switching, the goal of the effect is to produce "sonar-like" waves, ie a particles with fading trails
with the added option of using multiple colors for the waves (with the colors changing as the waves cycle)
The colors cycle according to a color pattern and are take from a palette. This can produce some neat effects.
It should be noted that all the waves have the same speed, size, trails, etc
Waves can be set to either bounce back or wrap when reaching the start/end of the segment lines
They can be set to have their colors change every time they bounce, or only when the reach their starting point
We need to keep the trail colors consistent as they bounce or wrap:
For example if a wave moving forward, not bouncing, moves off the end of the strip and wraps to the start again
it will change color, but we want to keep the part of the trail that has not wrapped yet the previous color
so that it looks like the wave is moving off the segment set.
To do this we keep track of the next wave color, its current color and its previous color
(Note, since particles only have one native color index value, we use the life and maxLife values to store the previous and next wave colors)
Then whenever we draw part of a wave we check where the part is and set its color appropriately
When a wave either wraps or bounces, we change its colors.

The color changing happens in moveParticle() using setPartColor();
while getPartPixelColor() is used to get the correct color for each particle piece
(for trails there's a small extra bit to cover the trails of bouncing waves)
*/
//---------------------------------------------------------------------------------------------------------

//Updates all the particles
//Note that this effect is set up to draw particles along segment lines
//So each particle is draw along all segment lines, with the longest segment setting the maximum particle position
//Outline:
//(Note the unlike ParticlesSL, all the scanner particles have the same properties, ie bounce, speed, etc
//so we don't actually get them from the particles, rather we use the variables from the effect)
//On each update cycle we check each particle to see if it should move, if so, we move it (dealing with bouncing and wrapping)
//Then, if the particle moved, we set the background color on the segment line that the particle has moved off of (the last pixel in the trail)
//Next, even if the particle has not moved, we re-draw the trails and the particle body
//This prevents another, faster particle from wiping out a slower on, only to have the slower one suddenly re-appear once it's time to move it again
//To minimize the number of re-draws (we don't need to re-draw if no particles have moved) we limit the overall update rate
//to that of the fastest particle. This is re-checked every update.
//Bounce and end behavior:
//For a particle to bounce, it must reverse it's direction once it hits either end of the segSet's lines
//However, how/when it bounces is a matter of opinion. I have opted for the following:
//The particle only bounces when it's main body (not trail) reaches an end point.
//Both the front and rear trails wrap back on themselves as the particle bounces
//Ie the head of the trail moves back down the strip, opposite the direction of the particle
//The rear trail is always drawn last.
//In practice this means that particles with two trails mimics the classic "cylon" scanner look, where the front of the 
//trail moves disappears off the strip (it is actually wrapping back, but is over written by the rear trail, which is drawn after)
//While for particles with only a rear trail, it naturally fades as like it would for a physical streamer/flame/etc
//Finally, for particles with only a front trail the trail also wraps back, this does look a little weird, but 
//there's not a good real world approximation to this kind of particle, so w/e.
//For particles where the body size is larger than one, when bounce happens, the entire body reverses direction at once
//This is not visually noticeable, and makes coding easier. But it does mean there's no "center" of a particle
void ScannerSL::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;
        //need to reset the update rate each cycle, to make sure we always set it to the fastest particle

        //refill the background if directed (if we're using a dynamic rainbow or something)
        if(fillBG || blend){
            segDrawUtils::fillSegSetColor(*segSet, *bgColor, bgColorMode);
        }

        //re-fetch the segment vars in-case they've been modified
        numParticles = particleSet.length;
        numLines = segSet->numLines;
        numSegs = segSet->numSegs;
        longestSeg = segSet->segNumMaxNumLines;

        //for each particle, in order:
        //move it to it's next position (ie line number)
        //then set the line at the previous location to the BgColor (including trail ends)
        //(Setting the background has some tricks, see the code comments below)
        //then draws new trails if needed
        //then draws the particle in it's new position
        //Note that drawing a particle draws it across all segment lines, so the particle is replicated multiple times
        //This order helps prevent overwriting different parts of the particles if there are clashes
        //As a particle passes over another, we want to maintain the look of both particles as much as possible
        //This is why we always re-draw all the particles with each update, so that a fast particle doesn't erase a slower one
        //Also we pay special attention when setting the background pixel:
        //If a previous particle from the set has already overwritten the background pixel, then we don't need to set it
        //To track this, we record the color that we expect the background pixel to be (it's the last color in the trail/body)
        //Then when we go to set the background, we check the current color of the pixel
        //If it doesn't match, then we don't overwrite.
        //For segments with multiple lines we use the particle pixel that's on the longest segment to check the background color
        //For the longest segment the full particle will always be drawn (it may be truncated on shorted segments)
        //so we know that if the trail pixel has not been over-written, it should not have been over-written on other segments
        //In any case, this makes the programming much simpler, saves storage, and seems to work well in practice from what I can tell
        for (uint8_t i = 0; i < numParticles; i++) {
            //get the particle from the set, and record some vars locally for ease of access
            particlePtr = particleSet.particleArr[i];
            position = particlePtr->position; //the current position of the particle
            direction = particlePtr->direction; //the direction of motion (true moves towards the last pixel in the segmentSet)
            startDirect = particlePtr->bounce; //We store the initial direction of the particle in its bounce var

            //particles run from 0 to the maximum segment line number (numLines), either bouncing or wrapping depending on mode
            //if we're bouncing, we don't want anything to wrap past the end/start of the segment lines
            //so we want to keep the maxPosition larger than the trailLineNum can reach
            //ie the numLines plus the tailLength, as this is will allow the tail to extend off the strip,
            //which will be discarded by the setPixelColor func, as the pixel is out of bounds
            //If we're not bouncing, then we want to wrap back to 0, so our maxPosition is just numLines
            if (bounce) {
                maxPosition = numLines + trailSize + 1;
            } else {
                maxPosition = numLines;
            }

            //all scanner particle are the same speed, so they all update at once
            //particlePtr->lastUpdateTime = currentTime;
            moveParticle(particlePtr);

            //get the current, previous and next particle colors
            partColor = paletteUtilsPS::getPaletteColor(*palette, particlePtr->colorIndex);
            nextColor = paletteUtilsPS::getPaletteColor(*palette, particlePtr->maxLife);
            prevColor = paletteUtilsPS::getPaletteColor(*palette, particlePtr->life);

            //get the multiplier for the particle direction (1 or -1)
            //we need this for drawing the trails and the particle body in the correct direction
            directStep = particleUtilsPS::getDirectStep(direction);

            //if we have 4 trails or more, we are in infinite trails mode, so we don't touch the previous leds
            //otherwise we need to set the last pixel in the trail to the background
            if ( trailType < 4 && (!fillBG && !blend) ) {
                //if we don't have trails, we just need to turn off the first trail pixel
                //otherwise we need to switch the pixel at the end of the trail
                if (trailType == 0 || trailType == 3) {
                    trailLineNum = getTrailLocAndColor(true, 1, maxPosition);
                } else if( trailType == 1 || trailType == 2 ){
                    //in the case of two trails, we only need to set the rear trail, since the 
                    //front one will be overwritten as the particle moves
                    trailLineNum = getTrailLocAndColor(true, trailSize + 1, maxPosition);
                }

                //get the physical pixel location and the color it's meant to be
                pixelNum = segDrawUtils::getPixelNumFromLineNum(*segSet, numLines, longestSeg, trailLineNum);
                //only turn off the pixel if it hasn't been touched by another particle's trail (or something else)
                //this prevents background holes from being placed in other particles
                //For segments with multiple lines we use the particle pixel that's on the longest segment to check the background color
                //For the longest segment the full particle will always be drawn (it may be truncated on shorted segments)
                //so we know that if the trail pixel has not been over-written, it should not have been over-written on other segments
                if(segSet->leds[pixelNum] == trailEndColors[i]){
                    segDrawUtils::drawSegLine(*segSet, trailLineNum, *bgColor, bgColorMode);
                }
            }

            //draw the particle trails and body on each segment in the segment set at the particle's line position
            //Note that we basically re-calculate the particle colors for each segment, which isn't the best
            //but we more-or-less need to do it this way to deal with color modes
            for(uint16_t j = 0; j < numSegs; j++){
                //if we have trails, draw them
                //if it has two, we draw the trail in front of the particle first, followed by the one behind it
                //this gets the correct look when bouncing with two trails
                if (trailType != 0 && trailType < 4) {
                    //draw a trail, extending to the front or rear, dimming with each step, wrapping according to the maxPosition
                    //we draw the trail front first, so that on bounces the brighter part of the trail over-writes the dimmer part
                    for (uint8_t k = trailSize; k > 0; k--) {
                        
                        //Note that getTrailLocAndColor() also stores the color of the trail piece in targetColor

                        //if we have two trails, we need to draw the front trail
                        if (trailType == 2 || trailType == 3) {
                            trailLineNum = getTrailLocAndColor(false, k, maxPosition);
                            setTrailColor(trailLineNum, j, k);
                        }
                        
                        //draw the rear trail
                        if (trailType == 1 || trailType == 2) {
                            trailLineNum = getTrailLocAndColor(true, k, maxPosition);
                            setTrailColor(trailLineNum, j, k);
                            //If we have rear trails, we need to record the trail end color at the end of the trail
                            //but only if we're writing to the longest seg (see notes in the background setting code above, and the intro)
                            if(k == trailSize && j == longestSeg){
                                trailEndColors[i] = segSet->leds[pixelNum];
                            }
                        }
                    }
                }

                //draw the main particle
                //we always start at the particle's head and move opposite the direction of motion
                //Note that if we're bouncing, moveParticle() will shift the particle back by it's size when a bounce happens
                //so we don't need to worry about (position - k * directStep) becoming negative
                for (uint16_t k = 0; k < size; k++) {

                    if (bounce) {
                        trailLineNum = (position - k * directStep);
                    } else {
                        //add numLEDs to prevent the value from being negative before the mod
                        //(arduino handles mods of negative numbers weirdly)
                        trailLineNum = addMod16PS( position, numLines - k * directStep, numLines); //( (position - k * directStep) + numLEDs) % numLEDs;
                    }

                    //get the color of the body part based on its position relative to the head
                    colorTarget = getPartPixelColor(trailLineNum, true);

                    //get the pixel location and color and set it
                    pixelNum = segDrawUtils::getPixelNumFromLineNum(*segSet, numLines, j, trailLineNum);
                    colorFinal = segDrawUtils::getPixelColor(*segSet, pixelNum, colorTarget, colorMode, j, trailLineNum);
                    
                    if(blend){
                        segSet->leds[pixelNum] += colorFinal;
                    } else {
                        segSet->leds[pixelNum] = colorFinal;
                    }
                    //Need to check to dim the pixel color manually
                    //b/c we're not calling setPixelColor directly
                    segDrawUtils::handleBri(*segSet, pixelNum);

                    //if we don't have a rear trail, then the next pixel that needs to be set to background
                    //is the last pixel in the particle body, so we record it's color
                    //but only if we're writing to the longest seg (see notes in the background setting code above, and the intro)
                    if( k == (size - 1) && (trailType == 0 || trailType == 3) && j == longestSeg ){
                        trailEndColors[i] = segSet->leds[pixelNum];
                    }
                }
            }
        }
        showCheckPS();
    }
}

//Moves the particle according to it's direction
//also handles bounce behavior
//when bouncing, the whole particle body is reversed, so it's head position is 
//shifted back/forward equal to it's size
//It also handles changing the particle's color 
void ScannerSL::moveParticle(particlePS *particlePtr) {
    position = particlePtr->position;

    //get the multiplier for the direction (1 or -1)
    directStep = particleUtilsPS::getDirectStep(direction);

    //get the next position of the particle 
    //if we're bouncing, then the particle can step outside of the strip
    //due to the mod amount, we catch this below
    position = addMod16PS(position, maxPosition + directStep, maxPosition);

    //if we're bouncing, and we're either at the start or end of the segment lines
    //we need to reverse direction, and shift the particle's position by size from either end
    //Due to how the mod works, particles outside of the strip
    //will have positions >= numLines
    if(bounce && position >= numLines){
        if(direction){
            position = numLines - size;
            direction = false;
            //If we're changing colors every bounce, or we're back at the starting direction
            //we need to set a new particle color
            if(bounceChange || !startDirect){
                setPartColor(particlePtr);
            }
        } else{
            position = size - 1;
            direction = true;
            //If we're changing colors every bounce, or we're back at the starting direction
            //we need to set a new particle color
            if(bounceChange || startDirect){
                setPartColor(particlePtr);
            }
        }
        particlePtr->direction = direction;
    } else if( (direction && position == 0) || (!direction && position == numLines - 1) ){
        //If we're not bouncing, we need to change the particle's color 
        //when it either passes 0 or the last line, depending on its direction.
        //If bounce change is false, the 
        setPartColor(particlePtr);
    }

    //update the particle's position
    particlePtr->position = position;
}

//Gives the particle a new color while also setting the next and previous color indexes (for the palette)
//The Previous color is stored in the particle's life property
//The current color is stored in the particle's colorIndex property
//The next color is stored in the particle's maxLife property
//randModes:
//  0: Colors will be chosen in order from the pattern (not random)
//  1: Colors will be chosen randomly from the pattern
//  2: Colors will be chosen at random from the pattern,
//     but the same color won't be repeated in a row
//  3: new colors will not be chosen, the particle colors will be locked to whatever they currently are
void ScannerSL::setPartColor(particlePS *particlePtr){
    particlePtr->life = particlePtr->colorIndex;

    if(randMode != 3){
        particlePtr->colorIndex = particlePtr->maxLife;
    }
    
    //Get the next particle color based on the random mode
    if(randMode == 0){
        //Get the next particle color, taking into account the number of active particles
        //(don't want to have every particle repeat the pattern individually, rather all the particles do the pattern together)
        particlePtr->maxLife = patternUtilsPS::getPatternVal(*pattern, patternIndexCount + numParticles);
        //Once we've set a color, we need to advance the pattern index for the next particle
        patternIndexCount = addMod16PS(patternIndexCount, 1, pattern->length);
    } else if(randMode == 1){
        //get a color at random from the pattern
        particlePtr->maxLife = patternUtilsPS::getRandVal(*pattern);
    } else if(randMode == 2){
        //get a color at random from the pattern (but it won't be the same as the current color)
        particlePtr->maxLife = patternUtilsPS::getShuffleVal(*pattern, particlePtr->colorIndex);
    }
    
}

//writes out the trail color according to the pixel number in the trail (ie 0 - trailSize)
//the trail is blended towards background color according to the trailSize
void ScannerSL::setTrailColor(uint16_t trailLineNum, uint8_t segNum, uint8_t trailPixelNum){
    //get the physical pixel location, it's color, and the target background color
    pixelNum = segDrawUtils::getPixelNumFromLineNum(*segSet, numLines, segNum, trailLineNum);
    
    trailBgColor = segDrawUtils::getPixelColor(*segSet, pixelNum, *bgColor, bgColorMode, segNum, trailLineNum);
    colorTarget = segDrawUtils::getPixelColor(*segSet, pixelNum, colorTarget, colorMode, segNum, trailLineNum);

    //blend the color into the background according to where we are in the trail
    colorFinal = particleUtilsPS::getTrailColor(colorTarget, trailBgColor, trailPixelNum, trailSize, dimPow);
    //output the color
    if(blend){
        segSet->leds[pixelNum] += colorFinal;
    } else {
        segSet->leds[pixelNum] = colorFinal;
    }    
    //Need to check to dim the pixel color manually
    //b/c we're not calling setPixelColor directly
    segDrawUtils::handleBri(*segSet, pixelNum);             
}

//returns the position of a trail pixel(local to the segment) based on the trail direction, and the mod amount
//trailDirect = true => a rear trail
//trailDirect = false => a front trail
//Note that it also stores the trail pixel color in colorTarget.
uint16_t ScannerSL::getTrailLocAndColor(bool trailDirect, uint8_t trailPixelNum, uint16_t maxPosition) { 
    //get the multiplier for the direction (1 or -1)
    //sets if the trail will be drawn forwards or backwards
    trailDirectionAdj = particleUtilsPS::getDirectStep(trailDirect);

    //since we draw the body of the particle behind the lead pixel, 
    //we need to offset rear trails by the body size
    sizeAdj = size - 1;

    if(!trailDirect){
        sizeAdj = 0;
    }

    //worked this formula out by hand, basically just adds/subtracts the trail location from the particle location
    //wrapping according to the maxPosition (note that we add the maxPosition to prevent this from being negative, arduino handles mods of negative numbers weirdly)
    //Because we mod by the maxPosition, bouncing trails can extend off the end of the strip in either direction
    //(due to the way mod works, anything outside of the segment set always ends up with a position of > numLine,
    //no matter the particle direction)
    //This behavior is intended so we can handle re-direct any trails off the end of the strip, to wrap back on themselves
    //( position + maxPosition - trailDirectionAdj * ( directStep * ( trailPixelNum + sizeAdj ) ) ) % maxPosition;
    trailLocOutput = addMod16PS(position, maxPosition - trailDirectionAdj * ( directStep * ( trailPixelNum + sizeAdj ) ), maxPosition);

    //If we're bouncing and the trail part would go off the end of the segment set
    //we need to re-direct the trail part back along the strip, in the opposite direction of the particle
    //like you'd see on a flame or stream irl, which traces the particle's previous motion (or next for front trails))
    //to do this we take the portion of the trail that is currently off the strip 
    //and redirect it back along the strip according to the direction and trail direction
    if(bounce && trailLocOutput >= numLines){
        //The re-directed trail position depends on the direction of the particle and trail direction
        //For rear trails the particle direction changes before ahead of the trail
        //So the trail needs to be directed in the opposite direction to the particle's motion
        //For front trails, the opposite is true, because the trail needs to change direction ahead of the particle
        if((trailDirect && direction) || (!trailDirect && !direction) ){  
            trailLocOutput = maxPosition - trailLocOutput;
        } else {
            trailLocOutput = (numLines - 1) - mod16PS( trailLocOutput, (numLines - 1) );
        }
        //displace the trail by the size of the particle opposite to the direction of the particle
        trailLocOutput = trailLocOutput + directStep * sizeAdj;

        //For this bounce-return case, we still need to keep the trail the previous color
        //This is most easily caught here rather than in getPartPixelColor()
        //We also only need to keep the previous color if the particle is changing color for this bounce
        if( bounceChange || (startDirect == direction) ){
            colorTarget = prevColor;
        } else {
            colorTarget = partColor;
        }
    } else {
        //if we aren't handling the bounce-return case, we just get the color as normal
        colorTarget = getPartPixelColor(trailLocOutput, trailDirect);
    }

    return trailLocOutput;
}

//Returns the color of a particle's pixel so that the wave colors change correctly as they bounce or wrap
CRGB ScannerSL::getPartPixelColor(uint16_t partPixelLoc, bool trailDirect){
    if(bounce){ //For bouncing particles, it turns out that the correct color is always the trail color (except the for the case in getTrailLedLoc())
        return partColor;
    } else {
        //If we're not bouncing, then any particle pieces that have not yet wrapped must be kept the previous particle color
        //while any that have wrapped must be set to the next color
        if(trailDirect){ //trail behind particle
            //if we're moving forward, the particle wraps to 0, so any trail parts that have position greater
            //then the particle head's need to be the previous color
            //(This is reversed for a particle going backwards)
            if( (direction && partPixelLoc > position) || (!direction && partPixelLoc < position)){
                return prevColor;
            }
        } else { //trail ahead of particle
            //If the trail is wrapping ahead of the particle, then it needs to be the next particle color
            if( (direction && partPixelLoc < position) || (!direction && partPixelLoc > position)){
                return nextColor;
            }
        }
        //if the particle part is not wrapped, then it just needs to be the particle color
        return partColor;
    }
}
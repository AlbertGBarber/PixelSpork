#include "ParticlesSL.h"

//constructor for automatically making a particle set according to the passed in options
ParticlesSL::ParticlesSL(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, uint8_t numParticles, uint8_t direction,
                         uint16_t baseSpeed, uint16_t speedRange, uint16_t size, uint16_t sizeRange,
                         uint8_t trailType, uint8_t trailSize, uint8_t trailRange, uint8_t bounce,
                         uint8_t colorIndex, bool randColor)
    : palette(&Palette)  //
{
    init(BgColor, SegSet);
    numLines = segSet->numLines;
    particleSetTemp = particleUtilsPS::buildParticleSet(numParticles, numLines, direction, baseSpeed,
                                                        speedRange, size, sizeRange, trailType, trailSize,
                                                        trailRange, bounce, colorIndex, randColor);
    particleSet = &particleSetTemp;
    trailEndColors = (CRGB *)malloc(numParticles * sizeof(CRGB));
}

//constructor for using a particle set you've already made
ParticlesSL::ParticlesSL(SegmentSetPS &SegSet, particleSetPS &ParticleSet, palettePS &Palette, CRGB BgColor)
    : particleSet(&ParticleSet), palette(&Palette)  //
{
    init(BgColor, SegSet);
    trailEndColors = (CRGB *)malloc((particleSet->length) * sizeof(CRGB));
}

//destructor
//since particleSetTemp is only ever set using particleUtilsPS::buildParticleSet() (which makes everything using new)
//we need to clean up it's memory. This includes the particles themselves.
//However if particleSetTemp has not been set, then trying to clean it up will probably cause an error
//by default, the particleArr pointer is NULL, so we can check for that to confirm if particleSetTemp has been used or not
ParticlesSL::~ParticlesSL() {
    //clear the memory of the existing particles (to prevent a memory leak)
    particleUtilsPS::freeParticleSet(particleSetTemp);
    free(trailEndColors);
}

//initializes the core variables of the effect
void ParticlesSL::init(CRGB BgColor, SegmentSetPS &SegSet) {
    //bind the rate and segSet pointer vars since they are inherited from BaseEffectPS
    //The effect uses the rates of the particles, but all effects must have a Rate var, so we make one up
    uint16_t Rate = 80;
    bindSegSetPtrPS();
    bindClassRatesPS();

    //bind background color pointer
    bindBGColorPS();
    //initial fill to clear out any previous effects
    segDrawUtils::fillSegSetColor(*segSet, *bgColor, bgColorMode);
}

//sets the effect to use a new particle set
//we need to remake the trail end color array if there are more particles than previously
//to avoid having left over trails, we'll redraw the background
void ParticlesSL::setParticleSet(particleSetPS &newParticleSet) {

    //We only need to make a new trail end color array if 
    //the current ones aren't large enough
    //This helps prevent memory fragmentation by limiting the number of heap allocations
    //but this may use up more memory overall.
    if( alwaysResizeObj_PS || (newParticleSet.maxLength > particleSet->maxLength) ) {
        free(trailEndColors);
        trailEndColors = (CRGB *)malloc((newParticleSet.maxLength) * sizeof(CRGB));
    }

    particleSet = &newParticleSet;

    segDrawUtils::fillSegSetColor(*segSet, *bgColor, bgColorMode);
}

//resets all the particles to their start locations
void ParticlesSL::reset() {
    particleUtilsPS::resetParticleSet(*particleSet);
}

/* Updates all the particles
Note that this effect is set up to draw particles along segment lines
So each particle is draw along all segment lines, with the longest segment setting the maximum particle position
Outline:
On each update cycle we check each particle to see if it should move, if so, we move it (dealing with bouncing and wrapping)
Then, if the particle moved, we set the background color on the segment line that the particle has moved off of (the last pixel in the trail)
Next, even if the particle has not moved, we re-draw the trails and the particle body
This prevents another, faster particle from wiping out a slower on, only to have the slower one suddenly re-appear once it's time to move it again
To minimize the number of re-draws (we don't need to re-draw if no particles have moved) we limit the overall update rate
to that of the fastest particle. This is re-checked every update.
Bounce and end behavior:
For a particle to bounce, it must reverse it's direction once it hits either end of the segSet's lines
However, how/when it bounces is a matter of opinion. I have opted for the following:
The particle only bounces when it's main body (not trail) reaches an end point.
Both the front and rear trails wrap back on themselves as the particle bounces
Ie the head of the trail moves back down the strip, opposite the direction of the particle
The rear trail is always drawn last.
In practice this means that particles with two trails mimics the classic "cylon" scanner look, where the front of the
trail moves disappears off the strip (it is actually wrapping back, but is over written by the rear trail, which is drawn after)
While for particles with only a rear trail, it naturally fades as like it would for a physical streamer/flame/etc
Finally, for particles with only a front trail the trail also wraps back, this does look a little weird, but
there's not a good real world approximation to this kind of particle, so w/e.
For particles where the body size is larger than one, when bounce happens, the entire body reverses direction at once
This is not visually noticeable, and makes coding easier. But it does mean there's no "center" of a particle */
void ParticlesSL::update() {
    currentTime = millis();

    if( (currentTime - prevTime) >= updateRate ) {
        prevTime = currentTime;
        //need to reset the update rate each cycle, to make sure we always set it to the fastest particle
        updateRate = 65535;  //max value of a uint16_t. This is an update rate of 65.5 sec.

        //refill the background if directed (if we're using a dynamic rainbow or something)
        if( fillBG || blend ) {
            segDrawUtils::fillSegSetColor(*segSet, *bgColor, bgColorMode);
        }

        //re-fetch the segment vars in-case they've been modified
        numParticles = particleSet->length;
        numLines = segSet->numLines;
        numSegs = segSet->numSegs;
        longestSeg = segSet->segNumMaxNumLines;

        /* for each particle, in order:
        move it to it's next position (ie line number) (if needed)
        then set the line at the previous location to the BgColor (including trail ends)
        (Setting the background has some tricks, see the code comments below)
        then draws new trails if needed
        then draws the particle in it's new position
        Note that drawing a particle draws it across all segment lines, so the particle is replicated multiple times
        This order helps prevent overwriting different parts of the particles if there are clashes
        As a particle passes over another, we want to maintain the look of both particles as much as possible
        This is why we always re-draw all the particles with each update, so that a fast particle doesn't erase a slower one
        Also we pay special attention when setting the background pixel:
        If a previous particle from the set has already overwritten the background pixel, then we don't need to set it
        To track this, we record the color that we expect the background pixel to be (it's the last color in the trail/body)
        Then when we go to set the background, we check the current color of the pixel
        If it doesn't match, then we don't overwrite.
        For segments with multiple lines we use the particle pixel that's on the longest segment to check the background color
        For the longest segment the full particle will always be drawn (it may be truncated on shorted segments)
        so we know that if the trail pixel has not been over-written, it should not have been over-written on other segments
        In any case, this makes the programming much simpler, saves storage, and seems to work well in practice from what I can tell */
        for( uint8_t i = 0; i < numParticles; i++ ) {
            //get the particle from the set, and record some vars locally for ease of access
            particlePtr = particleSet->particleArr[i];
            position = particlePtr->position;    //the current position of the particle
            speed = particlePtr->speed;          //the speed of the particle
            direction = particlePtr->direction;  //the direction of motion (true moves towards the last pixel in the segmentSet)
            size = particlePtr->size;            //the length of the main body of the particle
            trailType = particlePtr->trailType;  //the type of trail for the particle (see above for types)
            trailSize = particlePtr->trailSize;  //the length of the trail(s) of the particle (only applies if the pixel has a trail)
            bounce = particlePtr->bounce;        //sets if the particle wraps to the beginning of the segSet once it reaches the end, or if it reverses direction (it bounces)

            //record the fastest particle for accurate effect updating
            if( speed < updateRate ) {
                updateRate = speed;
            }

            partColor = paletteUtilsPS::getPaletteColor(*palette, particlePtr->colorIndex);

            //particles run from 0 to the maximum segment line number (numLines), either bouncing or wrapping depending on mode
            //if we're bouncing, we don't want anything to wrap past the end/start of the segment lines
            //so we want to keep the maxPosition larger than the trailLineNum can reach
            //ie the numLines plus the tailLength, as this is will allow the tail to extend off the strip,
            //which will be discarded by the setPixelColor func, as the pixel is out of bounds
            //If we're not bouncing, then we want to wrap back to 0, so our maxPosition is just numLines
            if( bounce ) {
                maxPosition = numLines + trailSize + 1;
            } else {
                maxPosition = numLines;
            }

            //if enough time has passed, we need to move the particle
            movePart = ((currentTime - particlePtr->lastUpdateTime) >= speed);
            //if the particle needs to move, move it and record the time
            if( movePart ) {
                particlePtr->lastUpdateTime = currentTime;
                moveParticle(particlePtr);
            }

            //get the multiplier for the particle direction (1 or -1)
            //we need this for drawing the trails and the particle body in the correct direction
            directStep = particleUtilsPS::getDirectStep(direction);

            //if we have 4 trails or more, we are in infinite trails mode, so we don't touch the previous leds
            //otherwise we need to set the last pixel in the trail to the background
            if( trailType < 4 && (!fillBG && !blend) ) {
                //if we don't have trails, we just need to turn off the first trail pixel
                //otherwise we need to switch the pixel at the end of the trail
                if( trailType == 0 || trailType == 3 ) {
                    trailLineNum = getTrailLedLoc(true, 1, maxPosition);
                } else if( trailType == 1 || trailType == 2 ) {
                    //in the case of two trails, we only need to set the rear trail, since the
                    //front one will be overwritten as the particle moves
                    trailLineNum = getTrailLedLoc(true, trailSize + 1, maxPosition);
                }

                //get the physical pixel location and the color it's meant to be
                //segDrawUtils::getPixelColor(segSet, &pixelInfo, *bgColor, bgColorMode, trailLineNum);
                pixelNum = segDrawUtils::getPixelNumFromLineNum(*segSet, longestSeg, trailLineNum);
                //only turn off the pixel if it hasn't been touched by another particle's trail (or something else)
                //this prevents background holes from being placed in other particles
                //For segments with multiple lines we use the particle pixel that's on the longest segment to check the background color
                //For the longest segment the full particle will always be drawn (it may be truncated on shorted segments)
                //so we know that if the trail pixel has not been over-written, it should not have been over-written on other segments
                if( segSet->leds[pixelNum] == trailEndColors[i] ) {
                    //segSet->leds[pixelInfo.pixelLoc] = pixelInfo.color;
                    segDrawUtils::drawSegLine(*segSet, trailLineNum, *bgColor, bgColorMode);
                }
            }

            //draw the particle trails and body on each segment in the segment set at the particle's line position
            //Note that we basically re-calculate the particle colors for each segment, which isn't the best
            //but we more-or-less need to do it this way to deal with color modes
            for( uint16_t j = 0; j < numSegs; j++ ) {
                //if we have trails, draw them
                //if it has two, we draw the trail in front of the particle first, followed by the one behind it
                //this gets the correct look when bouncing with two trails
                if( trailType != 0 && trailType < 4 ) {
                    //draw a trail, extending to the front or rear, dimming with each step, wrapping according to the maxPosition
                    //we draw the end of the trail first, so that on bounces the brighter part of the trail over-writes the dimmer part
                    for( uint8_t k = trailSize; k > 0; k-- ) {

                        //if we have two trails, we need to draw the front trail
                        if( trailType == 2 || trailType == 3 ) {
                            trailLineNum = getTrailLedLoc(false, k, maxPosition);
                            setTrailColor(trailLineNum, j, k);
                        }

                        //draw the rear trail
                        if( trailType == 1 || trailType == 2 ) {
                            trailLineNum = getTrailLedLoc(true, k, maxPosition);
                            setTrailColor(trailLineNum, j, k);
                            //If we have rear trails, we need to record the trail end color at the end of the trail
                            //but only if we're writing to the longest seg (see notes in the background setting code above, and the intro)
                            if( k == trailSize && j == longestSeg ) {
                                trailEndColors[i] = segSet->leds[pixelNum];
                            }
                        }
                    }
                }

                //draw the main particle
                //we always start at the particle's head and move opposite the direction of motion
                //Note that if we're bouncing, moveParticle() will shift the particle back by it's size when a bounce happens
                //so we don't need to worry about (position - k * directStep) becoming negative
                for( uint16_t k = 0; k < size; k++ ) {

                    if( bounce ) {
                        trailLineNum = (position - k * directStep);
                    } else {
                        //add numLEDs to prevent the value from being negative before the mod
                        //(arduino handles mods of negative numbers weirdly)
                        trailLineNum = addMod16PS(position, numLines - k * directStep, numLines);  //( (position - k * directStep) + numLEDs) % numLEDs;
                    }

                    //get the pixel location and color and set it
                    pixelNum = segDrawUtils::getPixelNumFromLineNum(*segSet, j, trailLineNum);
                    colorFinal = segDrawUtils::getPixelColor(*segSet, pixelNum, partColor, colorMode, j, trailLineNum);

                    if( blend ) {
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
                    if( k == (size - 1) && (trailType == 0 || trailType == 3) && j == longestSeg ) {
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
void ParticlesSL::moveParticle(particlePS *particlePtr) {
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
    if( bounce && position >= numLines ) {
        if( direction ) {
            position = numLines - size;
            direction = false;
        } else {
            position = size - 1;
            direction = true;
        }
        particlePtr->direction = direction;
    }

    //update the particle's position
    particlePtr->position = position;
}

//writes out the trail color according to the pixel number in the trail (ie 0 - trailSize)
//the trail is blended towards background color according to the trailSize
void ParticlesSL::setTrailColor(uint16_t trailLineNum, uint8_t segNum, uint8_t trailPixelNum) {
    //get the physical pixel location, it's color, and the target background color
    pixelNum = segDrawUtils::getPixelNumFromLineNum(*segSet, segNum, trailLineNum);

    colorTarget = segDrawUtils::getPixelColor(*segSet, pixelNum, *bgColor, bgColorMode, segNum, trailLineNum);
    colorFinal = segDrawUtils::getPixelColor(*segSet, pixelNum, partColor, colorMode, segNum, trailLineNum);

    //blend the color into the background according to where we are in the trail
    colorFinal = particleUtilsPS::getTrailColor(colorFinal, colorTarget, trailPixelNum, trailSize, dimPow);
    //output the color
    if( blend ) {
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
uint16_t ParticlesSL::getTrailLedLoc(bool trailDirect, uint8_t trailPixelNum, uint16_t maxPosition) {
    //get the multiplier for the direction (1 or -1)
    //sets if the trail will be drawn forwards or backwards
    trailDirectionAdj = particleUtilsPS::getDirectStep(trailDirect);

    //since we draw the body of the particle behind the lead pixel,
    //we need to offset rear trails by the body size
    sizeAdj = size - 1;

    if( !trailDirect ) {
        sizeAdj = 0;
    }

    //worked this formula out by hand, basically just adds/subtracts the trail location from the particle location
    //wrapping according to the maxPosition (note that we add the maxPosition to prevent this from being negative, arduino handles mods of negative numbers weirdly)
    //Because we mod by the maxPosition, bouncing trails can extend off the end of the strip in either direction
    //(due to the way mod works, anything outside of the segment set always ends up with a position of > numLine,
    //no matter the particle direction)
    //This behavior is intended so we can handle re-direct any trails off the end of the strip, to wrap back on themselves
    //( position + maxPosition - trailDirectionAdj * ( directStep * ( trailPixelNum + sizeAdj ) ) ) % maxPosition;
    trailLocOutput = addMod16PS(position, maxPosition - trailDirectionAdj * (directStep * (trailPixelNum + sizeAdj)), maxPosition);

    //If we're bouncing and the trail part would go off the end of the segment set
    //we need to re-direct the trail part back along the strip, in the opposite direction of the particle
    //like you'd see on a flame or stream irl, which traces the particle's previous motion (or next for front trails))
    //to do this we take the portion of the trail that is currently off the strip
    //and redirect it back along the strip according to the direction and trail direction
    if( bounce && trailLocOutput >= numLines ) {
        //The re-directed trail position depends on the direction of the particle and trail direction
        //For rear trails the particle direction changes before ahead of the trail
        //So the trail needs to be directed in the opposite direction to the particle's motion
        //For front trails, the opposite is true, because the trail needs to change direction ahead of the particle
        if( (trailDirect && direction) || (!trailDirect && !direction) ) {
            trailLocOutput = maxPosition - trailLocOutput;
        } else {
            trailLocOutput = (numLines - 1) - mod16PS(trailLocOutput, (numLines - 1));
        }
        //displace the trail by the size of the particle opposite to the direction of the particle
        trailLocOutput = trailLocOutput + directStep * sizeAdj;
    }

    return trailLocOutput;
}
#include "ColorWipeSL.h"

//Constructor using pattern and palette
ColorWipeSL::ColorWipeSL(SegmentSet &SegmentSet, palettePS *Palette, patternPS *Pattern, uint16_t WipeLength, uint8_t Style,
                    bool Simult, bool Alternate, bool WipeDirect, uint16_t Rate):
    segmentSet(SegmentSet), palette(Palette), pattern(Pattern), wipeLength(WipeLength), style(Style), simult(Simult), alternate(Alternate), wipeDirect(WipeDirect)
    {    
        init(Rate);
	} 

//Constructor using palette alone 
ColorWipeSL::ColorWipeSL(SegmentSet &SegmentSet, palettePS *Palette, uint16_t WipeLength, uint8_t Style,
                    bool Simult, bool Alternate, bool WipeDirect, uint16_t Rate):
    segmentSet(SegmentSet), palette(Palette), wipeLength(WipeLength), style(Style), simult(Simult), alternate(Alternate), wipeDirect(WipeDirect)
    {    
        setPaletteAsPattern();
        init(Rate);
	}

//Constructor for a single color wipe
ColorWipeSL::ColorWipeSL(SegmentSet &SegmentSet, CRGB WipeColor, uint16_t WipeLength, uint8_t Style,
                    bool Simult, bool Alternate, bool WipeDirect, uint16_t Rate):
    segmentSet(SegmentSet), wipeLength(WipeLength), style(Style), simult(Simult), alternate(Alternate), wipeDirect(WipeDirect)
    {    
        paletteTemp = paletteUtilsPS::makeSingleColorPalette(WipeColor);
        palette = &paletteTemp;
        setPaletteAsPattern();
        init(Rate);
	}

//destructor
ColorWipeSL::~ColorWipeSL(){
    free(paletteTemp.paletteArr);
    free(patternTemp.patternArr);
}

//Sets up the core variables for the effect 
void ColorWipeSL::init(uint16_t Rate){
    //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    //Store the inital wipe direction for reference later
    startingDirect = wipeDirect;
    setWipeLength(wipeLength);
    reset();
}

//Resets various wipe variables to their starting states,
//restarting the wipe
//(Note that if looping, startingDirect and the segmentSet direction may be different than their inital values)
void ColorWipeSL::reset(){
    done = false;
    wipeNumSeq = 0;
    wipeLine = 0;
    wipeDirect = startingDirect;
    bgWipe = false;
    patOffset = 0;
    wipeCount = 0;
}

//sets the pattern to match the current palette
//ie for a palette length 5, the pattern would be 
//{0, 1, 2, 3, 4}
void ColorWipeSL::setPaletteAsPattern(){
    patternTemp = generalUtilsPS::setPaletteAsPattern(palette);
    pattern = &patternTemp;
}

//Sets the length for the color wipe(s)
//The number of wipes is automatically worked out based on the number of lines in the segment set
//If 0 is passed in, the wipe length will be set to the number of lines in the segment set
void ColorWipeSL::setWipeLength(uint16_t newLength){
    wipeLength = newLength;

    numLines = segmentSet.maxSegLength;
    //if wipelength is 0 we'll do a full wipe using all the lines in the segment set
    if (wipeLength == 0 || wipeLength > numLines) {
        wipeLength = numLines;
    }

    //Work out how many wipes will fit onto the segment set (rounded up so we always fill all the lines)
    numWipes = ceil( (float)numLines / wipeLength );
}

//A quick way of changing all of the looping variables at once.
//There should be enough variables to cover most looping effect variations.
//An explanation of each of the variables (ignore the n in the arg names):
//  looped -- Sets if the wipes loop or not. Looping wipes automatically restart everytime a wipe is finished.
//            The other variables are only relevant if the wipe is looping, because they modify subsequent loops.
//  shiftPatLoop -- If true, the pattern will be shifted forward by 1 with every loop, 
//                  changing the colors of each segment / line with each wipe
//                  (this is done with an offset, it does not change the existing pattern)
//  bgLoop -- If true, then the background color (default 0) will be used as the color wipe every other loop
//            Ie, we wipe a color and then wipe off, looping
//  altWipeDirLoop -- If true, then the wipe direction will be flipped after each loop
//                    (note that this modifies the original startingDirect value)
//  bgAltLoop -- Only used if altWipeDirLoop is true. 
//               If true, the the wipe direction will only swap on colored wipes, not on background ones.
//               Ie colored wipe->background wipe->(wipe direction flips)->colored wipe->background wipe->(flip)->etc
//               If false, then the wipe direction will flip every loop
//               ie colored wipe->(wipe direction flips)->background wipe->(wipe direction flips)->colored wipe->etc
//  altSegDirLoop -- If true, then the segment set direction will be flipped with each loop
//                   This is different than flipping the wipe direction, since it makes the first wipe 
//                   start at the opposite end of the segment set, rather than having the wipe just move in the oppsite direction 
//                   !!Note that is modifies the direction setting for the segment set             
void ColorWipeSL::setUpLoop(bool nLooped, bool nShiftPatLoop, bool nBgLoop, bool nAltWipeDirLoop, bool nBgAltLoop, bool nAltSegDirLoop ){
    looped = nLooped;
    altSegDirLoop = nAltSegDirLoop;
    altWipeDirLoop = nAltWipeDirLoop,
    bgLoop = nBgLoop;
    bgAltLoop = nBgAltLoop; //only matters if altWipeDirLoop is true
    shiftPatLoop = nShiftPatLoop;
}

//Resets variables to loop the color wipe
//The various looping variables are explained in setUpLoop()
void ColorWipeSL::resetLoop(){
    //set the basic wipe variables so we start a new wipe
    done = false;
    wipeNumSeq = 0;
    wipeLine = 0;

    //Flip the segment direction if needed
    //This is different than flipping the wipe direction, since it makes the first wipe start at the opposite end of the segment set
    //Rather than having the wipe just move in the oppsite direction
    if(altSegDirLoop){
        segmentSet.flipSegDirects();
    }
    
    //If we're doing background wipes everyother loop, we need to flip the bgWipe indicator
    if(bgLoop && !bgWipe){
        bgWipe = true;
    } else{
        bgWipe = false;
    }

    //Determine if we need to flip the wipe direction
    //The loop can be set to flip whenever we do a background wipe (using bgAltLoop), 
    //or to skip over background wipes (ie only flipping when we are drawing the wipe pattern -> every other loop)
    //So if bgAltLoop is true, then we only want to flip if we're not background wiping (bgWipe is false)
    //Otherwise (bgAltLoop is false), we flip whenever we start a new loop
    if(altWipeDirLoop && ( !bgAltLoop || (bgAltLoop && !bgWipe) ) ){
        startingDirect = !startingDirect;
    }
    wipeDirect = startingDirect;

    //If we're moving the pattern colors, we shift them using patOffset
    //We only shift them when we're not doing a background wipe
    if(shiftPatLoop && !bgWipe){
        patOffset = addMod16PS(patOffset, 1, pattern->length);
    }
}

//Updates the effect
//Basically the end goal of the effect is to wipe the segment lines with colors from a pattern and palette
//The wipe length is variable, with shorter wipes being repeated across the segment set lines until all the lines are filled
//ie if you have a segment set with 24 lines, and want to do wipes of 6 lines, you'll have 4 wipes total (24/6 = 4)
//The wipes can either happen in sequence, one wipe of 6, then the next wipe of 6, etc
//Or they can happen all at once -- the first line of each wipe is colored, then the second, etc
//Wipes have a direction, with the option to alternate the direction for each wipe.
//Once all the wipes are done, we set a flag, ending the wipes, or reseting them if we're looping.
//Getting all this to work is more fidilly than complicated, with most contitions being created to seperate
//between simultaneous and sequential wiping. 
//At the core, for each update, we go over each wipe, pick the next line to color based on the wipe direction
//and then color it in segment by segment.
//Then we check if the wipe is finished (our line number matches the final line), 
//if so, we flag done, to end the wipe, or reset it if we're looping
//If not, we advance to the next line.
void ColorWipeSL::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;

        //if the wipe is finished, we can just jump out of the update() to end it
        if(done){
            return;
        }

        //If we're wiping all the wipe lengths at once, 
        //we need to make sure out first wipe always has the correct direction
        if(simult){
            wipeDirect = startingDirect;
        }

        numSegs = segmentSet.numSegs;

        //The core loop, for each wipe:
        //  If the wipes are simultaneous, we draw the next line for each wipe
        //  If the wipes are not simultaneous, we still run over all of them, 
        //  but we skip every loop iteration except for the one that matches wipeNumSeq,
        //  where we draw the next line
        for(uint16_t i = 0; i < numWipes; i++){
            
            //If we're wiping each wipe length squentially, one at a time
            //we only want to draw a single line with each update. 
            //The line is tracked by wipeNumSeq. 
            //We skip over all other loop iterations
            if(!simult && i != wipeNumSeq){
                continue;
            }

            //formulas for getting the segment set line number from the loop iteration
            //if moving forward then this is just the wipeLine, offset by which wipe we're on
            //if moving backwards, then it's the end point of each wipe (the start of the next wipe - 1) offset backwards by the wipeLine
            if(wipeDirect){
                lineNum = (wipeLength * i) + wipeLine;
            } else {
                lineNum = (wipeLength * (i + 1) - 1) - wipeLine;
            }

            //For cases where the wipe length doesn't fit evenly into the segment set number of lines there will be overflow
            //ie 24 lines with a wipe length of 7 will do 4 wipes to make sure we cover all the segment lines,
            //but there is some overflow (4 * 7 = 28 wipe lines)
            //So some of our wipe lines will be off the end of the segment set.
            //We want to skip these so that the effect looks cohesive, and doesn't "hang" drawing lines that don't exist
            if(lineNum > numLines - 1){
                //If the wipes are simultaneous, then we just want to skip any lines that are off the segment set
                //(trying to skip directly to the lines that are on the segment set is messy because you still need to go 
                //through all the loop iterations for the other wipes)
                if(simult){
                    continue;
                }
                //If the wipes are not simultaneous, then we know we're on the last wipe, so we can cut the wipe short if needed
                //(wipeNumSeq == numWipes - 1)
                if(wipeDirect){
                    //If wipe direct is true then we're moving forward,
                    //so the previous wipe lines will be on the strip, and we're just now about to run off it.
                    //We need to end the wipe right now, rather than trying to wipe any remaing lines in the wipe length
                    //So we set the wipeLine to wipeLength - 1, which will end the wipe at the end of the update()
                    wipeLine = wipeLength - 1; 
                } else {
                    //If the wipe is moving backwards, then we'll be trying to start the wipe off the segment set
                    //We need to jump to the first wipe line that is actually on the segment set, and then
                    //allow the wipe to proceed.
                    wipeLine = lineNum - numLines + 1;
                    lineNum = numLines - 1;
                }
            }

            //Color in each of the pixels in the current line number
            //We go segment by segment so we can capture the wipe style and any color mode effects
            for (uint16_t j = 0; j < numSegs; j++) {
                if(bgWipe){ //if we're doing a background wipe, only relevant when looping
                    modeOut = bgColorMode;
                    colorOut = *bgColor;
                } else {
                    //get the pixel color according to the style and pattern
                    switch (style) {
                        case 0: //alternates colors for each wipe
                        default:
                            palIndex = patternUtilsPS::getPatternVal( pattern, i + patOffset);
                            break;

                        case 1: //alternates colors for each line
                            palIndex = patternUtilsPS::getPatternVal( pattern, wipeLine + patOffset );
                            break;

                        case 2: //alternates colors for each segment
                            palIndex = patternUtilsPS::getPatternVal( pattern, j + patOffset );
                            break;
                    }
                    modeOut = colorMode;
                    colorOut = paletteUtilsPS::getPaletteColor( palette, palIndex );
                }
                
                //output the color to the pixel, note that if the color mode is non-zero, it will override the wipe style
                pixelNum = segDrawUtils::getPixelNumFromLineNum(segmentSet, numLines, j, lineNum);
                segDrawUtils::setPixelColor(segmentSet, pixelNum, colorOut, modeOut, j, lineNum);
            }

            //If we're drawing all the color wipes at once and alternating their directions
            //we need to flip the direction for the next wipe
            if(simult && alternate){
                wipeDirect = !wipeDirect;
            }
        }

        //If we havn't reached the wipe length yet, we need to move on to the next line to wipe
        //Otherwise, the wipe is finished
        if(wipeLine < wipeLength - 1){
            wipeLine++;
        } else {
            //All the wipes are finished, so we flag done, and possibly reset if looping
            //We also advance the wipeCount, to track how many total wipe cycles we've finished
            if(simult || ( !simult && wipeNumSeq >= numWipes - 1 ) ){
                done = true;
                wipeCount++;
                if(looped){
                    resetLoop();
                }
            } else {
                //If we're wiping sequentially, each individual wipe ends, but we have not finished the total
                //wipes, so we need to setup a new one
                wipeNumSeq++;
                wipeLine = 0;
                if(alternate){
                    wipeDirect = !wipeDirect;
                }
            }
        }

        showCheckPS(); 
    }
}
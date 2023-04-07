#include "ColorWipeSLSeg.h"

//Constructor using pattern and palette
ColorWipeSLSeg::ColorWipeSLSeg(SegmentSet &SegSet, palettePS &Palette, patternPS &Pattern, uint16_t WipeLength, uint8_t Style,
                         bool Simult, bool Alternate, bool WipeDirect, bool SegMode, uint16_t Rate):
    SegSet(SegSet), palette(&Palette), pattern(&Pattern), wipeLength(WipeLength), style(Style), simult(Simult), alternate(Alternate), wipeDirect(WipeDirect), segMode(SegMode)
    {    
        init(Rate);
	} 

//Constructor using palette alone 
ColorWipeSLSeg::ColorWipeSLSeg(SegmentSet &SegSet, palettePS &Palette, uint16_t WipeLength, uint8_t Style,
                         bool Simult, bool Alternate, bool WipeDirect, bool SegMode, uint16_t Rate):
    SegSet(SegSet), palette(&Palette), wipeLength(WipeLength), style(Style), simult(Simult), alternate(Alternate), wipeDirect(WipeDirect), segMode(SegMode)
    {   
        setPaletteAsPattern();
        init(Rate);
	}

//Constructor for a single color wipe
ColorWipeSLSeg::ColorWipeSLSeg(SegmentSet &SegSet, CRGB WipeColor, uint16_t WipeLength, uint8_t Style,
                         bool Simult, bool Alternate, bool WipeDirect, bool SegMode, uint16_t Rate):
    SegSet(SegSet), wipeLength(WipeLength), style(Style), simult(Simult), alternate(Alternate), wipeDirect(WipeDirect), segMode(SegMode)
    {    
        paletteTemp = paletteUtilsPS::makeSingleColorPalette(WipeColor);
        palette = &paletteTemp;
        setPaletteAsPattern();
        init(Rate);
	}

//destructor
ColorWipeSLSeg::~ColorWipeSLSeg(){
    free(paletteTemp.paletteArr);
    free(patternTemp.patternArr);
}

//Sets up the core variables for the effect 
void ColorWipeSLSeg::init(uint16_t Rate){
    //bind the rate and SegSet pointer vars since they are inherited from BaseEffectPS
    bindSegPtrPS();
    bindClassRatesPS();
    //Store the inital wipe direction for reference later
    startingDirect = wipeDirect;
    //default both the seg and line wipe lengths to 0
    //This means that if we switch segModes, we'll default to doing a full wipe (0 does a full wipe)
    segWipeLen = 0;
    lineWipeLen = 0;
    //Set the wipe length (will also set either segWipeLen or lineWipeLen to wipeLength depending on segMode )
    setWipeLength(wipeLength);
    reset();
}

//Resets various wipe variables to their starting states,
//restarting the wipe
//(Note that if looping, startingDirect and the SegSet direction may be different than their inital values)
void ColorWipeSLSeg::reset(){
    done = false;
    wipeNumSeq = 0;
    wipeStep = 0;
    wipeDirect = startingDirect;
    bgWipe = false;
    patOffset = 0;
    loopCount = 0;
}

//sets the pattern to match the current palette
//ie for a palette length 5, the pattern would be 
//{0, 1, 2, 3, 4}
void ColorWipeSLSeg::setPaletteAsPattern(){
    patternTemp = generalUtilsPS::setPaletteAsPattern(*palette);
    pattern = &patternTemp;
}

//Sets the length for the color wipe(s)
//The number of wipes is automatically worked out based on the number of lines in the segment set
//If 0 is passed in, the wipe length will be set to the number of lines in the segment set
//Depending on the segMode, either segWipeLen or lineWipeLen will be set to the new wipeLength
//The seg or line wipe lengths are used as the wipe length if we change segMode when looping
void ColorWipeSLSeg::setWipeLength(uint16_t newLength){
    wipeLength = newLength;

    numLines = SegSet.numLines;
    numSegs = SegSet.numSegs;

    //Record the wipe length into either segWipeLen or lineWipeLen
    if(segMode){
        segOrLineLimit = numSegs;
        segWipeLen = wipeLength;
    } else{ 
        segOrLineLimit = numLines;
        lineWipeLen = wipeLength;
    }

    //if wipeLength is 0 we'll do a full wipe using all the lines in the segment set
    if (wipeLength == 0 || wipeLength > segOrLineLimit) {
        wipeLength = segOrLineLimit;
    }

    //Work out how many wipes will fit onto the segment set (rounded up so we always fill all the lines)
    numWipes = ceil( (float)segOrLineLimit / wipeLength );
}

//Sets the segMode and changes the wipeLength to either segWipeLen or lineWipeLen (recalculating numWipes)
void ColorWipeSLSeg::setSegMode(bool newSegMode){
    segMode = newSegMode;
    //Change the wipe length
    //Note that because segMode is a bool, it is either 1 or 0, reducing the wipeLength to either segWipeLen or lineWipeLen
    setWipeLength(segWipeLen * segMode + lineWipeLen * !segMode);
}

//A quick way of changing all of the looping variables at once.
//There should be enough variables to cover most looping effect variations.
//An explanation of each of the variables (ignore the n in the arg names):
//  looped -- Sets if the wipes loop or not. Looping wipes automatically restart every time a wipe is finished.
//            The other variables are only relevant if the wipe is looping, because they modify subsequent loops.
//  bgLoopFreq (min 2) -- Sets how often a bgWipe is done. ie 2 will be every other loop, 3, every 3 loops, etc
//                        The minimum is 2, because wiping the background every loop (1) isn't useful
//  bgLoop -- If true, then the background color (default 0) will be used as the color wipe every <<bgLoopFreq>> loop
//            Ie, we wipe a color and then wipe off, looping
//  loopFreq (min 1) -- Sets on what loops shiftPatLoop, altWipeDirLoop, and altSegDirLoop trigger
//                               ie 1 will trigger them every loop, 2 every other loop, etc
//                               This allows you to build some really neat effects just by changing the freq.
//  shiftPatLoop -- If true, the pattern will be shifted forward by 1 with every <<loopFreq>> loop, 
//                  changing the colors of each segment / line with each wipe
//                  (this is done with an offset, it does not change the existing pattern)
//  altWipeDirLoop -- If true, then the wipe direction will be flipped every <<loopFreq>> loop
//                    (note that this modifies the original startingDirect value)
//  bgAltLoop -- Only used if altWipeDirLoop is true. 
//               If true, the the wipe direction will only swap on colored wipes, not on background ones.
//               Ie colored wipe->background wipe->(wipe direction flips)->colored wipe->background wipe->(flip)->etc
//               If false, then the wipe direction will flip every loop
//               ie colored wipe->(wipe direction flips)->background wipe->(wipe direction flips)->colored wipe->etc
//  altSegDirLoop -- If true, then the segment set direction will be flipped every <<loopFreq>> loop
//                   This is different than flipping the wipe direction, since it makes the first wipe 
//                   start at the opposite end of the segment set, rather than having the wipe just move in the opposite direction    
//  altSegModeLoop -- If true, will switch segMode setting the effect from wiping segment lines to whole segments, or visa versa.
//                    When swapping, the wipeLength will be set to segWipeLen or lineWipeLen depending on the segMode.
//                    see setWipeLength() for info on how these are set. 
//                    Note that altSegModeLoop triggers every loop. 
//                    For me, this seemed like the best option rather than tying it to a freq. 
//                    It seemed weird to want to switch segModes for multiple loops, you might as well just create two different ColorWipes.
void ColorWipeSLSeg::setUpLoop(bool nLooped, uint8_t nBgLoopFreq, bool nBgLoop, uint8_t nLoopFreq, bool nShiftPatLoop,
                            bool nAltWipeDirLoop, bool nBgAltLoop, bool nAltSegDirLoop, bool nAltSegModeLoop ){
    looped = nLooped;
    altSegDirLoop = nAltSegDirLoop;
    altWipeDirLoop = nAltWipeDirLoop,
    bgLoop = nBgLoop;
    bgAltLoop = nBgAltLoop; //only matters if altWipeDirLoop is true
    shiftPatLoop = nShiftPatLoop;
    altSegModeLoop = nAltSegModeLoop;

    //Set the loop freq, which has a min of 1 (switching loop vars every loop)
    loopFreq = nLoopFreq;
    if(loopFreq <= 0){
        loopFreq = 1;
    }

    //Set the bgLoopFreq, which has a min value of 2 (switching loop vars every other loop)
    //If we allowed a freq of 1, then the background would be set every loop 
    bgLoopFreq = nBgLoopFreq;
    if(bgLoopFreq < 2){
        bgLoopFreq = 2;
    }
}

//Resets variables to loop the color wipe
//The various looping variables are explained in setUpLoop()
void ColorWipeSLSeg::resetLoop(){
    //set the basic wipe variables so we start a new wipe
    done = false;
    wipeNumSeq = 0;
    wipeStep = 0;

    loopCount++;

    //Flip the segMode
    //This switches the effect from drawing on segment lines to whole segments or visa versa
    //When we switch we also want to swap what wipeLength we're doing, either to segWipeLen or lineWipeLen
    if(altSegModeLoop){
        setSegMode(!segMode);
    }

    //Set the effect to do a background color wipe every bgLoopFreq number off loops
    //(we add 1 to the loopCount because we want the bg to trigger after the first wipe, with the freq default being 2)
    if(bgLoop && (mod16PS(loopCount + 1, bgLoopFreq ) == 0) ){
        bgWipe = true;
    } else{
        bgWipe = false;
    }
    
    //Change the other looping vars every loopFreq number of loops
    //This allows you to create a wide range of wiping loop effects
    if( mod16PS(loopCount, loopFreq ) == 0 ){
        //Flip the segment direction
        //This is different than flipping the wipe direction, since it makes the first wipe start at the opposite end of the segment set
        //Rather than having the wipe just move in the opposite direction
        if(altSegDirLoop){
            segDirect = !segDirect;
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
}

//Updates the effect
//Note that the effect can either be configured to wipe across segment lines or whole segments.
//Basically the end goal of the effect is to wipe the segment lines or segs with colors from a pattern and palette
//The wipe length is variable, with shorter wipes being repeated across the segment set until all the lines or segs are filled
//ie if you have a segment set with 24 lines, and want to do wipes of 6 lines, you'll have 4 wipes total (24/6 = 4)
//The wipes can either happen in sequence, one wipe of 6, then the next wipe of 6, etc
//Or they can happen all at once -- the first line of each wipe is colored, then the second, etc
//Wipes have a direction, with the option to alternate the direction for each wipe.
//Once all the wipes are done, we set a flag, ending the wipes, or reseting them if we're looping.
//Getting all this to work is more finicky than complicated, with most conditions being created to separate
//between simultaneous and sequential wiping or line and segment wiping. 
//At the core, for each update, we go over each wipe, pick the next line to color based on the wipe direction
//and then color it in segment by segment (or line be line if segMode is true)
//Then we check if the wipe is finished (our line number matches the final line), 
//if so, we flag done, to end the wipe, or reset it if we're looping
//If not, we advance to the next line.
void ColorWipeSLSeg::update(){
    currentTime = millis();
    
    //Check for update time, or skip if the wipes are "done"
    //We adjust the update rate by segRateAdj when in segMode to slow it by a fixed amount
    //(this assumes that the segment set has more segment lines than whole segments, so you may want a slower update rate for the segments)
    if( !done && ( currentTime - prevTime ) >= (*rate + segRateAdj * segMode) ) {
        prevTime = currentTime;

        //If we're wiping all the wipe lengths at once, 
        //we need to make sure out first wipe always has the correct direction
        if(simult){
            wipeDirect = startingDirect;
        }
        
        numSegs = SegSet.numSegs;
        numLines = SegSet.numLines;

        //Set the maximum wipe value, we use this later to skip any parts of wipes that are off the segment set
        if(segMode){
            segOrLineLimit = numSegs - 1;
        } else {
            segOrLineLimit = numLines - 1;
        }
        
        //The core loop, for each wipe:
        //  If the wipes are simultaneous, we draw the next line for each wipe
        //  If the wipes are not simultaneous, we still run over all of them, 
        //  but we skip every loop iteration except for the one that matches wipeNumSeq,
        //  where we draw the next line
        for(uint16_t i = 0; i < numWipes; i++){
            
            //If we're wiping each wipe length sequentially, one at a time
            //we only want to draw a single line with each update. 
            //The line is tracked by wipeNumSeq. 
            //We skip over all other loop iterations
            if(!simult && i != wipeNumSeq){
                continue;
            }
            
            //Note: segOrLineNum represents the next segment or segment line to be wiped.
            //      wipeStep is what wipe number we're on

            //formulas for getting the segment set line number from the loop iteration
            //if moving forward then this is just the wipeStep, offset by which wipe we're on
            //if moving backwards, then it's the end point of each wipe (the start of the next wipe - 1) offset backwards by the wipeStep
            if(wipeDirect){
                segOrLineNum = (wipeLength * i) + wipeStep;
            } else {
                segOrLineNum = (wipeLength * (i + 1) - 1) - wipeStep;
            }

            //For cases where the wipe length doesn't fit evenly into the segment set number there will be overflow
            //ie 24 lines with a wipe length of 7 will do 4 wipes to make sure we cover all the segment lines,
            //but there is some overflow (4 * 7 = 28 wipe lines)
            //So some of our wipe will be off the end of the segment set.
            //We want to skip the extra bits so that the effect looks cohesive, and doesn't "hang" drawing lines that don't exist
            if(segOrLineNum > segOrLineLimit){
                //If the wipes are simultaneous, then we just want to skip any wipe parts that are off the segment set
                //(trying to skip directly to the parts that are on the segment set is messy because you still need to go 
                //through all the loop iterations for the other wipes)
                if(simult){
                    continue;
                }

                //If the wipes are not simultaneous, then we know we're on the last wipe, so we can cut the wipe short if needed
                //(wipeNumSeq == numWipes - 1)
                if(wipeDirect){
                    //If wipe direct is true then we're moving forward,
                    //so the previous wipe lines will be on the strip, and we're just now about to run off it.
                    //We need to end the wipe right now, rather than trying to wipe any remaining lines in the wipe length
                    //So we set the wipeStep to wipeLength - 1, which will end the wipe at the end of the update()
                    wipeStep = wipeLength - 1; 
                } else {
                    //If the wipe is moving backwards, then we'll be trying to start the wipe off the segment set
                    //We need to jump to the first wipe line that is actually on the segment set, and then
                    //allow the wipe to proceed.
                    if(segMode){
                        wipeStep = segOrLineNum - numSegs + 1;
                    } else {
                        wipeStep = segOrLineNum - numLines + 1;
                    }
                }

                //Set the current wipe seg or line to the wipe limit
                segOrLineNum = segOrLineLimit;
            }
            
            //Reverse the line or segment number if the segment is to be reversed
            if(!segDirect){
                segOrLineNum = segOrLineLimit - segOrLineNum;
            }

            //either wipe a segment or a line depending on segMode
            if(segMode){
                doSegWipe(i, wipeStep, segOrLineNum);
            } else {
                doLineWipe(i, wipeStep, segOrLineNum);
            }

            //If we're drawing all the color wipes at once and alternating their directions
            //we need to flip the direction for the next wipe
            if(simult && alternate){
                wipeDirect = !wipeDirect;
            }
        }
    
        //If we haven't reached the wipe length yet, we need to move on to the next line to wipe
        //Otherwise, the wipe is finished
        if(wipeStep < wipeLength - 1){
            wipeStep++;
        } else {
            //All the wipes are finished, so we flag done, and possibly reset if looping
            //We also advance the loopCount, to track how many total wipe cycles we've finished
            if(simult || ( !simult && wipeNumSeq >= numWipes - 1 ) ){
                done = true;
                if(looped){
                    resetLoop();
                }
            } else {
                //If we're wiping sequentially, each individual wipe ends, but we have not finished the total
                //wipes, so we need to setup a new one
                wipeNumSeq++;
                wipeStep = 0;
                if(alternate){
                    wipeDirect = !wipeDirect;
                }
            }
        }

        showCheckPS(); 
    }
}

//Wipes a segment line by running over each segment, getting the pixel on the segment at the line number, and setting its color
void ColorWipeSLSeg::doLineWipe(uint16_t wipeNum, uint16_t wipeStep, uint16_t lineNum){

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
                    palIndex = patternUtilsPS::getPatternVal( *pattern, wipeNum + patOffset);
                    break;
                case 1: //alternates colors for each line
                    palIndex = patternUtilsPS::getPatternVal( *pattern, wipeStep + patOffset );
                    break;
                case 2: //alternates colors for each segment
                    palIndex = patternUtilsPS::getPatternVal( *pattern, j + patOffset );
                    break;
            }
            modeOut = colorMode;
            colorOut = paletteUtilsPS::getPaletteColor( *palette, palIndex );
        }
        
        //output the color to the pixel, note that if the color mode is non-zero, it will override the wipe style
        pixelNum = segDrawUtils::getPixelNumFromLineNum(SegSet, numLines, j, lineNum);
        segDrawUtils::setPixelColor(SegSet, pixelNum, colorOut, modeOut, j, lineNum);
    }        
}

//Wipes a whole segment by running all the segment lines, getting the segment pixel for each line and setting its color
void ColorWipeSLSeg::doSegWipe(uint16_t wipeNum, uint16_t wipeStep, uint16_t segNum){

    //Color in each of the pixels in the current segment number
    //We go line by line so we can capture the wipe style and any color mode effects
    for (uint16_t j = 0; j < numLines; j++) {
        if(bgWipe){ //if we're doing a background wipe, only relevant when looping
            modeOut = bgColorMode;
            colorOut = *bgColor;
        } else {
            //get the pixel color according to the style and pattern
            switch (style) {
                case 0: //alternates colors for each wipe
                default:
                    palIndex = patternUtilsPS::getPatternVal( *pattern, wipeNum + patOffset);
                    break;
                case 1: //alternates colors for each line
                    palIndex = patternUtilsPS::getPatternVal( *pattern, j + patOffset );
                    break;
                case 2: //alternates colors for each segment
                    palIndex = patternUtilsPS::getPatternVal( *pattern, wipeStep + patOffset );
                    break;
            }
            modeOut = colorMode;
            colorOut = paletteUtilsPS::getPaletteColor( *palette, palIndex );
        }

        //output the color to the pixel, note that if the color mode is non-zero, it will override the wipe style
        pixelNum = segDrawUtils::getPixelNumFromLineNum(SegSet, numLines, segNum, j);
        segDrawUtils::setPixelColor(SegSet, pixelNum, colorOut, modeOut, segNum, j);
    }
}
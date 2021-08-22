#include "RandomColorsPS.h"

RandomColorsPS::RandomColorsPS(SegmentSet &SegmentSet, palletPS Pallet, uint16_t NumPixels, CRGB BgColor,  uint8_t TotalSteps, uint16_t Rate):
    segmentSet(SegmentSet), pallet(Pallet), numPixels(NumPixels), bgColor(BgColor)
    {    
        //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
        bindSegPtrPS();
        bindClassRatesPS();
        setTotalSteps(this->totalSteps = TotalSteps);
        initPixelArrays();
	}

//binds the pallet to a new one
void RandomColorsPS::setPallet(palletPS newPallet){
    pallet = newPallet;
}

void RandomColorsPS::initPixelArrays(){
    if(ledArray){
        for(int i = 0; i < numPixels; i++){
            delete[] ledArray[i];
            delete[] colorIndexArr[i];
        }
        delete[] ledArray;
        delete[] colorIndexArr;
    }
    ledArray = new uint16_t*[numPixels];

    colorIndexArr = new CRGB*[numPixels];

    for(int i = 0; i < numPixels; i++){
        ledArray[i] = new uint16_t[ arraySteps ];
        colorIndexArr[i] = new CRGB[ arraySteps ];
    }
    reset();
}

void RandomColorsPS::reset(){
    startUpDone = false;
    totalSteps_ = 0;
    segDrawUtils::fillSegSetColor(segmentSet, bgColor, bgColorMode);
}

void RandomColorsPS::setTotalSteps(uint8_t newTotalSteps){
    totalSteps = newTotalSteps;
    arraySteps = totalSteps + 1;
    initPixelArrays();
}

void RandomColorsPS::setNumPixels(uint16_t newNumPixels){
    numPixels = newNumPixels;
    initPixelArrays();
}

void RandomColorsPS::update(){
    currentTime = millis();
    //if we're using an external rate variable, get its value
    globalRateCheckPS();
    if( ( currentTime - prevTime ) >= rate ) {
        prevTime = currentTime;
        uint16_t numActiveLeds = segmentSet.numActiveSegLeds;

        if(!startUpDone){
            totalSteps_++;
            if(arraySteps == totalSteps_){
                startUpDone = true;
            }
        }

        if(fillBG){
            segDrawUtils::fillSegSetColor(segmentSet, bgColor, bgColorMode);
        }

        // sets a random set of pixels to a random or indicated color(s)
        for (uint16_t i = 0; i < numPixels; i++) {
            for(uint8_t j = 0; j < totalSteps_; j++){
                if(j == 0){
                    ledArray[i][0] = random(numActiveLeds);
                    pickColor( ledArray[i][0] );
                    colorIndexArr[i][0] = color;
                } else {
                    segDrawUtils::getPixelColor(segmentSet, &pixelInfo, bgColor, bgColorMode, ledArray[i][j]);
                    color = segDrawUtils::getCrossFadeColor(colorIndexArr[i][j], pixelInfo.color, j, totalSteps);
                }
                segDrawUtils::setPixelColor(segmentSet, pixelInfo.pixelLoc, color, 0, pixelInfo.segNum, pixelInfo.lineNum);
            }
        }
        incrementPixelArrays();
        showCheckPS();
    }
}

void RandomColorsPS::pickColor(uint16_t pixelNum){
    uint8_t palletLength = pallet.length;
    switch (palletLength) {
        case 0: // 0 pallet length, no pallet, so set colors at random
            color = segDrawUtils::randColor();
            break;
        case 1: // pallet length one means all the pixels must be the same color
            color = palletUtilsPS::getPalletColor(pallet, 0);
            break;
        default: // we're picking from a set of colors
            color = palletUtilsPS::getPalletColor(pallet, random(palletLength));
            break;
    }
    segDrawUtils::getPixelColor(segmentSet, &pixelInfo, color, colorMode, pixelNum);
    color = pixelInfo.color;
}

void RandomColorsPS::incrementPixelArrays(){
    for (uint16_t i = 0; i < numPixels; i++) {
        for(uint8_t j = arraySteps - 1; j > 0; j--){
            ledArray[i][j] = ledArray[i][j - 1];
            colorIndexArr[i][j] = colorIndexArr[i][j - 1];
        }
    }
}
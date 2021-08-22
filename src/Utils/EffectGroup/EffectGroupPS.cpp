//#include "EffectGroupPS.h"

//constructor
//note that if RunTime is passed in as 0, infinite will be set to true
// EffectGroupPS::EffectGroupPS(EffectBasePS **effGroup, uint8_t NumEffects, uint16_t RunTime):
// runTime(RunTime), numEffects(NumEffects), group(effGroup)
// {
//     infinite = false;
//     if(runTime == 0){
//       infinite = true;  
//     }
//     reset();
// }

// #ifndef  FADE_PS
// EffectGroupPS::EffectGroupPS(EffectBasePS **effGroup, uint8_t NumEffects, uint16_t RunTime, uint16_t FadeRunTime):
// fadeRunTime(FadeRunTime), runTime(RunTime), numEffects(NumEffects), group(effGroup)
// {
//     EffectGroupPS(group, numEffects, runTime);
// }
// #endif

// #ifndef  FADE_PS
// void EffectGroupPS::setupFader(uint16_t newFadeRunTime){
//     if(fadeInc){
//         fadeRunTime = newFadeRunTime;
//         fadeInStarted = false;
//         fadeOutStarted = false;
//         if(!effectFader){
//             delete effectFader;
//             effectFader = new EffectFaderPS(group, numEffects, false, fadeRunTime);
//         } else {
//             effectFader->reset(group, numEffects, false);
//             effectFader->runTime = fadeRunTime;
//         }
//         if(fadeRunTime == 0){
//             effectFader->active = false;
//         }
//     }
// }
// #endif

// //set a new effect group and reset
// void EffectGroupPS::setGroup(EffectBasePS **newGroup, uint8_t newNumEffects){
//     group = newGroup;
//     numEffects = newNumEffects;
//     reset();
// }

// //reset the loop vars, restarting the effect group
// void EffectGroupPS::reset(void){
//     done = false;
//     started = false;
//     #ifndef  FADE_PS
//         setupFader(fadeRunTime);
//     #endif
// }

// void EffectGroupPS::update(void){
//     if(!done){
//         currentTime = millis();
//         //if this is the first time we've updated, set the started flag, and record the start time
//         if(!started){ 
//             startTime = currentTime;
//             started = true;

//             #ifndef  FADE_PS
//                 if(!fadeInStarted){
//                     if(!fadeInActive){
//                         effectFader->done = true;
//                     } else {
//                         effectFader->direct = true;
//                         fadeInStarted = true;
//                         effectFader->reset(); 
//                     }
//                 }
//             #endif
//         }

//         #ifndef  FADE_PS
//             if(effectFader->active && !effectFader->done){
//                 effectFader->update();
//             }

//             if( !infinite && !fadeOutStarted && ( currentTime - startTime ) >=  runTime - fadeRunTime){
//                 if(!fadeOutActive){
//                     effectFader->done = true;
//                 } else {
//                     effectFader->direct = false;
//                     fadeOutStarted = true;
//                     effectFader->reset();
//                 }
//             }
//         #endif

//         //if we've not reached the time limit (or we're running indefinitly)
//         //call all the effects' update functions
//         if( infinite || ( currentTime - startTime )  <= runTime ) {
//             for(int i = 0; i < numEffects; i++){
//                 (group[i])->update();
//             }
//         } else {
//             done = true;
//             #ifndef  FADE_PS
//                 effectFader->resetBrightness();
//             #endif
//         }
//     }
// }

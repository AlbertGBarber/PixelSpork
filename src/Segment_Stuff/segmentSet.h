#ifndef segmentSet_h
#define segmentSet_h

#include "FastLED.h"
#include "segmentSection.h"
#include "segment.h"
#include "Palette_Stuff/palettePS.h"
//TODO:
//-- Scrap turning off segments, full behavior too hard to implement, current behavior is confusing
//-- Add function somewhere to automatically sync a rainbow or gradent across multiple segment sets
//-- Move direction settings to segDrawUtils?

/*
An explaination of segment structure:
	Basic hierarchy:
			SegmentSet--->Segment{}--->Segment--->segmentSection{}--->segmentSection(stored in PROGMEM)
			where {} indicate an array and ---> indicate a pointer,
			so, in words; a SegmentSet points to an array of Segment pointers. These pointers
			point to individual segments, which are each pointers to an array of segmentSections.
			segmentSections contain a pixel number and a length, stored in PROGMEM.
			
	So what does each level represent?:
			Going up the hierarchy:
			@ segmentSection:
				Description:
					A segmentSection region of a section, indicated by a start pixel number and a length.
				The length is the total length of the region, including the start pixel. For example:
				{12, 20} would be a section starting with 12, running 20 pixels to 31. 
				Length can be negative (see paragraph below)
				!!(length counting starts at 1 to be easier for non-programmers)
				
					A segment section can have a negative length. This allows you to change the direction of sections
				(this is to account for segments with multiple sections, where one or more of the sections are facing the wriong way)
				for example lets say I have two sections in a line, both 7 pixels long,
				for whatever reason they are aligned as follows 0,1,2,3,4,5,6,13,12,11,10,9,8,7 where 6 outputs data to 7
				we want to animate a smooth line (so it looks like 6 is connected to 13)
				if they are defined {0, 7} and {7, 7}, then animations will jump from 6 to 7. This is not what we want.
				Instead, if we define the second section as {13, -7}, the code will treat 13 as the first pixel, and count backwards
				This will produce the effect we want.
								
				Data representation:
					segmentSection consists of a struct storing one uint16_t labeled startPixel and 
					one int16_t labeled length. Both of these must be stored in Flash by adding < const PROGMEM > when you 
					define a segmentSection. ie const PROGMEM segmentSection x = {0, 10}, where x is
					your section name.
			
			@ segmentSection{}
				Description:
					An array of segmentSections, used to form a whole segment. For example { {12, 20}, {30, 10} }
					is an array of segmentSections the first stating at pixel 12, running for 20 pixels, and the 
					second starting at 30 and running for 10 pixels. Order doesn't matter ie { {30, 10}, {12, 20} } is fine,
					but section orientation is important. To produce effects as intended, all sections should be oriented so that the the end of one
					connects to the beginning of the next. Use negative sections lengths to to this if needed.
					If they are oriented the same, but the effect runs in reverse along each section, reverse the order of the
					sections in the segmentSection{} array.
					Also note that you will save memory if you declare the sections in the array (like the example) instead of 
					using variables for each section. Also you can include the same section in mutliple section arrays, or even
					overlaps between seperate section arrays, however; the animation effects will overwrte each other, leading to 
					unpredictable results. 
					
					*Warning: do not include overlaps in the same segment array (ie { { 10, 5 }, {11, 8} } <-- second section starts within first one )
					Doing so will cause the total segment length to be miscalculated, and also break some animations.
				
				Data representation:
					It's just a straight forward array of segmentSections.
			
			@ Segment
				Description:
					A segment is a group of pixels considered to be one continuous string. Animation functions primarily act
					on segments one at a time. Segments are useful for seperating regions of grouped pixels that are in 
					geometric shapes, like rings, spheres, cubes, etc. Segments are specified with a direction (that can be changed),
					a direction value of true indicates that the segment is should be treated in asecending order, while a value of false
					indicates a decending order. Ie, if a segment is { {0, 5}, {10, 5} }, a true direction will treat the segment as starting at 0, going up,
					while a direction of false will start the segment at 15, going down. 
					A segment is declared with a segmentSection array and
					the length of the segmentSection array (number of sections). Typically this is done as 
					< Segment segmentNum = {SIZE(x), x} > where x is a segmentSection array.
					
				Data representation:
					Segment is a class constructed using the number of segmentSections and a pointer to the segmentSections array
					It gives access to four data points:
						numSec; A uint8_t equal to the number of sections in the passed in array.
						totalLength; A uint16_t equal to the combined length of all the sections.
						section; Which is of type segmentSection, and is a pointer to the array of sections.
						dirct; A boolean indicating the segment direction
					It also gives access to two functions:
						getSecStartPixel( uint8_t secNum ); returns the start pixel of the indicated section number (secNum is the section's position in the segmentSection array).
						getSecLength( uint8_t secNum ); same as above, but returns the section length.
						
			@Segment{}	
				Description:
					An array of segment pointers, used for arranging segments in groups (see SegmentSet for details).
					The order of the segments in the array is important, as they will be animated in order.
					It is important to note that it's an array of pointers to segments, not an array of
					segments themselves. This is done to save memory, but the array must use the following 
					syntax: 
					Segment{} must be written as Segment *array[] = { &segment0, &segment1, &segment2, etc...}
					where array, segment0, segment1, segment2, etc, are variable names of your choosing.
				
				Data representation:
					It's just a straight forward array of Segments.
			
			@ SegmentSet:
				Description:
					Container for an array of segments. Most animation functions operate using a set of segments. 
					For some animations, the order of the segment array matters: the segments will be treated as being 
					ordered to match the array. A segmentSet is declared with a segment array and
					the length of the segment array (number of segments). Typically this is done as 
					< SegmentSet segment_setx = {SIZE(x), x} > where x is a segment array. 
					
				Data representation:	
					SegmentSet is a class constructed using the number of segements, and a pointer to the segment array.
					It gives access to three data points:
						numSegs; A uint8_t equal to the segment array length passed to the constructor.
						maxSegLength; A uint16_t equaling the maxium segment length of the segments in the segment array.
						segNumMaxSegLength; A uint8_t equaling the number of the segment that has the maximum length
						ie, if the segment array has one segment of length 8, and one of length 12, 12 will be the maxSegLength.
						segArr; a double pointer of type Segment, used to access the segment array.
					It also gives access to a number of functions:
						getTotalSegLength(uint8_t segNum): returns the totalLength of the segment specified by the array index (segNum is the section's position in the segment array)
						getTotalNumSec(uint8_t segNum): returns the total number of sections in the segment specified by the array index.
						getSectionPtr(uint8_t segNum): returns the pointer to the segment's sections array specified by the array index.
						getSecStartPixel(uint8_t segNum, uint8_t secNum): returns the start pixel of the specified section in the specified segment. (ie getSecStartPixel(0, 0) would return the start pixel of the first section in the first segment in the segments array)
						getSecLength(uint8_t segNum, uint8_t secNum): returns the length of the specified section in the specified segment.
						getSegDirection(uint8_t segNum): returns a boolean direction of the specified segment
						setAllSegDirection(boolean dirct): sets all the segments in the segmentSet to the specified direction
						setSegDirection(uint8_t segNum, boolean dirct): sets the direction of the specified segment to the specified direction 
						flipSegDirectionEvery(uint8_t freq, boolean startAtFirst): flips the direction of every freq segment, starting with the first segment according to startAtFirst
						setsegDirectionEvery(uint8_t freq, boolean direction, boolean startAtFirst): sets the direction of every freq segment, starting with the first segment according to startAtFirst
						setSegActive(uint8_t segNum, boolean state): turns a segment on or off, off segments are not drawn onto, and do not count towards the total segment length
*/
//=====================================================================
/*
Example of a typical segmentSet definition:
This segmentSet will consist of four segments with two sections each. 

(size is defined as  "SIZE(x) (sizeof((x)) / sizeof((x)[0]))" in the library)

//define four segment arrays with two sections each, alternating directions
//each section is length 5, the start points allow the segments to be interwoven.
//Note that you could define each section seperately and combine them into an array
//ie sec0 = {0,5} , sec1 = {10, 5}, const PROGMEM segmentSection sec0_array[] = { sec0, sec1 };
//This takes up more ram memory, and should only be used if you need to reuse sections in multiple segment sets

const PROGMEM segmentSection sec0_array[] = { {0, 5}, {10, 5} };
Segment segment0 = { SIZE(sec0_array), sec0_array, true };

const PROGMEM segmentSection sec1_array[] = { {5, 5}, {15, 5} };
Segment segment1 = { SIZE(sec1_array), sec1_array, false };

const PROGMEM segmentSection sec2_array[] = { {20, 5}, {30, 5} };
Segment segment2 = { SIZE(sec2_array), sec2_array, true };

const PROGMEM segmentSection sec3_array[] = { {25, 5}, {35, 5} };
Segment segment3 = { SIZE(sec3_array), sec3_array, false };

//define the segment array and the segmentSet (must inlucde the * and the &'s)
Segment *segment_array[] = { &segment0 , &segment1, &segment2, &segment3 };
SegmentSet segmentset0 = {leds, numLeds, SIZE(segment_array), segment_array};

//Note: leds is your FastLED array of leds, numLeds is it's length

//Note that for the ESP family of chips (and possibly others) omit the const PROGMEM
//from the segment definitions 

//That's it, segmentSet0 can be passed to whatever effects you choose.

//==============================================================

//Rainbows and Gradients
//Many effects support the color modes used in segDrawUtils::getPixelColor() 
//These modes allow you to easily set effects to use a rainbow of colors or a specific palette gradient
//The variables that control the rainbow/gradient are specific to each segmentSet:

	gradPalette --The palette of colors used for the gradient. Set this to what ever palette you like.
				 By default it is tied to paletteTemp, which is a palette of green and purple
	gradLenVal -- The gradient length for color modes 1 & 7 (default val is the number of leds in the segment set)
	gradSegVal -- The gradient length for color modes 3 & 9 (default val is the number of segments in the segmentSet)
	gradLineVal -- The gradient length for color modes 4 & 10 (defaulted val is the length of the longest segment (maxSeglength) )\
	rainbowVal -- (default 255) HSV style value for the rainbows drawn on the segmentSet
	rainbowSatur -- (default 255) HSV style saturation for the rainbows drawn on the segmentSet

The gradient lengths are seperated by mode to make it easy to switch between color modes
By changing these vars you can change how the rainbow or gradient looks.

You can reset to defaults by calling resetGradVals()

Lets say you your using color mode 1, and you segmentSet is length 50.
But you set gradLenVal to 100.
This means that only half the rainbow or gradient would show up on the strip
This is where offsets come in.

Each segmentSet has a gradOffset variable. This is defaulted to 0.
This sets a constant offset for rainbows and gradients, which is added to the color mode input var. 
So, based on the previous example, lets say you set the offset to 50
This means that the first pixel will show the color of the 50th step in the gradient
Then the next will be the 51st and so forth. Overall the second half of the gradient will be shown.

Now, this isn't super useful by itself, but what if we could shift the offset over time. That 
way the whole gradient could cycle accross the strip.
Helpfully this functionality is build right into all segmentSets (with some help from segDrawUtils)
We can set up a constantly shifting offset using the following segmentSet vars:

	offsetRateOrig -- (default 30ms) the base offset rate, this is tied to offsetRate on startup
	*offsetRate -- (ms) The offset rate used for the segmentSet. This is pointer, which by default is bound to
				   offsetRateOrig, but can be rebound to point to an external variable for easy rate control
				   (ie offsetRate = &yourExternalRate)
	runOffset (default false) -- Sets if the shifing offset should be active or not
    offsetDirect (default true) -- The direction of the offset (false is reverse)

So by setting an offsetRate and setting runOffset to true, we can have an automatically shifting offset
The offset is updated in segDrawUtils using setGradOffset(), which is automatically called
in any effect that has a colorMode option whenever a pixel color is updated (is using a gradient or rainbow mode)

Calling the offset update function as part of an effect does limit the rate to that of the effect
(you can't update it faster than the effect updates)
So you can use SegOffsetCycle, which only sets the offset

Please note that there's one offset, but several vars for the gradient length.
Where the offset wraps back to 0 depends on the gradient length,
so if you have multiple effects with different color modes on the same segment
you will get weird behavior in the offset. This should not be common. And in the case you'd like to 
do this, just use two segementSets.

//================================================================

//Brightness:
//"brightness" sets the brightness of the segmentSet (relative to the overall strip brightness) 
//255 -> same brightness as all the global FastLED brightness setting
//DO NOT use this in effects, it is meant as a set-up once type of control
//Note that effectFader will change this value during fades (but should reset to the original once done)
*/

class SegmentSet {
	public:
		SegmentSet(struct CRGB *segLeds, uint16_t segAllLeds, Segment **segmentsArr, uint8_t numSegments);
		//Object.*pointerToMember
		//ObjectPointer->*pointerToMember
		
	    uint8_t
			rainbowVal = 255, 	//HSV style value for the rainbows drawn on the segmentSet
			rainbowSatur = 255, //HSV style saturation for the rainbows drawn on the segmentSet
	  		brightness = 255, 	//brightness of the segment (relative to the overall strip brightness) 
		  				  		//255 = same brightness as strip
						  		//DO NOT use this in effects, it is meant as a set-up once type of control
						  		//Note that effectFader will change this value during fades (but should reset to the original once done)
	    	segNumMaxSegLength,
			numSegs;

		uint16_t
			maxSegLength, //the length of the longest segment in the set
			numLeds, //the total number of pixels in the set
			numActiveSegLeds, //the number of pixels in all the active segments
			getTotalSegLength(uint8_t segNum),
			//getSectionPtr(uint8_t segNum),
			getSecStartPixel(uint8_t segNum, uint8_t secNum),
			getTotalNumSec(uint8_t segNum);
		
	  	int16_t
			getSecLength(uint8_t segNum, uint8_t secNum);

		uint16_t
			offsetRateOrig = 30, //default setting for color mode 5 and 6 of segDrawUtils::setPixelColor in ms
			*offsetRate,
			gradOffset = 0, 	//sets the offset used when calling the wheel function in segDrawUtils.h
		  				   		//this adjusts the start point of any rainbow drawn on the segmentSet
						   		//can also be controlled externally using RainbowOffsetCycle.h
			gradLenVal, 		//used in color modes of segDrawUtils::setPixelColor
			gradLineVal, 		//used in color modes of segDrawUtils::setPixelColor
			gradSegVal; 		//used in color modes of segDrawUtils::setPixelColor
								
		unsigned long
			offsetUpdateTime = 0; //the last time the offset value was automatically updated
		
	  	bool
		  	runOffset = false,
			offsetDirect = true, 
			getSegDirection(uint8_t segNum),
			getSegActive(uint8_t segNum);
	
	  	segmentSection* 
			getSecArrPtr(uint8_t segNum);
		
	  	Segment
			**segArr;

	  	Segment*
	  		getSegPtr(uint8_t segNum);
	  
	    CRGB
	  	 	*leds; //pointer to the FastLed leds array
		
		palettePS //palettes for gradients
			paletteTemp,
			*gradPalette;

	  	void
		  	resetGradVals(),
			setAllSegDirection(boolean direction),
			setSegDirection(uint8_t segNum, boolean dirct),
			flipSegDirectionEvery(uint8_t freq, boolean startAtFirst),
			setsegDirectionEvery(uint8_t freq, boolean direction, boolean startAtFirst),
			setBrightness(uint8_t newBrightness),
			setSegActive(uint8_t segNum, boolean state);
	  
	private:	
	  	uint16_t 
			getMaxSegLength(void),
			getsegNumMaxSegLength(void),
			getNumActiveSegLeds(void);
			
		bool
			checkSegFreq(uint8_t freq, uint8_t num, boolean startAtFirst);
};

#endif
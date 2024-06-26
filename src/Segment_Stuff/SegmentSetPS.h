#ifndef SegmentSetPS_h
#define SegmentSetPS_h

#include "FastLED.h"
#include "segmentSectionsPS.h"
#include "SegmentPS.h"
#include "Palette_Stuff/PaletteList/paletteListPS.h"
#include "MathUtils/mathUtilsPS.h"
#include "./Include_Lists/GlobalVars/GlobalVars.h"  //need alwaysResizeObj_PS

//TODO:
//-- Add function somewhere to automatically sync a rainbow or gradient across multiple segment sets
//	(sets offsets and lengths to sync them up, more complicated to match lines)

//Honestly, this page is a bit of a mess in terms of layout......
//(the info should still be correct tho)
//It's probably easier to read the Segments wiki pages.

/*
An explanation of segment structure:
	Basic hierarchy:
			SegmentSetPS--->SegmentPS{}--->SegmentPS--->segmentSection{}--->segmentSecCont(struct stored in PROGMEM)
																|---->segmentSecMix(struct stored in PROGMEM)
			where {} indicate an array and ---> indicate a pointer,
			so, in words; a SegmentSetPS points to an array of SegmentPS pointers. These pointers
			point to individual segments, which are each pointers to an array of segmentSectionsPS.
			segmentSectionsPS come in two flavors:
				segmentSecCont is a set of continuous pixels, represented as a starting pixel and a length.
				segmentSecMix contains an array of pixel addresses, and the length of the array
			Each segment can only have one type of segment section.
			
	So what does each level represent?:
			Going up the hierarchy:
			@ segmentSection:
			Comes in two types, you can only have one type per segment:
			Type 1: Continuous
				Description:
					A length of continuous pixels, indicated by a start pixel number and a length.
					The length is the total number of pixels in the section, including the start pixel. For example:
					{12, 20} would be a section starting with 12, running 20 pixels to 31.
					The shortest length is 1. 
					Length can be negative (see paragraph below).
					You can have multiple sections in one segment (see segmentSection{} below)
					
					A segment section can have a negative length. This allows you to change the direction of sections
					(this is to account for segments with multiple sections, where one or more of the sections are physically facing the wrong way)
					For example lets say I have two sections in a line, both 7 pixels long,
					for whatever reason they are aligned as follows 0,1,2,3,4,5,6,13,12,11,10,9,8,7 where 6 outputs data to 7
					We want to animate a smooth gradient line (so it looks like 6 is connected to 13)
					If they are defined {0, 7} and {7, 7}, then animations will jump from 6 to 7. This is not what we want.
					Instead, if we define the second section as {13, -7}, the code will treat 13 as the first pixel, and count backwards.

					Overall, this section type works best for long lengths of pixels, where all of them are in order
					Such as specifying an entire strip, or splitting the strip into multiple sections.
								
				Data representation:
					segmentSecCont consists of a struct storing one uint16_t startPixel and one int16_t length. 
					Both of these must be stored in Flash by adding < const PROGMEM > when you 
					define a segment section. ie const PROGMEM segmentSecCont x = {0, 10}, where x is your section name.
			
			Type 2: Mixed
				Description:
					This section type contains an array of pixel addresses, and the length of the array (number of pixels in the section)
					The pixel addresses can be in any order, and from anywhere on the strip.
					The length must be positive, min of 1.
					EX: A section containing pixel numbers 3, 6, 8, 10, etc, with a set length
					This type of section is good for when you want a segment made of mostly single pixels

				Data representation:
					segmentSecMix consists of a struct storing a pointer to a uint16_t array of pixel locations
					and one uint16_t length (but the maximum length is still the int16_t max to match with segmentSecCont)
					Both of these must be stored in Flash by adding < const PROGMEM > when you 
					define a segmentSecMix. 
					const PROGMEM uint16_t pixel_arr[] = {0, 2, 1};
					const PROGMEM segmentSecMix segmentSec = { pixel_arr, SIZE(pixel_arr) };

			@ segmentSecCont or segmentSecMix{}
				Description:
					An array of segment sections, used to form a whole segment. For example { {12, 20}, {30, 10} }
					is an array of segmentSecCont the first stating at pixel 12, running for 20 pixels, and the 
					second starting at 30 and running for 10 pixels. 
					The order of the sections is the order they will be written to:
					ie { {30, 10}, {12, 20} } will treat the section starting at 30 as coming before the section starting at 12.
					Use negative sections lengths if needed (see notes above).
					You can also reverse entire section arrays using the segment direction setting (see segments below)
					Also note that you will save memory if you declare the sections in the array (like the example) instead of 
					using variables for each section. Also you can include the same section in multiple section arrays.

					See the example segment set below for an example on how to define a set of mixed sections
					
					*Warning: do not include overlaps in the same segment array (ie { { 10, 5 }, {11, 8} } <-- second section starts within first one )
					Doing so will cause the total segment length to be miscalculated, and also break some animations.
				
				Data representation:
					It's an array of segmentSecCont's.
					EX segmentSecCont sec0[] = { {0, 24}, {32, 10} }; 
				
			NOTE that segment sections are stored in program memory, and so cannot be changed after they are created
			(this helps save precious ram)

			For both types of section it can be useful to treat the section as if it were a single led
			(so that the section is always one color)
			This helps with certain pixel arrangements
			It can also be used to trick segment line effects into coloring whole segments
			It's not a silver bullet for everything
			You set a section to be "single" by passing an extra true argument when creating sections:
			ex: {0, 24, true}
			This flags the rest of the code to treat the section as a single pixel, returning a length of 1 when asked
			The sections are filled in with color (by copying the color of the first section pixel to the rest of the section)
			when the segDrawUtils::show() function is called

			Single sections can be useful in making effects display along whole segments (perpendicular to segment lines)
			For example, say I have a shape of 5 rings, length 24, 16, 12, 8, 1, each continuous
			If I define each ring as it's own segment then the segment lines will be along the ring's radius
			So effects will move around the rings.
			But if I wanted the effects to move from the inner ring to the outer, with each ring being one color at a time
			I could create a set of segments where each segment was a radial line of pixels,
			but it would be easier to represent the rings as a series of single pixels all in one segment
			like: segmentSecCont sec0[] = { {0, 24, true}, {24, 16, true}, {40, 12, true}, {52, 8, true}, {60, 1} }; 
			So now when a effect draws on this segment, it essentially sees 5 pixels in a line, one for each ring,
			but when the pixels are displayed, each segment will be filled in

			Remember that segments can only have one section type, so some shapes may be tricky to represent as single pixels
			(something like a sun with rays sticking out will be tricky to get working this way)
			
			@ SegmentPS
				Description:
					A segment is a group of sections considered to be one continuous string. 
					Segments are useful for creating regions of grouped pixels that are in geometric shapes, like rings, spheres, cubes, etc. 
					Segments are specified with a direction (that can be changed),
					a value of true indicates that the segment is should be treated in ascending order, while a value of false
					indicates a descending order. 
					Ie, if a segment is { {0, 5}, {10, 5} }, a true direction will treat the segment as starting at 0, going up,
					while a direction of false will start the segment at 15, going down. 
					A segment is declared with a segmentSecCont array or a segmentSecMix and
					the length of segmentSecCont array/segmentSecMix. Typically this is done as 
					< SegmentPS segmentNum = {SIZE(x), x} > where x is a segmentSection array.
					Note that a segment can only have one section type, not both.
					
				Data representation:
					SegmentPS is a class, either constructed with a segmentSecCont{} or a segmentSecMix
					(see segment.h for full constructors)
					It gives access to four data points:
						numSec; A uint8_t equal to the number of sections in the passed in array.
						totalLength; A uint16_t equal to the combined length of all the sections.
						secContPtr; Which is a pointer to the array of segmentSecCont's.
						secMixPtr; Which is a pointer to the segmentSecMix struct.
						direct; A bool indicating the segment direction
						hasSingle: A bool indicating if the segment has any sections that are treated as single pixels
					Note that a segment can only have one section type, so one of the section pointers will be Null
					It also gives access to functions:
						getSecStartPixel( uint8_t secNum ); Returns the start pixel of the indicated section number (secNum is the section's position in the segmentSection array).'
														    (Only works for continuous sections)
						getSecMixPixel(uint8_t secNum, uint16_t pixelNum ); Returns the physical address of the pixel at pixelNum in the mixed section pixel array
														    (Only works for mixed sections)
						getSecLength( uint8_t secNum ); same as above, but returns the section length (will return 1 if the section is "single")
						getSecTrueLength( uint8_t secNum ); Returns the real section length, disregarding any single status
						
			@SegmentPS{}	
				Description:
					An array of segment pointers, used for arranging segments in groups (see SegmentSetPS for details).
					The order of the segments in the array is important, as they will be animated in order.
					It is important to note that it's an array of pointers to segments, not an array of
					segments themselves. This is done to save memory, but the array must use the following 
					syntax: 
					SegmentPS{} must be written as SegmentPS *array[] = { &segment0, &segment1, &segment2, etc...}
					where array, segment0, segment1, segment2, etc, are variable names of your choosing.
				
				Data representation:
					It's an array of Segments.
			
			@ SegmentSetPS:
				Description:
					Container for an array of segments. All animation functions operate using a set of segments. 
					The order of the segment array matters: the segments will be treated as being ordered to match the array. 
					A SegmentSetPS is declared with a segment array and the length of the segment array (number of segments). 
					Typically this is done as < SegmentSetPS segment_setx = {SIZE(x), x} > where x is a segment array. 
					
				Data representation (needs updating!);
					SegmentSetPS is a class constructed using the number of segments, and a pointer to the segment array.
					It gives access to three data points:
						* numSegs: A uint8_t equal to the segment array length passed to the constructor.
						* numLines: A uint16_t equaling the maximum segment length of the segments in the segment array.
						* segNumMaxNumLines: A uint8_t equaling the number of the segment that has the maximum length
						  ie, if the segment array has one segment of length 8, and one of length 12, 12 will be the numLines.
						* segArr: a double pointer of type SegmentPS, used to access the segment array.
						* segProgLengths: An array that stores the combined lengths of all previous segments up to the current segment
						  For example, if we have 4 segments in the set, each with length 10,
=                         Then the segProgLengths array would be {0, 10, 20, 30}. 
                          This is useful for calculating a pixel's location is relative to the whole segment set.
						  (note that the array is dynamically sized (see "Changing Segments" below for more))
					It also gives access to a number of functions:
						* getTotalSegLength(uint16_t segNum): returns the totalLength of the segment specified by the array index (segNum is the section's position in the segment array)
						* getTotalNumSec(uint16_t segNum): returns the total number of sections in the segment specified by the array index.
						* getSecStartPixel(uint16_t segNum, uint8_t secNum): returns the start pixel of the specified section in the specified segment. (ie getSecStartPixel(0, 0) would return the start pixel of the first section in the first segment in the segments array)
						* getSecMixPixel(uint16_t segNum, uint8_t secNum, uint16_t pixelNum); returns the specified pixel in a mixed section
						* getSecMixArrPtr(uint16_t segNum); Returns a pointer to the segment's segmentSecMix section, returns null if there isn't one
					    * getSecContArrPtr(uint16_t segNum); //Returns a pointer to the segment's segmentSecCont section, returns null if there isn't one
						* getSegPtr(uint16_t segNum); Returns a pointer to the specified segment object.
						* getSecLength(uint16_t segNum, uint8_t secNum): returns the length of the specified section in the specified segment. Will return 1 if the segment is "single"
						* getSecTrueLength(uint16_t segNum, uint8_t secNum): returns the real length of the section, ignoring if the segment is "single"
						* getSegDirection(uint16_t segNum): returns a bool direction of the specified segment
						* setAllSegDirection(bool direct): sets all the segments in the SegmentSetPS to the specified direction
						* setSegDirection(uint16_t segNum, bool direct): sets the direction of the specified segment to the specified direction 
						* flipSegDirects(): Flips the direction of all the segments in the segment set, ie all segments with direct = true become false, and visa versa
						* flipSegDirectionEvery(uint8_t freq, bool startAtFirst): flips the direction of every freq segment, starting with the first segment according to startAtFirst
						* setSegDirectionEvery(uint8_t freq, bool direction, bool startAtFirst): sets the direction of every freq segment, starting with the first segment according to startAtFirst
						* getSegHasSingle(uint16_t segNum): Returns true if the segment has any "single" sections
						* getSecIsSingle(uint16_t segNum, uint8_t secNum); Returns true if the passed in section is "single"
		
	SegmentPS sets also have a number of variables for effecting color modes, and also a gradient palette
	See Rainbows and Gradients section below for info.
*/
//=====================================================================
/*
Example of a typical SegmentSetPS definition:
This SegmentSetPS will consist of four segments with two sections each. 
Segment3 is defined using a mixed section, while all others use continuous sections

(size is defined as  "SIZE(x) (sizeof((x)) / sizeof((x)[0]))" in the library)

Define four segment arrays with two sections each, alternating directions
each section is length 5, the start points allow the segments to be interwoven.
Note that you could define each section separately and combine them into an array
ie sec0 = {0,5} , sec1 = {10, 5}, const PROGMEM segmentSection sec0_array[] = { sec0, sec1 };
This takes up more ram memory, and should only be used if you need to reuse sections in multiple segment sets.

const PROGMEM segmentSecCont sec0_array[] = { {0, 5}, {10, 5} };
SegmentPS segment0 = { sec0_array, SIZE(sec0_array), true };

const PROGMEM segmentSecCont sec1_array[] = { {5, 5}, {15, 5} };
SegmentPS segment1 = { sec1_array, SIZE(sec1_array), false };

const PROGMEM segmentSecCont sec2_array[] = { {20, 5}, {30, 5} };
SegmentPS segment2 = { sec2_array, SIZE(sec2_array), true };

//segment 3 is defined using a mixed section, so all the led addresses are held in an array
const PROGMEM uint16_t sec3_arr[] = {25, 26, 27, 28, 29, 35, 36, 37, 38, 39};
	//Note that you MUST put the mixed section(s) in an array.
	//The code is a short hand, and skips defining a variable for the section
	//by placing it directly in the array (hence the double curly brackets)
	//To define the sections the long way, see the multiple mixed section example below
const PROGMEM segmentSecMix segmentSec3[] = { { sec3_arr, SIZE(sec3_arr) } };
SegmentPS segment3 = { segmentSec3, SIZE(segmentSec3), true };

//If you have multiple mixed sections you'd need to stick them in an array like:
//You should only need multiple mixed sections if you're setting them as single pixels (see section notes above)
//--------------------------------------------------------------------------------------|
//const PROGMEM uint16_t sec3_arr1[] = {25, 26, 27, 28, 29}; <-The first section		|
//const PROGMEM segmentSecMix segmentSec3_1 = { sec3_arr1, SIZE(sec3_arr1) };			|
																						|
//const PROGMEM uint16_t sec3_arr2[] = {35, 36, 37, 38, 39}; <- The second section		|
//const PROGMEM segmentSecMix segmentSec3_2 = { sec3_arr2, SIZE(sec3_arr2) };			|
																						|
//Put the sections in an array															|
//const PROGMEM segmentSecMix segSec3Arr[] = { segmentSec3_1, segmentSec3_2 };			|
//SegmentPS segment3 = { segSec3Arr, SIZE(segSec3Arr), true }; <-The final segment		|
//--------------------------------------------------------------------------------------|

//Alternate definition of segment 3 using continuous sections
//----------------------------------------------------------------------|
//const PROGMEM segmentSecCont sec3_array[] = { {25, 5}, {35, 5} };		|
//SegmentPS segment3 = { sec3_array, SIZE(sec3_array), false };			|
//----------------------------------------------------------------------|

//define the segment array and the SegmentSetPS (must include the * and the &'s)
SegmentPS *segment_array[] = { &segment0 , &segment1, &segment2, &segment3 };
SegmentSetPS segmentSet0 = {leds, numLeds, segment_array, SIZE(segment_array)};

//Note: leds is your FastLED array of leds, numLeds is it's length

//Note that for the ESP family of chips (and possibly others) omit the const PROGMEM from the segment definitions 

//That's it, segmentSet0 can be passed to whatever effects you choose.

//==============================================================

Rainbows and Gradients
	Many effects support the color modes used in segDrawUtils::getPixelColor() 
	These modes allow you to easily set effects to use a rainbow of colors or a specific palette gradient
	The variables that control the rainbow/gradient are specific to each SegmentSetPS:

		gradPalette --The palette of colors used for the gradient. Set this to what ever palette you like.
					By default it is tied to paletteTemp, which is a palette of green and purple
		gradLenVal -- The gradient length for color modes 1 & 6 (default val is the number of leds in the segment set)
		gradSegVal -- The gradient length for color modes 2 & 7 (default val is the number of segments in the SegmentSetPS)
		gradLineVal -- The gradient length for color modes 3 & 8 (defaulted val is the length of the longest segment (numLines) )
		val -- (default 255) HSV style value for the rainbows drawn on the SegmentSetPS
		sat -- (default 255) HSV style saturation for the rainbows drawn on the SegmentSetPS

The gradient lengths are separated by mode to make it easy to switch between color modes
By changing these vars you can change how the rainbow or gradient looks.

You can reset to defaults by calling resetGradVals()

Lets say you your using color mode 1, and your SegmentSetPS is length 50.
But you set gradLenVal to 100.
This means that only half the rainbow or gradient would show up on the strip
This is where offsets come in.

Each SegmentSetPS has a gradOffset variable. This is defaulted to 0.
This sets a constant offset for rainbows and gradients, which is added to the color mode input var. 
This offset is out of 255 for rainbows and gradOffsetMax for custom gradients (default 255)
So, based on the previous example, lets say you set the offset to 50
This means that the first pixel will show the color of the 50th step in the gradient
Then the next will be the 51st and so forth. 
So, to shift the gradient across half the strip you would set the offset to 255/2 = 127.5
(use gradOffsetMax in place of 255 for custom gradients)

Now, this isn't super useful by itself, but what if we could shift the offset over time. That 
way the whole gradient could cycle across the strip.
Helpfully this functionality is build right into all segmentSets (with some help from segDrawUtils)
We can set up a constantly shifting offset using the following SegmentSetPS vars:

	offsetRateOrig -- (default 30ms) the base offset rate, this is tied to offsetRate on startup
	*offsetRate -- (ms) The offset rate used for the SegmentSetPS. This is pointer, which by default is bound to
				   offsetRateOrig, but can be rebound to point to an external variable for easy rate control
				   (ie offsetRate = &yourExternalRate)
	runOffset (default false) -- Sets if the shifting offset should be active or not
    offsetDirect (default true) -- The direction of the offset (false is reverse)
	offsetStep (default 1) -- The number of offset steps applied when the offset is updated
							  Increasing this speeds up the offset change, but makes it skip steps
							  This is mainly useful for speeding up rainbow offset modes
							  because they tend to move slowly (due to having a gradient length of 255)
							  so speeding it up without just using a lower update time (and doing more cpu work)
							  is useful.
    gradOffsetMax (default 255, max 65000) -- ONLY USED FOR CUSTOM GRADIENTS, NOT RAINBOWS. Sets the maximum number of 
	                                          gradient steps used to cycle through the whole custom gradient palette
											  The number of steps per color is gradOffsetMax/(num colors in gradient palette)
											  20 steps should be good enough for most colors.

So by setting an offsetRate and setting runOffset to true, we can have an automatically shifting offset
The offset is updated in segDrawUtils using setGradOffset(), which is automatically called
in any effect that has a colorMode option whenever a pixel color is updated (is using a gradient or rainbow mode)

Calling the offset update function as part of an effect does limit the rate to that of the effect
(you can't update it faster than the effect updates)
So you can use SegOffsetCycle, which only sets the offset

Please note that there's only one offset value per segment set, 
so if you have multiple effects with different color modes on the same segment set
you will get weird behavior because each effect will change the offset.
This should not be common. And in the case you'd like to do this, just use two segmentSets.

//================================================================

Brightness:
	"brightness" sets the brightness of the SegmentSetPS (relative to the overall strip brightness) 
	255 -> same brightness as all the global FastLED brightness setting 
	(so a segment set brightness setting of 127 would be 1/2 as bright as the rest of the strip)
	DO NOT use this in effects, it is meant as a set-up once type of control
	Note that effectFader will change this value during fades (but should reset to the original once done)

//================================================================

Changing Segments:
	Segments and Segment Sets are not stored in program memory, so it's possible to change them during runtime.
	Overall I recommend against this since it's not well tested, but if you must you should:
		* Use the setSegment() function to change segments in the segment set. This automatically
		  re-calculates various segment dependent vars for the segment set.
		* If you change the number of segments in the set, you must call calcSetVars() to re-calc various 
		  segment set vars. I really recommend NOT doing this, and just using multiple segment sets instead!
		* If you change the length of a segment (by swapping a section), you must call the segment's 
		  getSegTotLen() function AND the segment set's calcSetVars() function to re-calc 
		  various settings.  Remember that sections are read only, so you cannot change their properties.
	Also note, that calling setSegment() or calcSetVars() will re-size the segProgLengths array.
	The array is allocated dynamically and follows typical Pixel Spork dynamic allocation rules 
	(see https://github.com/AlbertGBarber/PixelSpork/wiki/Effects-Advanced#managing-dynamic-memory-and-fragmentation 
	for more)
*/

class SegmentSetPS {
    public:
        SegmentSetPS(struct CRGB *Leds, uint16_t LedArrSize, SegmentPS **SegArr, uint16_t NumSegs);
	
		~SegmentSetPS();

        //Segment Set Vars
        uint16_t
            numSegs,            //Total number of segments in the segment set
            numLines,           //the length of the longest segment in the set
            segNumMaxNumLines,  //The number of the seg with the maximum length
            ledArrSize,         //The size of the FastLED array (total number of pixels, including any extras for dummy or duplicate pixels)
            numLeds;            //the total number of pixels in the segment set (treating isSingle segments as one pixel)
		
		uint16_t
			*segProgLengths = nullptr;

		SegmentPS
            **segArr = nullptr;

        CRGB
            *leds = nullptr;  //pointer to the FastLed leds array

        //Color Mode Vars
		uint8_t
            val = 255,  //HSV style "value" value for the rainbows drawn on the SegmentSetPS
            sat = 255,  //HSV style saturation for the rainbows drawn on the SegmentSetPS
            brightness = 255,    //brightness of the segment (**relative** to the overall strip brightness)
                                 //255 = same brightness as strip
                                 //DO NOT use this in effects, it is meant as a set-up once type of control
                                 //Note that effectFader will change this value during fades (but should reset to the original once done)
            offsetStep = 3;      //The number of gradient offset steps applied when the gradOffset is updated

        uint16_t
            offsetRateOrig = 30,            //default setting for color mode 5 and 6 of segDrawUtils::setPixelColor in ms
            *offsetRate = &offsetRateOrig,  //The rate of change of the gradient offset, by default bound to offsetRateOrig
            gradOffset = 0,                 //This adjusts the start step of any rainbow drawn or gradient on the SegmentSetPS
                                            //Is adjusted automatically when runOffset is true.
            gradOffsetMax = 256,            //The maximum value of gradOffset + 1, ONLY APPLIES for custom gradients
            gradLenVal,                     //The gradient length for linear color mode gradients. (Color modes 1 & 6)
            gradLineVal,                    //The gradient length for radial color mode gradients. (Color Modes 2 & 7)
            gradSegVal;                     //The gradient length for linear segment line color mode gradients. (Color Modes 3 & 8)

        unsigned long
            offsetUpdateTime = 0;  //The last time (in ms) the gradOffset value was automatically updated

        bool
            runOffset = false,
            offsetDirect = true;

		//Palette for Color Modes Gradients
        palettePS 
            *gradPalette = nullptr;
		
		//Functions for getting segment and section properties
		uint16_t 
			getTotalSegLength(uint16_t segNum),
            getSecStartPixel(uint16_t segNum, uint8_t secNum),                   //only works with continuous sections!!
            getSecMixPixel(uint16_t segNum, uint8_t secNum, uint16_t pixelNum),  //only works with mixed sections!!
            getTotalNumSec(uint16_t segNum);

        int16_t
            getSecLength(uint16_t segNum, uint8_t secNum),      //Returns the length of the section, "single" sections will be returned as 1
            getSecTrueLength(uint16_t segNum, uint8_t secNum);  //Returns the length of the section, disregards the section's "single" setting

		bool
			getSegDirection(uint16_t segNum),
            getSegHasSingle(uint16_t segNum),
            getSecIsSingle(uint16_t segNum, uint8_t secNum);

		//Function for changing a Segment in the Set
		void 
			setSegment(SegmentPS &newSegment, uint16_t segNum);

        //Functions for setting core segment set properties based on the segments in the ser
        //These are called when the segment set is created or when a segment is changed via setSegment(),
		//but if you change any of the segments properties directly, 
        //You should re-call calcSetVars()
        void
			calcSetVars(), //calls all the below functions
			setProgLengthArr(),
            setNumLines(void),
            setNumLeds(void);

        //Functions for Changing Segment Directions
        void
            //flipSetOrder(),
            flipSegDirects(),
            setAllSegDirection(bool direction),
            setSegDirection(uint16_t segNum, bool direct),
            flipSegDirectionEvery(uint8_t freq, bool startAtFirst),
            setSegDirectionEvery(uint8_t freq, bool direction, bool startAtFirst);
		
		//Functions related to Color Modes
        void
            resetGradVals();
		
		//Functions for getting various segment pointers
		SegmentPS 
			*getSegPtr(uint16_t segNum);
		
		const segmentSecCont 
			*getSecContArrPtr(uint16_t segNum);  //Returns a pointer to the segment's segmentSecCont section, returns null if there isn't one

        const segmentSecMix 
			*getSecMixArrPtr(uint16_t segNum);  //Returns a pointer to the segment's segmentSecMix section, returns null if there isn't one

    private:
        bool
            checkSegFreq(uint8_t freq, uint16_t segNum, bool startAtFirst);
		
		uint16_t
			maxNumSegs = 0;
};

#endif
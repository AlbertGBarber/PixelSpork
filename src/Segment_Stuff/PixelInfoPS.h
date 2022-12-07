#ifndef pixelInfoPS_h
#define pixelInfoPS_h

//struct used in segDrawUtils::getPixelColor
//used to store all the variables accociated with a segment pixel for setting it's color
struct pixelInfoPS {
  CRGB color; //the color the pixel should be based on the colorMode
  uint16_t pixelLoc; //it's physical address on the strip (not local to segment set)
  uint16_t segNum; //the number of the segment it's located in
  uint16_t lineNum; //the line number it's on based on the maximum segment length in the set
} ;

#endif
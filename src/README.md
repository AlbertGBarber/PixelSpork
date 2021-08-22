# PixelStrip

Library of effects for ws2812b leds. Intended to run on Arduino compatible platforms.
To install create a folder named PixelStrip in the libraries folder of your Arduino install directory. Copy the files from this directory into the PixelStrip folder.
Requires Adafruit Neopixel library.

Note that, although the code is bug-free, it's architecture is flawed. Effects are direct functions that lock the processor for the duration
of the effect. I intend to re-write all the effects to be state and update based. 

#include "LPD8806.h"
#include "SPI.h"


// declare the number of LEDs
int numLEDs = 160;


// create an instance of the LED strip class. use hardware SPI for the writes.
// connect data in to pin 11 and clock in to pin 13
LPD8806 strip = LPD8806(numLEDs);


void setup() {
  // Start up the LED strip
  strip.begin();

  // Update the strip, to start they are all 'off'
  strip.show();
}


void loop() {
  
  // flash four times in blue
  for (int i = 0; i < 4; i++) {
    
    setSolidColor(strip.Color(0, 0, 127));
    delay(400);
    turnOff();
    delay(400);
  }
  
  
  // color wipe purple
  colorWipe(strip.Color(127, 0, 127), 10);
  delay(400);
  
  
  // turn off for a moment
  turnOff();
  delay(400);
  
  
  /*
  // Send a simple pixel chase in...
  colorChase(strip.Color(127,0,0), 10);  	// full brightness red
  colorChase(strip.Color(127,127,0), 10);	// orange
  colorChase(strip.Color(0,127,0), 10);		// green
  colorChase(strip.Color(0,127,127), 10);	// teal
  colorChase(strip.Color(0,0,127), 10);		// blue
  colorChase(strip.Color(127,0,127), 10);	// violet

  // fill the entire strip with...
  colorWipe(strip.Color(127,0,0), 10);		// red
  colorWipe(strip.Color(0, 127,0), 10);		// green
  colorWipe(strip.Color(0,0,127), 10);		// blue

  rainbow(10);
  rainbowCycle(0);  // make it go through the cycle fairly fast
  */
}


void setSolidColor(uint32_t color) {
  
  for (int i = 0; i < strip.numPixels(); i++) {
    
    strip.setPixelColor(i, color);
  }
  strip.show();
}


void turnOff() {
  
  setSolidColor(0);
}


void fadeWhite(uint8_t wait) {
  
  int i, j;
  
  for (i = 0; i < strip.numPixels(); i++) {
    for (j = 0; j < 128; j++) {
      strip.setPixelColor(i, j + (i * 10), j + (i * 10), j + (i * 10));
      strip.show();
    }
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  int i, j;
   
  for (j=0; j < 384; j++) {     // 3 cycles of all 384 colors in the wheel
    for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel( (i + j) % 384));
    }  
    strip.show();   // write all the pixels out
    delay(wait);
  }
}

// Slightly different, this one makes the rainbow wheel equally distributed 
// along the chain
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;
  
  for (j=0; j < 384 * 5; j++) {     // 5 cycles of all 384 colors in the wheel
    for (i=0; i < strip.numPixels(); i++) {
      // tricky math! we use each pixel as a fraction of the full 384-color wheel
      // (thats the i / strip.numPixels() part)
      // Then add in j which makes the colors go around per pixel
      // the % 384 is to make the wheel cycle around
      strip.setPixelColor(i, Wheel( ((i * 384 / strip.numPixels()) + j) % 384) );
    }  
    strip.show();   // write all the pixels out
    delay(wait);
  }
}

// fill the dots one after the other with said color
// good for testing purposes
void colorWipe(uint32_t c, uint8_t wait) {
  int i;
  
  for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

// Chase a dot down the strip
// good for testing purposes
void colorChase(uint32_t c, uint8_t wait) {
  int i;
  
  for (i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, 0);  // turn all pixels off
  } 
  
  for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      if (i == 0) { 
        strip.setPixelColor(strip.numPixels()-1, 0);
      } else {
        strip.setPixelColor(i-1, 0);
      }
      strip.show();
      delay(wait);
  }
}

/* Helper functions */

//Input a value 0 to 384 to get a color value.
//The colours are a transition r - g -b - back to r

uint32_t Wheel(uint16_t WheelPos)
{
  byte r, g, b;
  switch(WheelPos / 128)
  {
    case 0:
      r = 127 - WheelPos % 128;   //Red down
      g = WheelPos % 128;      // Green up
      b = 0;                  //blue off
      break; 
    case 1:
      g = 127 - WheelPos % 128;  //green down
      b = WheelPos % 128;      //blue up
      r = 0;                  //red off
      break; 
    case 2:
      b = 127 - WheelPos % 128;  //blue down 
      r = WheelPos % 128;      //red up
      g = 0;                  //green off
      break; 
  }
  return(strip.Color(r,g,b));
}

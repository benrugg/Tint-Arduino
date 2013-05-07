/* NOTE: This sketch is made for the Arduino Ethernet. This one works perfectly via the Serial port.
         Open the serial monitor and send color values separated by a comma and ending with a period:
         8046df,7d4ade,7b4cdd,784fdc,7453da,7355da,6f59d8,6e5ad7,6a5ed6,6762d5.
*/


#include "LPD8806.h"
#include "SPI.h"




// ------------------------------- LED setup -------------------------------

// declare the number of LEDs
int numLEDs = 160;


// create an instance of the LED strip class. use pin 6 for data in (DI) and
// pin 8 for clock in (CI) (we're not using the faster hardware SPI (pins 11
// and 13) because they're used by the Ethernet interface)
LPD8806 strip = LPD8806(numLEDs, 6, 8);


/*
// create an instance of the LED strip class. Use hardware SPI, so connect DI
// to pin 11 and CI to pin 13.
LPD8806 strip = LPD8806(numLEDs);
*/


// declare other variables
int currentLED = 0;
char incomingBuffer[6];
int currentBufferIndex = 0;



// -------------------------- handle color commands ------------------------

// set a specific LED using a hex color
void setLED(int ledNum, String hexColor) {
  
  // if the hex isn't 6 character, just quit here
  if (hexColor.length() != 6) return;
  
  
  // separate the hex string into rgb and convert it from 8 bit (0-255) to 7 bit (0-127)
  String hexPart;
  unsigned int r;
  unsigned int g;
  unsigned int b;
  
  hexPart = hexColor.substring(0, 2);
  r = hexToDec(hexPart) / 2;
  
  hexPart = hexColor.substring(2, 4);
  g = hexToDec(hexPart) / 2;
  
  hexPart = hexColor.substring(4, 6);
  b = hexToDec(hexPart) / 2;
  
  
  // set the LED to this color
  strip.setPixelColor(ledNum, r, g, b);
}


unsigned int hexToDec(String hexString) {
  
  // NOTE: This function can handle a positive hex value from 0 - 65,535 (a four digit hex string).
  //       For larger/longer values, change "unsigned int" to "long" in both places.
  
  unsigned int decValue = 0;
  int nextInt;
  
  for (int i = 0; i < hexString.length(); i++) {
    
    nextInt = int(hexString.charAt(i));
    if (nextInt >= 48 && nextInt <= 57) nextInt = map(nextInt, 48, 57, 0, 9);
    if (nextInt >= 65 && nextInt <= 70) nextInt = map(nextInt, 65, 70, 10, 15);
    if (nextInt >= 97 && nextInt <= 102) nextInt = map(nextInt, 97, 102, 10, 15);
    nextInt = constrain(nextInt, 0, 15);
    
    decValue = (decValue * 16) + nextInt;
  }
  
  return decValue;
}




// -------------------------- core runtime functions ------------------------

void setup() {
  
  // Start up the LED strip
  strip.begin();
  
  
  // Update the strip, to start they are all 'off'
  strip.show();
  
  
  // Enable Serial output for testing
  Serial.begin(9600);
}


void loop() {
  
  // if we have any incoming info from the Serial port, read into the buffer
  while (Serial.available() > 0) {
    
    // read the next character
    char nextChar = (char)Serial.read();
    
    
    // if the current character is a comma or a period, consider the buffer complete
    // and send the color value to the LED function
    if (nextChar == ',' || nextChar == '.') {
      
      // set the next LED
      setLED(currentLED++, String(incomingBuffer));
      
      
      // if we've set all the LEDs, start over at the beginning of the strip
      if (currentLED >= numLEDs) currentLED = 0;
      
      
      // start our buffer over
      currentBufferIndex = 0;
      
      
      // if we reached a period, tell the strip to show its colors
      if (nextChar == '.') strip.show();
      
      
    // else, add the character to the buffer
    } else {
      
      incomingBuffer[currentBufferIndex++] = nextChar;
      
      if (currentBufferIndex >= 6) currentBufferIndex = 0;
    }
  }
  
  
  // delay for a moment, just so we don't loop too fast
  delay(1);
}

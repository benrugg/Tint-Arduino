/* NOTE: This is the first RGB Wall sketch that's made for the Arduino Ethernet
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



// -------------------------- handle color commands ------------------------

// this function handles url requests
boolean handleURLRequest(char* urlCharArray) {
    
    // make a string from the character array so we can manipulate it more easily
    String urlString = String(urlCharArray);
    
    
    // xxx
    //Serial.println("Full String:");
    //Serial.println(urlString);
    
    
    // if the url doesn't contain a comma, return false (404) (this is our quick
    // and dirty way of making sure hits like "/favicon.ico" won't mess with the
    // LEDs)
    if (urlString.indexOf(',') == -1) return false;
    
    
    // clear the strip to start it over
    // xxx can remove this later when we're in production and always getting the full number of pixels
    for (int i = 0; i < numLEDs; i++) strip.setPixelColor(i, 0);
    
    
    // if the first character of the string is a forward slash, remove it
    if (urlString.charAt(0) == '/') urlString = urlString.substring(1);
    
    
    // xxx
    //Serial.println("No Slash:");
    //Serial.println(urlString);
    
    
    // Parse the url to get the colors from it (in a format of 6 digit rgb hex codes, separated
    // by commas: E690CC,EFD988,9BA456, etc) (also, convert it from 8 bit (0-255) to 7 bit (0-127)).
    // As we do this, set each led to the color that is specified.
    String currentHex;
    int nextCommaIndex = 0;
    String hexPart;
    unsigned int r;
    unsigned int g;
    unsigned int b;
    int ledNum = 0;
    
    while (urlString.length() > 0) {
      
      // get the next index of a comma
      nextCommaIndex = urlString.indexOf(',');
      
      
      // xxx
      //Serial.println("nextCommaIndex:");
      //Serial.println(nextCommaIndex);
      
      
      // if we don't have a comma remaining in the string, just get the rest of the string
      if (nextCommaIndex == -1) {
        
        currentHex = urlString;
        urlString = "";
        
      // else, get the portion of the string up to the next comma
      } else {
        
        currentHex = urlString.substring(0, nextCommaIndex);
        urlString = urlString.substring(nextCommaIndex + 1);
      }
      
      
      // xxx
      //Serial.println("currentHex:");
      //Serial.println(currentHex);
      //Serial.println("urlString:");
      //Serial.println(urlString);
      
      
      // if the currentHex isn't 6 colors, skip it
      if (currentHex.length() != 6) continue;
      
      
      // convert the 8 bit hex into 7 bit rgb
      hexPart = currentHex.substring(0, 2);
      r = hexToDec(hexPart) / 2;
      
      
      // xxx
      //Serial.println("currentHex:");
      //Serial.println(currentHex);
      //Serial.println("hexPart:");
      //Serial.println(hexPart);
      //Serial.println("r:");
      //Serial.println(r);
      
      
      hexPart = currentHex.substring(2, 4);
      g = hexToDec(hexPart) / 2;
      
      
      // xxx
      //Serial.println("currentHex:");
      //Serial.println(currentHex);
      //Serial.println("hexPart:");
      //Serial.println(hexPart);
      //Serial.println("g:");
      //Serial.println(g);
      
      
      hexPart = currentHex.substring(4, 6);
      b = hexToDec(hexPart) / 2;
      
      
      // xxx
      //Serial.println("currentHex:");
      //Serial.println(currentHex);
      //Serial.println("hexPart:");
      //Serial.println(hexPart);
      //Serial.println("b:");
      //Serial.println(b);
      
      
      // xxx
      //Serial.println("currentHex again:");
      //Serial.println(currentHex);
      //Serial.println("currentHex plus colon:");
      //Serial.println(currentHex + ":");
      //Serial.println("currentHex plus rgb:");
      //Serial.println(currentHex + ": (" + r + ", " + g + ", " + b + ")");
      
      
      // set the next LED to this color
      strip.setPixelColor(ledNum, r, g, b);
      ledNum++;
      
      
      // if we've already set all our LED's, end the loop here
      if (ledNum >= numLEDs) break;
    }
    
    
    // tell the LED strip to show its updated colors
    strip.show();
    
    
    // xxx
    delay(1000);
    
    
    // return true to output status 200
    return true;
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
  
  // delay for a moment
  delay(10);
}




// ----------------------------------- test ---------------------------------

void serialEvent() {
  
  String incomingString = "";
  
  while (Serial.available() > 0) {
    
    incomingString += (char)Serial.read();
  }
  
  Serial.print("received: ");
  Serial.println(incomingString);
  
  /*
  byte incomingByte;
  
  // send data only when you receive data:
  if (Serial.available() > 0) {
    
    // read the incoming byte:
    incomingByte = Serial.read();

    // say what you got:
    //Serial.print("I received: ");
    //Serial.println(incomingByte, DEC);
  }
  */
  
  
  char test1[ ] = "/FF00FF,00ee99,99cc78,ef742c";
  Serial.println("testing the first string");
  Serial.println(test1);
  handleURLRequest(test1);
  
  delay(2000);
  
  char test2[ ] = "/FF00FF,00ee99,99cc78,ef742c,FF00FF,00ee99,99cc78,ef742c";
  Serial.println("testing the second string");
  Serial.println(test2);
  handleURLRequest(test2);
  
  delay(2000);
  
  char test3[ ] = "/FF00FF,00ee99,99cc78,ef742c,FF00FF,00ee99,99cc78,ef742c,FF00FF,00ee99,99cc78,ef742c";
  Serial.println("testing the third string");
  Serial.println(test3);
  handleURLRequest(test3);
  
  delay(2000);
  
  char test4[ ] = "/7b4cdd,7058d8,6663d4,5b70d0,507ccb,4687c7,3b93c2,319fbe,26abba,1bb7b5";
  Serial.println(test4);
  handleURLRequest(test4);
  
  delay(2000);
  
  char test5[ ] = "/8046df,7d4ade,7b4cdd,784fdc,7453da,7355da,6f59d8,6e5ad7,6a5ed6,6762d5";
  Serial.println(test5);
  handleURLRequest(test5);
  
  delay(2000);
  
  
  // The core Arduino code seems to have a bug. If these lines are uncommented, the whole thing 
  // freezes after the first test!
  
  /*
  //char test6[ ] = "/8046df,7d4ade,7b4cdd,784fdc,7453da,7355da,6f59d8,6e5ad7,6a5ed6,6762d5,6664d4,6267d3,6169d2,5d6dd1,5a71cf,5873cf,5576cd,5378cc,507ccb,4d80ca,4b82c9,4885c8,4687c7";
  char test6[ ] = "/784fdc,6664d4,5378cc,418dc5,2da3bd,1bb7b5,33bfa9,5bc19c,83c48f,abc782,d3c974,fbcc67,ffa954,ff8643,ff6331,ff4020,ff1d0e,ff0008,ff0035,ff0065,ff0092,ff00be,ff00eb";
  Serial.println(test6);
  handleURLRequest(test6);
  
  delay(2000);
  
  char test7[ ] = "/8046df,7d4ade,7b4cdd,784fdc,7453da,7355da,6f59d8,6e5ad7,6a5ed6,6762d5,6664d4,6267d3,6169d2,5d6dd1,5a71cf"; //,5873cf,5576cd,5378cc,507ccb,4d80ca,4b82c9,4885c8,4687c7,438bc6,3f8fc4,3e90c4,3a94c2,3996c1,359ac0,329dbf,319fbe,2da3bd,2ca5bc,28a9bb,25acb9,23aeb9,20b2b7"; //,1eb4b6,1bb7b5,18bbb4,16bdb3,1dbdb1,21beaf,28bead,2fbfab,33bfa9,3abfa7,3ec0a6,45c0a3,4dc1a1,50c1a0,58c19d,5bc19c,62c29a,6ac297,6dc396,75c394,78c393,80c490,87c48e,8bc58d,92c58a,95c589,9dc686,a4c684,a8c683,afc780,b3c77f,bac87d,c1c87a,c5c879,ccc977,d0c976,d7c973,deca71,e2ca70,e9cb6d,edcb6c,f4cb6a,fbcc67,ffcc66,ffc663,ffc261,ffbc5e,ffb65b,ffb359,ffac56,ffa954,ffa351,ff9f50,ff994d,ff9349,ff8f48,ff8945,ff8643,ff8040,ff793d,ff763b,ff7038,ff6c36,ff6633,ff6030,ff5c2e,ff562b,ff5329,ff4d26,ff4623,ff4321,ff3d1e,ff391d,ff331a,ff2d16,ff2915,ff2312,ff2010,ff1a0d,ff130a,ff1008,ff0a05,ff0603,ff0000,ff0008,ff000c,ff0014,ff0018,ff0020,ff0028,ff002d,ff0035,ff0039,ff0041,ff0049,ff004d,ff0055,ff0059,ff0061,ff0069,ff006d,ff0075,ff0079,ff0082,ff008a,ff008e,ff0096,ff009a,ff00a2,ff00aa,ff00ae,ff00b6,ff00ba,ff00c2,ff00ca,ff00ce,ff00d7,ff00db,ff00e3,ff00eb,ff00ef,ff00f7,ff00fb";
  Serial.println(test7);
  handleURLRequest(test7);
  
  delay(2000);
  */
}

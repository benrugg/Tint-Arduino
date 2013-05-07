/* NOTE: This sketch is made for the Arduino Ethernet.
         
         **This one works with all 160 LED's!**
         
         Changes from the last one:
          - got rid of the arrays for current R/G/B values in favor of just using the
            strip's internal memory instead (which saves a lot of memory...)
*/


#include "LPD8806.h"
#include "SPI.h"
#include <Ethernet.h>



// ------------------------------- debugging -------------------------------

boolean isDebugging = false;



// ------------------------------- LED setup -------------------------------

// declare the number of LEDs
// (NOTE: make sure to set the lines below for the new color arrays to this number as well!)
int numLEDs = 160;


// initialize some other variables used for the fade animation
int fadeSteps = 20;
byte newR[160];
byte newG[160];
byte newB[160];


// create an instance of the LED strip class. use pin 6 for data in (DI) and
// pin 8 for clock in (CI) (we're not using the faster hardware SPI (pins 11
// and 13) because they're used by the Ethernet interface)
LPD8806 strip = LPD8806(numLEDs, 6, 8);


/*
// create an instance of the LED strip class. Use hardware SPI, so connect DI
// to pin 11 and CI to pin 13.
LPD8806 strip = LPD8806(numLEDs);
*/



// ---------------------------- web server setup ---------------------------

// set the MAC address of this specific Arduino Ethernet board
byte macAddress[] = {0x90, 0xA2, 0xDA, 0x0D, 0x27, 0x29};


// Initialize the Ethernet server library (using DHCP for the ip address)
// (on port 80 for HTTP)
EthernetServer server(80);



// -------------------------- core runtime functions ------------------------

void setup() {
  
  // wait a brief moment to give the strips a chance to get powered up
  delay(500);
  
  
  // start up the LED strip
  strip.begin();
  
  
  // update the strip, to start they are all 'off'
  strip.show();
  
  
  // wait a brief moment and tell the strip to show again (this fixes a bug
  // where the last couple LED's don't update reliably the first time)
  delay(10);
  strip.show();
  
  
  // enable Serial output for debugging
  if (isDebugging) Serial.begin(9600);
  
  
  // start the Ethernet connection and the server
  Ethernet.begin(macAddress);
  server.begin();
  
  
  // output what IP the server is listening on
  if (isDebugging) {
    
    Serial.print(F("server is at "));
    Serial.println(Ethernet.localIP());
  }
}


void loop() {
  
  // listen for incoming clients
  EthernetClient client = server.available();
  
  
  // when we have a connected client...
  if (client) {
    
    // output for debugging
    if (isDebugging) Serial.println(F("new client"));
    
    
    // keep track of which LED we're setting
    int currentLED = 0;
    int numLEDsProcessed = 0;
    
    
    // declare variables for the buffer we're going to read in
    char incomingBuffer[6];
    int currentBufferIndex = 0;
    
    
    // delcare flags to keep track of what we're doing
    boolean isWaitingForCommand = true;
    boolean isReceivingCommand = false;
    boolean receivedInvalidCommand = false;
    
    
    // declare a variable so we can know when we've received two blank lines in
    // a row (which signifies the end of the message)
    boolean isAtEndOfLine = true;
    
    
    // while the client stays connected...
    while (client.connected()) {
      
      // if there is any available incoming data...
      if (client.available()) {
        
        // read the next character
        char nextChar = client.read();
        
        
        // print what we received, for debugging
        if (isDebugging) Serial.write(nextChar);
        
        
        // if we're still waiting for a command, ignore everything until we get to a question mark or a newline
        if (isWaitingForCommand) {
          
          // if we just received a newline, ignore this entire incoming message (because
          // we didn't receive a valid command. maybe a different URL was requested, such
          // as /favicon.ico)
          if (nextChar == '\n') {
            
            // output for debugging
            if (isDebugging) Serial.println(F("\n\nreceived an invalid command\n"));
            
            
            // set the flags
            receivedInvalidCommand = true;
            isWaitingForCommand = false;
            isAtEndOfLine = true;
            
            
          // else, if we received a question mark, then start receiving the command
          } else if (nextChar == '?') {
            
            // output for debugging
            if (isDebugging) Serial.println(F("\n\nstarting to receive command\n"));
            
            
            // set the flag
            isWaitingForCommand = false;
            isReceivingCommand = true;
          }
          
          
        // else, if we're receiving a command, continue parsing each character as it comes
        } else if (isReceivingCommand) {
          
          // if the current character is a comma or a period, consider the buffer complete
          // and send the color value to the LED function
          if (nextChar == ',' || nextChar == '.') {
            
            // store the value for this new color
            storeNewColor(currentLED, String(incomingBuffer));
            
            
            // increment our LED counters
            currentLED++;
            numLEDsProcessed++;
            
            
            // if we've set all the LEDs, start over at the beginning of the strip (this shouldn't
            // happen, but it's here as a safeguard)
            if (currentLED >= numLEDs) currentLED = 0;
            
            
            // start our buffer over
            currentBufferIndex = 0;
            
            
            // if we reached a period, stop receiving the command and tell the strip to fade in
            // its new colors
            if (nextChar == '.') {
              
              // set the flag to stop receiving the command
              isReceivingCommand = false;
              
              
              // output for debugging
              if (isDebugging) Serial.println(F("\n\nfinished command\n"));
              
              
              // fade in the new colors
              fadeInNewColors();
            }
            
            
          // else, we have a single character that we just read, so add the character to
          // the buffer
          } else {
            
            incomingBuffer[currentBufferIndex++] = nextChar;
            
            if (currentBufferIndex >= 6) currentBufferIndex = 0;
          }
          
          
        // else, we're waiting to get to the end of the incoming message so we can send a
        // response
        } else {
          
          // if we've reached a fully blank line, reset everything, send a reply and disconnect
          if (nextChar == '\n' && isAtEndOfLine) {
            
            // send a standard http response header (with an access control header that will
            // allow ajax requests to be called from any host)
            client.println(F("HTTP/1.1 200 OK"));
            client.println(F("Content-Type: text/html"));
            client.println(F("Access-Control-Allow-Origin: *"));
            client.println(F("Connnection: close"));
            client.println();
            
            
            // send info back about what we processed
            if (receivedInvalidCommand) {
              
              client.print(F("invalid color string"));
              
            } else {
              
              client.print(F("processed "));
              client.print(numLEDsProcessed);
              client.print(F(" colors"));
            }
            
            
            // break out of the loop, so we can disconnect
            break;
          }
          
          
          // if we reached the end of a line, set the flag
          if (nextChar == '\n') {
            
            isAtEndOfLine = true;
            
          // else, if we have a real character (anything other than a carriage-return), set the flag
          } else if (nextChar != '\r') {
            
            isAtEndOfLine = false;
          }
        }
      }
    }
    
    
    // delay for a milisecond to give the web browser time to receive the data
    delay(1);
    
    
    // close the connection
    client.stop();
    
    
    // output for debugging
    if (isDebugging) Serial.println(F("client disonnected"));
  }
}



// -------------------------- handle color commands ------------------------

// store the value for the new color that we'll set a specific LED (using a hex color
// as the input)
void storeNewColor(int ledNum, String hexColor) {
  
  // if the hex isn't 6 character, just quit here
  if (hexColor.length() != 6) return;
  
  
  // separate the hex string into rgb and convert it from 8 bit (0-255) to 7 bit (0-127)
  // then store them in the array {r, g, b}
  String hexPart;
  
  hexPart = hexColor.substring(0, 2);
  newR[ledNum] = (byte) hexToDec(hexPart) / 2;
  
  hexPart = hexColor.substring(2, 4);
  newG[ledNum] = (byte) hexToDec(hexPart) / 2;
  
  hexPart = hexColor.substring(4, 6);
  newB[ledNum] = (byte) hexToDec(hexPart) / 2;
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


void fadeInNewColors() {
  
  // for each fade step...
  for (int remainingFadeSteps = fadeSteps; remainingFadeSteps >= 1; remainingFadeSteps--) {
    
    // for each LED...
    for (int ledNum = 0; ledNum < numLEDs; ledNum++) {
      
      // get the current color value for this LED and extract its RGB components
      uint32_t currentColor = strip.getPixelColor(ledNum);
      byte currentG = (currentColor >> 16) & 255;
      byte currentR = (currentColor >> 8) & 255;
      byte currentB = currentColor & 255;
      
      
      // calculate the next color for each LED
      int r = ((newR[ledNum] - currentR) / remainingFadeSteps) + currentR;
      int g = ((newG[ledNum] - currentG) / remainingFadeSteps) + currentG;
      int b = ((newB[ledNum] - currentB) / remainingFadeSteps) + currentB;
      
      
      // and then set it in the strip
      strip.setPixelColor(ledNum, r, g, b);
    }
    
    
    // show the colors (at their current fade step)
    strip.show();
  }
  
  
  // show the strip again, just to reliably lock in the last colors
  delay(10);
  strip.show();
}

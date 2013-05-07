/* NOTE: This sketch is made for the Arduino Ethernet.
         
         Changes from the last one:
          - added a function for outputting the current key colors
          - commands that can be sent:
              - /c/?8046df,8046df,etc
              - /f/?ff9900
              - /g/
*/


#include "LPD8806.h"
#include "SPI.h"
#include <Ethernet.h>



// ------------------------------- debugging -------------------------------

boolean isDebugging = true;




// ------------------------------- LED setup -------------------------------

// declare the number of LEDs
// (NOTE: make sure to set the lines below for the new color arrays to this number as well!)
int numLEDs = 160;


// initialize the arrays to hold the new color values that we're going to fade to
byte newR[160];
byte newG[160];
byte newB[160];


// initalize a couple variables to keep track of which colors are key colors in the gradient we're displaying
byte keyColorIndexes[20];
byte numKeyColors = 0;


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
    
    
    // read the command that we received
    char command = readCommand(client);
    
    
    // output for debugging
    if (isDebugging) {
      
      Serial.print(F("received command: "));
      Serial.println(command);
    }
    
    
    // set variables that we'll use to output a response
    int processedColors = 0;
    boolean successfulSolidColor = false;
    
    
    // if we received a command to set the colors, read and process them now
    if (command == 'c') {
      
      processedColors = readAndProcessColors(client);
      
      
    // else, if we received a command to flash a solid color, read and process it now
    } else if (command == 'f') {
      
      successfulSolidColor = readAndFlashSolidColor(client);
    }
    
    
    // read the rest of the headers
    readAllHeaders(client);
    
    
    // write the response heaers
    boolean wasStatusOK = ((command == 'f' && successfulSolidColor) || (command == 'c' && processedColors > 0) || command == 'g');
    writeResponseHeaders(client, wasStatusOK);
    
    
    // if we processed and displayed colors, output a short message
    if (command == 'c') {
      
      client.print(F("processed "));
      client.print(processedColors);
      client.print(F(" colors"));
      
      
    // else, if we flashed a solid color, output an acknowledgement
    } else if (command == 'f') {
      
      if (successfulSolidColor) {
        
        client.print(F("flashed solid color"));
        
      } else {
        
        client.print(F("no solid color"));
      }
      
      
    // else, if we received a command to output our key colors, output them now
    } else if (command == 'g') {
      
      outputKeyColors(client);
      
      
    // else, return a message saying that we received an invalid command
    } else {
      
      client.print(F("invalid command"));
    }
    
    
    // delay for a milisecond to give the web browser time to receive the data
    delay(1);
    
    
    // close the connection
    client.stop();
    
    
    // output for debugging
    if (isDebugging) Serial.println(F("client disconnected"));
  }
}










// ----------------------- handle web request/response ---------------------

char readCommand(EthernetClient client) {
  
  // declare variables for reading in the data
  char incomingBuffer[5];
  int currentBufferIndex = 0;
  boolean isReceivingCommandName = false;
  
  
  // while the client stays connected, keep looping...
  while (client.connected()) {
    
    // if there is any available incoming data...
    if (client.available()) {
      
      // read the next character and add it to the buffer
      char nextChar = client.read();
      incomingBuffer[currentBufferIndex++] = nextChar;
      
      
      // if we're receiving the command name...
      if (isReceivingCommandName) {
        
        // if this character is a question mark or a space, then we've received the command
        if (nextChar == '?' || nextChar == ' ') {
          
          // if the command was blank, treat it as "c" for backwards compatibility with
          // the older rgb wall code
          if (incomingBuffer[0] == '?') incomingBuffer[0] = 'c';
          
          
          // if the command was "c", "f" or "g", then it's a valid command, so return it
          if (incomingBuffer[0] == 'c' || incomingBuffer[0] == 'f' || incomingBuffer[0] == 'g') {
            
            return incomingBuffer[0];
            
            
          // else, return "i" for invalid command
          } else {
            
            return 'i';
          }
          
          
        // else, if we've read three characters and we still haven't received the command,
        // return "i" for invalid command
        } else if (currentBufferIndex == 3) {
          
          return 'i';
        }
        
      // else, we're looking for the string "GET /"
      } else {
        
        // if we've read the first five bytes, check to see if the first three are the value "GET"
        if (currentBufferIndex == 5) {
          
          // if they are "GET", then set the flag so we'll start receiving the command name
          if (incomingBuffer[0] == 'G' && incomingBuffer[1] == 'E' && incomingBuffer[2] == 'T') {
            
            // set the flag
            isReceivingCommandName = true;
            
            
            // reset the buffer that we're reading into
            incomingBuffer[0] = incomingBuffer[1] = incomingBuffer[2] = incomingBuffer[3] = incomingBuffer[4] = ' ';
            currentBufferIndex = 0;
            
            
          // else, return "i" for invalid command
          } else {
            
            return 'i';
          }
        }
      }
    }
  }
  
  
  // if for some reason the client gets disconnected, quit with "i" for invalid command
  return 'i';
}


int readAndProcessColors(EthernetClient client) {
  
  // keep track of which LED we're setting
  int currentLED = 0;
  int numLEDsProcessed = 0;
  
  
  // declare variables for reading in the data
  char incomingBuffer[6];
  int currentBufferIndex = 0;
  
  
  // declare a variable for a timeout
  unsigned long timeoutTime = millis() + 10000;
  
  
  // reset our count of key colors
  numKeyColors = 0;
  
  
  // while the client stays connected, keep looping...
  while (client.connected()) {
    
    // if there is any available incoming data...
    if (client.available()) {
      
      // read the next character and add it to the buffer
      char nextChar = client.read();
      
      
      // if the current character is an asterisk, the current color is a "key" color, so
      // add it to our array of key colors
      if (nextChar == '*') {
        
        // add the index of this color as a "key" color
        keyColorIndexes[numKeyColors++] = currentLED;
        
        
        // reset the number of key colors if it gets too big (to protect against buffer overflow)
        if (numKeyColors > 19) numKeyColors = 0;
        
        
      // else, if the current character is a comma or a period, consider the buffer complete
      // and send the color value to the LED function
      } else if (nextChar == ',' || nextChar == '.') {
        
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
          
          // output for debugging
          if (isDebugging) Serial.println(F("\nfinished reading colors\n"));
          
          
          // fade in the new colors (at normal speed)
          fadeInNewColors(12);
          
          
          // stop here
          return numLEDsProcessed;
        }
        
        
      // else, we have a single character that we just read, so add the character to
      // the buffer
      } else {
        
        incomingBuffer[currentBufferIndex++] = nextChar;
        
        if (currentBufferIndex >= 6) currentBufferIndex = 0;
      }
    }
    
    
    // if we've been reading or waiting for more than 10 seconds, time out
    if (millis() > timeoutTime) return 0;
  }
}


boolean readAndFlashSolidColor(EthernetClient client) {
  
  // declare variables for reading in the data
  char incomingBuffer[6];
  int currentBufferIndex = 0;
  unsigned long timeoutTime = millis() + 2000;
  
  
  // while the client stays connected, keep looping...
  while (client.connected()) {
    
    // if there is any available incoming data...
    if (client.available()) {
      
      // read the next character and add it to the buffer
      char nextChar = client.read();
      incomingBuffer[currentBufferIndex++] = nextChar;
      
      
      // when we get to the sixth character, flash the color and stop here
      if (currentBufferIndex == 6) {
        
        flashSolidColor(String(incomingBuffer));
        
        return true;
      }
    }
    
    
    // if we've been reading or waiting for more than 2 seconds, time out
    if (millis() > timeoutTime) return false;
  }
  
  
  // if for some reason the client gets disconnected, quit and return false
  return false;
}


void readAllHeaders(EthernetClient client) {
  
  // declare a variable so we can know when we've received two blank lines in
  // a row (which signifies the end of the message)
  boolean isAtEndOfLine = false;
  unsigned long timeoutTime = millis() + 5000;
  
  
  // while the client stays connected, keep looping...
  while (client.connected()) {
    
    // if there is any available incoming data...
    if (client.available()) {
      
      // read the next character
      char nextChar = client.read();
      
      
      // if the next character is a new line...
      if (nextChar == '\n') {
        
        // if we were already at the end of a line, stop here
        if (isAtEndOfLine) return;
        
        
        // otherwise, set the flag and continue
        isAtEndOfLine = true;
        
        
      // else if the character is some other content (besides a line feed), set the flag
      } else if (nextChar != '\r') {
        
        isAtEndOfLine = false;
      }
    }
    
    
    // if we've been reading or waiting for more than 5 seconds, time out
    if (millis() > timeoutTime) return;
  }
}


void writeResponseHeaders(EthernetClient client, boolean wasRequestOK) {
  
  // if the client isn't connected, just quit
  if (!client.connected()) return;
  
  
  // if this request was processed successfully, output a 200 header
  if (wasRequestOK) {
    
    client.println(F("HTTP/1.1 200 OK"));
    
    
  // else, output a 404 header
  } else {
    
    client.println(F("HTTP/1.1 404 Not Found"));
  }
  
  
  // for the rest of the header lines, send a standard http response header (including an 
  // access control header that will allow ajax requests to be called from any host)
  client.println(F("Content-Type: text/html"));
  client.println(F("Access-Control-Allow-Origin: *"));
  client.println(F("Connnection: close"));
  client.println();
}











// -------------------------- process gradient color commands ------------------------

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


void fadeInNewColors(int fadeSteps) {
  
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










// -------------------------- flash a solid color --------------------------

void flashSolidColor(String hexColor) {
  
  // get the rgb decimal value from the hex string:
  
  // separate the hex string into rgb and convert it from 8 bit (0-255) to 7 bit (0-127)
  // then store them in the array {r, g, b}
  String hexPart;
  
  hexPart = hexColor.substring(0, 2);
  byte r = (byte) hexToDec(hexPart) / 2;
  
  hexPart = hexColor.substring(2, 4);
  byte g = (byte) hexToDec(hexPart) / 2;
  
  hexPart = hexColor.substring(4, 6);
  byte b = (byte) hexToDec(hexPart) / 2;
  
  
  // set the entire strip to the new color
  for (int ledNum = 0; ledNum < numLEDs; ledNum++) {
    
    strip.setPixelColor(ledNum, r, g, b);
  }
  
  
  // show the colors
  strip.show();
  
  
  // show the strip again, just to reliably lock in the color
  delay(10);
  strip.show();
  
  
  // wait a moment
  delay(400);
  
  
  // fade the original colors back in slowly
  fadeInNewColors(60);
}










// ------------------------- output our key colors -------------------------

void outputKeyColors(EthernetClient client) {
  
  // if we don't have any key colors, output #000000
  if (numKeyColors == 0) {
    
    client.print(F("#000000"));
    
    
  // else, loop through the key colors and output them
  } else {
    
    for (int i = 0; i < numKeyColors; i++) {
      
      client.print(F("#"));
      
      client.print(decToHex(newR[keyColorIndexes[i]] * 2, 2));
      client.print(decToHex(newG[keyColorIndexes[i]] * 2, 2));
      client.print(decToHex(newB[keyColorIndexes[i]] * 2, 2));
      
      client.print(F(" "));
      client.print(round(((float) keyColorIndexes[i] / (numLEDs - 1)) * 100));
      client.print(F("%"));
      
      if (i + 1 < numKeyColors) client.print(F(","));
    }
  }
}










// ---------------------- hex/dec color conversion utilities -----------------------

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


String decToHex(byte decValue, byte desiredStringLength) {
  
  // NOTE: This function can handle a positive decimal value from 0 - 255, and it will pad it
  //       with 0's (on the left) if it is less than the desired string length.
  //       For larger/longer values, change "byte" to "unsigned int" or "long" for the decValue parameter.
  
  String hexString = String(decValue, HEX);
  while (hexString.length() < desiredStringLength) hexString = "0" + hexString;
  
  return hexString;
}


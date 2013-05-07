/* NOTE: This sketch is made for the Arduino Ethernet. This is the first one that actually works
         to light up the LED's through a web browser! Open the serial monitor just so you can
         see the debugging console (and so you know what ip address the arduino is listening on)
         Then go to the url like this:
         http://192.168.1.82/?8046df,7d4ade,7b4cdd,784fdc,7453da,7355da,6f59d8,6e5ad7,6a5ed6,6762d5,6664d4,6267d3,6169d2,5d6dd1,5a71cf,5873cf,5576cd,5378cc,507ccb,4d80ca,4b82c9,4885c8,4687c7,438bc6,3f8fc4,3e90c4,3a94c2,3996c1,359ac0,329dbf,319fbe,2da3bd,2ca5bc,28a9bb,25acb9,23aeb9,20b2b71eb4b6,1bb7b5,18bbb4,16bdb3,1dbdb1,21beaf,28bead,2fbfab,33bfa9,3abfa7,3ec0a6,45c0a3,4dc1a1,50c1a0,58c19d,5bc19c,62c29a,6ac297,6dc396,75c394,78c393,80c490,87c48e,8bc58d,92c58a,95c589,9dc686,a4c684,a8c683,afc780,b3c77f,bac87d,c1c87a,c5c879,ccc977,d0c976,d7c973,deca71,e2ca70,e9cb6d,edcb6c,f4cb6a,fbcc67,ffcc66,ffc663,ffc261,ffbc5e,ffb65b,ffb359,ffac56,ffa954,ffa351,ff9f50,ff994d,ff9349,ff8f48,ff8945,ff8643,ff8040,ff793d,ff763b,ff7038,ff6c36,ff6633,ff6030,ff5c2e,ff562b,ff5329,ff4d26,ff4623,ff4321,ff3d1e,ff391d,ff331a,ff2d16,ff2915,ff2312,ff2010,ff1a0d,ff130a,ff1008,ff0a05,ff0603,ff0000,ff0008,ff000c,ff0014,ff0018,ff0020,ff0028,ff002d,ff0035,ff0039,ff0041,ff0049,ff004d,ff0055,ff0059,ff0061,ff0069,ff006d,ff0075,ff0079,ff0082,ff008a,ff008e,ff0096,ff009a,ff00a2,ff00aa,ff00ae,ff00b6,ff00ba,ff00c2,ff00ca,ff00ce,ff00d7,ff00db,ff00e3,ff00eb,ff00ef,ff00f7,ff00fb.
         Or use the rgb wall website to hit that url via ajax!
*/


#include "LPD8806.h"
#include "SPI.h"
#include <Ethernet.h>



// ------------------------------- debugging -------------------------------

boolean isDebugging = false;



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



// ---------------------------- web server setup ---------------------------

// set the MAC address of this specific Arduino Ethernet board
byte macAddress[] = {0x90, 0xA2, 0xDA, 0x0D, 0x27, 0x29};


// Initialize the Ethernet server library (using DHCP for the ip address)
// (on port 80 for HTTP)
EthernetServer server(80);



// -------------------------- core runtime functions ------------------------

void setup() {
  
  // Start up the LED strip
  strip.begin();
  
  
  // Update the strip, to start they are all 'off'
  strip.show();
  
  
  // Enable Serial output for debugging
  if (isDebugging) Serial.begin(9600);
  
  
  // start the Ethernet connection and the server
  Ethernet.begin(macAddress);
  server.begin();
  
  
  // output what IP the server is listening on
  if (isDebugging) {
    
    Serial.print("server is at ");
    Serial.println(Ethernet.localIP());
  }
}


void loop() {
  
  // listen for incoming clients
  EthernetClient client = server.available();
  
  
  // when we have a connected client...
  if (client) {
    
    // output for debugging
    if (isDebugging) Serial.println("new client");
    
    
    // reset the strip
    // resetStrip();
    
    
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
            if (isDebugging) Serial.println("\n\nreceived an invalid command\n");
            
            
            // set the flags
            receivedInvalidCommand = true;
            isWaitingForCommand = false;
            isAtEndOfLine = true;
            
            
          // else, if we received a question mark, then start receiving the command
          } else if (nextChar == '?') {
            
            // output for debugging
            if (isDebugging) Serial.println("\n\nstarting to receive command\n");
            
            
            // set the flag
            isWaitingForCommand = false;
            isReceivingCommand = true;
          }
          
          
        // else, if we're receiving a command, continue parsing each character as it comes
        } else if (isReceivingCommand) {
          
          // if the current character is a comma or a period, consider the buffer complete
          // and send the color value to the LED function
          if (nextChar == ',' || nextChar == '.') {
            
            // set the next LED
            setLED(currentLED, String(incomingBuffer));
            
            
            // increment our LED counters
            currentLED++;
            numLEDsProcessed++;
            
            
            // if we've set all the LEDs, start over at the beginning of the strip (this shouldn't
            // happen, but it's here as a safeguard)
            if (currentLED >= numLEDs) currentLED = 0;
            
            
            // start our buffer over
            currentBufferIndex = 0;
            
            
            // if we reached a period, tell the strip to show its colors, and stop receiving
            // the command
            if (nextChar == '.') {
              
              // show the colors
              strip.show();
              
              
              // set the flag to stop receiving the command
              isReceivingCommand = false;
              
              
              // output for debugging
              if (isDebugging) Serial.println("\n\nfinished command\n");
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
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Access-Control-Allow-Origin: *");
            client.println("Connnection: close");
            client.println();
            
            
            // send info back about what we processed
            if (receivedInvalidCommand) {
              
              client.print("invalid color string");
              
            } else {
              
              client.print("processed ");
              client.print(numLEDsProcessed);
              client.print(" colors");
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
    if (isDebugging) Serial.println("client disonnected");
  }
}



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


// set the whole strip to be off
void resetStrip() {
  
  for(int i = 0; i < strip.numPixels(); i++) strip.setPixelColor(i, 0);
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

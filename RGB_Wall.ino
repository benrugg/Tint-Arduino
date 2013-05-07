/* NOTE: In order to compile this sketch, first make sure to edit apps-conf.h in the WiShield library so
         it defines APP_WISERVER and doesn't define APP_WEBSERVER
*/

/* NOTE: This sketch seems to work unreliably unless it's given some time to run between restarts. It
         also may need to be restarted once or twice to actually work. (Try closing and reopening the
         serial monitor to restart it). (Also, remember to wait a full minute for everything to start
         and the red light to turn on).
*/


#include "LPD8806.h"
#include "SPI.h"
#include <WiServer.h>


// ------------------------------- Wireless set up -------------------------------

#define WIRELESS_MODE_INFRA	1
#define WIRELESS_MODE_ADHOC	2

unsigned char local_ip[] = {192,168,1,150};	// IP address of WiShield
unsigned char gateway_ip[] = {192,168,1,254};	// router or gateway IP address
unsigned char subnet_mask[] = {255,255,255,0};	// subnet mask for the local network
const prog_char ssid[] PROGMEM = {"NewWorld"};		// max 32 bytes

unsigned char security_type = 2;	// 0 - open; 1 - WEP; 2 - WPA; 3 - WPA2

// WPA/WPA2 passphrase
const prog_char security_passphrase[] PROGMEM = {"Espionage99"};	// max 64 characters

// WEP 128-bit keys
// sample HEX keys
prog_uchar wep_keys[] PROGMEM = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,	// Key 0
				  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Key 1
				  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Key 2
				  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	// Key 3
				};

// setup the wireless mode
// infrastructure - connect to AP
// adhoc - connect to another WiFi device
unsigned char wireless_mode = WIRELESS_MODE_INFRA;

unsigned char ssid_len;
unsigned char security_passphrase_len;




// ------------------------------- LED setup -------------------------------

// declare the number of LEDs
int numLEDs = 160;


// create an instance of the LED strip class. use pin 6 for data in (DI) and
// pin 8 for clock in (CI) (we're not using the faster hardware SPI (pins 11
// and 13) because they're used by the wishield. in fact, almost all the
// pins are used by the wishield)
LPD8806 strip = LPD8806(numLEDs, 6, 8);




// -------------------------- handle color commands ------------------------

// this function handles url requests
boolean handleURLRequest(char* urlCharArray) {
    
    // make a string from the character array so we can manipulate it more easily
    String urlString = String(urlCharArray);
    
    
    // if the url doesn't contain a comma, return false (404) (this is our quick
    // and dirty way of making sure hits like "/favicon.ico" won't mess with the
    // LEDs)
    if (urlString.indexOf(',') == -1) return false;
    
    
    // if the first character of the string is a forward slash, remove it
    if (urlString.charAt(0) == '/') urlString = urlString.substring(1);
    
    
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
      
      
      // if we don't have a comma remaining in the string, just get the rest of the string
      if (nextCommaIndex == -1) {
        
        currentHex = urlString;
        urlString = "";
        
      // else, get the portion of the string up to the next comma
      } else {
        
        currentHex = urlString.substring(0, nextCommaIndex);
        urlString = urlString.substring(nextCommaIndex + 1);
      }
      
      
      // if the currentHex isn't 6 colors, skip it
      if (currentHex.length() != 6) continue;
      
      
      // convert the 8 bit hex into 7 bit rgb
      hexPart = currentHex.substring(0, 2);
      r = hexToDec(hexPart) / 2;
      
      hexPart = currentHex.substring(2, 4);
      g = hexToDec(hexPart) / 2;
      
      hexPart = currentHex.substring(4, 6);
      b = hexToDec(hexPart) / 2;
      
      
      // xxx
      Serial.println(currentHex + ": (" + r + ", " + g + ", " + b + ")");
      
      
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
  
  
  // Initialize WiServer and give it the event callback function for handling requests
  WiServer.init(handleURLRequest);
  
  
  // Enable Serial output and ask WiServer to generate log messages (optional)
  // xxx turn off verbose mode later?...
  Serial.begin(57600);
  WiServer.enableVerboseMode(true);
}


void loop() {

  // Run WiServer
  WiServer.server_task();
  
  
  // delay for a moment
  delay(10);
}




// ----------------------------------- test ---------------------------------

void serialEvent() {
  
  byte incomingByte;
  
  // send data only when you receive data:
  if (Serial.available() > 0) {
    
    // read the incoming byte:
    incomingByte = Serial.read();

    // say what you got:
    Serial.print("I received: ");
    Serial.println(incomingByte, DEC);
  }
  
  char test1[ ] = "/FF00FF,00ee99,99cc78,ef742c";
  Serial.println("testing the first string");
  Serial.println(test1);
  handleURLRequest(test1);
  
  delay(8000);
  
  char test2[ ] = "/FF00FF,00ee99,99cc78,ef742c,FF00FF,00ee99,99cc78,ef742c";
  Serial.println("testing the second string");
  Serial.println(test2);
  handleURLRequest(test2);
  
  delay(8000);
  
  char test3[ ] = "/FF00FF,00ee99,99cc78,ef742c,FF00FF,00ee99,99cc78,ef742c,FF00FF,00ee99,99cc78,ef742c";
  Serial.println("testing the third string");
  Serial.println(test3);
  handleURLRequest(test3);
}

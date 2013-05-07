/* NOTE: In order to compile this sketch, first make sure to edit apps-conf.h in the WiShield library so
         it defines APP_WISERVER and doesn't define APP_WEBSERVER
*/

/* NOTE: This sketch seems to work unreliably unless it's given some time to run between restarts. It
         also may need to be restarted once or twice to actually work. (Try closing and reopening the
         serial monitor to restart it). (Also, remember to wait a full minute for everything to start
         and the red light to turn on).
*/


#include <WiServer.h>
//#include "LPD8806.h"
//#include "SPI.h"



// ------------------------------- Wireless set up -------------------------------
/*
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

*/


// ------------------------------- LED setup -------------------------------

// declare the number of LEDs
int numLEDs = 160;


// create an instance of the LED strip class. use hardware SPI for the writes.
// connect data in to pin 11 and clock in to pin 13
//LPD8806 strip = LPD8806(numLEDs);




// -------------------------- handle color commands ------------------------

// this function handles url requests
boolean handleURLRequest(char* urlCharArray) {
    
    /*
    // xxx old code, here for reference:
    
    // Check if the requested URL matches "/"
    if (strcmp(URL, "/") == 0) {
        // Use WiServer's print and println functions to write out the page content
        WiServer.print("<html>");
        WiServer.print("Hello World!");
        WiServer.print("</html>");
        
        // URL was recognized
        return true;
    }
    
    // URL not found
    return false;
    
    */
    
    
    // make a string from the character array so we can manipulte it more easily
    String urlString = String(urlCharArray);
    
    
    // parse the url to get the colors from it (in a format of 6 digit rgb hex codes, separated
    // by commas: E690CC,EFD988,9BA456, etc) (also, convert it from 8 bit (0-255) to 7 bit (0-127))
    String currentHex;
    int nextCommaIndex = 0;
    String hexPart;
    unsigned int r;
    unsigned int g;
    unsigned int b;
    
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
    }
    
    
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
  
  // Initialize WiServer and give it the event callback function for handling requests
  //WiServer.init(handleURLRequest);
  
  // Enable Serial output and ask WiServer to generate log messages (optional)
  // xxx turn off verbose mode later?...
  Serial.begin(57600);
  //WiServer.enableVerboseMode(true);
}

void loop(){

  // Run WiServer
  //WiServer.server_task();
  
  char charTest[ ] = "E690CC,EFD988,9BA460,00FF00,000000,FFFFFF,EEEEEE";
  
  handleURLRequest(charTest);
  
  // delay for a moment
  delay(1000);
}

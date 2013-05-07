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


// create an instance of the LED strip class. use hardware SPI for the writes.
// connect data in to pin 11 and clock in to pin 13
//LPD8806 strip = LPD8806(numLEDs);




// -------------------------- handle color commands ------------------------

// this function handles url requests
boolean handleURLRequest(char* urlString) {
    
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
    
    
    // parse the url to get the colors from it
    int i = 0;
    int currentNumber = 0;
    int currentNumDigits = 0;
    char currentChar;
    
    while(urlString[i] != '\0') {
      
      // get the current character in the string
      currentChar = urlString[i];
      
      
      // as a fail-safe, quit if we've been processing more than a certain number of digits
      i++;
      if (i > 100) break;
      
      
      // if our character is not a number or a comma, skip it
      if (byte(currentChar) != 44 && (byte(currentChar) < 48 || byte(currentChar) > 57)) continue;
      
      
      // if we've added up more than 3 digits, skip this number
      if (currentNumDigits > 3) {
        
        currentNumber = 0;
        currentNumDigits = 0;
        
        continue;
      }
      
      
      // when we get to a comma, then we've got our number
      if (currentChar == ',') {
        
        Serial.println(currentNumber);
        
        currentNumber = 0;
        currentNumDigits = 0;
        
        continue;
      }
      
      
      // otherwise, add the current character to our number
      currentNumber = (currentNumber * 10) + (int(currentChar) - 48);
      currentNumDigits++;
    }
    
    
    // print the last number
    Serial.println(currentNumber);
    
    
    // return true to output status 200
    return true;
}




// -------------------------- core runtime functions ------------------------

void setup() {
  
  // Initialize WiServer and give it the event callback function for handling requests
  WiServer.init(handleURLRequest);
  
  // Enable Serial output and ask WiServer to generate log messages (optional)
  // xxx turn off verbose mode later?...
  Serial.begin(57600);
  WiServer.enableVerboseMode(true);
}

void loop(){

  // Run WiServer
  WiServer.server_task();
  
  // delay for a moment
  delay(10);
}

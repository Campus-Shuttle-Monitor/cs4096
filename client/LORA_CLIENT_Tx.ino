#include <SPI.h> //Import SPI Library 
#include <RH_RF95.h> // RF95 from RadioHead Library 
#include <avr/wdt.h> //Watchdog timer from AVR Library
#include "CRC8.h" //Checksum generator from CRC Library

#define TRACKER_ID_LENGTH 4 //Arbitrary tracker ID length
#define TRACKER_ID "IO92" //Arbitrary tracker ID...For extensibility, implement a way to generate ID for each tracker in future

#define RFM95_CS 10 //CS if Lora connected to pin 10
#define RFM95_RST 9 //RST of Lora connected to pin 9
#define RFM95_INT 2 //INT of Lora connected to pin 2

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 434.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

CRC8 crc;

String NMEA_coordinates = ""; // a string to hold incoming NMEA sentence (GPS automatically concatenates checksum at the end)
boolean string_complete = false; // boolean that determines whether string is completely read from incoming serial stream
String GPGLL = "$GPGLL"; // GPxxx header of desired NMEA string
unsigned long startTime, stopTime;
String TRACKER_ID_CRC = TRACKER_ID; //a string that will concatenate tracker ID with checksum...Format: "<tracker_ID>*<tracker_crc>"

void setup() 
{
//Initialize Serial Monitor
  Serial.begin(9600);
  
  wdt_enable(WDTO_8S); //enabling watchdog timer so Uno resets if program hangs
  Serial.println("Start Program");
  
// Reset LoRa Module 
  pinMode(RFM95_RST, OUTPUT); 
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

//Initialize LoRa Module
  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  

 //Set the default frequency 434.0MHz
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }

  rf95.setTxPower(18); //Transmission power of the Lora Module
  
  // reserve 200 bytes for the NMEA_coordinates:
  NMEA_coordinates.reserve(200);

  //Generate CRC for tracker ID and concatenate to TRACKER_ID_CRC
  crc.add((uint8_t*)TRACKER_ID, TRACKER_ID_LENGTH);
  TRACKER_ID_CRC += "*";
  TRACKER_ID_CRC += crc.getCRC();
}


void loop() {
    wdt_reset();
    // print the string when a newline arrives:
    
    if (string_complete) {
        if (NMEA_coordinates.substring(0, 6) == GPGLL) {
            Serial.println("================================================");
            String tracker_coord = TRACKER_ID_CRC + NMEA_coordinates;
            //building radio packet...Format: <tracker_ID>*<tracker_checksum><NMEA_sentence_w_checksum>
            const char* radiopacket = tracker_coord.c_str();
            Serial.println(radiopacket);
            //If coordinates contain 'V', data not valid
            if (NMEA_coordinates.indexOf('V') != -1){
                Serial.println("Searching for satellites. Position fix not yet found");
                startTime = millis();
                rf95.send((uint8_t *)"Searching for satellites. Position fix not yet found", 52);
                stopTime = millis();
                Serial.print("Transmission Time: ");
                Serial.print(stopTime-startTime);
                Serial.println("ms");
                Serial.println("Delaying for 10 seconds now\n");
                delay(5000);
                wdt_reset();
                delay(5000);
            }
            else {
                startTime = millis();
                rf95.send((uint8_t *)radiopacket, 57);
                stopTime = millis();
                Serial.print("Transmission Time: ");
                Serial.print(stopTime-startTime);
                Serial.println("ms");
                Serial.println("Sent. Delaying for 10 seconds now\n");
                delay(5000);
                wdt_reset();
                delay(5000);
            }
        }

        // clear NMEA coordinate string
        NMEA_coordinates = "";
        string_complete = false;
    }
    
}

/*
SerialEvent occurs whenever a new data comes in the
hardware serial RX. This routine is run between each
time loop() runs, so using delay inside loop can delay
response. Multiple bytes of data may be available.
*/
void serialEvent() {
    while (Serial.available()) {
        // get the new byte:
        char inChar = (char) Serial.read();
        // add it to the NMEA_coordinates:
        NMEA_coordinates += inChar;
        // if the incoming character is a newline, set a flag
        // so the main loop can do something about it:
        if (inChar == '\n') {
            string_complete = true;
        }
    }
}

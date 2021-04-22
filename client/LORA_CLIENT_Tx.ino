#include <SPI.h> //Import SPI Library 
#include <RH_RF95.h> // RF95 from RadioHead Library 
#include <avr/wdt.h> //Watchdog timer from AVR Library
#include "CRC8.h" //Checksum generator from CRC Library
#include <AESLib.h> //Symmetric encryption from AES Library
#include <Base64.h> //Encoding from Base64 Library
#include "AES_KEY.h" //Import AES_KEY not stored in source control. Must be exactly 16 characters

//Format of radiopacket: <tracker_ID>*<tracker_checksum><NMEA_sentence_w_checksum>

#define TRACKER_ID_LENGTH 4 //Arbitrary tracker ID length
#define TRACKER_ID "IO92" //Arbitrary tracker ID...For extensibility, implement a way to generate ID for each tracker in future

#define CRC_LENGTH 2
#define GPGLL_LENGTH 50
#define RADIOPACKET_LENGTH (GPGLL_LENGTH + TRACKER_ID_LENGTH + CRC_LENGTH + 1)
#define ENCRYPTION_PADDING_LENGTH (16 - RADIOPACKET_LENGTH%16)
#define RADIOPACKET_WITH_PADDING_LENGTH (RADIOPACKET_LENGTH + ENCRYPTION_PADDING_LENGTH)

#define RFM95_CS 10 //CS if Lora connected to pin 10
#define RFM95_RST 9 //RST of Lora connected to pin 9
#define RFM95_INT 2 //INT of Lora connected to pin 2

// Change to 915.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

CRC8 crc;

String NMEA_coordinates = ""; // a string to hold incoming NMEA sentence (GPS automatically concatenates checksum at the end)
boolean string_complete = false; // boolean that determines whether string is completely read from incoming serial stream
String GPGLL = "$GPGLL"; // GPxxx header of desired NMEA string
unsigned long startTime, stopTime;
String TRACKER_ID_CRC = TRACKER_ID; //a string that will concatenate tracker ID with checksum...Format: "<tracker_ID>*<tracker_crc>"
String padding = ""; //whitespace padding for radiopacket to be encrypted

const uint8_t KEY[] = AES_KEY;

void setup() 
{
//Initialize Serial Monitor
  Serial.begin(9600);
  
  wdt_enable(WDTO_4S); //enabling watchdog timer so Uno resets if program hangs
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
  

 //Set the default frequency 915.0MHz
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
  TRACKER_ID_CRC += String(crc.getCRC(), HEX); //append crc in hex form to end of string

  //create whitespace padding
  for (int i = 0; i < ENCRYPTION_PADDING_LENGTH; i++) {
      padding += ' ';
  }
  
}


void loop() {
    wdt_reset();
    // print the string when a newline arrives:
    if (string_complete) {
        if (NMEA_coordinates.substring(0, 6) == GPGLL) {
            Serial.println("================================================");
            //building radiopacket
            uint8_t radiopacket[RADIOPACKET_WITH_PADDING_LENGTH+1];
            //If coordinates contain 'V', data not valid
            if (NMEA_coordinates.indexOf('V') != -1){
                String tracker_coord = "Searching for satellites. Position fix not yet found            ";
                Serial.println(tracker_coord);
                tracker_coord.getBytes(radiopacket,RADIOPACKET_WITH_PADDING_LENGTH+1);
                aes128_enc_multiple(KEY,radiopacket, RADIOPACKET_WITH_PADDING_LENGTH);
                uint8_t encodedLen = base64_enc_len(RADIOPACKET_WITH_PADDING_LENGTH);
                uint8_t encoded[encodedLen+1];
                base64_encode((char*)encoded, (char*)radiopacket, RADIOPACKET_WITH_PADDING_LENGTH);
                
                startTime = millis();
                rf95.send(encoded, sizeof(encoded));
                stopTime = millis();
                
                Serial.println("Delaying for 3 seconds now\n");
                delay(1500);
                wdt_reset();
                delay(1500);
            }
            else {
                String tracker_coord = TRACKER_ID_CRC + NMEA_coordinates + padding;
                tracker_coord.getBytes(radiopacket,RADIOPACKET_WITH_PADDING_LENGTH+1);
                aes128_enc_multiple(KEY,radiopacket, RADIOPACKET_WITH_PADDING_LENGTH);
                uint8_t encodedLen = base64_enc_len(RADIOPACKET_WITH_PADDING_LENGTH);
                uint8_t encoded[encodedLen+1];
                base64_encode((char*)encoded, (char*)radiopacket, RADIOPACKET_WITH_PADDING_LENGTH);
                
                startTime = millis();
                rf95.send(encoded, sizeof(encoded));
                stopTime = millis();
                
                Serial.println("Sent. Delaying for 3 seconds now\n");
                
                delay(1500);
                wdt_reset();
                delay(1500);
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
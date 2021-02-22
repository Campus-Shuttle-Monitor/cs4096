#include <SPI.h> //Import SPI librarey 
#include <RH_RF95.h> // RF95 from RadioHead Librarey 

#define RFM95_CS 10 //CS if Lora connected to pin 10
#define RFM95_RST 9 //RST of Lora connected to pin 9
#define RFM95_INT 2 //INT of Lora connected to pin 2

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 434.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

String NMEA_coordinates = ""; // a string to hold incoming NMEA data
boolean string_complete = false; // boolean that determines whether string is completely read from incoming serial stream
String GPGLL = "$GPGLL"; // GPxxx header of desired NMEA string
unsigned long startTime, stopTime, duration;

void setup() 
{
 
//Initialize Serial Monitor
  Serial.begin(9600);
  
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
}


void loop() {
    // print the string when a newline arrives:
    if (string_complete) {
        if (NMEA_coordinates.substring(0, 6) == GPGLL) {
            Serial.println("================================================");
            Serial.println(NMEA_coordinates);
            const char* radiopacket = NMEA_coordinates.c_str();
            if (strlen(radiopacket) != 52){
                Serial.println("Searching for satellites. Position fix not yet found");
                startTime = millis();
                rf95.send((uint8_t *)"Searching for satellites. Position fix not yet found", 52);
                stopTime = millis();
                Serial.print("Transmission Time: ");
                Serial.print(stopTime-startTime);
                Serial.println("ms");
                Serial.println("Delaying for 10 seconds now\n");
                delay(10000);
            }
            else {
                startTime = millis();
                rf95.send((uint8_t *)radiopacket, 50);
                stopTime = millis();
                Serial.print("Transmission Time: ");
                Serial.print(stopTime-startTime);
                Serial.println("ms");
                Serial.println("Sent. Delaying for 10 seconds now\n");
                delay(10000);
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
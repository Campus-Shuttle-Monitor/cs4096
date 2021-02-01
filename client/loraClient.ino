//Arduino Raspberry Pi wireless Comunnication through LoRa - SX1278
//Send 0 to 9 from Arduino through Radio head LoRa without ACK
//Code for: www.circuitdigest.com
//Dated: 19-4-20198

#include <SPI.h> //Import SPI librarey 
#include <RH_RF95.h> // RF95 from RadioHead Librarey 

#define RFM95_CS 10 //CS if Lora connected to pin 10
#define RFM95_RST 9 //RST of Lora connected to pin 9
#define RFM95_INT 2 //INT of Lora connected to pin 2

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 434.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

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
}

char value = 48;

void loop()
{
  Serial.println(value);
  char radiopacket[1] = { value };
  rf95.send((uint8_t *)radiopacket, 1);
  Serial.println("sent");

  delay(1000);
  value++;
  if (value > '9')
    value = 48;
}

# Shuttle Tracker

The trackers are prototyped with Uno's. Each Uno has a **LoRa** (stands for **Lo**ng **Ra**nge- has at least a 2 mile range) radio module and a **GPS** module attached. The GPS uses trilateration from multiple satellite signals to calculate the position of the shuttle. It then serially transmits the information to the Uno in [NMEA](https://www.rfwireless-world.com/Terminology/GPS-sentences-or-NMEA-sentences.html) format. Because only the latitude and longitude are of interest, the Uno will grab the _$GPGLL_ NMEA string from the GPS, encrypt the string using AES symmetric encryption, and use the LoRa module to serially transmit the message to the Raspberry Pi's LoRa module. The RPi's LoRa module will receive the message and serially transmit this information to the RPi. The RPi will decrypt the message, parse the string, and send an HTTPS POST request with the latitude and longitude information to the server. The shuttle tracker application will then be able to update the frontend with the realtime location of the shuttle.

![](images/TrackerCommunication.png?raw=true)

**UPDATE (03/27/2021):** Successful tracking of the entire campus shuttle route for a single tracker actually involved two RPi's. This is because there were too many obstructions (tall buildings) for just one RPi to receive data of the entire route. Best RPi positions were the North window and South window of the top floor of University Commons. To visualize this data, you can upload the _NorthWestWindow.kml_ and _SouthWindow.kml_ found in the _FieldTest/kml_ direcotry to [Google Earth](https://earth.google.com/web/). We noticed occasional interference in the RPi's reception of the radio packets, but because the Uno is transmitting the coordinates every 3 seconds, the interference does not significantly affect results. If desirable, the delay between transmissions can be shortened to compensate for the occasional interference that will be encountered.

**TODO (Documented in Git Issues):**

1. Current implementation is for a single tracker. Must extend functionality of multiple trackers by finding a way to implement turn-based Uno transmissions to avoid collision of radio packets when received by the RPi's

2. Current implementation is still susceptible to a replay attack. Currently, Pi checks the hours, minutes, and seconds before sending it to Firebase to make sure received radio packet is within 5 seconds of the current time. This doesn't entirely prevent the replay attack because a hacker could capture the packets from the day before and re-transmit the packets the next day at the very same time it was captured the day before. We must perform date checking along with time checking. However, this requires that the Uno send a different NMEA sentence than GPGLL. We probably want GPRMC instead of GPGLL.

## Hardware

Components needed:

- 2 Raspberry Pi's (any RPi with GPIO pins and WiFi capability should work. We used our personal [RPi 4B](https://www.amazon.com/CanaKit-Raspberry-4GB-Starter-Kit/dp/B07V5JTMV9/ref=sr_1_2?dchild=1&keywords=raspberry+pi+4b&qid=1612308205&sr=8-2) initially but had the final implementation on more affordable [RPi Zero W](https://www.amazon.com/Vilros-Raspberry-Kit-Premium-Essential-Accessories/dp/B0748NK116/ref=pd_di_sccai_5?pd_rd_w=Y4XrR&pf_rd_p=c9443270-b914-4430-a90b-72e3e7e784e0&pf_rd_r=DJYV6W1AESWC3CYERM5X&pd_rd_r=14b96814-d2e7-4d73-a52e-89ab3754b4f7&pd_rd_wg=hoceY&pd_rd_i=B0748NK116&psc=1))
- 1 Arduino Uno (any Uno clone should work. We used [Elegoo Uno R3](https://www.amazon.com/ELEGOO-Board-ATmega328P-ATMEGA16U2-Compliant/dp/B01EWOE0UU/ref=sxts_sxwds-bia-wc-rsf1_0?crid=3939LJ6SW7KCR&cv_ct_cx=elegoo+uno+r3&dchild=1&keywords=elegoo+uno+r3&pd_rd_i=B01EWOE0UU&pd_rd_r=ec55e4ea-786d-4654-bb84-64a90e6dcf24&pd_rd_w=LOKWh&pd_rd_wg=wMGsh&pf_rd_p=5d815bf0-8407-4925-96a4-1fe69f424373&pf_rd_r=Q7CBKQAC62KQ14VDHHXB&psc=1&qid=1612308182&sprefix=elegoo+uno%2Caps%2C182&sr=1-1-526ea17f-3f73-4b50-8cd8-6acff948fa5a))
- 1 GPS Module w/ UART interface (We used [Ublox Neo-6M](https://www.amazon.com/HiLetgo-GY-NEO6MV2-Controller-Ceramic-Antenna/dp/B01D1D0F5M/ref=sr_1_10?dchild=1&keywords=neo+ublox+6m+gps&qid=1612307597&sr=8-10))
- 3 LoRa Radio Modules w/ SPI interface and 915 Mhz operation capability (We used [RFM95W](https://www.adafruit.com/product/3072))
- 3 SMA Female Jack Connectors (We bought a [pack of 10](https://www.amazon.com/Female-Socket-Mount-Adapter-Connectors/dp/B07QH6TWRY/ref=sr_1_17?dchild=1&keywords=Edge-Launch+SMA+Connector+for+1.6mm+%2F+0.062%22+Thick+PCBs&qid=1615353617&sr=8-17))
- 3 915 Mhz SMA Antennas (We just bought a pack of [UFL to SMA antennas](https://www.amazon.com/Connector-868-915MHz-Lora32u4-Internet-WIshiOT/dp/B07LCKNN4H/ref=sr_1_5?dchild=1&keywords=915mhz+antenna+sma+lora&qid=1612975029&sr=8-5) and disconnected the UFL to SMA connector so we could just screw the antennas onto the SMA Female Jack Connectors)
- Soldering Iron, Solder Wire, Jumper Wires (generic tools to connect and solder components together)

## Setup

If you already got the RPi and Uno hardware set up and configured, head down to the [next section](#Running-the-Shuttle-Tracker)

### RPi

Refer to the table below for the pinout connections.

| Raspberry Pi | LoRa Module |
| ------------ | ----------- |
| 3.3V         | VIN         |
| Ground       | GND         |
| GPIO 10      | MOSI        |
| GPIO 9       | MISO        |
| GPIO 11      | SCK         |
| GPIO 8       | CS          |
| GPIO 4       | G0          |
| GPIO 17      | G1          |
| GPIO 18      | G2          |
| GPIO 27      | G3          |
| GPIO 22      | RST         |

#### Configuring RPi

SPI needs to be enabled on the RPi.

Type the following command in the configuration window:

```
sudo raspi-config
```

> ![](images/RPiConfig/1.png?raw=true)

Go to **Interfacing Options** and enable SPI interface

> ![](images/RPiConfig/2.png?raw=true) > ![](images/RPiConfig/3.png?raw=true)

Make sure pip3 and python3 are updated to the latest version.

Install the GPIO package to control the GPIO pins on the RPi:

```
pip3 install RPi.GPIO
```

Install SPI package to control SPI communication between LoRa and RPi:

```
pip3 install spidev
```

Install pyLoRa package to use the radio modules associated with LoRa:

```
pip3 install pyLoRa
```

Add the package path information to RPi. Alternatively, use the following commands to manually download the libraries and use the same directory when cloning the repository:

```
sudo apt-get install python-rpi.gpio python3-rpi.gpio
sudo apt-get install python-spidev python3-spidev
```

Install the 8 bit checksum library

```
pip3 install crc8
```

Install the micropyGPS library to parse the NMEA string

```
pip3 install git+https://github.com/inmcm/micropyGPS.git
```

Install the simplekml library to create kml files that will allow for easy visualization of field tests on Google Earth

```
pip3 install git+https://github.com/eisoldt/simplekml.git
```

### Uno

Refer to the tables below for pinout connections.

| Uno Board | LoRa Module |
| --------- | ----------- |
| 3.3V      | VIN         |
| Ground    | GND         |
| D10       | CS          |
| D2        | G0          |
| D13       | SCK         |
| D12       | MISO        |
| D11       | MOSI        |
| D9        | RST         |

| Uno Board | GPS Module |
| --------- | ---------- |
| 5V        | VCC        |
| Ground    | GND        |
| Rx        | Tx         |

Navigate to **tools>Manage Libraries** and install the following:

- CRC library by Rob Tillaart

Download the following ZIP files:

- RadioHead ZIP file from the [AirSpayce page](http://www.airspayce.com/mikem/arduino/RadioHead/)
- [Base64](https://github.com/adamvr/arduino-base64)
- [AESLib](https://github.com/DavyLandman/AESLib)

Add these files to your Arduino IDE libraries by navigating to **Sketch>Add .ZIP Library**. Restart your Arduino IDE.

## Running the Shuttle Tracker

Clone the repository:

```
git clone https://github.com/Campus-Shuttle-Monitor/cs4096.git
```

### RPi

Navigate to the **server** directory in the command line and set the AES_KEY environment variable. This key will be used to decrypt the radio packet and must be exactly 16 characters. Make sure that it matches the key that the Uno uses to encrypt the radio packet. You can set an example key by running the following on the command line:

```
export AES_KEY="ExampleAESKeyTst"
```

There are currently 2 RPi's being used, so we need a way to identify which RPi is sending the coordinates to the server. Set a unique RPI_ID environment variable. For example:

```
export RPI_ID=1
```

Once the AES key and RPi ID is set, you can run the following command to start the server:

```
python3 LORA_PI_Rx.py
```

This part is not applicable to the master branch. In the debug branch, you'll be prompted to name the logger and kml files. This is for field testing and debugging purposes. You should name it something meaningful. We found naming it after the location of the RPi useful.

### Uno

Navigate to the **client** directory. You must create a file named **AES_KEY.h** in this directory. This is where you will define an AES key to encrypt the radio packet. This key must be exactly 16 characters and must match the key that the RPi uses to decrypt the radio packet. You can set an example key by including the following in your AES_KEY.h file:

```
#define AES_KEY "ExampleAESKeyTst"
```

Once you've saved the file, open the **LORA_CLIENT_Tx.ino** sketch in the Arduino IDE. _Disconnect_ the GPS's Tx connection to the Uno's Rx pin. Upload the sketch to the Uno. Once the upload is complete, _reconnect_ the GPS's Tx pin to the Uno's Rx pin. The reason for the disconnection and reconnection is due to the Uno using its own Tx and Rx pins to serially upload the sketch to the microcontroller. Once upload is complete, those pins can be used for serial communication with other devices. Make sure the Serial monitor's baud rate is set to 9600.

### Results

The RPi should now be receiving data from the Uno.

If you are on the debug branch, you should see the encrypted message outputted to the command line. If there is no interference, you should then see the decrypted message outputted, following a "successfully parsed and logged" print statement. If the HTTPS POST request to the server is successful, you should see a "Data sent to server successfully" print statement. Otherwise, you should see a request failed with status code and response message print statement.

If you are on the master branch, you should see a "Received" print statement on reception of a radio packet and a "Data sent to server successfully" print statement if the HTTPS POST request to the server is successful. Otherwise, you should see a request failed with status code and response message print statement.

## Resources

- https://circuitdigest.com/microcontroller-projects/raspberry-pi-with-lora-peer-to-peer-communication-with-arduino
- https://www.rfwireless-world.com/Terminology/GPS-sentences-or-NMEA-sentences.html
- https://www.airspayce.com/mikem/arduino/RadioHead/
- https://groups.google.com/g/radiohead-arduino?pli=1
- https://pypi.org/project/pyLoRa/
- https://pypi.org/project/crc8/
- https://pypi.org/project/simplekml/
- https://github.com/rpsreal/pySX127x
- https://github.com/inmcm/micropyGPS
- https://github.com/adamvr/arduino-base64
- https://github.com/DavyLandman/AESLib
- https://www.ecfr.gov/cgi-bin/text-idx?SID=6ffb3bf3dfec8f3673f7ea39148bf5aa&mc=true&node=pt47.1.15&rgn=div5
- https://predictabledesigns.com/most-important-decision-when-creating-wireless-product/
- https://miliohm.com/multi-clients-lora-with-a-server-communication/ (Although this article isn't using the same set up, it might be useful for figuring out how to interface with multiple Uno's)

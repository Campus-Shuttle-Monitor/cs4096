# Shuttle Tracker

The trackers are prototyped with Arduino Uno's. Each Uno has a **LoRa** (stands for **Lo**ng **Ra**nge- has at least a 3 mile range) radio module and a **GPS** module attached. The GPS uses trilateration from multiple satellite signals to calculate the position of the shuttle. It then serially transmits the information to the Uno in [NMEA](http://aprs.gids.nl/nmea/) format. Because only the latitude and longitude are of interest, the Uno will grab the _$GPGLL_ NMEA string and use the LoRa module to serially transmit the string to the Raspberry Pi's LoRa module. The RPi's LoRa module will receive the string and serially transmit this information to the RPi. The RPi will parse the string and deliver the location to the shuttle tracker application.

![](images/TrackerCommunication.png?raw=true)

Check out this [video](https://www.youtube.com/watch?v=ccRfZrJZzaI&feature=youtu.be). I was able to get an example set up with the RPi communicating with a singular Uno. The Uno was connected to the Mac and was outputting to the serial monitor every time it transmitted a radio packet. The RPi was connected to the external monitor and was outputting to the command line every time it received a radio packet. So far I've tested a distance of ~30 feet. Yet to test 3 mile range.

## Hardware

Components needed:

- 1 Raspberry Pi (any RPi that is not RPi Zero should work. I'm using [RPi 4B](https://www.amazon.com/CanaKit-Raspberry-4GB-Starter-Kit/dp/B07V5JTMV9/ref=sr_1_2?dchild=1&keywords=raspberry+pi+4b&qid=1612308205&sr=8-2))
- 2 Arduino Uno's (any Uno clone should work. I'm using [Elegoo Uno R3](https://www.amazon.com/ELEGOO-Board-ATmega328P-ATMEGA16U2-Compliant/dp/B01EWOE0UU/ref=sxts_sxwds-bia-wc-rsf1_0?crid=3939LJ6SW7KCR&cv_ct_cx=elegoo+uno+r3&dchild=1&keywords=elegoo+uno+r3&pd_rd_i=B01EWOE0UU&pd_rd_r=ec55e4ea-786d-4654-bb84-64a90e6dcf24&pd_rd_w=LOKWh&pd_rd_wg=wMGsh&pf_rd_p=5d815bf0-8407-4925-96a4-1fe69f424373&pf_rd_r=Q7CBKQAC62KQ14VDHHXB&psc=1&qid=1612308182&sprefix=elegoo+uno%2Caps%2C182&sr=1-1-526ea17f-3f73-4b50-8cd8-6acff948fa5a))
- 3 LoRa Radio Modules w/ SPI interface (I'm using [Sx1278 Ra-02](https://www.amazon.com/ACROBOTIC-Breakout-Arduino-ESP8266-Raspberry/dp/B07MNH5W65/ref=sr_1_3?dchild=1&keywords=lora+sx1278&qid=1612308136&sr=8-3))
- 2 GPS Modules w/ UART interface (I'm using [Ublox Neo-6M](https://www.amazon.com/HiLetgo-GY-NEO6MV2-Controller-Ceramic-Antenna/dp/B01D1D0F5M/ref=sr_1_10?dchild=1&keywords=neo+ublox+6m+gps&qid=1612307597&sr=8-10))

## Setup

If you already got the RPi and Uno hardware set up and configured, head down to the [next section](#Running-the-Shuttle-Tracker)

### RPi

Refer to the table below for the pinout connections.

| Raspberry Pi | LoRa Module |
| ------------ | ----------- |
| 3.3V         | 3.3V        |
| Ground       | Ground      |
| GPIO 10      | MOSI        |
| GPIO 9       | MISO        |
| GPIO 11      | SCK         |
| GPIO 8       | NSS/Enable  |
| GPIO 4       | DIO 0       |
| GPIO 17      | DIO 1       |
| GPIO 18      | DIO 2       |
| GPIO 27      | DIO 3       |
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

### Uno

Refer to the tables below for pinout connections.

| Uno Board | LoRa Module |
| --------- | ----------- |
| 3.3V      | 3.3V        |
| Ground    | Ground      |
| D10       | NSS/Enable  |
| D2        | DIO 0       |
| D13       | SCK         |
| D12       | MISO        |
| D11       | MOSI        |
| D9        | RST         |

| Uno Board | GPS Module |
| --------- | ---------- |
| 5V        | Vcc        |
| Ground    | Ground     |
| Rx        | Tx         |

Install the SPI library by navigating to **tools>Manage Libraries**.

Download the [RadioHead library](http://www.airspayce.com/mikem/arduino/RadioHead/RadioHead-1.113.zip) and add it to your Arduino IDE libraries by navigating to **Sketch>Add .ZIP Library**. Restart your Arduino IDE.

## Running the Shuttle Tracker

Clone the repository:

```
git clone https://github.com/Campus-Shuttle-Monitor/cs4096.git
```

### RPi

Navigate to the **server** directory in the command line and run the following command:

```
python3 LORA_PI_Rx.py
```

### Uno

Navigate to the **client** directory and open the **LORA_CLIENT_Tx.ino** sketch. _Disconnect_ the GPS's Tx connection to the Uno's Rx pin. Upload the sketch to the Uno. Once the upload is complete, _reconnect_ the GPS's Tx pin to the Uno's Rx pin. The reason for the disconnection and reconnection is due to the Uno using its own Tx and Rx pins to serially upload the sketch to the microcontroller. Once upload is complete, those pins can be used for serial communication with other devices. Make sure the Serial monitor's baud rate is set to 9600.

### Results

The RPi should now be receiving data from the Uno. If you see some incomplete NMEA strings (cluster of commas instead of full GPS data), don't worry. This is because the GPS is trying to calculate it's position. This usually happens on initial power up. You should get full NMEA strings after a few minutes.

## Resources

- https://circuitdigest.com/microcontroller-projects/raspberry-pi-with-lora-peer-to-peer-communication-with-arduino
- https://www.airspayce.com/mikem/arduino/RadioHead/
- https://github.com/rpsreal/pySX127x
- https://groups.google.com/g/radiohead-arduino?pli=1
- https://pypi.org/project/pyLoRa/ (We're already using this package, and apparently it supports encrypted communication so we need to look into this before security audit)

#### Might be helpful

- https://miliohm.com/multi-clients-lora-with-a-server-communication/ (Although this article isn't using the same set up, it might still help us figure out how to interface with multiple Uno's)
- https://www.ecfr.gov/cgi-bin/text-idx?SID=6ffb3bf3dfec8f3673f7ea39148bf5aa&mc=true&node=pt47.1.15&rgn=div5 (regulations on radio...need to figure out unlicensed frequency band info for US)
- https://predictabledesigns.com/most-important-decision-when-creating-wireless-product/ (also might be helpful for frequency band info)

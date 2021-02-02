# Shuttle Tracker (README still in progress...)

The trackers will be prototyped with Arduino Uno's. Each Uno will have a **LoRa** (stands for **Lo**ng **Ra**nge- has at least a 3 mile range) radio module and a **GPS** module attached. The GPS will connect to satellites to calculate the position of the shuttle and will serially transmit the information to the Uno. This information is in [NMEA](http://aprs.gids.nl/nmea/) format. Because only the latitude and longitude are of interest, the Uno will grab the _$GPGLL_ NMEA string and use the LoRa module to transmit the string to the Raspberry Pi's LoRa module. The RPi's LoRa module will receive the string and serially transmit this information to the RPi. The RPi will parse the string and send it to the web server in order to deliver the location to the shuttle tracker application.

![](schematics/TrackerCommunication.png?raw=true)

Check out this [video](https://www.youtube.com/watch?v=ccRfZrJZzaI&feature=youtu.be). I was able to get an example set up with the RPi communicating with a singular Uno. The Uno was connected to the Mac and was outputting to the serial monitor every time it transmitted a radio packet. The RPi was connected to the external monitor and was outputting to the command line every time it received a radio packet. So far I've tested a distance of ~30 feet. Yet to test 3 mile range.

## Hardware

Components needed:

- 1 Raspberry Pi (any RPi that is not RPi Zero should work. I'm using [RPi 4B](https://www.amazon.com/CanaKit-Raspberry-4GB-Starter-Kit/dp/B07V5JTMV9/ref=sr_1_2?dchild=1&keywords=raspberry+pi+4b&qid=1612308205&sr=8-2))
- 2 Arduino Uno's (any Uno clone should work. I'm using [Elegoo Uno R3](https://www.amazon.com/ELEGOO-Board-ATmega328P-ATMEGA16U2-Compliant/dp/B01EWOE0UU/ref=sxts_sxwds-bia-wc-rsf1_0?crid=3939LJ6SW7KCR&cv_ct_cx=elegoo+uno+r3&dchild=1&keywords=elegoo+uno+r3&pd_rd_i=B01EWOE0UU&pd_rd_r=ec55e4ea-786d-4654-bb84-64a90e6dcf24&pd_rd_w=LOKWh&pd_rd_wg=wMGsh&pf_rd_p=5d815bf0-8407-4925-96a4-1fe69f424373&pf_rd_r=Q7CBKQAC62KQ14VDHHXB&psc=1&qid=1612308182&sprefix=elegoo+uno%2Caps%2C182&sr=1-1-526ea17f-3f73-4b50-8cd8-6acff948fa5a))
- 3 LoRa Radio Modules w/ SPI interface (I'm using [Sx1278 Ra-02](https://www.amazon.com/ACROBOTIC-Breakout-Arduino-ESP8266-Raspberry/dp/B07MNH5W65/ref=sr_1_3?dchild=1&keywords=lora+sx1278&qid=1612308136&sr=8-3))
- 2 GPS Modules w/ UART interface (I'm using [Ublox Neo-6M](https://www.amazon.com/HiLetgo-GY-NEO6MV2-Controller-Ceramic-Antenna/dp/B01D1D0F5M/ref=sr_1_10?dchild=1&keywords=neo+ublox+6m+gps&qid=1612307597&sr=8-10))

## Setup

### Raspberry Pi

![](schematics/RPi-Setup.png?raw=true)

### Uno

![](schematics/Uno-Setup.png?raw=true)

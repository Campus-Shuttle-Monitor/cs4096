from time import sleep
from SX127x.LoRa import *
from SX127x.board_config import BOARD
import logging
import parse
import simplekml
import datetime
import requests
import os

#name of kml and logger file
NAME = ""

#set up logging
logger = logging.Logger

#set up variables to build kml file that will map coordinates on Google Earth
coordList = []
kml = simplekml.Kml()

URL_BASE = 'https://realtime-location-gateway-waz932k.uc.gateway.dev/shuttle/'

BOARD.setup()

#NOTE: Valid decoded payload from tracker must be in the format: <trackerID_with_CRC><NMEA_sentence_with_CRC>
#       Example: AA11*27$GPGLL,3757.30780,N,09146.63871,W,232417.00,A,A*71

class LoRaRcvCont(LoRa):
    def __init__(self, verbose=False):
        super(LoRaRcvCont, self).__init__(verbose)
        self.set_mode(MODE.SLEEP)
        self.set_dio_mapping([0] * 6)
        self.key = os.environ.get('AES_KEY')
        if not self.key:
            print("\nERROR: Set environment variable AES_KEY. Must be exactly 16 characters\n")
            exit(1)
        if len(self.key) != 16:
            print("\nERROR: AES_KEY must be exactly 16 characters\n")
            exit(1)

    def start(self):
        self.reset_ptr_rx()
        self.set_mode(MODE.RXCONT)
        while True:
            sleep(.5)
            rssi_value = self.get_rssi_value()
            status = self.get_modem_status()
            sys.stdout.flush()

    def on_rx_done(self):
        print("\nReceived: ")
        self.clear_irq_flags(RxDone=1)
        payload = self.read_payload(nocheck=True)
        decoded_payload = parse.decryptPayload(payload, self.key)        
        
        print(decoded_payload)

        logging.info(decoded_payload)

        #if decoded payload contains start of NMEA sentence
        if '$' in decoded_payload:
            NMEA_start_index = decoded_payload.index('$')
            trackerID_data = parse.parseTrackerID(decoded_payload[:NMEA_start_index])
            #if calculated tracker ID checksum matches received tracker checksum
            if trackerID_data[0]:
                #parse NMEA sentence
                coord_data = parse.parseNMEA(decoded_payload[NMEA_start_index:])
                #if received NMEA coordinates pass checksum
                if coord_data[0]:
                    recvd_time = coord_data[3]
                    cur_time = datetime.datetime.now()
                    time_delta = datetime.timedelta(
                        hours=cur_time.hour - recvd_time[0],
                        minutes=cur_time.minute - recvd_time[1],
                        seconds=cur_time.second - recvd_time[2]
                    )
                    """
                    datetime.timedelta will reduce to days, seconds, microseconds
                    There should be no difference in days since current time should be after NMEA timestamp
                    So the difference in seconds should be less than or equal to 5 (assuming 5 second window)
                    Included edge case for if NMEA timestamp and datetime.datetime.now() are out of sync, so the
                    current time can be 5 seconds before the NMEA time. There are 86400 seconds in a day, so if
                    the day is -1 and seconds >= 86395, then it is a diff of 5 seconds the other way.
                    """
                    if (time_delta.days == 0 and time_delta.seconds <= 5) \
                            or (time_delta.days == -1 and time_delta.seconds >= 86395):
                        tup = tuple(coord_data[1:3])
                        kml.newpoint(coords=[tup])
                        coordList.append(tup)
                        print("sucessfully parsed and logged")
                        body = {
                            'longitude': coord_data[1],
                            'latitude': coord_data[2]
                            #'time': coord_data[3]
                        }
                        resp = requests.post(URL_BASE + trackerID_data[1], data=body)
                        if resp.status_code < 300:
                            print('Data sent to server successfully!')
                            logging.info('Data sent successfully!')
                        else:
                            print('Request failed with status: {} and message: {}'.format(resp.status_code, resp.text))
                            logging.info('Request failed with status: {} and message: {}'.format(resp.status_code, resp.text))
                    else:
                        print('Received data with outdated timestamp')
                        logging.info('Received data with outdated timestamp')

        self.set_mode(MODE.SLEEP)
        self.reset_ptr_rx()
        self.set_mode(MODE.RXCONT) 

if __name__ == '__main__':
    lora = LoRaRcvCont(verbose=False)
    lora.set_mode(MODE.STDBY)

    #  Medium Range  Defaults after init are 434.0MHz, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on 13 dBm

    lora.set_pa_config(pa_select=1)

    try:
        print("Name of logger and kml file: ", end="")
        NAME = input()
        
        logging.basicConfig(
        filename= "../FieldTest/log/" + NAME + '.log',
        filemode='w+',
        format='%(asctime)s %(levelname)-8s %(message)s',
        datefmt='%Y-%m-%d %H:%M:%S',
        level=logging.DEBUG)

        logging.info('-------------- PROGRAM START --------------')

        lora.set_freq(915.0)
        lora.start()
    except KeyboardInterrupt:
        sys.stdout.flush()
        print("")
        sys.stderr.write("KeyboardInterrupt\n")
    finally:
        sys.stdout.flush()
        print("")
        lora.set_mode(MODE.SLEEP)
        logging.info('--------------- PROGRAM END ---------------')
        kml.newlinestring(coords=coordList)
        kml.save("../FieldTest/kml/" + NAME + ".kml")
        BOARD.teardown()


from time import sleep
from SX127x.LoRa import *
from SX127x.board_config import BOARD
import logging
import parse
import simplekml

#name of kml and logger file
NAME = ""

#set up logging
logger = logging.Logger

#set up variables to build kml file that will map coordinates on Google Earth
coordList = []
kml = simplekml.Kml()

BOARD.setup()

#NOTE: Valid decoded payload from tracker must be in the format: <trackerID_with_CRC><NMEA_sentence_with_CRC>
#       Example: AA11*27$GPGLL,3757.30780,N,09146.63871,W,232417.00,A,A*71

class LoRaRcvCont(LoRa):
    def __init__(self, verbose=False):
        super(LoRaRcvCont, self).__init__(verbose)
        self.set_mode(MODE.SLEEP)
        self.set_dio_mapping([0] * 6)
        self.key = "ExampleAESKeyTst"

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
                    tup = tuple(coord_data[1:])
                    kml.newpoint(coords=[tup])
                    coordList.append(tup)
                    print("sucessfully parsed and logged")

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


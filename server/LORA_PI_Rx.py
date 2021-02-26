from time import sleep
from SX127x.LoRa import *
from SX127x.board_config import BOARD
import logging
from parseNMEA import parseNMEA
import simplekml

#name of kml and logger file
NAME = 'FieldTest'

#set up logging
logger = logging.Logger

#set up variables to build kml file that will map coordinates on Google Earth
coordList = []
kml = simplekml.Kml()

BOARD.setup()

class LoRaRcvCont(LoRa):
    def __init__(self, verbose=False):
        super(LoRaRcvCont, self).__init__(verbose)
        self.set_mode(MODE.SLEEP)
        self.set_dio_mapping([0] * 6)

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
        decoded_payload = bytes(payload).decode("utf-8",'ignore') 
        print(decoded_payload)
        logging.info(decoded_payload)

        data = parseNMEA(decoded_payload)
        if data[0] == 1:
            tup = tuple(data[1:])
            kml.newpoint(coords=[tup])
            coordList.append(tup)

        self.set_mode(MODE.SLEEP)
        self.reset_ptr_rx()
        self.set_mode(MODE.RXCONT) 

if __name__ == '__main__':
    logging.basicConfig(
        filename= "../FieldTest/log/" + NAME + '.log',
        filemode='w+',
        format='%(asctime)s %(levelname)-8s %(message)s',
        datefmt='%Y-%m-%d %H:%M:%S',
        level=logging.DEBUG)

    logging.info('-------------- PROGRAM START --------------')

    lora = LoRaRcvCont(verbose=False)
    lora.set_mode(MODE.STDBY)

    #  Medium Range  Defaults after init are 434.0MHz, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on 13 dBm

    lora.set_pa_config(pa_select=1)

    try:
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

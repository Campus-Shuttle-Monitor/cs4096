import time
import requests
import logging
import os

URL = 'https://realtime-location-gateway-waz932k.uc.gateway.dev/shuttle/1'
BASE_NAME = 'server_log'

if __name__ == '__main__':
    i = 1
    NAME = BASE_NAME
    while os.path.exists("../FieldTest/server_log/" + NAME + '.log'):
        NAME = BASE_NAME + '-' + str(i)
        i = i + 1
    with open("../FieldTest/server_log/" + NAME + '.log', 'w') as f:
        f.close()

    logging.basicConfig(
        filename="../FieldTest/server_log/" + NAME + '.log',
        filemode='a',
        format='%(asctime)s %(levelname)-8s %(message)s',
        datefmt='%Y-%m-%d %H:%M:%S',
        level=logging.INFO)
    logger = logging.Logger('LoRa_LOG', logging.DEBUG)
    logger.setLevel(logging.NOTSET)

    while True:
        resp = requests.get(URL)
        logging.info(resp.text)
        time.sleep(1)

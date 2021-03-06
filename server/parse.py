from micropyGPS import MicropyGPS
import crc8

# parses NMEA sentence
# returns [num_parsed_sentences, longitude, latitude] where 
# longitude and latitude are in degree decmimal format
# num_parsed_sentences is 
#   1 if sentence was able to pass base sentence catcher with clean CRC,
#   0 if sentence was not able to pass with clean CRC
def parseNMEA(sentence):
    gps = MicropyGPS()
    for x in sentence:
        gps.update(x)

    lon = gps.longitude
    lat = gps.latitude

    if lon[2] == 'W':
        lon = -lon[0] - lon[1]/60
    else:
        lon = lon[0] + lon[1]/60

    if lat[2] == 'S':
        lat = -lat[0] - lat[1]/60
    else:
        lat = lat[0] + lat[1]/60

    return [gps.parsed_sentences, lon, lat]

#parses trackerID and validates checksum
#parameter must be a string in the following format: <tracker_ID>*<tracker_CRC>
#returns
#   True if calculated tracker ID CRC  matches received tracker CRC
#   False otherwise
#
#only works if the CRC is exactly 2 digits which is okay for now since the tracker ID is hardcoded on client side
#   but should be fixed in the future
def parseTrackerID(trackerID_with_CRC):
    #making sure * delimiter wasn't lost to interference
    if trackerID_with_CRC[-3] == '*':
        #parse trackerID_with_CRC into tracker ID and tracker checksum
        tracker_ID = trackerID_with_CRC[:-3]
        tracker_CRC = trackerID_with_CRC[-2:]
               
        #validating if parsed checksum are digits for later int conversion
        if tracker_CRC.isdigit():
            #calculate checksum
            calculate_CRC = crc8.crc8()
            calculate_CRC.update(tracker_ID.encode('utf-8','ignore'))
                    
            #if calculated tracker checksum matches received tracker checksum
            return int(calculate_CRC.hexdigest(),16) == int(tracker_CRC)
    
    return False

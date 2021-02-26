from micropyGPS import MicropyGPS

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



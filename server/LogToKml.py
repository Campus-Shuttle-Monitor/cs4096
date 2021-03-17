#script to create kml file from log file...will save in path cs4096/FieldTest/kml/
#To execute: python3 LogToKml.py <log_file> <desired_kml_file_name>

import sys
from parse import parseNMEA
import simplekml

#set up variables to build kml file that will map coordinates on Google Earth
coordList = []
kml = simplekml.Kml()

with open(sys.argv[1]) as f:
    line = f.readline()
    while line:
        data = parseNMEA(line)
        if data[0] == 1:
            tup = tuple(data[1:])
            kml.newpoint(coords=[tup])
            coordList.append(tup)
        line = f.readline()

kml.newlinestring(coords=coordList)
kml.save("../FieldTest/kml/" + sys.argv[2])

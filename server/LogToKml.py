import sys
from parseNMEA import parseNMEA
import simplekml

#set up variables to build kml file that will map coordinates on Google Earth
coordList = []
kml = simplekml.Kml()

with open(sys.argv[1]) as f:
    line = f.readline()
    while line:
        print(line)
        data = parseNMEA(line)
        print(data)
        if data[0] == 1:
            tup = tuple(data[1:])
            kml.newpoint(coords=[tup])
            coordList.append(tup)
        line = f.readline()

kml.newlinestring(coords=coordList)
kml.save("../FieldTest/kml/" + sys.argv[2])
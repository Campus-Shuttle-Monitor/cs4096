String NMEA_coordinates = ""; // a string to hold incoming NMEA data
boolean string_complete = false; // boolean that determines whether string is completely read from incoming serial stream
String GPGLL = "$GPGLL"; // GPxxx header of desired NMEA string
void setup() {
    // initialize serial:
    Serial.begin(9600);
    // reserve 200 bytes for the NMEA_coordinates:
    NMEA_coordinates.reserve(200);
}

void loop() {
    // print the string when a newline arrives:
    if (string_complete) {
        if (NMEA_coordinates.substring(0, 6) == GPGLL) {
            Serial.println(NMEA_coordinates);
        }

        // clear NMEA coordinate string
        NMEA_coordinates = "";
        string_complete = false;
    }
}

/*
SerialEvent occurs whenever a new data comes in the
hardware serial RX. This routine is run between each
time loop() runs, so using delay inside loop can delay
response. Multiple bytes of data may be available.
*/
void serialEvent() {
    while (Serial.available()) {
        // get the new byte:
        char inChar = (char) Serial.read();
        // add it to the NMEA_coordinates:
        NMEA_coordinates += inChar;
        // if the incoming character is a newline, set a flag
        // so the main loop can do something about it:
        if (inChar == '\n') {
            string_complete = true;
        }
    }
}

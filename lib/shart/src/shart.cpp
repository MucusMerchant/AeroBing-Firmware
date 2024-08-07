#include "shart.h"

Shart::Shart() {
}

// Initialize some stuff plus everything on shart
void Shart::init(uint32_t chipTimeOffset) {

  this->chipTimeOffset = chipTimeOffset;

  initPins();
  // dont do this if no USB connection
  initSerial();

  // initialize storage and transmission
  initSD();
  initRadio();

  // initialize sensors
  initICM20948();
  initBMP388();
  initADXL375();
  initGTU7();

}

void Shart::collect() {

  // Reset all data values between each read
  //memset(data.f, 0, NUM_DATA_POINTS * sizeof(float));

  // Get time since program start in ms
  collectTime();

  // Only collect data when sensors are marked as AVAILABLE
  updateStatusICM20948(); if (getStatusICM20948() == AVAILABLE) collectDataICM20948();
  updateStatusBMP388();   if (getStatusBMP388()   == AVAILABLE) collectDataBMP388();
  updateStatusADXL375();  if (getStatusADXL375()  == AVAILABLE) collectDataADXL375();
  collectDataGTU7();
  
  PRINT_DATARATE()

}

void Shart::send() {

  // Print data to usb serial debugging on
  PRINT_DATA(sensor_packet, gps_packet)

  // Generate checksums for each packet
  CHECKSUM(sensor_packet)
  CHECKSUM(gps_packet)

  // Write to flash, send to radio
  if (getStatusSD() != PERMANENTLY_UNAVAILABLE) saveData();
  transmitData(); // check radio stage
  // set gps_ready flag to false no matter what
  gps_ready = false;

}

// If unavailable, try to reconnect to lost components
void Shart::reconnect() {

  // reconnect storage and sensors
  if (getStatusBMP388()    == UNINITIALIZED) initBMP388();
  if (getStatusADXL375()   == UNINITIALIZED) initADXL375();
  if (getStatusICM20948()  == UNINITIALIZED) initICM20948();
  
}

// for slow reinitialization functions that are thread-safe (cannot be sharing a bus)
// if storing/transmitting status in a shared array, this could cause issues
void Shart::threadedReconnect() {

  if (getStatusSD() == UNAVAILABLE) initSD();
  
}

// Check if all components marked available
bool Shart::getSystemStatus() {

	return (
    getStatusBMP388()   == AVAILABLE &&
		getStatusICM20948() == AVAILABLE &&
		getStatusADXL375()  == AVAILABLE &&
		getStatusSD()       == AVAILABLE);
    
}

// Initializes Teensy 4.1 pins
void Shart::initPins() {

  // Enable onboard Teensy 4.1 LED and turn it on!
  // If the Teensy is on, this LED will be on.
  pinMode(ONBOARD_LED_PIN, OUTPUT);
  digitalWrite(ONBOARD_LED_PIN, HIGH);

  // set chip select pins to low
  digitalWrite(ICM_CS, LOW);
  digitalWrite(BMP_CS, LOW);
  digitalWrite(ADXL_CS, LOW);
  delay(5);

}

// Initializes serial monitors with the specified baud rate
void Shart::initSerial() {

  // Begin the serial communication
  Serial.begin(USB_SERIAL_BAUD_RATE);

  // Delay 200ms
  delay(200);

}

// get current chip time in milliseconds
void Shart::collectTime() {

  // (we don't need microsecond precision, we are only collecting at < 1kHz)
  sensor_packet.data.ms = micros() - chipTimeOffset;
  gps_packet.data.ms    = micros() - chipTimeOffset;

}
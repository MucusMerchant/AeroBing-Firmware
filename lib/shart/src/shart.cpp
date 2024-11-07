#include "shart.h"

Shart::Shart() {
  // meow
}

// Initialize some stuff plus everything on shart
void Shart::init() {

  this->chipTimeOffset = micros();

  // initialize pins, storage, and transmission
  initPins();
  initSerial();
  initSD();
  
  // initialize sensors
  initICM20948();
  initLSM6DSO32();
  initBMP388();
  initADXL375();
  initGTU7();

  #ifndef START_ON_POWERUP
  awaitStart();
  #endif

}

// Wait until 5 0xBB bytes are received
void Shart::awaitStart() {
  while (1) {
    if (SERIAL_PORT.available() >= 5) {
      for (int i = 0; i < 5; i++) {
        if (SERIAL_PORT.read() != 0xBB) break;
        if (i == 4) return;
      }
    }
  }
}

void Shart::collect() {

  // Reset all data values between each read?
  //memset(data.f, 0, NUM_DATA_POINTS * sizeof(float));

  // Get time since program start in us
  collectTime();

  // Only collect data when sensors are marked as AVAILABLE
  updateStatusICM20948();  if (getStatusICM20948()  == AVAILABLE) collectDataICM20948();
  updateStatusBMP388();    if (getStatusBMP388()    == AVAILABLE) collectDataBMP388();
  updateStatusADXL375();   if (getStatusADXL375()   == AVAILABLE) collectDataADXL375();
  updateStatusLSM6DSO32(); if (getStatusLSM6DSO32() == AVAILABLE) collectDataLSM6DSO32();
  collectDataGTU7(); // GPS status doesn't matter here
  
  PRINT_DATARATE()

}

void Shart::send() {

  // Generate checksums for each packet
  CHECKSUM(sensor_packet)
  CHECKSUM(gps_packet)

  // Write to flash, send to radio
  if (getStatusSD() != PERMANENTLY_UNAVAILABLE) saveData();
  transmitData(); // check radio stage?
  // set gps_ready flag to false no matter what to make sure we don't send the same data twice
  gps_ready = false;

}

// If unavailable, try to reconnect to lost components
void Shart::reconnect() {

  #ifdef ATTEMPT_RECONNECT
  // reconnect storage and sensors
  if (getStatusBMP388()    == UNINITIALIZED) initBMP388();
  if (getStatusADXL375()   == UNINITIALIZED) initADXL375();
  if (getStatusICM20948()  == UNINITIALIZED) initICM20948();
  if (getStatusLSM6DSO32() == UNINITIALIZED) initLSM6DSO32();

  #endif
  
}

// for slow reinitialization functions that are thread-safe (cannot be sharing a bus)
// if storing/transmitting status in a shared array, this could cause issues
void Shart::threadedReconnect() {

  if (getStatusSD() == UNAVAILABLE) initSD();
  
}

// stuff to do when Shart wraps up
void Shart::maybeFinish() {
  if (SERIAL_PORT.available() >= 5) {
    for (int i = 0; i < 5; i++) {
      if (SERIAL_PORT.read() != 0xAA) return;
    }
    file.truncate();
    file.close();
    delay(1000);
    exit(0);
  }
  // meow
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

  // set SPI chip select pins to low
  digitalWrite(ICM_CS, LOW);
  digitalWrite(BMP_CS, LOW);
  digitalWrite(ADXL_CS, LOW);
  delay(5);

}

// get current chip time in milliseconds
void Shart::collectTime() {

  // populate packets with current time minus offset from start
  sensor_packet.data.us = micros() - chipTimeOffset;
  gps_packet.data.us    = micros() - chipTimeOffset;

}
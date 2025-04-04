#include "shart.h"

Shart::Shart() {
  // meow
}

// Initialize some stuff plus everything on shart
void Shart::init() {

  this->chipTimeOffset = micros();

  // initialize pins, storage, and transmission
  initSerial();
  initPins();
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

// Wait until we receive a start command packet
void Shart::awaitStart() {

  bool packet_received;

  for (;;) {

    RECEIVE_PACKET(command_packet, MAIN_SERIAL_PORT, packet_received)
    if (packet_received && command_packet.data.command == START_COMMAND) return; 

  }

}

void Shart::collect() {

  // Get time since program start in us
  collectTime();

  // Only collect data when sensors are marked as AVAILABLE
  updateStatusICM20948();  if (ICMStatus  == AVAILABLE) collectDataICM20948();
  updateStatusBMP388();    if (BMPStatus  == AVAILABLE) collectDataBMP388();
  updateStatusADXL375();   if (ADXLStatus == AVAILABLE) collectDataADXL375();
  updateStatusLSM6DSO32(); if (LSMStatus  == AVAILABLE) collectDataLSM6DSO32();
  collectDataGTU7(); // GPS status doesn't matter here

  setStatusByte();

}

void Shart::send() {

  // Generate checksums for each packet
  CHECKSUM(sensor_packet)
  CHECKSUM(gps_packet)

  // Write to flash, send to radio
  if (SDStatus != PERMANENTLY_UNAVAILABLE) saveData();
  transmitData(); // check radio status?
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

  if (SDStatus == UNAVAILABLE) initSD();
  
}

// stuff to do when Shart wraps up
void Shart::maybeFinish() {

  bool packet_received;

  RECEIVE_PACKET(command_packet, MAIN_SERIAL_PORT, packet_received)

  if (packet_received && command_packet.data.command == STOP_COMMAND && SDStatus == AVAILABLE) {
    file.truncate();
    file.close();
    sd.end();
    initSD();
    //pinMode(ONBOARD_LED_PIN, OUTPUT);
    //digitalWrite(ONBOARD_LED_PIN, LOW); // turn off Teensy light
    //delay(1000);
    //exit(0);
  }
  // meow
}

// Check if all components marked available
bool Shart::getSystemStatus() {

	return (
    BMPStatus  == AVAILABLE &&
		ICMStatus  == AVAILABLE &&
		ADXLStatus == AVAILABLE &&
		SDStatus   == AVAILABLE);
    
}

// fill the status byte in the sensor packet. each bit set to 1 if component good, otherwise 0
void Shart::setStatusByte() {
  sensor_packet.data.status = 0;
  sensor_packet.data.status |= (ICMStatus == AVAILABLE)  << ICM_STATUS_OFFSET;
  sensor_packet.data.status |= (BMPStatus == AVAILABLE)  << BMP_STATUS_OFFSET;
  sensor_packet.data.status |= (ADXLStatus == AVAILABLE) << ADXL_STATUS_OFFSET;
  sensor_packet.data.status |= (LSMStatus == AVAILABLE)  << LSM_STATUS_OFFSET;
  sensor_packet.data.status |= (SDStatus == AVAILABLE)   << SD_STATUS_OFFSET;
  sensor_packet.data.status |= (analogRead(41) > 712) << PYRO_STATUS_OFFSET;
  sensor_packet.data.reserved = sd_file_opened;
}

// Initializes Teensy 4.1 pins
void Shart::initPins() {

  // Enable onboard Teensy 4.1 LED and turn it on!
  // If the Teensy is on, this LED will be on.
  pinMode(ONBOARD_LED_PIN, OUTPUT);
  pinMode(41, INPUT);
  // pinMode(ICM_CS, OUTPUT);
  // pinMode(BMP_CS, OUTPUT);
  // pinMode(ADXL_CS, OUTPUT);
  digitalWrite(ONBOARD_LED_PIN, HIGH);

  // set SPI chip select pins to high to prevent unexpected behavior
  // commented out because we have pull-up resistors
  // digitalWrite(ICM_CS, HIGH);
  // digitalWrite(BMP_CS, HIGH);
  // digitalWrite(ADXL_CS, HIGH);
  delay(5);

}

// get current chip time in milliseconds
void Shart::collectTime() {

  // populate packets with current time minus offset from start
  sensor_packet.data.us = micros() - chipTimeOffset;
  gps_packet.data.us    = micros() - chipTimeOffset;

}
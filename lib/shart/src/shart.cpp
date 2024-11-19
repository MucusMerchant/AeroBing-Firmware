#include "shart.h"

Shart::Shart() {
  // meow
}

// Initialize some stuff plus everything on shart
void Shart::init() {

  #ifndef START_ON_POWERUP
  awaitStart();
  #endif

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

}

// Wait until we receive a start command packet
void Shart::awaitStart() {

  command_p command_packet;
  bool packet_received;

  for (;;) {
    #if MAIN_SERIAL_PORT == USB_SERIAL_PORT
    packet_received = receivePacketType<usb_serial_class, command_p>(&command_packet, &MAIN_SERIAL_PORT);
    #else
    packet_received = receivePacketType<HardwareSerial, command_p>(&command_packet, &MAIN_SERIAL_PORT);
    #endif
    if (packet_received)
    while (1)
    //MAIN_SERIAL_PORT.printf("%d\n", *(int*) (&command_packet + 4));//command_packet.data.command);
    //MAIN_SERIAL_PORT.printf("%d\n", *(int*) (&command_packet + 4));
    //MAIN_SERIAL_PORT.printf("%d\n", *(int*) (&command_packet.data.command));
    //MAIN_SERIAL_PORT.printf("%d\n", command_packet.data.command);
    // what the fuck is going on here i dont know
    if (packet_received && *(int*) (&command_packet.data.command) == START_COMMAND) return; 
  }

}

void Shart::collect() {

  // Reset all data values between each read?
  //memset(data.f, 0, NUM_DATA_POINTS * sizeof(float));

  // Get time since program start in us
  collectTime();

  // Only collect data when sensors are marked as AVAILABLE
  updateStatusICM20948();  if (ICMStatus  == AVAILABLE) collectDataICM20948();
  updateStatusBMP388();    if (BMPStatus  == AVAILABLE) collectDataBMP388();
  updateStatusADXL375();   if (ADXLStatus == AVAILABLE) collectDataADXL375();
  updateStatusLSM6DSO32(); if (LSMStatus  == AVAILABLE) collectDataLSM6DSO32();
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

  command_p command_packet;
  bool packet_received;

  #if MAIN_SERIAL_PORT == USB_SERIAL_PORT
  packet_received = receivePacketType<usb_serial_class, command_p>(&command_packet, &MAIN_SERIAL_PORT);
  #else
  packet_received = receivePacketType<HardwareSerial, command_p>(&command_packet, &MAIN_SERIAL_PORT);
  #endif
  if (packet_received && *(int*) (&command_packet.data.command) == STOP_COMMAND) {
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
    BMPStatus  == AVAILABLE &&
		ICMStatus  == AVAILABLE &&
		ADXLStatus == AVAILABLE &&
		SDStatus   == AVAILABLE);
    
}

// Initializes Teensy 4.1 pins
void Shart::initPins() {

  // Enable onboard Teensy 4.1 LED and turn it on!
  // If the Teensy is on, this LED will be on.
  pinMode(ONBOARD_LED_PIN, OUTPUT);
  digitalWrite(ONBOARD_LED_PIN, HIGH);

  // set SPI chip select pins to high to prevent unexpected behavior
  digitalWrite(ICM_CS, HIGH);
  digitalWrite(BMP_CS, HIGH);
  digitalWrite(ADXL_CS, HIGH);
  delay(5);

}

// get current chip time in milliseconds
void Shart::collectTime() {

  // populate packets with current time minus offset from start
  sensor_packet.data.us = micros() - chipTimeOffset;
  gps_packet.data.us    = micros() - chipTimeOffset;

}
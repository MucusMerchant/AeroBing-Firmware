/*******************************************************************************
* File Name: sensors.cpp
*
* Description:
*   Implementation for all sensor-specific methods
*
* Author: Andrew Shen-Costello
*
* Spring, 2024
* TODO: Initialization for faster chips: don't want to consume more power than
* necessary, i.e. set them to slower data rate if possible
*
* TODO: Consider switch to LSM6DSO32 (6DoF) + a magnetometer instead of a 9DoF chip
* LSM6DSO32 must be set to high-performance mode
* Another option: LSM9DS1 9DoF, mag only 100Hz, separate SPI chip select pins for
* acc/gyr, mag
* RM3100 magnetometer? kinda sketch but ppl us it
* bought this shit asap $60 baby EMPTY WALLET WOOO, we still need a got 6DoF
* 
*
* Version:  1.0.0
*
*******************************************************************************/

#include "shart.h"

/*******************************************************************************
* Initializers
*
*   Initialize and configure sensors
*
*******************************************************************************/

void Shart::initICM20948() {
  
  if (!icm.init()) {
    UPDATE_STATUS(ICMStatus, UNINITIALIZED)
    ERROR("ICM initialization failed!")
    return;
  }
  UPDATE_STATUS(ICMStatus, AVAILABLE)

}

// Initialize the BMP SPI connection
// TODO: consider power cycling to avoid issues with clock synchronization
void Shart::initBMP388() {

  // Initialize in software SPI mode
  //delay(100);
  //if (!bmp.begin_SPI(BMP_CS, BMP_SCK, BMP_MISO, BMP_MOSI)) {
  if (!bmp.begin_SPI(BMP_CS, &BMP_SPI_BUS)) {
    UPDATE_STATUS(BMPStatus, UNINITIALIZED)
    ERROR("BMP initialization failed!")
    return;
  }

  // Turn off oversampling, instead just take data at 200Hz (we can process noise later)
  //bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  //bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_200_HZ);

  UPDATE_STATUS(BMPStatus, AVAILABLE)

}

// ADXL375 range is fixed at +/-200G
// TODO: maybe try power cycling here: clock synchonization gets messed up sometimes if SCL gets unplugged
void Shart::initADXL375() {

  if (!adxl.begin()) {
    UPDATE_STATUS(ADXLStatus, UNINITIALIZED);
    ERROR("ADXL initialization failed!")
    return;
  }
  UPDATE_STATUS(ADXLStatus, AVAILABLE)

}


/*******************************************************************************
* Status checkers
*
*   Before collecting data, check and update the status of sensors. Depending on
*   the determined status, init or collect functions may be flagged.
*
*******************************************************************************/

void Shart::updateStatusBMP388() {

  // The chipID() function has been modified to ACTUALLY read the chip_id register
  if (bmp.chipID() != BMP_CHIP_ID) {
    UPDATE_STATUS(BMPStatus, UNAVAILABLE)
    ERROR("BMP not found!")
    return;
  }

  UPDATE_STATUS(BMPStatus, AVAILABLE);
}

// See if icm is connected (under the covers, checks device id)
void Shart::updateStatusICM20948() {
  
  if (!icm.connected()) {
    UPDATE_STATUS(ICMStatus, UNINITIALIZED)
    ERROR("ICM reading failed!")
    return;
  }

  UPDATE_STATUS(ICMStatus, AVAILABLE)
  
}

// Check if device id matches expected
void Shart::updateStatusADXL375() {

  if (adxl.getDeviceID() != ADXL_CHIP_ID) {
    UPDATE_STATUS(ADXLStatus, UNAVAILABLE);
    ERROR("ADXL not found!")
    return;
  }

  UPDATE_STATUS(ADXLStatus, AVAILABLE)

}

/*******************************************************************************
* Collectors
*
*   This is the meat of the project, but really the simplest part. Most of this
*   code is copied from somewhere else or taken from a library.
*   The increments for index calculations were a choice, it was very inconvenient
*   to change every entry every time something needed to be changed. Very minimal
*   performance tradeoff.
*
*******************************************************************************/

// collect data from the ADXL375, 49mG per LSB so multiply by 49/1000 = 0.049 for units in G
void Shart::collectDataADXL375() {

  // Collect raw data from axis registers
  int16_t x, y, z;
  adxl.getXYZ(x, y, z);
  
  sensor_packet.data.adxl_acc_x = x;
  sensor_packet.data.adxl_acc_y = y;
  sensor_packet.data.adxl_acc_z = z;

}

// collect data from the ICM w/ modified ZaneL's library
void Shart::collectDataICM20948() {

  float gyro_x, gyro_y, gyro_z;
  float accel_x, accel_y, accel_z;
  float mag_x, mag_y, mag_z;

  icm.task();
  icm.readGyroData(&gyro_x, &gyro_y, &gyro_z);
  icm.readAccelData(&accel_x, &accel_y, &accel_z);
  icm.readMagData(&mag_x, &mag_y, &mag_z);
 
  sensor_packet.data.acc_x = accel_x;
  sensor_packet.data.acc_y = accel_y;
  sensor_packet.data.acc_z = accel_z;
  sensor_packet.data.gyr_x = gyro_x;
  sensor_packet.data.gyr_y = gyro_y;
  sensor_packet.data.gyr_z = gyro_z;
  sensor_packet.data.mag_x = mag_x;
  sensor_packet.data.mag_y = mag_y;
  sensor_packet.data.mag_z = mag_z;
  
}

// collect data from the BMP388 over SPI
// VERY IMPORTANT: "the_sensor.settings.op_mode = BMP3_MODE_NORMAL" in begin_SPI in Adafruit_BMP3XX.cpp or else very slow
void Shart::collectDataBMP388() {

  // take temperature and pressure, ignore altitude estimate to avoid expensive calculations
  // library is still doing lots of compensation; maybe in the future do this in PP
  bmp.performReading();
  sensor_packet.data.temp = bmp.temperature; // in *C
  sensor_packet.data.pres = bmp.pressure; // in Pa

}

// Status getters
Status Shart::getStatusICM20948() { return ICMStatus; }

Status Shart::getStatusBMP388() { return BMPStatus; }

Status Shart::getStatusADXL375() { return ADXLStatus; }
/* 

CHANGES TO SENSOR LIBRARIES: these may need updates if new chips/libraries are used

*** In bmp3.h ***
int8_t bmp3_read_chip_id(struct bmp3_dev *dev);

*** In bmp3.c ***
int8_t bmp3_read_chip_id(struct bmp3_dev *dev)
{
    int8_t rslt;
    uint8_t chip_id = 0;

    // Check for null pointer in the device structure
    rslt = null_ptr_check(dev);

    // Proceed if null check is fine 
    if (rslt == BMP3_OK)
    {
        // Read the chip-id of bmp3 sensor 
        bmp3_get_regs(BMP3_REG_CHIP_ID, &chip_id, 1, dev);
    }

    return chip_id;
}

*** In Adafruit_BMP3XX.cpp ***
uint8_t Adafruit_BMP3XX::chipID(void) { return bmp3_read_chip_id(&the_sensor); }

*** Respective declarations in Adafruit_BMP3XX.h ***

*/

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
* Issues:
*   
*   Circuitry issue: when you unplug BMP gnd, teensy shorts(?) and the program dies
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

void Shart::initBNO055() {


  if (icm.begin(ICM_CS, ICM_SPI_BUS) != ICM_20948_Stat_Ok) {
    UPDATE_STATUS(BNOStatus, UNINITIALIZED)
    ERROR("BNO initialization failed!")
    return;
  }
  
  UPDATE_STATUS(BNOStatus, AVAILABLE)

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
// might work: only call adxl.begin if first initialization or just powered off, maybe this is 'uninitialized'
// need way to check for a power cycle: see if a certain register is zero.
// weird behavior. calibration sometimes does not load, sometimes you enter extremely noisy mode
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

void Shart::updateStatusBNO055() {
  // idk if this works bruh
  if (icm.status != ICM_20948_Stat_Ok) {
    UPDATE_STATUS(BNOStatus, UNINITIALIZED)
    ERROR("BNO reading failed!")
    return;
  }

  UPDATE_STATUS(BNOStatus, AVAILABLE)
  
}

// logic: first check if chipID found. if not, still unavailable. once found, move to collect, check if initialized
//you can't initialize something that isn't connected!
// check chipID -> return unavailable or check initialized -> return available or uninitialized
// in shart.cpp collect data: call all reconnects to update statussies before collecting, this can always be synchronous
// the reinit functions are called separately in main (reconnect)
void Shart::updateStatusADXL375() {

  if (adxl.getDeviceID() != ADXL_CHIP_ID) {
    UPDATE_STATUS(ADXLStatus, UNAVAILABLE);
    ERROR("ADXL not found!")
    return;
  }
  // potentially slow, commented out for now
  /*
  // read power_ctl register
  int8_t ctl = adxl.getPowerControl();
  
  int8_t xc,yc,zc;
  adxl.getTrimOffsets(&xc, &yc, &zc);
  
  // If bit 3 of pwr_ctl register is not set, sensor is not collecting data, we need to reinitialize
  if (!(ctl==0x8)) {
    UPDATE_STATUS(ADXLStatus, UNINITIALIZED)
    ERROR("ADXL not initialized!")
    return;
  }
  */
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

// collect data from the BNO055 over I2C
void Shart::collectDataBNO055() {

  // Sensor events defined by the Adafruit BNO library
  sensor_packet.data.acc_x = icm.accX();
  sensor_packet.data.acc_y = icm.accY();
  sensor_packet.data.acc_z = icm.accZ();
  sensor_packet.data.gyr_x = icm.gyrX();
  sensor_packet.data.gyr_y = icm.gyrY();
  sensor_packet.data.gyr_z = icm.gyrZ();
  sensor_packet.data.mag_x = icm.magX();
  sensor_packet.data.mag_y = icm.magY();
  sensor_packet.data.mag_z = icm.magZ();
  
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
Status Shart::getStatusBNO055() { return BNOStatus; }

Status Shart::getStatusBMP388() { return BMPStatus; }

Status Shart::getStatusADXL375() { return ADXLStatus; }
/* 

CHANGES TO SENSOR LIBRARIES: these may need updates if new chips/libraries are used

*** In bmp3.h ***
int8_t bmp3_read_chip_id(struct bmp3_dev *dev);
int8_t bmp3_read_sensor_time(struct bmp3_dev *dev);

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
// read BMP power ctl register at 0x1B
int8_t bmp3_read_sensor_time(struct bmp3_dev *dev) {
    int8_t rslt;
    uint8_t ctl = 0;

    rslt = null_ptr_check(dev);

    if (rslt == BMP3_OK)
    {
        bmp3_get_regs(0x0C, &ctl, 1, dev);
    }

    return ctl;
}
*** In Adafruit_BMP3XX.cpp ***
uint8_t Adafruit_BMP3XX::chipID(void) { return bmp3_read_chip_id(&the_sensor); }
uint8_t Adafruit_BMP3XX::sensorTime(void) { return bmp3_read_sensor_time(&the_sensor); }

*** Respective declarations in Adafruit_BMP3XX.h ***

*/

/*
*** in Adafruit_ADXL375.cpp ***
  int8_t Adafruit_ADXL375::getPowerControl(void) {
      return readRegister(ADXL3XX_REG_POWER_CTL);
  }
  
*/
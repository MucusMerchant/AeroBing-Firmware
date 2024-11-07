#ifndef __TEENSY_ICM_20948_H__
#define __TEENSY_ICM_20948_H__

#include <SPI.h>

// InvenSense drivers and utils
#include "Icm20948.h"
#include "SensorTypes.h"
#include "Icm20948MPUFifoControl.h"

#define SPI_DEV SPI1
#define NO_PRINT

/*************************************************************************
  Defines
*************************************************************************/
// default values are specific to our current implementation.
typedef struct {
  int cs_pin                  = 0;
  int spi_speed               = 1000000;
  SPIClass *spi_bus           = &SPI1;
  int mode                    = 1;
  bool enable_gyroscope       = true;
  bool enable_accelerometer   = true;
  bool enable_magnetometer    = true;
  bool enable_quaternion      = false;
  int gyroscope_frequency     = 225;
  int accelerometer_frequency = 225;
  int magnetometer_frequency  = 225;
  int quaternion_frequency    = 225;

} TeensyICM20948Settings;

/*************************************************************************
  Class
*************************************************************************/

class TeensyICM20948
{
  public:

    TeensyICM20948(TeensyICM20948Settings settings);
    TeensyICM20948(int cs_pin, SPIClass *spi_bus);

    bool init();
    void task();
    bool connected();

    bool gyroDataIsReady();
    bool accelDataIsReady();
    bool magDataIsReady();
    bool quatDataIsReady();

    void readGyroData(float *x, float *y, float *z);
    void readAccelData(float *x, float *y, float *z);
    void readMagData(float *x, float *y, float *z);
    void readQuatData(float *w, float *x, float *y, float *z);
    
    friend inline void build_sensor_event_data(void * context, enum inv_icm20948_sensor sensortype, uint64_t timestamp, const void * data, const void *arg);
    friend inline int idd_io_hal_read_reg(void * context, uint8_t reg, uint8_t * rbuffer, uint32_t rlen);
    friend inline int idd_io_hal_write_reg(void * context, uint8_t reg, const uint8_t * wbuffer, uint32_t wlen);
  
  private:
    inv_icm20948_t icm_device;
    TeensyICM20948Settings settings;
    //int chipSelectPin = 10;
    //int spiSpeed = 4000000;
    int32_t cfg_acc_fsr = 16; // Default = +/- 4g. Valid ranges: 2, 4, 8, 16
    int32_t cfg_gyr_fsr = 2000; // Default = +/- 2000dps. Valid ranges: 250, 500, 1000, 2000

    int rc = 0;

    float gyro_x, gyro_y, gyro_z;
    float accel_x, accel_y, accel_z;
    float mag_x, mag_y, mag_z;
    float quat_w, quat_x, quat_y, quat_z;

    bool accel_data_ready = false;
    bool gyro_data_ready = false;
    bool mag_data_ready = false;
    bool quat_data_ready = false;

    void check_rc(int rc, const char * msg_context);
    int load_dmp3(void);
    inv_bool_t interface_is_SPI(void);
    void icm20948_apply_mounting_matrix(void);
    void icm20948_set_fsr(void);
    int icm20948_sensor_setup(void);

    
    
};


#endif // __TEENSY_ICM_20948_H__
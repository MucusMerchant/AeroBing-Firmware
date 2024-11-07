/*************************************************************************
  Includes
*************************************************************************/

#include "Teensy-ICM-20948.h"

/*************************************************************************
  HAL Functions for Arduino
*************************************************************************/

inline int idd_io_hal_read_reg(void * context, uint8_t reg, uint8_t * rbuffer, uint32_t rlen)
{
  TeensyICM20948Settings *spi_config = (TeensyICM20948Settings *) context;
  SPIClass *bus = spi_config->spi_bus;
  int cs        = spi_config->cs_pin;
  int speed     = spi_config->spi_speed;
  
  bus->beginTransaction(SPISettings(speed, MSBFIRST, SPI_MODE0));
  bus->transfer(0x00);
  bus->endTransaction();

  digitalWrite(cs, LOW);
  bus->beginTransaction(SPISettings(speed, MSBFIRST, SPI_MODE0));
  bus->transfer(((reg & 0x7F) | 0x80));
  for (uint32_t indi = 0; indi < rlen; indi++)
  {
    *(rbuffer + indi) = bus->transfer(0x00);
  }
  bus->endTransaction();
  digitalWrite(cs, HIGH);

  return 0;
}

inline int idd_io_hal_write_reg(void * context, uint8_t reg, const uint8_t * wbuffer, uint32_t wlen)
{
  TeensyICM20948Settings *spi_config = (TeensyICM20948Settings *) context;
  SPIClass *bus = spi_config->spi_bus;
  int cs        = spi_config->cs_pin;
  int speed     = spi_config->spi_speed;

  bus->beginTransaction(SPISettings(speed, MSBFIRST, SPI_MODE0));
  bus->transfer(0x00);
  bus->endTransaction();

  digitalWrite(cs, LOW);
  bus->beginTransaction(SPISettings(speed, MSBFIRST, SPI_MODE0));
  bus->transfer((reg & 0x7F) | 0x00);
  for (uint32_t indi = 0; indi < wlen; indi++)
  {
    bus->transfer(*(wbuffer + indi));
  }
  bus->endTransaction();
  digitalWrite(cs, HIGH);

  return 0;
}

/*************************************************************************
  Invensense Variables
*************************************************************************/
// fix this shit

#define EXPECTED_WHOAMI 0xEA
#define AK0991x_DEFAULT_I2C_ADDR  0x0C
#define THREE_AXES 3
static int unscaled_bias[THREE_AXES * 2];

static const float cfg_mounting_matrix[9] = {
  1.f, 0, 0,
  0, 1.f, 0,
  0, 0, 1.f
};

static const uint8_t dmp3_image[] = {
#include "icm20948_img.dmp3a.h"
};

/*************************************************************************
  Invensense Functions
*************************************************************************/

void TeensyICM20948::check_rc(int rc, const char * msg_context) 
{
  if (rc < 0) {
    #ifndef NO_PRINT
    Serial.println("ICM20948 ERROR!");
    #endif
    while (1);
  }
}

int TeensyICM20948::load_dmp3(void) 
{
  int rc = 0;
  rc = inv_icm20948_load(&icm_device, dmp3_image, sizeof(dmp3_image));
  return rc;
}

static void inv_icm20948_sleep_us(int us) 
{
  delayMicroseconds(us);
}

static void inv_icm20948_sleep(int ms) 
{
  delay(ms);
}

static uint64_t inv_icm20948_get_time_us(void) 
{
  return micros();
}

inv_bool_t TeensyICM20948::interface_is_SPI(void)
{
  return true;
}

void TeensyICM20948::icm20948_apply_mounting_matrix(void) 
{
  int ii;

  for (ii = 0; ii < INV_ICM20948_SENSOR_MAX; ii++) {
    inv_icm20948_set_matrix(&icm_device, cfg_mounting_matrix, (inv_icm20948_sensor)ii);
  }
}

void TeensyICM20948::icm20948_set_fsr(void) 
{
  inv_icm20948_set_fsr(&icm_device, INV_ICM20948_SENSOR_RAW_ACCELEROMETER, (const void *)&cfg_acc_fsr);
  inv_icm20948_set_fsr(&icm_device, INV_ICM20948_SENSOR_ACCELEROMETER, (const void *)&cfg_acc_fsr);
  inv_icm20948_set_fsr(&icm_device, INV_ICM20948_SENSOR_RAW_GYROSCOPE, (const void *)&cfg_gyr_fsr);
  inv_icm20948_set_fsr(&icm_device, INV_ICM20948_SENSOR_GYROSCOPE, (const void *)&cfg_gyr_fsr);
  inv_icm20948_set_fsr(&icm_device, INV_ICM20948_SENSOR_GYROSCOPE_UNCALIBRATED, (const void *)&cfg_gyr_fsr);
}

int TeensyICM20948::icm20948_sensor_setup(void)
{
  int rc;
  uint8_t whoami = 0xff;

  inv_icm20948_soft_reset(&icm_device);

  // Get whoami number
  rc = inv_icm20948_get_whoami(&icm_device, &whoami);

  if (whoami != EXPECTED_WHOAMI) {
    #ifndef NO_PRINT
    Serial.print("Bad WHOAMI value = 0x");
    Serial.println(whoami, HEX);
    #endif
    return rc;
  }

  // Setup accel and gyro mounting matrix and associated angle for current board
  inv_icm20948_init_matrix(&icm_device);

  // set default power mode
  rc = inv_icm20948_initialize(&icm_device, dmp3_image, sizeof(dmp3_image));
  if (rc != 0) {
    #ifndef NO_PRINT
    Serial.println("Icm20948 Initialization failed.");
    #endif
    return rc;
  }

  // Configure and initialize the ICM20948 for normal use

  // Initialize auxiliary sensors
  inv_icm20948_register_aux_compass( &icm_device, INV_ICM20948_COMPASS_ID_AK09916, AK0991x_DEFAULT_I2C_ADDR);
  rc = inv_icm20948_initialize_auxiliary(&icm_device);
  if (rc == -1) {
    #ifndef NO_PRINT
    Serial.println("Compass not detected...");
    #endif
  }

  icm20948_apply_mounting_matrix();

  icm20948_set_fsr();

  // re-initialize base state structure
  inv_icm20948_init_structure(&icm_device);

  return 0;
}

bool TeensyICM20948::connected() {
  
  uint8_t whoami;
  inv_icm20948_get_whoami(&icm_device, &whoami);

  return whoami == EXPECTED_WHOAMI;

}

static uint8_t icm20948_get_grv_accuracy(void) 
{
  uint8_t accel_accuracy;
  uint8_t gyro_accuracy;

  accel_accuracy = (uint8_t)inv_icm20948_get_accel_accuracy();
  gyro_accuracy = (uint8_t)inv_icm20948_get_gyro_accuracy();
  return (min(accel_accuracy, gyro_accuracy));
}

static uint8_t convert_to_generic_ids[INV_ICM20948_SENSOR_MAX] = {
  INV_SENSOR_TYPE_ACCELEROMETER,
  INV_SENSOR_TYPE_GYROSCOPE,
  INV_SENSOR_TYPE_RAW_ACCELEROMETER,
  INV_SENSOR_TYPE_RAW_GYROSCOPE,
  INV_SENSOR_TYPE_UNCAL_MAGNETOMETER,
  INV_SENSOR_TYPE_UNCAL_GYROSCOPE,
  INV_SENSOR_TYPE_BAC,
  INV_SENSOR_TYPE_STEP_DETECTOR,
  INV_SENSOR_TYPE_STEP_COUNTER,
  INV_SENSOR_TYPE_GAME_ROTATION_VECTOR,
  INV_SENSOR_TYPE_ROTATION_VECTOR,
  INV_SENSOR_TYPE_GEOMAG_ROTATION_VECTOR,
  INV_SENSOR_TYPE_MAGNETOMETER,
  INV_SENSOR_TYPE_SMD,
  INV_SENSOR_TYPE_PICK_UP_GESTURE,
  INV_SENSOR_TYPE_TILT_DETECTOR,
  INV_SENSOR_TYPE_GRAVITY,
  INV_SENSOR_TYPE_LINEAR_ACCELERATION,
  INV_SENSOR_TYPE_ORIENTATION,
  INV_SENSOR_TYPE_B2S
};

inline void build_sensor_event_data(void * context, enum inv_icm20948_sensor sensortype, uint64_t timestamp, const void * data, const void *arg) 
{
  float raw_bias_data[6];
  inv_sensor_event_t event;
  TeensyICM20948 *sensor = (TeensyICM20948 *) context;
  uint8_t sensor_id = convert_to_generic_ids[sensortype];

  memset((void *)&event, 0, sizeof(event));
  event.sensor = sensor_id;
  event.timestamp = timestamp;
  switch (sensor_id) 
  {
    case INV_SENSOR_TYPE_UNCAL_GYROSCOPE:
      memcpy(raw_bias_data, data, sizeof(raw_bias_data));
      memcpy(event.data.gyr.vect, &raw_bias_data[0], sizeof(event.data.gyr.vect));
      memcpy(event.data.gyr.bias, &raw_bias_data[3], sizeof(event.data.gyr.bias));
      memcpy(&(event.data.gyr.accuracy_flag), arg, sizeof(event.data.gyr.accuracy_flag));
      break;
    case INV_SENSOR_TYPE_UNCAL_MAGNETOMETER:
      memcpy(raw_bias_data, data, sizeof(raw_bias_data));
      memcpy(event.data.mag.vect, &raw_bias_data[0], sizeof(event.data.mag.vect));
      memcpy(event.data.mag.bias, &raw_bias_data[3], sizeof(event.data.mag.bias));
      memcpy(&(event.data.gyr.accuracy_flag), arg, sizeof(event.data.gyr.accuracy_flag));
      break;
    case INV_SENSOR_TYPE_GYROSCOPE:
      memcpy(event.data.gyr.vect, data, sizeof(event.data.gyr.vect));
      memcpy(&(event.data.gyr.accuracy_flag), arg, sizeof(event.data.gyr.accuracy_flag));

      // WE WANT THIS
      sensor->gyro_x = event.data.gyr.vect[0];
      sensor->gyro_y = event.data.gyr.vect[1];
      sensor->gyro_z = event.data.gyr.vect[2];
      sensor->gyro_data_ready = true;
      break;

    case INV_SENSOR_TYPE_GRAVITY:
      memcpy(event.data.acc.vect, data, sizeof(event.data.acc.vect));
      event.data.acc.accuracy_flag = inv_icm20948_get_accel_accuracy();
      break;
    case INV_SENSOR_TYPE_LINEAR_ACCELERATION:
    case INV_SENSOR_TYPE_ACCELEROMETER:
      memcpy(event.data.acc.vect, data, sizeof(event.data.acc.vect));
      memcpy(&(event.data.acc.accuracy_flag), arg, sizeof(event.data.acc.accuracy_flag));

      // WE WANT THIS
      sensor->accel_x = event.data.acc.vect[0];
      sensor->accel_y = event.data.acc.vect[1];
      sensor->accel_z = event.data.acc.vect[2];
      sensor->accel_data_ready = true;
      break;

    case INV_SENSOR_TYPE_MAGNETOMETER:
      memcpy(event.data.mag.vect, data, sizeof(event.data.mag.vect));
      memcpy(&(event.data.mag.accuracy_flag), arg, sizeof(event.data.mag.accuracy_flag));

      // WE WANT THIS
      sensor->mag_x = event.data.mag.vect[0];
      sensor->mag_y = event.data.mag.vect[1];
      sensor->mag_z = event.data.mag.vect[2];
      sensor->mag_data_ready = true;
      break;

    case INV_SENSOR_TYPE_GEOMAG_ROTATION_VECTOR:
    case INV_SENSOR_TYPE_ROTATION_VECTOR:
      memcpy(&(event.data.quaternion.accuracy), arg, sizeof(event.data.quaternion.accuracy));
      memcpy(event.data.quaternion.quat, data, sizeof(event.data.quaternion.quat));
      break;
    case INV_SENSOR_TYPE_GAME_ROTATION_VECTOR:
      memcpy(event.data.quaternion.quat, data, sizeof(event.data.quaternion.quat));
      event.data.quaternion.accuracy_flag = icm20948_get_grv_accuracy();

      // WE WANT THIS
      sensor->quat_w = event.data.quaternion.quat[0];
      sensor->quat_x = event.data.quaternion.quat[1];
      sensor->quat_y = event.data.quaternion.quat[2];
      sensor->quat_z = event.data.quaternion.quat[3];
      sensor->quat_data_ready = true;
      break;

    case INV_SENSOR_TYPE_BAC:
      memcpy(&(event.data.bac.event), data, sizeof(event.data.bac.event));
      break;
    case INV_SENSOR_TYPE_PICK_UP_GESTURE:
    case INV_SENSOR_TYPE_TILT_DETECTOR:
    case INV_SENSOR_TYPE_STEP_DETECTOR:
    case INV_SENSOR_TYPE_SMD:
      event.data.event = true;
      break;
    case INV_SENSOR_TYPE_B2S:
      event.data.event = true;
      memcpy(&(event.data.b2s.direction), data, sizeof(event.data.b2s.direction));
      break;
    case INV_SENSOR_TYPE_STEP_COUNTER:
      memcpy(&(event.data.step.count), data, sizeof(event.data.step.count));
      break;
    case INV_SENSOR_TYPE_ORIENTATION:
      //we just want to copy x,y,z from orientation data
      memcpy(&(event.data.orientation), data, 3 * sizeof(float));
      break;
    case INV_SENSOR_TYPE_RAW_ACCELEROMETER:
    case INV_SENSOR_TYPE_RAW_GYROSCOPE:
      memcpy(event.data.raw3d.vect, data, sizeof(event.data.raw3d.vect));
      break;
    default:
      return;
  }
}

static enum inv_icm20948_sensor idd_sensortype_conversion(int sensor)
{
  switch (sensor)
  {
    case INV_SENSOR_TYPE_RAW_ACCELEROMETER:
      return INV_ICM20948_SENSOR_RAW_ACCELEROMETER;
    case INV_SENSOR_TYPE_RAW_GYROSCOPE:
      return INV_ICM20948_SENSOR_RAW_GYROSCOPE;
    case INV_SENSOR_TYPE_ACCELEROMETER:
      return INV_ICM20948_SENSOR_ACCELEROMETER;
    case INV_SENSOR_TYPE_GYROSCOPE:
      return INV_ICM20948_SENSOR_GYROSCOPE;
    case INV_SENSOR_TYPE_UNCAL_MAGNETOMETER:
      return INV_ICM20948_SENSOR_MAGNETIC_FIELD_UNCALIBRATED;
    case INV_SENSOR_TYPE_UNCAL_GYROSCOPE:
      return INV_ICM20948_SENSOR_GYROSCOPE_UNCALIBRATED;
    case INV_SENSOR_TYPE_BAC:
      return INV_ICM20948_SENSOR_ACTIVITY_CLASSIFICATON;
    case INV_SENSOR_TYPE_STEP_DETECTOR:
      return INV_ICM20948_SENSOR_STEP_DETECTOR;
    case INV_SENSOR_TYPE_STEP_COUNTER:
      return INV_ICM20948_SENSOR_STEP_COUNTER;
    case INV_SENSOR_TYPE_GAME_ROTATION_VECTOR:
      return INV_ICM20948_SENSOR_GAME_ROTATION_VECTOR;
    case INV_SENSOR_TYPE_ROTATION_VECTOR:
      return INV_ICM20948_SENSOR_ROTATION_VECTOR;
    case INV_SENSOR_TYPE_GEOMAG_ROTATION_VECTOR:
      return INV_ICM20948_SENSOR_GEOMAGNETIC_ROTATION_VECTOR;
    case INV_SENSOR_TYPE_MAGNETOMETER:
      return INV_ICM20948_SENSOR_GEOMAGNETIC_FIELD;
    case INV_SENSOR_TYPE_SMD:
      return INV_ICM20948_SENSOR_WAKEUP_SIGNIFICANT_MOTION;
    case INV_SENSOR_TYPE_PICK_UP_GESTURE:
      return INV_ICM20948_SENSOR_FLIP_PICKUP;
    case INV_SENSOR_TYPE_TILT_DETECTOR:
      return INV_ICM20948_SENSOR_WAKEUP_TILT_DETECTOR;
    case INV_SENSOR_TYPE_GRAVITY:
      return INV_ICM20948_SENSOR_GRAVITY;
    case INV_SENSOR_TYPE_LINEAR_ACCELERATION:
      return INV_ICM20948_SENSOR_LINEAR_ACCELERATION;
    case INV_SENSOR_TYPE_ORIENTATION:
      return INV_ICM20948_SENSOR_ORIENTATION;
    case INV_SENSOR_TYPE_B2S:
      return INV_ICM20948_SENSOR_B2S;
    default:
      return INV_ICM20948_SENSOR_MAX;
  }
}

/*************************************************************************
  Class Functions
*************************************************************************/

TeensyICM20948::TeensyICM20948(TeensyICM20948Settings settings)
{
  this->settings = settings;
}

// uxe defsault settings, only specify spi bus
TeensyICM20948::TeensyICM20948(int cs_pin, SPIClass *spi_bus) {
  settings.cs_pin = cs_pin;
  settings.spi_bus = spi_bus;
}

bool TeensyICM20948::init()
{
  // Setup SPI
  pinMode(settings.cs_pin, OUTPUT);
  digitalWrite(settings.cs_pin, HIGH);
  settings.spi_bus->begin();

  settings.spi_bus->beginTransaction(SPISettings(settings.spi_speed, MSBFIRST, SPI_MODE0));
  settings.spi_bus->transfer(0x00);
  settings.spi_bus->endTransaction();

  // Initialize icm20948 serif structure
  struct inv_icm20948_serif icm20948_serif;
  icm20948_serif.context   = &settings; // no need
  icm20948_serif.read_reg  = idd_io_hal_read_reg;
  icm20948_serif.write_reg = idd_io_hal_write_reg;
  icm20948_serif.max_read  = 1024 * 16; // maximum number of bytes allowed per serial read
  icm20948_serif.max_write = 1024 * 16; // maximum number of bytes allowed per serial write
  icm20948_serif.is_spi = interface_is_SPI();

  // Reset icm20948 driver states
  inv_icm20948_reset_states(&icm_device, &icm20948_serif);
  inv_icm20948_register_aux_compass(&icm_device, INV_ICM20948_COMPASS_ID_AK09916, AK0991x_DEFAULT_I2C_ADDR);

  // Setup the icm20948 device
  rc |= icm20948_sensor_setup();

  if (icm_device.selftest_done && !icm_device.offset_done)
  {
    // If we've run self test and not already set the offset.
    inv_icm20948_set_offset(&icm_device, unscaled_bias);
    icm_device.offset_done = 1;
  }

  // Now that Icm20948 device is initialized, we can proceed with DMP image loading
  // This step is mandatory as DMP image is not stored in non volatile memory
  rc |= load_dmp3();
  //check_rc(rc, "Error sensor_setup/DMP loading.");

  // Set mode
  inv_icm20948_set_lowpower_or_highperformance(&icm_device, settings.mode);

  // Set frequency
  rc |= inv_icm20948_set_sensor_period(&icm_device, idd_sensortype_conversion(INV_SENSOR_TYPE_GAME_ROTATION_VECTOR), 1000 / settings.quaternion_frequency);
  rc |= inv_icm20948_set_sensor_period(&icm_device, idd_sensortype_conversion(INV_SENSOR_TYPE_GYROSCOPE), 1000 / settings.gyroscope_frequency);
  rc |= inv_icm20948_set_sensor_period(&icm_device, idd_sensortype_conversion(INV_SENSOR_TYPE_ACCELEROMETER), 1000 / settings.accelerometer_frequency);
  rc |= inv_icm20948_set_sensor_period(&icm_device, idd_sensortype_conversion(INV_SENSOR_TYPE_MAGNETOMETER), 1000 / settings.magnetometer_frequency);

  // Enable / disable
  rc |= inv_icm20948_enable_sensor(&icm_device, idd_sensortype_conversion(INV_SENSOR_TYPE_GYROSCOPE), settings.enable_gyroscope);
  rc |= inv_icm20948_enable_sensor(&icm_device, idd_sensortype_conversion(INV_SENSOR_TYPE_ACCELEROMETER), settings.enable_accelerometer);
  rc |= inv_icm20948_enable_sensor(&icm_device, idd_sensortype_conversion(INV_SENSOR_TYPE_GAME_ROTATION_VECTOR), settings.enable_quaternion);
  rc |= inv_icm20948_enable_sensor(&icm_device, idd_sensortype_conversion(INV_SENSOR_TYPE_MAGNETOMETER), settings.enable_magnetometer);
  return !(bool)rc;
}

void TeensyICM20948::task()
{
  inv_icm20948_poll_sensor(&icm_device, this, build_sensor_event_data);
}

bool TeensyICM20948::gyroDataIsReady()
{
  return gyro_data_ready;
}

bool TeensyICM20948::accelDataIsReady()
{
  return accel_data_ready;
}

bool TeensyICM20948::magDataIsReady()
{
  return mag_data_ready;
}

bool TeensyICM20948::quatDataIsReady()
{
  return quat_data_ready;
}

void TeensyICM20948::readGyroData(float *x, float *y, float *z)
{
  *x = gyro_x;
  *y = gyro_y;
  *z = gyro_z;
  gyro_data_ready = false;
}

void TeensyICM20948::readAccelData(float *x, float *y, float *z)
{
  *x = accel_x;
  *y = accel_y;
  *z = accel_z;
  accel_data_ready = false;
}

void TeensyICM20948::readMagData(float *x, float *y, float *z)
{
  *x = mag_x;
  *y = mag_y;
  *z = mag_z;
  mag_data_ready = false;
}

void TeensyICM20948::readQuatData(float *w, float *x, float *y, float *z)
{
  *w = quat_w;
  *x = quat_x;
  *y = quat_y;
  *z = quat_z;
  quat_data_ready = false;
}

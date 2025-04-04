/*******************************************************************************
* File Name: export.cpp
*
* Description:
*   Provides local and external data storage/transmission capabilities. Writes
*   data to an onboard SD card and communicates to ground station via radio.
*
* Author: Andrew Shen-Costello
* Date: Spring, 2024
* TODO: integrated memory solution: we want a chip with 4-pin SPI I/O, around
*   4-8Gbits, and NAND flash architecture for fast WRITE
*   For efficient writes, use a large buffer
*   https://www.digikey.com/en/products/detail/alliance-memory-inc/AS5F34G04SNDA-08LIN/16704698
*   https://www.digikey.com/en/products/detail/alliance-memory-inc/AS5F38G04SND-08LIN/13282917
*       ^ this one has 8Gbit
*   https://protosupplies.com/product/w25n02g/ <- this is only 2Gbit, but that may be all we need
*
*
* Version:  1.0.0
*
*******************************************************************************/

#include "shart.h"

// Initialize the SD card
void Shart::initSD() {

  // Increment the number of initialization attempts
  sd_num_connection_attempts++;

  // Have we exceeded our attempts? If so, set status and early return
  if (sd_num_connection_attempts > SD_MAX_NUM_CONNECTION_ATTEMPTS) {
    UPDATE_STATUS(SDStatus, PERMANENTLY_UNAVAILABLE, MAIN_SERIAL_PORT)
    ERROR("SD card lost permanently!", MAIN_SERIAL_PORT)
    return;
  }

  if (!sd.begin(SD_CONFIG)) {
    UPDATE_STATUS(SDStatus, UNAVAILABLE, MAIN_SERIAL_PORT)
    ERROR("SD card initialization failed!", MAIN_SERIAL_PORT)
    return;
  }

  //sd.remove(LOG_FILENAME);
  // Open or create file - truncate existing file.
  String file_name = LOG_FILENAME;
  int counter = 1;
  while (sd.exists(file_name + String(counter) + ".poop")) {
    counter++;
  }
  sd_file_opened = counter;
  file_name += String(counter) + ".poop";
  if (!file.open(file_name.c_str(), O_WRONLY | O_CREAT | O_TRUNC)) {
    UPDATE_STATUS(SDStatus, UNAVAILABLE, MAIN_SERIAL_PORT)
    ERROR("Failed to open log file!", MAIN_SERIAL_PORT)
    return;
  }
  // File must be pre-allocated to avoid huge
  // delays searching for free clusters.
  if (!file.preAllocate(LOG_FILE_SIZE)) {
    UPDATE_STATUS(SDStatus, UNAVAILABLE, MAIN_SERIAL_PORT)
    ERROR("pre-allocate failed!", MAIN_SERIAL_PORT)
    file.close();
    return;
  }
  // initialize the RingBuf.
  sd_num_connection_attempts = 0;
  rb.begin(&file);
  UPDATE_STATUS(SDStatus, AVAILABLE, MAIN_SERIAL_PORT)
  return;

}

// Initializes serial bus with the specified baud rate
void Shart::initSerial() {

  #ifdef USB_SERIAL_MODE
    USB_SERIAL_PORT.begin(USB_SERIAL_BAUD_RATE);
    delay(200);
  #else
    initRadio();
  #endif

}

void Shart::initRadio() {

  // Initialize radio serial port
  RADIO_SERIAL_PORT.begin(RADIO_BAUD_RATE);
  RADIO_SERIAL_PORT.addMemoryForWrite(&bigassbuffer, sizeof(bigassbuffer));
  RADIO_SERIAL_PORT.setTimeout(RADIO_TIMEOUT_MS);

  return;

}

// Save data to SD card, this code copied from example in SDFat library
// this might be faster once QSPI is implemented w/ integrated memory.
void Shart::saveData() {

  size_t n = rb.bytesUsed();
  if ((n + file.curPosition()) > (LOG_FILE_SIZE - 20)) {
    UPDATE_STATUS(SDStatus, UNAVAILABLE, MAIN_SERIAL_PORT)
    ERROR("File full!", MAIN_SERIAL_PORT)
    return;
  }
  if (n >= 512 && !file.isBusy()) {
    // Not busy only allows one sector before possible busy wait.
    // Write one sector from RingBuf to file.
    if (512 != rb.writeOut(512)) {// || !file.sync()) {
      UPDATE_STATUS(SDStatus, UNAVAILABLE, MAIN_SERIAL_PORT)
      ERROR("Writeout failed!", MAIN_SERIAL_PORT)
      return;
    }
  }

  rb.write(reinterpret_cast<unsigned char *>(&sensor_packet), sizeof(sensor_p));
  if (gps_ready) rb.write(reinterpret_cast<unsigned char *>(&gps_packet), sizeof(gps_p));
  
  if (rb.getWriteError()) {
    // Error caused by too few free bytes in RingBuf.
    UPDATE_STATUS(SDStatus, UNAVAILABLE, MAIN_SERIAL_PORT)
    ERROR("Write error!", MAIN_SERIAL_PORT)
    return;
  }

}

// Transmit binary data via radio, beware of endian-ness. Network standard is big endian, but no point in converting twice
// TODO: packet should include a byte indicating the status of all sensors
void Shart::transmitData() {
  if (sensor_packet_counter % RADIO_SEND_EVERY_N == 0)
    MAIN_SERIAL_PORT.write(reinterpret_cast<unsigned char *>(&sensor_packet), sizeof(sensor_p));
  sensor_packet_counter++;
  if (gps_ready) MAIN_SERIAL_PORT.write(reinterpret_cast<unsigned char *>(&gps_packet), sizeof(gps_p));

}
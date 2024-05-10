/*******************************************************************************
* File Name: ShartLogger.ino
*
* Description:
*   There is not much to see here. Check out shart.h
*
* Author: Andrew Shen-Costello
* Date: Spring, 2024
*
*
* Version:  1.0.0
*
*******************************************************************************/
#include <Arduino.h>
#include <TeensyThreads.h> // Multithreading library specific to the Teensyduino
#include <shart.h> // The lovely shart library

// Global Shart singleton to be used throughout program execution
Shart shart; 

void reconnectLoop() {

  while (1) {
    shart.threadedReconnect();
  }

}

void setup() {

  // Initialize SHART
  shart.init();
  
  // configure threads for reconnect, set slice to 150 micros
  threads.setSliceMicros(150);

  // create a thread for the reconnect loop, assign it low priority, in ticks (milliseconds)
  //threads.setTimeSlice(threads.addThread(reconnectLoop), 1);

  while (!Serial.available()) {
    Serial.println("Enter anything to proceed");
    delay(500);
  }

}

void loop() {

  // Reinitialize disconnected chips when necessary
  shart.reconnect();

  // Collect data from sensors
  shart.collect();

  // Store and transmit collected data
  shart.send();

}

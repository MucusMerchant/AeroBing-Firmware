//#define BRIDGE

#ifndef BRIDGE
#include <Arduino.h>
#include <UbxGpsConfig.h>
#include <UbloxGps.h>

#define COMPUTER_BAUDRATE 115200
#define GPS_BAUDRATE 9600

#define DATETIME_FORMAT "%04d.%02d.%02d %02d:%02d:%02d"
#define DATETIME_LENGTH 20

UbloxGps<NavPvtPacket> gps(Serial2);

char datetime[DATETIME_LENGTH];

void setup()
{
    Serial.begin(COMPUTER_BAUDRATE);
    UbxGpsConfig<HardwareSerial, usb_serial_class> *ubxGpsConfig = new UbxGpsConfig<HardwareSerial, usb_serial_class>(Serial2, Serial);
    ubxGpsConfig->setBaudrate(GPS_BAUDRATE);
    ubxGpsConfig->setMessage(UbxGpsConfigMessage::NavPvt);
    ubxGpsConfig->setRate(100);
    ubxGpsConfig->configure();
    //delete ubxGpsConfig;

    gps.begin(GPS_BAUDRATE);
}

void loop()
{
    gps.update();
    if (gps.isReady())
    {
        const NavPvtPacket &packet = gps.getPacket();
    
        
        snprintf(datetime, DATETIME_LENGTH, DATETIME_FORMAT, packet.year, packet.month, packet.day, packet.hour, packet.min, packet.sec);
        
        Serial.print(datetime);
        Serial.print(',');
        Serial.print(packet.lon / 10000000.0, 7);
        Serial.print(',');
        Serial.print(packet.lat / 10000000.0, 7);
        Serial.print(',');
        Serial.print(packet.alt / 1000.0, 3);
        Serial.print(',');
        Serial.print(packet.velN);
        Serial.print(',');
        Serial.print(packet.velE);
        Serial.print(',');
        Serial.print(packet.velD);
        Serial.print(',');
        Serial.print(packet.gSpeed * 0.0036, 5);
        Serial.print(',');
        Serial.print(packet.heading / 100000.0, 5);
        Serial.print(',');
        Serial.print(packet.fixType);
        Serial.print(',');
        Serial.println(packet.numSV);
        
    }
}

#else
// this allows you to use u-center while plugged into the Teensy
#include <Arduino.h>

#define COMPUTER_BAUDRATE 9600
#define GPS_BAUDRATE 9600

void setup()
{
    Serial.begin(COMPUTER_BAUDRATE);
    Serial2.begin(GPS_BAUDRATE);
}

void loop()
{
    // If there is data from the GPS receiver, read it and send it to the computer or vice versa.
    if (Serial2.available())
    {
        Serial.write(Serial2.read());
    }

    if (Serial.available())
    {
        Serial2.write(Serial.read());
    }
}
#endif
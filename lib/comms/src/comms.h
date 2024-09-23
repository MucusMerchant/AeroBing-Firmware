// this file contains all of the necessary typedefs and packets for ALL operations of the flight computer. this includes both outgoing and incoming packets
#ifndef COMMS_H
#define COMMS_H

#include <HardwareSerial.h>

#define HEADER_LENGTH    4
#define NUM_PACKET_TYPES 3
#define SYNC             0xAA // idk chat gpt said alternating bits are good
// first byte is the type id, second is the payload length REMEMBER TO UPDATE THESE
#define TYPE_SENSOR      0x0B
#define TYPE_GPS         0xCA
#define TYPE_POOP        0x33

// nested ternaries to map packet types to their respective sizes: important for several functions
#define TYPE_SIZE(type) ( \
    type == TYPE_SENSOR ? sizeof(sensor_p) : \
    type == TYPE_GPS    ? sizeof(gps_p) : \
    type == TYPE_POOP   ? 0  : \
    0 )

// macro to update a packet object with the checksum of its payload, braces to avoid redeclaration of temporary variables
#define CHECKSUM(p) \
    {                                                                  \
        unsigned char *payload = (unsigned char *) &p;                 \
        memset(payload + 2, 0, 2);                                     \
        for (unsigned int i = HEADER_LENGTH; i < sizeof(p); i++) {     \
            p.c_a += payload[i];                                       \
            p.c_b += p.c_a;                                            \
        }                                                              \
    }

typedef unsigned char packet_t;

// Header common to all packet types
// only one sync byte (0xAA), then a type, defined by the packet_t enum, then two checksum bytes
struct packet_base {

    const unsigned char sync = SYNC;
    const unsigned char type;
    unsigned char c_a; // single-byte addition is actually slower bc of sign-extending, msybe should cast before calculating, may be insignificant though
    unsigned char c_b;

    packet_base(packet_t t) : type(t) {}

};

// Sensor packet includes IMU data, altimeter data
// if you make changes to this struct, they should respect packed alignment
// if packing is impossible, make sure to explicitly specify the padding in the struct to its straightfoward to interpret on the Python end
struct sensor_p : public packet_base {

    struct {
        uint32_t us;
        int32_t adxl_acc_x;
        int32_t adxl_acc_y;
        int32_t adxl_acc_z;
        float acc_x;
        float acc_y;
        float acc_z;
        float gyr_x;
        float gyr_y;
        float gyr_z;
        float mag_x;
        float mag_y;
        float mag_z;
        float temp;
        float pres;
    } data;

    sensor_p() : packet_base(TYPE_SENSOR), data{} {}

};

struct gps_p : public packet_base {

    struct {
        uint32_t      us;
        int32_t       lat;
        int32_t       lon;
        int32_t       alt;
        int32_t       veln;
        int32_t       vele;
        int32_t       veld;
        uint32_t      eph;
        uint32_t      epv;
        uint32_t      sacc;
        int32_t       gspeed;
        float         pdop;
        unsigned char nsats;
        unsigned char fix_type;
        unsigned char valid;
        unsigned char flags;
        // blah blah
    } data;

    gps_p() : packet_base(TYPE_GPS), data{} {}

};
// struct gps_message {
// 	uint64_t time_usec{0};
// 	int32_t lat;		///< Latitude in 1E-7 degrees
// 	int32_t lon;		///< Longitude in 1E-7 degrees
// 	int32_t alt;		///< Altitude in 1E-3 meters (millimeters) above MSL
// 	float yaw;		///< yaw angle. NaN if not set (used for dual antenna GPS), (rad, [-PI, PI])
// 	float yaw_offset;	///< Heading/Yaw offset for dual antenna GPS - refer to description for GPS_YAW_OFFSET
// 	uint8_t fix_type;	///< 0-1: no fix, 2: 2D fix, 3: 3D fix, 4: RTCM code differential, 5: Real-Time Kinematic
// 	float eph;		///< GPS horizontal position accuracy in m
// 	float epv;		///< GPS vertical position accuracy in m
// 	float sacc;		///< GPS speed accuracy in m/s
// 	float vel_m_s;		///< GPS ground speed (m/sec)
// 	Vector3f vel_ned;	///< GPS ground speed NED
// 	bool vel_ned_valid;	///< GPS ground speed is valid
// 	uint8_t nsats;		///< number of satellites used
// 	float pdop;		///< position dilution of precision
// };
/*
// initialize radio serial and storage device
void initRadio(HardwareSerial &serial, int baud_rate, unsigned char *aux_buffer = nullptr, int aux_buffer_length = 0) {
    serial.begin(baud_rate);
    serial.addMemoryForWrite(aux_buffer, aux_buffer_length);
    while (!serial) {};
}*/
/*
bool initStorage() {
    return true;
}
*/
// this assumes the packet passed in is initialized with correct type, i.e. correct size
void receivePacket(packet_base &p, packet_t type, HardwareSerial &serial);

    // readPacket()? from storage maybe

void sendPacket(packet_base &p, packet_t type, HardwareSerial &serial);

// rewrite this function for whatever storage library we using
void writePacket(packet_base &p, packet_t type);

// make flush/sync accessible, only SD
void syncSD();




/*
class poop {
    public:
        sensor_p packet;
    
};

int main() {
    poop poop;
    
    poop.packet.temp = 69;
    sendPacket(poop.packet, TYPE_SENSOR);
    std::cout << poop.packet.c_a << " " << poop.packet.c_b << std::endl;
    return 0;
}
*/


// how do we fit threaded GPS messages into shart?
// we need a thread-safe buffer that holds packets, sends in bursts
//      Best way I've thought of so far: have GPS running constantly on a different thread
//      When it finds data, GPS thread memcpy()s packet somewhere accessible to the main thread, provides a flag to indicate data is available, then continues
//      This functionality could literally just be built into the GPS library
//      The only possible thread conflict is at the copied GPS array. maybe lock it just in case
//      
//      
// general re-design option: could get rid of the class separation, instead just have functions implemented in different .cpp files.

#endif
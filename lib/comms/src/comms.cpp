#include "comms.h"


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
void receivePacket(packet_base &p, packet_t type, HardwareSerial &serial) {
    // unsigned char *payload = (unsigned char *) &p;
    // unsigned char sum[2];

    // p.c_a = sum[0];
    // p.c_b = sum[1];
    // TODO
}

    // readPacket()? from storage maybe

void sendPacket(packet_base &p, packet_t type, HardwareSerial &serial) {
    unsigned char len = TYPE_SIZE(type);
    CHECKSUM(p)
    serial.write((unsigned char *) &p, len + HEADER_LENGTH);
}

// rewrite this function for whatever storage library we using
void writePacket(packet_base &p, packet_t type) {
    unsigned char len = TYPE_SIZE(type);
    //CHECKSUM(p, len);//checksum not necessary here, we assume data integrity
}

// make flush/sync accessible, only SD
void syncSD() {

}




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

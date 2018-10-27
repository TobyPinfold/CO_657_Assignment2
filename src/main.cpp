#include <mbed.h>
#include <rtos.h>
#include <mbed_events.h>
#include <iostream>
#include <EthernetInterface.h>
#include <LM75B.h>
#include <C12832.h>
using namespace std;
 
struct PACKET {
        uint16_t senderID;
        uint8_t sequenceNumber;
        uint8_t packetOptions;        
        uint16_t temperature;
        uint8_t buttonPresses;
        uint8_t checksum;
};

C12832 lcd (D11, D13, D12, D7, D10);
LM75B temperatureSensor(PTE25, PTE24);

uint16_t getTemperature();
void sendPacket(PACKET packet);
void connectEthernet(EthernetInterface eth);

int main() {
    EthernetInterface eth;
    PACKET packet;

    connectEthernet(eth);
    uint16_t temperature = getTemperature();
    float x = temperature;
    lcd.cls();
    lcd.printf("Temperature is %.2f C\n", x);
    sendPacket(packet);
}

void connectEthernet(EthernetInterface eth) {
    nsapi_error_t status = eth.connect();
    eth.connect();
    lcd.locate(0,0);
    lcd.printf("IP Address is %s \n", eth.get_ip_address());
}

uint16_t getTemperature() {
    uint16_t temperature = 0.0;
    temperature = temperatureSensor.read();
    return temperature;
}

void setSequenceNumber(int) {
    

}

void setPacketOptions() {


}

void setButtonPresses() {


}

void setTemperature() {


}

void generateChecksum() {

}

void sendPacket(PACKET packet) {
    
}
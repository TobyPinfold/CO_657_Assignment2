#include <mbed.h>
#include <EthernetInterface.h>
#include <LM75B.h>
#include <C12832.h>

using namespace std;

struct PACKET
{
    uint16_t senderID;
    uint8_t sequenceNumber;
    uint8_t packetOptions;
    uint16_t temperature;
    uint8_t buttonPresses;
    uint8_t checksum;
    uint64_t packet;
};

C12832 lcd(D11, D13, D12, D7, D10);
LM75B temperatureSensor(PTE25, PTE24);

float getTemperature();
void sendPacket(EthernetInterface eth, PACKET &packet);
void connectEthernet(EthernetInterface eth);

int main()
{
    EthernetInterface eth;
    PACKET packet;
    connectEthernet(eth);
    packet.senderID = 21486;
    packet.temperature = (uint16_t)getTemperature();

    sendPacket(eth, packet);
}

void connectEthernet(EthernetInterface eth)
{
    nsapi_error_t status = eth.connect();
    eth.connect();
    lcd.locate(0, 0);
    lcd.printf("IP Address is %s \n", eth.get_ip_address());
    wait_ms(2000);
}

float getTemperature()
{
    uint16_t temperature = temperatureSensor.read();
    uint16_t temp2 = (temperature << 8) | (temperature >> 8);
    return temp2 / 256.0;
}

void setSequenceNumber(int)
{
}

void setPacketOptions()
{
}

void setButtonPresses()
{
}

void setTemperature()
{
}

void generateChecksum()
{
}

void sendPacket(EthernetInterface eth, PACKET &packet)
{
    UDPSocket sock(&eth);
    SocketAddress sockAddr;

    lcd.cls();
    lcd.locate(0, 0);
    uint64_t val = 0xdb12010019202000;

    int status = sock.sendto("http://lora.kent.ac.uk", 1789, &val, sizeof(uint64_t));
    if (0 > status)
    {
        lcd.printf("Error sending data\n %n", status);
    }
    else
    {
        lcd.printf("sent?");
    }
}
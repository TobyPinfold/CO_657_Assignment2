
#include <main.h>

bool useCCITT = true;
volatile bool retryflag = false;

PACKET packet;
SocketAddress socketAddress;
volatile bool acknowledgeFlag = false;

int main()
{
    connectEthernet(eth);

    sw2.fall(&onSW2Pressed);
    sw3.fall(&onSW3Pressed);
    joystickDown.fall(&onJoystickDownPressed);
    joystickUp.fall(&onJoystickUpPressed);
    joystickLeft.fall(&onJoystickLeftPressed);
    joystickRight.fall(&onJoystickRightPressed);
    joystickFire.fall(&onJoystickFirePressed);

    temperaturePollingThread.start(callback(&temperaturePollingQueue, &EventQueue::dispatch_forever));
    temperaturePollingQueue.call_every(10000, &sendTemperatureUpdate);
    // sendTemperatureUpdate();
}   


void sendTemperatureUpdate()
{
    UDPSocket sock(&eth);
    sock.set_timeout(2000);

     if((sequenceNumber % 10 == 0) || retryflag) {
        acknowledgeFlag = true;
    } else {
        acknowledgeFlag = false;
    }

    uint32_t response = 0x0000000000000000;
    
    if(!retryflag) {
         
        sequenceNumber++;
        setSenderID(21486);
        setSequenceNumber();
        setTemperature();
        setButtonPresses();
    
    } else {

        temperaturePollingThread.wait(1000 * (std::pow(retryCount, 2)));
    }
      
    setPacketOptions(retryflag, useCCITT, acknowledgeFlag);
        
    if(useCCITT) {
        generateCCITTChecksum();
    } else {
         generateChecksum();
    }

    uint64_t builtPacket = buildPacket();
    sock.sendto("lora.kent.ac.uk", 1789, &builtPacket, sizeof(uint64_t));

    if(acknowledgeFlag) {
            
        nsapi_size_or_error_t status = sock.recvfrom(&socketAddress, &response, sizeof(response));
       
        response = 0x0000000000000000 | (response >> 16);
        
        if((response != sequenceNumber) || (status < 0)) {
                
            retryflag = true;
            retryCount++;

        } else {
                
            retryCount = 0;
            retryflag = false;
            clearPressedButtons();
        }
    } else {
        clearPressedButtons();
    }
}


void retry () {

    setPacketOptions(retryflag, useCCITT, acknowledgeFlag);
    uint64_t builtPacket = buildPacket();
    sendPacket(eth, builtPacket);

}


void connectEthernet(EthernetInterface eth)
{
    eth.connect();
}





void setSenderID(uint16_t id)
{
    packet.senderID = id;
}



void setSequenceNumber()
{
    if (sequenceNumber >= 254)
        sequenceNumber = 0;

    packet.sequenceNumber = sequenceNumber;
}




void setPacketOptions(bool retryFlag, bool ccittFlag, bool ackRequestFlag)
{
    uint8_t tempPacketOptions = 0;

    if (retryFlag)
        tempPacketOptions += 4;
        
    if (ccittFlag)
        tempPacketOptions += 2;

    if (ackRequestFlag)
        tempPacketOptions += 1;

    packet.packetOptions = tempPacketOptions;
}





void setButtonPresses()
{
    uint8_t buttonsPressed = 0;

    if (sw2Pressed)
        buttonsPressed += SW2_VAL;

    if (sw3Pressed)
        buttonsPressed += SW3_VAL;

    if (joystickUpPressed)
        buttonsPressed += JOYSTICK_DOWN_VAL;

    if (joystickDownPressed)
        buttonsPressed += JOYSTICK_UP_VAL;

    if (joystickLeftPressed)
        buttonsPressed += JOSYTICK_LEFT_VAL;

    if (joystickRightPressed)
        buttonsPressed += JOYSTICK_RIGHT_VAL;

    if (joystickFirePressed)
        buttonsPressed += JOYSTICK_FIRE_VAL;

    packet.buttonPresses = buttonsPressed;
}




void clearPressedButtons()
{
    sw2Pressed = false;
    sw3Pressed = false;
    joystickDownPressed = false;
    joystickUpPressed = false;
    joystickLeftPressed = false;
    joystickRightPressed = false;
    joystickFirePressed = false;
}





void setTemperature()
{
    uint16_t temperatureBigEndian = temperatureSensor.read();
    packet.temperature = temperatureBigEndian;
}





void generateChecksum()
{   
    uint8_t packetList[7];
    packetList[0] = packet.senderID;
    packetList[1] = packet.senderID >> 8;
    packetList[2] = packet.sequenceNumber;
    packetList[3] = packet.packetOptions;
    packetList[4] = packet.temperature;
    packetList[5] = packet.temperature >> 8;
    packetList[6] = packet.buttonPresses;

    uint8_t checksum = (packetList[0] ^ packetList[1] ^ packetList[2] ^ packetList[3] ^ packetList[4] ^ packetList[5] ^ packetList[6]);

    packet.checksum = checksum;
}





void sendPacket(EthernetInterface eth, uint64_t packet)
{
    UDPSocket sock(&eth);
}




uint64_t buildPacket()
{
    uint64_t padding = 0xFFFFFFFFFFFFFFFF;
    return (((packet.checksum & padding) << 56) |
            ((packet.buttonPresses & padding) << 48) |
            ((packet.temperature & padding) << 32) |
            ((packet.packetOptions & padding) << 24) |
            ((packet.sequenceNumber & padding) << 16) |
            (packet.senderID & padding));
}




void generateCCITTChecksum()
{
    uint8_t checksum = 0;
    uint8_t packetList[7];

    packetList[0] = packet.senderID;
    packetList[1] = (packet.senderID >> 8);
    packetList[2] = packet.sequenceNumber;
    packetList[3] = packet.packetOptions;
    packetList[4] = packet.temperature;
    packetList[5] = (packet.temperature >> 8);
    packetList[6] = packet.buttonPresses;

    for(int i = 0; i < 7; i++) {
        checksum = Table_CRC_8bit_CCITT[checksum ^ packetList[i]];
    }

    packet.checksum = checksum;
}

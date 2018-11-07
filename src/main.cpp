
#include <main.h>

bool useCCITT = true;

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

    tickerThread.start(&tickerQueue, &EventQueue::dispatch_forever);
    tickerQueue.call_every(10000, &sendTemperatureUpdate);
    tickerQueue.dispatch();

    while (true)
    {
        sleep();
    }
}

void sendTemperatureUpdate()
{
    PACKET packet;
    setSenderID(packet, 21486);
    setSequenceNumber(packet);

    if (packet.sequenceNumber % 10 == 0)
    {
        setPacketOptions(packet, false, useCCITT, true);
    }
    else
    {
        setPacketOptions(packet, false, useCCITT, false);
    }
    setTemperature(packet);

    setButtonPresses(packet);
    
    if(useCCITT) {
    
        generateCCITTChecksum(packet);

    } else {
    
        generateChecksum(packet);
   
    }
    uint64_t builtPacket = buildPacket(packet);
    sendPacket(eth, builtPacket);
    clearPressedButtons();
}

void connectEthernet(EthernetInterface eth)
{
    eth.connect();
}

void setSenderID(PACKET &packet, uint16_t id)
{
    packet.senderID = id;
}

void setSequenceNumber(PACKET &packet)
{
    if (sequenceNumber >= 254)
        sequenceNumber = 0;

    packet.sequenceNumber = sequenceNumber++;
}




void setPacketOptions(PACKET &packet, bool retryFlag, bool ccittFlag, bool ackRequestFlag)
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





void setButtonPresses(PACKET &packet)
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





void setTemperature(PACKET &packet)
{
    uint16_t temperatureBigEndian = temperatureSensor.read();
    packet.temperature = temperatureBigEndian;
}

void generateChecksum(PACKET &packet)
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
    SocketAddress sockAddr;

    int status = sock.sendto("lora.kent.ac.uk", 1789, &packet, sizeof(uint64_t));
}

uint64_t buildPacket(PACKET packet)
{
    uint64_t padding = 0xFFFFFFFFFFFFFFFF;
    return (((packet.checksum & padding) << 56) |
            ((packet.buttonPresses & padding) << 48) |
            ((packet.temperature & padding) << 32) |
            ((packet.packetOptions & padding) << 24) |
            ((packet.sequenceNumber & padding) << 16) |
            (packet.senderID & padding));
}

void generateCCITTChecksum(PACKET &packet)
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

    lcd.printf("%x", packetList[0]);
    lcd.printf("%x", packetList[1]);
    lcd.printf("%x", packetList[2]);
    lcd.printf("%x", packetList[3]);
    lcd.printf("%x", packetList[4]);
    lcd.printf("%x", packetList[5]);
    lcd.printf("%x", packetList[6]);


    for(int i = 0; i < 7; i++) {
        checksum = Table_CRC_8bit_CCITT[checksum ^ packetList[i]];
    }

    lcd.printf("/n %x", checksum);

    packet.checksum = checksum;
}

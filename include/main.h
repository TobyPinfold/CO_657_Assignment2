#include <mbed.h>
#include <EthernetInterface.h>
#include <LM75B.h>
#include <C12832.h>
#include "rtos.h"
#include <mbed_events.h>

#define SW2_VAL 1
#define SW3_VAL 2
#define JOYSTICK_UP_VAL 4
#define JOYSTICK_DOWN_VAL 8
#define JOSYTICK_LEFT_VAL 16
#define JOYSTICK_RIGHT_VAL 32
#define JOYSTICK_FIRE_VAL 64

Thread tickerThread;
EventQueue tickerQueue(EVENTS_EVENT_SIZE);

C12832 lcd(D11, D13, D12, D7, D10);
LM75B temperatureSensor(PTE25, PTE24);
EthernetInterface eth;

InterruptIn sw2(SW2);
InterruptIn sw3(SW3);
InterruptIn joystickUp(A2);
InterruptIn joystickDown(A3);
InterruptIn joystickLeft(A4);
InterruptIn joystickRight(A5);
InterruptIn joystickFire(D4);

volatile uint8_t buttonsPressed = 0;
volatile uint8_t sequenceNumber = 0;

bool sw2Pressed = false;
bool sw3Pressed = false;
bool joystickUpPressed = false;
bool joystickDownPressed = false;
bool joystickLeftPressed = false;
bool joystickRightPressed = false;
bool joystickFirePressed = false;

struct PACKET
{
    uint8_t checksum;
    uint8_t buttonPresses;
    uint16_t temperature;
    uint8_t packetOptions;
    uint8_t sequenceNumber;
    uint16_t senderID;
};

void buttonDown();

void sendTemperatureUpdate();

void sendPacket(EthernetInterface eth, uint64_t packet);

void setSenderID(PACKET &packet, uint16_t id);

void setSequenceNumber(PACKET &packet);

void setPacketOptions(PACKET &packet, bool retryFlag, bool ccittFlag, bool ackRequestFlag);

void setButtonPresses(PACKET &packet);

void setTemperature(PACKET &packet);

void generateChecksum(PACKET &packet);

void connectEthernet(EthernetInterface eth);

uint64_t buildPacket(PACKET packet);

void clearPressedButtons();

void onSW2Pressed()
{
    sw2Pressed = true;
}

void onSW3Pressed()
{
    sw3Pressed = true;
}

void onJoystickDownPressed()
{
    joystickDownPressed = true;
}

void onJoystickUpPressed()
{
    joystickUpPressed = true;
}

void onJoystickLeftPressed()
{
    joystickLeftPressed = true;
}

void onJoystickRightPressed()
{
    joystickRightPressed = true;
}

void onJoystickFirePressed()
{
    joystickFirePressed = true;
}

uint8_t ccittLookup(uint64_t packet);
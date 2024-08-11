#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma warning(disable:4996) 

// Link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

enum TriggerModes
{
    Off = 0x0,
    Rigid = 0x1,
    Pulse = 0x2,
    Rigid_A = 0x1 | 0x20,
    Rigid_B = 0x1 | 0x04,
    Rigid_AB = 0x1 | 0x20 | 0x04,
    Pulse_A = 0x2 | 0x20,
    Pulse_B = 0x2 | 0x04,
    Pulse_AB = 0x2 | 0x20 | 0x04,
    Calibration = 0xFC
};

enum PlayerLED
{
    PLED_OFF = 0,
    PLAYER_1 = 4,
    PLAYER_2 = 10,
    PLAYER_3 = 21,
    PLAYER_4 = 27,
    ALL = 31
};

enum MicrophoneLED
{
    MICLED_ON = 1,
    MICLED_OFF = 0
};

int rumbleFlag = 0xFC;
TriggerModes leftMode = Pulse;
TriggerModes rightMode = Off;
int leftTriggerForce[7];
int rightTriggerForce[7];
MicrophoneLED micLed = MICLED_ON;
PlayerLED playerLed = PLAYER_1;
int R = 0;
int G = 0;
int B = 255;
int leftMotor = 0;
int rightMotor = 0;
int speakerVolume = 0; // volume ranges from 0 to 100
int leftActuatorVolume = 100; // volume ranges from 0 to 100
int rightActuatorVolume = 100; // volume ranges from 0 to 100
bool clearBuffer = false;
const char *wavFileLocation = "C:\\Users\\Igor\\Documents\\DualSenseY\\audiotest.wav";

int main() {
    WSADATA wsaData;
    SOCKET SendSocket = INVALID_SOCKET;
    sockaddr_in RecvAddr;
    unsigned short Port = 64370;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed with error: " << WSAGetLastError() << std::endl;
        return 1;
    }

    // Create a socket for sending data
    SendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (SendSocket == INVALID_SOCKET) {
        std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Set up the RecvAddr structure with the IP address and port of the receiver
    RecvAddr.sin_family = AF_INET;
    RecvAddr.sin_port = htons(Port);
    RecvAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    while (true)
    {
        int8_t intArray[] = { rumbleFlag, leftMode, leftTriggerForce[0], leftTriggerForce[1], leftTriggerForce[2], leftTriggerForce[3], leftTriggerForce[4], leftTriggerForce[5], leftTriggerForce[6], rightMode, rightTriggerForce[0], rightTriggerForce[1], rightTriggerForce[2], rightTriggerForce[3], rightTriggerForce[4], rightTriggerForce[5], rightTriggerForce[6], micLed, playerLed, R, G, B, leftMotor, rightMotor, speakerVolume, leftActuatorVolume, rightActuatorVolume, clearBuffer};

        int intArraySize = sizeof(intArray);
        char SendBuf[1024];
        int BufLen = intArraySize;

        // Convert the int32_t array to a byte array
        memcpy(SendBuf, intArray, intArraySize);


        // Send a datagram to the receiver
        sendto(SendSocket, SendBuf, BufLen, 0, (SOCKADDR*)&RecvAddr, sizeof(RecvAddr));
        if (wavFileLocation != "") {
            sendto(SendSocket, wavFileLocation, strlen(wavFileLocation), 0, (SOCKADDR*)&RecvAddr, sizeof(RecvAddr));
            wavFileLocation = "";
        }

        Sleep(1);
    }
    return 0;
}

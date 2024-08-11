// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <windows.h>
#include <iostream>
#include <cstdlib>  
#include <direct.h> 
#include "Utils.h"
#include "Enums.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
//#pragma warning(disable:4996) 

#pragma comment(lib, "Ws2_32.lib")

enum TriggerModes
{
    Off = 0,
    Rigid = 1,
    Pulse = 5,
    Rigid_A = 2,
    Rigid_B = 3,
    Rigid_AB = 4,
    Pulse_A = 6,
    Pulse_B = 7,
    Pulse_AB = 8,
};

enum PlayerLED
{
    PLED_OFF = 0,
    PLAYER_1 = 1,
    PLAYER_2 = 2,
    PLAYER_3 = 3,
    PLAYER_4 = 4,
    ALL = 5
};

enum MicrophoneLED
{
    MICLED_ON = 1,
    MICLED_OFF = 0
};

int rumbleFlag = 0xFC;
TriggerModes leftMode = Off;
TriggerModes rightMode = Off;
int leftTriggerThreshold = 0;
int rightTriggerThreshold = 0;
int leftTriggerForce[7];
int rightTriggerForce[7];
MicrophoneLED micLed = MICLED_OFF;
PlayerLED playerLed = PLAYER_1;
int R;
int G;
int B;
int leftMotor = 0;
int rightMotor = 0;
float _speakerVolume = 1;
float _leftActuatorVolume = 1;
float _rightActuatorVolume = 1;
bool _clearBuffer = false;
bool lightbarTransition = false;
int transitionSteps = 0;
int transitionDelay = 0;
const char* directory = "dualsense-service\\haptics\\";
std::string wavFile = "";

void setTrigger(TriggerModes triggerMode, int triggerThreshold, bool rightTrigger, int force1, int force2, int force3, int force4, int force5, int force6, int force7) {
    if (rightTrigger) {
        rightMode = triggerMode;
        rightTriggerThreshold = triggerThreshold;
        rightTriggerForce[0] = force1;
        rightTriggerForce[1] = force2;
        rightTriggerForce[2] = force3;
        rightTriggerForce[3] = force4;
        rightTriggerForce[4] = force5;
        rightTriggerForce[5] = force6;
        rightTriggerForce[6] = force7;
    }
    else {
        leftMode = triggerMode;
        leftTriggerThreshold = triggerThreshold;
        leftTriggerForce[0] = force1;
        leftTriggerForce[1] = force2;
        leftTriggerForce[2] = force3;
        leftTriggerForce[3] = force4;
        leftTriggerForce[4] = force5;
        leftTriggerForce[5] = force6;
        leftTriggerForce[6] = force7;
    }
}

void setLightbar(int _R, int _G, int _B) {
    R = _R;
    G = _G;
    B = _B;
}

void playHaptics(std::string wavFileName, float speakerVolume, float leftActuatorVolume, float rightActuatorVolume, bool clearBuffer) {
    _speakerVolume = speakerVolume;
    _leftActuatorVolume = leftActuatorVolume;
    _rightActuatorVolume = rightActuatorVolume;
    _clearBuffer = clearBuffer;
    wavFile = wavFileName;
}

std::string GetWorkingDir() {
    char path[MAX_PATH] = "";
    GetCurrentDirectoryA(MAX_PATH, path);
    PathAddBackslashA(path);
    return path;
}

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

int startSendingToService() { //{"instructions":[{"type":1,"parameters":[0,2,2]}]}
    WSADATA wsaData;
    SOCKET SendSocket = INVALID_SOCKET;
    sockaddr_in RecvAddr;
    unsigned short Port = 6969;

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

    std::string workingDir = ReplaceAll(GetWorkingDir(), "\\", "/");
    std::string packet = "";
    while (true)
    {
        packet = "{\"instructions\":[{\"type\":9,\"parameters\":[0," + std::to_string(R) + "," + std::to_string(G) + "," + std::to_string(B) + ",20,2" + "]}]}"; // RGB transition animation
        sendto(SendSocket, packet.c_str(), strlen(packet.c_str()), 0, (SOCKADDR*)&RecvAddr, sizeof(RecvAddr));

        packet = "{\"instructions\":[{\"type\":1,\"parameters\":[0,2,12," + std::to_string(rightMode) + "," + std::to_string(rightTriggerForce[0]) + "," + std::to_string(rightTriggerForce[1])
            + "," + std::to_string(rightTriggerForce[2]) + "," + std::to_string(rightTriggerForce[3]) + "," + std::to_string(rightTriggerForce[4])
            + "," + std::to_string(rightTriggerForce[5]) + "," + std::to_string(rightTriggerForce[6]) + "]}]}";
        sendto(SendSocket, packet.c_str(), strlen(packet.c_str()), 0, (SOCKADDR*)&RecvAddr, sizeof(RecvAddr));

        packet = "{\"instructions\":[{\"type\":4,\"parameters\":[0," + std::to_string(leftTriggerThreshold) + "," + std::to_string(rightTriggerThreshold) + "]}]}";
        sendto(SendSocket, packet.c_str(), strlen(packet.c_str()), 0, (SOCKADDR*)&RecvAddr, sizeof(RecvAddr));

        packet = "{\"instructions\":[{\"type\":6,\"parameters\":[0,0]}]}";
        sendto(SendSocket, packet.c_str(), strlen(packet.c_str()), 0, (SOCKADDR*)&RecvAddr, sizeof(RecvAddr));

        if (wavFile != "") {
            packet = "{\"instructions\":[{\"type\":8,\"parameters\":[0,\"" +  workingDir + "game/bin/win64/haptics/" + wavFile + "\","+ std::to_string(_speakerVolume) +","+ std::to_string(_leftActuatorVolume)+"," + std::to_string(_rightActuatorVolume) + "," + std::to_string((int8_t)_clearBuffer) + "]}]}";
            sendto(SendSocket, packet.c_str(), strlen(packet.c_str()), 0, (SOCKADDR*)&RecvAddr, sizeof(RecvAddr));
            wavFile = "";
        }

        Sleep(1);
    }

    return 0;
}

DWORD64* jmpBackWeaponType;
unsigned short weaponTypeN = 0;
__attribute__((naked))
void weaponTypeTrampoline(){
    __asm {
        mov[rdi + 0x1C], r14d
        mov[weaponTypeN], r14d
        mov r14, [rsp + 0x28]
        mov[rdi + 0x20], ebp
        mov rcx, [rdi]
        mov rax, qword ptr[jmpBackWeaponType]
        jmp rax
    }
}

DWORD64* jmpBackPickup;
unsigned short pickupN = 0;
__attribute__((naked))
void pickupTrampoline() {
    __asm {
        inc qword ptr[rbx + 0x00002BA0]
        mov rcx, rsi
        mov rsi, [rsp + 0x30]
        mov [pickupN], 0x1
        mov rax, qword ptr[jmpBackPickup]
        jmp rax
    }
}

DWORD64* jmpBackClipsize;
unsigned short ClipSize = 0;
__attribute__((naked))
void clipsizeTrampoline() {
    __asm {
        mov[rdi + 0x000002B4], ebx
        mov[rdi + 0x000001AC], ebx
        mov[ClipSize], ebx
        mov rbx, [rsp + 0x30]
        mov rax, qword ptr[jmpBackClipsize]
        jmp rax
    }
}

DWORD64* jmpBackUse;
unsigned short UseN = 0;
__attribute__((naked))
void useTrampoline() {
    __asm {
        mov r12, [rsp + 0x00000158]
        xor r15b, r15b
        mov [rsi + 0x00002AF8], r13b
        mov [UseN], 0x1
        mov rax, qword ptr[jmpBackUse]
        jmp rax
    }
}

DWORD64* jmpBackHealth;
unsigned short HealthN = 0;
__attribute__((naked))
void healthTrampoline() {
    __asm {
        mov esi, [rsi + 0x00000200]
        mov[HealthN], esi
        mov eax, 0x00000000
        test esi, esi
        cmovs esi, eax
        mov rax, qword ptr[jmpBackHealth]
        jmp rax
    }
}

void inject() {
    setLightbar(255, 0, 0);
    Utils utils;

    Sleep(20000);

    std::cout << "----------------------------------" << std::endl;

    HMODULE clientDllModule = GetModuleHandle(L"client.dll");
    if (clientDllModule == NULL) {
        std::cerr << "Failed to get module handle." << std::endl;
        std::wcout << L"Module Handle: 0x" << std::hex << (uintptr_t)clientDllModule << std::endl;
    }
    else {
        std::cout << "Module client.dll found at: ";
        std::wcout << L"Module Handle: 0x" << std::hex << (uintptr_t)clientDllModule << std::endl;
    }

    HMODULE serverDllModule = GetModuleHandle(L"server.dll");
    if (serverDllModule == NULL) {
        std::cerr << "Failed to get module handle." << std::endl;
        std::wcout << L"Module Handle: 0x" << std::hex << (uintptr_t)serverDllModule << std::endl;
    }
    else {
        std::cout << "Module server.dll found at: ";
        std::wcout << L"Module Handle: 0x" << std::hex << (uintptr_t)serverDllModule << std::endl;
    }

    HMODULE vphysics2DllModule = GetModuleHandle(L"vphysics2.dll");
    if (vphysics2DllModule == NULL) {
        std::cerr << "Failed to get module handle." << std::endl;
        std::wcout << L"Module Handle: 0x" << std::hex << (uintptr_t)vphysics2DllModule << std::endl;
    }
    else {
        std::cout << "Module vphysics2.dll found at: ";
        std::wcout << L"Module Handle: 0x" << std::hex << (uintptr_t)vphysics2DllModule << std::endl;
    }

    Sleep(1000);

    std::cout << "----------------------------------" << std::endl;

    DWORD64* weapontypeAddress = utils.scanModuleMemory(serverDllModule, "44 89 77 1C 4C 8B 74 24 28 89 6F 20 48 8B 0F 48 8B 6C 24 48 48 85 C9 75 09 89 4F 24 48 83 C4 30 5F C3 48 8B 09 BA 02 00 00 00 E8 E2 86 FF FF 89 47 24 48 83 C4 30 5F C3 CC");
    std::cout << "weapontype function: ";
    std::cout << weapontypeAddress << std::endl;
    jmpBackWeaponType = utils.scanModuleMemory(serverDllModule, "48 8B 6C 24 48 48 85 C9 75 09 89 4F 24 48 83 C4 30 5F C3 48 8B 09 BA 02 00 00 00 E8 E2 86 FF FF 89 47 24 48 83 C4 30 5F C3");
    utils.Detour(reinterpret_cast<DWORD*>((void*)weapontypeAddress), weaponTypeTrampoline, 15);
    std::cout << "weapontype jmp back: ";
    std::cout << jmpBackWeaponType << std::endl;

    std::cout << "----------------------------------" << std::endl;

    DWORD64* pickupAddress = utils.scanModuleMemory(vphysics2DllModule, "FF 83 A0 2B 00 00 48 8B CE 48 8B 74 24 30");
    std::cout << "pickup function: ";
    std::cout << pickupAddress << std::endl;
    jmpBackPickup = utils.scanModuleMemory(vphysics2DllModule, "48 C1 E1 04 48 03 8B A8 2B 00 00");
    utils.Detour(reinterpret_cast<DWORD*>((void*)pickupAddress), pickupTrampoline, 14);
    std::cout << "pickup jmp back: ";
    std::cout << jmpBackPickup << std::endl;

    std::cout << "----------------------------------" << std::endl;

    //48 83 C4 20 5F C3 89 99 AC 01 00 00 48 8B 5C 24 30 48 83 C4 20 5F C3 48 89 5C 24 08
    DWORD64* clipsizeAddress = utils.scanModuleMemory(clientDllModule, "89 9F B4 02 00 00 89 9F AC 01 00 00 48 8B 5C 24 30");
    std::cout << "clipsize function: ";
    std::cout << clipsizeAddress << std::endl;
    jmpBackClipsize = utils.scanModuleMemory(clientDllModule, "48 83 C4 20 5F C3 89 99 AC 01 00 00 48 8B 5C 24 30 48 83 C4 20 5F C3 48 89 5C 24 08");
    utils.Detour(reinterpret_cast<DWORD*>((void*)clipsizeAddress), clipsizeTrampoline, 17);
    std::cout << "clipsize jmp back: ";
    std::cout << jmpBackClipsize << std::endl;

    std::cout << "----------------------------------" << std::endl;

    //BA 00 02 00 00 FF 90 A0 00 00 00 89 B7 00 02 00 00 48 8B 6C 24 48

    DWORD64* healthAddress = utils.scanModuleMemory(serverDllModule, "8B B6 00 02 00 00 B8 00 00 00 00 85 F6 0F 48 F0");
    std::cout << "health function: ";
    std::cout << healthAddress << std::endl;
    jmpBackHealth = utils.scanModuleMemory(serverDllModule, "39 B4 AF C0 06 00 00 74 20 48 8B 07");
    utils.Detour(reinterpret_cast<DWORD*>((void*)healthAddress), healthTrampoline, 16);
    std::cout << "health jmp back: ";
    std::cout << jmpBackHealth << std::endl;

    setLightbar(0, 255, 0);
}

void read() {
    WeaponType weapon;
    HoldingProp holding;
    unsigned short lastClipSize = 30;
    unsigned short lastHealth = 100;
    while (true) {
        Sleep(25);
        weapon = static_cast<WeaponType>(weaponTypeN);
        holding = static_cast<HoldingProp>(pickupN);

        // BROKEN - GETS TRIGGERED WHEN HEADCRABS JUMP
        //switch (holding)
        //{
        //case YES:
        //    if (UseN == 1) {
        //        setLightbar(140, 45, 0);
        //        playHaptics("grabbity_grab_03.wav", 1, 1, 1, true);
        //        Sleep(550);
        //        setLightbar(0, 0, 0);
        //        pickupN = 0;
        //        UseN = 0;
        //    }
        //    break;
        //}


        if (HealthN > lastHealth) {
            setLightbar(0, 255, 0);
            Sleep(550);
            setLightbar(0, 0, 0);
        }
        else if (HealthN < lastHealth) {
            setLightbar(255, 0, 0);
            Sleep(550);
            setLightbar(0, 0, 0);
        }

        switch (weapon)
        {
        case TOOL:
            setTrigger(Rigid, 255, true, 80, 170, 0, 0, 0, 0, 0);
            break;
        case PISTOL:
            if (ClipSize > 0 && ClipSize < lastClipSize) {
                setTrigger(Pulse, 150, true, 80, 170, 0, 0, 0, 0, 0);
                playHaptics("smg1_fire1.wav", 0, 1, 1, true);
            }
            else if (ClipSize == 0) {
                setTrigger(Pulse, 255, true, 160, 200, 100, 0, 0, 0, 0);
            }
            break;
        case SHOTGUN:
            if (ClipSize > 0 && ClipSize < lastClipSize) {
                setTrigger(Pulse_AB, 150, true, 255, 0, 255, 255, 255, 0, 0);
                playHaptics("shotgun_fire6.wav", 0, 1, 1, true);
            }
            else if (ClipSize == 0) {
                setTrigger(Pulse, 255, true, 160, 200, 100, 0, 0, 0, 0);
            }
            break;
        case SMG:
            if (ClipSize > 0 && ClipSize < lastClipSize) {
                setTrigger(Pulse_B, 150, true, 10, 255, 80, 0, 0, 0, 0);
                playHaptics("smg1_fire1.wav", 0, 1, 1, true);
                Sleep(100);
            }
            else if (ClipSize == 0) {
                setTrigger(Pulse, 255, true, 160, 200, 100, 0, 0, 0, 0);
            }
            break;
        default:
            break;
        }

        lastHealth = HealthN;
        lastClipSize = ClipSize;
    }
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: {
        AllocConsole();
        FILE* fDummy;
        freopen_s(&fDummy, "CONIN$", "r", stdin);
        freopen_s(&fDummy, "CONOUT$", "w", stderr);
        freopen_s(&fDummy, "CONOUT$", "w", stdout);

        HANDLE injectThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)inject, hModule, 0, 0);
        HANDLE controllerServiceThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)startSendingToService, hModule, 0, 0);
        HANDLE readThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)read, hModule, 0, 0);
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


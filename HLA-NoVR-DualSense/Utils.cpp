#include "pch.h"
#include "Utils.h"
#include <vector>
#include <psapi.h>
#include <string>
#include <iostream>

std::vector<uint8_t> Utils::patternToByteArray(const std::string& pattern, std::vector<bool>& mask) {
    std::vector<uint8_t> bytePattern;
    size_t pos = 0;
    while (pos < pattern.size()) {
        if (pattern[pos] == '?') {
            bytePattern.push_back(0);
            mask.push_back(false);
            pos += 2; // Skip the space after '?'
        }
        else {
            bytePattern.push_back(static_cast<uint8_t>(std::stoi(pattern.substr(pos, 2), nullptr, 16)));
            mask.push_back(true);
            pos += 3; // Skip two hex digits and a space
        }
    }
    return bytePattern;
}

uintptr_t Utils::aobscan(const BYTE* baseAddress, SIZE_T size, const std::vector<uint8_t>& bytePattern, const std::vector<bool>& mask) {
    for (SIZE_T i = 0; i < size - bytePattern.size(); ++i) {
        bool found = true;
        for (size_t j = 0; j < bytePattern.size(); ++j) {
            if (mask[j] && baseAddress[i + j] != bytePattern[j]) {
                found = false;
                break;
            }
        }
        if (found) {
            return reinterpret_cast<uintptr_t>(baseAddress) + i;
        }
    }
    return 0;
}

DWORD64* Utils::scanModuleMemory(HMODULE hModule, const std::string& pattern) {
    std::vector<bool> mask;
    std::vector<uint8_t> bytePattern = patternToByteArray(pattern, mask);

    MODULEINFO moduleInfo;
    if (GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(moduleInfo))) {
        const BYTE* baseAddress = static_cast<const BYTE*>(moduleInfo.lpBaseOfDll);
        SIZE_T moduleSize = moduleInfo.SizeOfImage;

        uintptr_t result = aobscan(baseAddress, moduleSize, bytePattern, mask);
        if (result) {
            // Print or handle the found address
            // Example: Output the result to the console or log it
            char buffer[256];
            snprintf(buffer, sizeof(buffer), "Pattern found at: 0x%llX", result);
            OutputDebugStringA(buffer);
            DWORD64* resultDWORD = reinterpret_cast<DWORD64*>(result);
            return resultDWORD;
        }
        else {
            OutputDebugStringA("Pattern not found in module.");
        }
    }
    else {
        OutputDebugStringA("Failed to get module information.");
    }
    return nullptr;
}

bool Utils::Detour(void* toHook, void* ourFunct, int len)
{
    if (len < 14) {
        return false;
    }

    DWORD oldProtection;
    VirtualProtect(toHook, len, PAGE_EXECUTE_READWRITE, &oldProtection);

    memset(toHook, 0x90, len);

    uint8_t* p = (uint8_t*)toHook;

    HMODULE hModule = GetModuleHandleA("WINMM.dll");
    if (hModule == nullptr) {
        return false;
    }
    uint64_t targetAddress = (DWORD64)ourFunct;

    // Movabs RAX, targetAddress
    p[0] = 0x48; // REX.W prefix
    p[1] = 0xB8; // MOV RAX, imm64
    *(uint64_t*)(p + 2) = targetAddress;

    // JMP RAX
    p[10] = 0xFF; // Opcode for JMP RAX
    p[11] = 0xE0; // 

    DWORD temp;
    VirtualProtect(toHook, len, oldProtection, &temp);

    return true;
}

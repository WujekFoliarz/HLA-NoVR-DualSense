#pragma once
#include <vector>
#include <string>

class Utils
{
public:
    static DWORD64* scanModuleMemory(HMODULE hModule, const std::string& pattern);
    static std::vector<uint8_t> patternToByteArray(const std::string& pattern, std::vector<bool>& mask);
    static uintptr_t aobscan(const BYTE* baseAddress, SIZE_T size, const std::vector<uint8_t>& bytePattern, const std::vector<bool>& mask);
    static bool Detour(void* toHook, void* ourFunct, int len);
};
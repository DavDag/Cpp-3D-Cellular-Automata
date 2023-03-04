#include "hwinfo.hpp"

#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <imgui.h>

#include <thread>
#include <vector>
#include <array>
#include <iostream>

#define BYTES_TO_MB_DOUBLE (1024.0 * 1024.0)

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <intrin.h>
#include <psapi.h>
#include <winternl.h>
#include <pdh.h>
#include <pdhmsg.h>
#endif

//
#ifdef _WIN32
HANDLE hCurrentProcess;
DWORD currentProcessId;
PDH_HQUERY gpuUsageQuery;
HCOUNTER gpuUsageCounter;
#endif // _WIN32


// ====================================
// OpenGL
// ====================================

const char* hwinfo::opengl::version() {
    static bool __computed = false;
    static const char* result;
    if (__computed) return result;
    __computed = true;
    //
    result = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    return result;
}

// ====================================
// GPU
// ====================================

const char* hwinfo::gpu::renderer() {
    static bool __computed = false;
    static const char* result;
    if (__computed) return result;
    __computed = true;
    //
    result = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    return result;
}

double hwinfo::gpu::usage() {
#ifdef _WIN32
    static DWORD bufferSize = sizeof(PDH_FMT_COUNTERVALUE_ITEM) * 128, itemCount = 0;
    static PDH_FMT_COUNTERVALUE_ITEM* pdhItems = (PDH_FMT_COUNTERVALUE_ITEM*) new unsigned char[bufferSize];
    // https://askldjd.wordpress.com/2011/01/05/a-pdh-helper-class-cpdhquery/
    PDH_STATUS Status = ERROR_SUCCESS;
    double percent = -1;
    while(true && gpuUsageQuery) {
        Status = PdhCollectQueryData(gpuUsageQuery);
        if (Status != ERROR_SUCCESS) break;
        //
        //DWORD bufferSize = 0, itemCount = 0;
        //PdhGetFormattedCounterArray(gpuUsageCounter, PDH_FMT_DOUBLE, &bufferSize, &itemCount, NULL);
        //if (Status != PDH_MORE_DATA) break;
        //
        PdhGetFormattedCounterArray(gpuUsageCounter, PDH_FMT_DOUBLE, &bufferSize, &itemCount, pdhItems);
        if (Status != ERROR_SUCCESS) break;
        percent = 0.0;
        for (int i = 0; i < itemCount; i++)
            percent += pdhItems[i].FmtValue.doubleValue;
        //
        break;
    }
    if (Status != ERROR_SUCCESS) printf("PDH error: 0x%x", Status);
    return percent;
#endif // _WIN32
}

const char* hwinfo::gpu::vendor() {
    static bool __computed = false;
    static const char* result;
    if (__computed) return result;
    __computed = true;
    //
    result = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    return result;
}

// ====================================
// CPU
// ====================================

int hwinfo::cpu::threadCount() {
    static bool __computed = false;
    static int result;
    if (__computed) return result;
    __computed = true;
    //
    result = std::thread::hardware_concurrency();
    return result;
}

const char* hwinfo::cpu::vendor() {
    static bool __computed = false;
    static char result[32];
    if (__computed) return result;
    __computed = true;
#ifdef _WIN32
    // https://learn.microsoft.com/en-us/cpp/intrinsics/cpuid-cpuidex?view=msvc-170
    int nIds;
    std::array<int, 4> cpui;
    std::vector<std::array<int, 4>> data;

    // Calling __cpuid with 0x0 as the function_id argument
    // gets the number of the highest valid function ID.
    __cpuid(cpui.data(), 0);
    nIds = cpui[0];
    for (int i = 0; i <= nIds; ++i) {
        __cpuidex(cpui.data(), i, 0);
        data.push_back(cpui);
    }

    // Capture vendor string
    char vendor[32];
    memset(vendor, 0, sizeof(vendor));
    *reinterpret_cast<int*>(vendor) = data[0][1];
    *reinterpret_cast<int*>(vendor + 4) = data[0][3];
    *reinterpret_cast<int*>(vendor + 8) = data[0][2];

    memcpy(result, vendor, sizeof(char) * 32);
    return result;
#endif // _WIN32
}

const char* hwinfo::cpu::brand() {
    static bool __computed = false;
    static char result[64] = "<undefined>";
    if (__computed) return result;
    __computed = true;
#ifdef _WIN32
    // https://learn.microsoft.com/en-us/cpp/intrinsics/cpuid-cpuidex?view=msvc-170
    int nExIds;
    std::array<int, 4> cpui;
    std::vector<std::array<int, 4>> extdata;

    // Calling __cpuid with 0x80000000 as the function_id argument
    // gets the number of the highest valid extended ID.
    __cpuid(cpui.data(), 0x80000000);
    nExIds = cpui[0];
    for (int i = 0x80000000; i <= nExIds; ++i) {
        __cpuidex(cpui.data(), i, 0);
        extdata.push_back(cpui);
    }

    // Interpret CPU brand string if reported
    if (nExIds >= 0x80000004) {
        static char brand[64];
        memset(brand, 0, sizeof(brand));
        memcpy(brand, extdata[2].data(), sizeof(cpui));
        memcpy(brand + 16, extdata[3].data(), sizeof(cpui));
        memcpy(brand + 32, extdata[4].data(), sizeof(cpui));
        memcpy(result, brand, sizeof(char) * 64);
    }

    return result;
#endif // _WIN32
}

double hwinfo::cpu::usage(double cores[]) {
#if _WIN32
    // https://stackoverflow.com/questions/53306819/accurate-system-cpu-usage-in-windows
    static PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION lastValues = new SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION[hwinfo::cpu::threadCount()];
    static PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION newValues = new SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION[hwinfo::cpu::threadCount()];
    //
    ULONG size;
    NtQuerySystemInformation(SystemProcessorPerformanceInformation, newValues, sizeof(newValues[0]) * hwinfo::cpu::threadCount(), &size);
    double percent = 0.0;
    for (DWORD i = 0; i < hwinfo::cpu::threadCount(); ++i) {
        double current_percent = (newValues[i].IdleTime.QuadPart - lastValues[i].IdleTime.QuadPart);
        current_percent /= ((newValues[i].KernelTime.QuadPart + newValues[i].UserTime.QuadPart) - (lastValues[i].KernelTime.QuadPart + lastValues[i].UserTime.QuadPart));
        current_percent = 1.0 - current_percent;
        current_percent *= 100.0;
        cores[i] = current_percent;
        percent += current_percent;
    }
    //
    memcpy(lastValues, newValues, sizeof(newValues[0]) * hwinfo::cpu::threadCount());
    return percent / hwinfo::cpu::threadCount();
#endif // _WIN32
}

// ====================================
// Memory
// ====================================

double hwinfo::mem::usageMb() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS_EX pmc;
    int succeded = (GetProcessMemoryInfo(hCurrentProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc)) != 0);
    return (succeded) ? pmc.PrivateUsage / BYTES_TO_MB_DOUBLE : -1;
#endif // _WIN32
}

double hwinfo::mem::availableMb() {
#ifdef _WIN32
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    return memInfo.ullAvailPhys / BYTES_TO_MB_DOUBLE;
#endif // _WIN32
}

double hwinfo::mem::physicalTotMb() {
    static bool __computed = false;
    static double result = -1;
    if (__computed) return result;
    __computed = true;
#ifdef _WIN32
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    result = memInfo.ullTotalPhys / BYTES_TO_MB_DOUBLE;
#endif // _WIN32
    return result;
}

// ====================================
// Dependencies
// ====================================

const char* hwinfo::deps::glfwVersion() {
    static bool __computed = false;
    static const char* result;
    if (__computed) return result;
    __computed = true;
    //
    result = glfwGetVersionString();
    return result;
}

const char* hwinfo::deps::glewVersion() {
    static bool __computed = false;
    static const char* result;
    if (__computed) return result;
    __computed = true;
    //
    result = reinterpret_cast<const char*>(glewGetString(GLEW_VERSION));
    return result;
}

const char* hwinfo::deps::imguiVersion() {
    static bool __computed = false;
    static const char* result;
    if (__computed) return result;
    __computed = true;
    //
    result = IMGUI_VERSION;
    return result;
}

// ====================================
// Internal
// ====================================

void hwinfo::init() {
#ifdef _WIN32
    hCurrentProcess = GetCurrentProcess();
    currentProcessId = GetCurrentProcessId();
    //
    PDH_STATUS Status;
    while (true) {
        Status = PdhOpenQuery(NULL, NULL, &gpuUsageQuery);
        if (Status != ERROR_SUCCESS) break;
        //
        WCHAR gpuUsageQueryBuffer[PDH_MAX_COUNTER_PATH];
        wsprintf(gpuUsageQueryBuffer, L"\\GPU Engine(pid_%d*)\\Utilization Percentage", currentProcessId);
        Status = PdhAddCounter(gpuUsageQuery, gpuUsageQueryBuffer, 0, &gpuUsageCounter);
        if (Status != ERROR_SUCCESS) break;
        //
        Status = PdhCollectQueryData(gpuUsageQuery);
        if (Status != ERROR_SUCCESS) break;
        //
        break;
    }
    if (Status != ERROR_SUCCESS) printf("PDH error: 0x%x\n", Status);
#endif // _WIN32
}

void hwinfo::exit() {
#if _WIN32
    if (gpuUsageQuery) PdhCloseQuery(gpuUsageQuery);
#endif // _WIN32
}

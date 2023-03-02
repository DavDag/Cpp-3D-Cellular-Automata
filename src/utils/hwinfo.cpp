#include "hwinfo.hpp"

#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <imgui.h>

#include <intrin.h>
#include <thread>
#include <vector>
#include <array>
#include <iostream>

const char* runtimeinfo::opengl::version() {
    return reinterpret_cast<const char*>(glGetString(GL_VERSION));
}

const char* runtimeinfo::gpu::renderer() {
    return reinterpret_cast<const char*>(glGetString(GL_RENDERER));
}

const char* runtimeinfo::gpu::vendor() {
    return reinterpret_cast<const char*>(glGetString(GL_VENDOR));
}

int runtimeinfo::cpu::threadCount() {
    return std::thread::hardware_concurrency();
}

const char* runtimeinfo::cpu::vendor() {
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

    return vendor;
#endif
}

const char* runtimeinfo::cpu::brand() {
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
        char brand[64];
        memset(brand, 0, sizeof(brand));
        memcpy(brand, extdata[2].data(), sizeof(cpui));
        memcpy(brand + 16, extdata[3].data(), sizeof(cpui));
        memcpy(brand + 32, extdata[4].data(), sizeof(cpui));
        return brand;
    }

    return nullptr;
#endif
}

const char* runtimeinfo::deps::glfwVersion() {
    return glfwGetVersionString();
}

const char* runtimeinfo::deps::glewVersion() {
    return reinterpret_cast<const char*>(glewGetString(GLEW_VERSION));
}

const char* runtimeinfo::deps::imguiVersion() {
    return IMGUI_VERSION;
}

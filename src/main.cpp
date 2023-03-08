#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <iostream>

#include "./utils/hwinfo.hpp"
#include "app.hpp"

//#define RES_FHD
#define RES_4K

void errorCallback(int errorCode, const char* description);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseBtnCallback(GLFWwindow* window, int key, int action, int mods);
void scrollCallback(GLFWwindow* window, double dx, double dy);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);

int initGLFW(GLFWwindow*& window, int w, int h, App& app);
int initGLEW();
int initImGui(GLFWwindow* window);

void exitGLFW(GLFWwindow*& window);
void exitGLEW();
void exitImGui();

int main(int argc, char* argv[]);
int WinMain(int argc, char* argv[]) { return main(argc, argv); }

int main(int argc, char* argv[]) {
    hwinfo::init();
    atexit(hwinfo::exit);

#ifdef RES_FHD
    int w = 1080, h = 720;
#endif // RES_FHD
#ifdef RES_4K
    int w = 1920, h = 1080;
#endif // RES_4K
    App app;

    // Init
    GLFWwindow* window;
    if (int err = initGLFW(window, w, h, app); err != 0) {
        std::cerr << "Error initializing GLFW, <code> = " << err << "\n";
        exit(err);
    }
    if (int err = initGLEW(); err != GLEW_OK) {
        std::cerr << "Error initializing GLEW, <code> = " << glewGetErrorString(err) << "\n";
        exit(err);
    }
    if (int err = initImGui(window); err != 0) {
        std::cerr << "Error initializing ImGui, <code> = " << err << "\n";
        exit(err);
    }

    app.initialize();

    // Main loop
    glfwSwapInterval(1);
    double lastFrameTimeSec = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        glfwGetFramebufferSize(window, &w, &h);
        glfwPollEvents();
        // update
        double now = glfwGetTime();
        double dtSec = (now - lastFrameTimeSec);
        lastFrameTimeSec = now;
        app.update(dtSec);
        // render
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::SetShortcutRouting(ImGuiMod_Ctrl | ImGuiKey_Tab, ImGuiKeyOwner_None);
        ImGui::SetShortcutRouting(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Tab, ImGuiKeyOwner_None);
        app.render(w, h);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // End
    exitImGui();
    exitGLEW();
    exitGLFW(window);
    exit(0);
}

// ==========================================================================================
// GLFW
// ==========================================================================================

int initGLFW(GLFWwindow*& window, int w, int h, App& app) {
    glfwSetErrorCallback(errorCallback);
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 4.5+ only
    window = glfwCreateWindow(w, h, "3D Cellular Automata", NULL, NULL);
    if (!window) return 2;
    glfwSetWindowUserPointer(window, &app);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseBtnCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwMakeContextCurrent(window);
    return 0;
}

void exitGLFW(GLFWwindow*& window) {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void errorCallback(int errorCode, const char* description) {
    std::cerr << "GLFW error, <code> = " << errorCode << " : " << description << "\n";
    exit(errorCode);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard) return;
    App& app = *(App*)glfwGetWindowUserPointer(window);
    if (action == GLFW_PRESS) app.onKeyDown(key);
    if (action == GLFW_RELEASE) app.onKeyUp(key);
    if (action == GLFW_REPEAT) app.onKeyDown(key); // TODO: Improve ?
}

void mouseBtnCallback(GLFWwindow* window, int key, int action, int mods) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;
    App& app = *(App*)glfwGetWindowUserPointer(window);
    if (action == GLFW_PRESS) app.onMouseBtnDown(key);
    if (action == GLFW_RELEASE) app.onMouseBtnUp(key);
}

void scrollCallback(GLFWwindow* window, double dx, double dy) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;
    App& app = *(App*)glfwGetWindowUserPointer(window);
    app.onMouseWheel(dx, dy);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    App& app = *(App*)glfwGetWindowUserPointer(window);
    app.onResize(width, height);
}

// ==========================================================================================
// GLEW
// ==========================================================================================

int initGLEW() {
    GLenum err = glewInit();
    if (GLEW_OK != err) return err;
    return GLEW_OK;
}

void exitGLEW() {
    //
}

// ==========================================================================================
// IMGUI
// ==========================================================================================

int initImGui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) return 1;
    if (!ImGui_ImplOpenGL3_Init("#version 450")) return 2;
    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig cfg;
#ifdef RES_FHD
    cfg.SizePixels = 16;
#endif // RES_FHD
#ifdef RES_4K
    cfg.SizePixels = 24;
#endif // RES_4K
    io.Fonts->AddFontDefault(&cfg);
    return 0;
}

void exitImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

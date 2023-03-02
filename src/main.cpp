#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <iostream>

#include "app/app.hpp"
#include "utils/hwinfo.hpp"

void errorCallback(int errorCode, const char* description);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseBtnCallback(GLFWwindow* window, int key, int action, int mods);
void scrollCallback(GLFWwindow* window, double dx, double dy);

int initGLFW(GLFWwindow*& window, App& app);
int initGLEW();
int initImGui(GLFWwindow* window);

void exitGLFW(GLFWwindow*& window);
void exitGLEW();
void exitImGui();

#ifdef _CONSOLE
int main(int argc, char* argv[])
#elif _WIN32
int WinMain(int argc, char* argv[])
#endif
{
    App app;

    // Init
    GLFWwindow* window;
    if (int err = initGLFW(window, app); err != 0) {
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

    // Main loop
    glfwSwapInterval(1);
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        app.render();
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

int initGLFW(GLFWwindow*& window, App& app) {
    glfwSetErrorCallback(errorCallback);
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 4.5+ only
    window = glfwCreateWindow(1920, 1080, "3D Cellular Automata", NULL, NULL);
    if (!window) return 2;
    glfwSetWindowUserPointer(window, &app);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseBtnCallback);
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
    App& app = *(App*)glfwGetWindowUserPointer(window);
    if (action == GLFW_PRESS) app.onKeyDown(key);
    if (action == GLFW_RELEASE) app.onKeyUp(key);
}

void mouseBtnCallback(GLFWwindow* window, int key, int action, int mods) {
    App& app = *(App*)glfwGetWindowUserPointer(window);
    if (action == GLFW_PRESS) app.onMouseBtnDown(key);
    if (action == GLFW_RELEASE) app.onMouseBtnUp(key);
}

void scrollCallback(GLFWwindow* window, double dx, double dy) {
    App& app = *(App*)glfwGetWindowUserPointer(window);
    app.onMouseWheel(dx, dy);
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
    cfg.SizePixels = 32;
    io.Fonts->AddFontDefault(&cfg);
    return 0;
}

void exitImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

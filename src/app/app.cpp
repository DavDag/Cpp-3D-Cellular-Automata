#include "app.hpp"
#include "../utils/hwinfo.hpp"

#include <GLFW/glfw3.h>
#include <gl/glew.h>

#include <string>

App::App()
	: _console(*this, 128, 256, false)
{
	this->_lastFrameTimeSec = 0;
}

void App::render(int w, int h) {
	//
	double now = glfwGetTime();
	double dtSec = (now - this->_lastFrameTimeSec);
	this->_lastFrameTimeSec = now;
	//
	glClear(GL_COLOR_BUFFER_BIT);
	this->_console.render(w, h);
	// this->_console.log("abcd");
}

void App::onKeyDown(int key) {
	if (key == GLFW_KEY_TAB) this->_console.toggle();
}

void App::onKeyUp(int key) {

}

void App::onMouseBtnDown(int btn) {

}

void App::onMouseBtnUp(int btn) {

}

void App::onMouseWheel(double dx, double dy) {

}

void App::onResize(int width, int height) {
	glViewport(0, 0, width, height);
}

void App::executeCmd(const char* cmd) {
	this->_console.log(cmd);

	// ==================================================
	// deps
	// retrieve dependencies versions
	if (strcmp("deps", cmd) == 0) {
		this->_console.log(
			"GLFW: %s\nGLEW: %s\nIMGUI: %s",
			runtimeinfo::deps::glfwVersion(),
			runtimeinfo::deps::glewVersion(),
			runtimeinfo::deps::imguiVersion()
		);
	}

	// ==================================================
	// hwinfo
	// retrieve hardware info (opengl, gpu & cpu)
	if (strcmp("hwinfo", cmd) == 0) {
		this->_console.log(
			"OpenGL: %s\nGPU: %s\nCPU: %s\nThreads: %d",
			runtimeinfo::opengl::version(),
			runtimeinfo::gpu::renderer(),
			runtimeinfo::cpu::brand(),
			runtimeinfo::cpu::threadCount()
		);
	}
}


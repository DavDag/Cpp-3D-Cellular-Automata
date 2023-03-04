#include "app.hpp"
#include "../utils/hwinfo.hpp"

#include <GLFW/glfw3.h>
#include <gl/glew.h>

#include <string>

App::App()
	: _console(*this, 128, 256, false), _display(*this, 0.5)
{
	this->_showUI = false;
}

void App::update(double dtSec) {
	this->_console.update(dtSec);
	this->_display.update(dtSec);
}

void App::render(int w, int h) {
	glClear(GL_COLOR_BUFFER_BIT);
	//
	if (this->_showUI) {
		this->_console.render(w, h);
		this->_display.render(w, h);
	}
}

void App::onKeyDown(int key) {
	if (key == GLFW_KEY_TAB) this->_showUI = !this->_showUI;
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

	// Parse cmd, substitute ' ' with '\0'
	// and "create" a string list.

	// ==================================================
	// deps
	// retrieve dependencies versions
	if (strcmp("deps", cmd) == 0) {
		this->_console.log(
			"GLFW: %s\nGLEW: %s\nIMGUI: %s",
			hwinfo::deps::glfwVersion(),
			hwinfo::deps::glewVersion(),
			hwinfo::deps::imguiVersion()
		);
	}

	// ==================================================
	// hwinfo
	// retrieve hardware info (opengl, gpu & cpu)
	if (strcmp("hwinfo", cmd) == 0) {
		this->_console.log(
			"OpenGL: %s\nGPU: %s | %s\nCPU: %s | %s\nThreads: %d\nRAM: %.2f (GB)",
			hwinfo::opengl::version(),
			hwinfo::gpu::vendor(), hwinfo::gpu::renderer(),
			hwinfo::cpu::vendor(), hwinfo::cpu::brand(),
			hwinfo::cpu::threadCount(),
			hwinfo::mem::physicalTotMb() / 1024
		);
	}
}


#include "app.hpp"

#include <GLFW/glfw3.h>
#include <gl/glew.h>

App::App()
	: _console(100, 50, false)
{
}

void App::render() {
	glClear(GL_COLOR_BUFFER_BIT);
	this->_console.render();
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


#pragma once

#include "./utils/console.hpp"
#include "./utils/usagedisplay.hpp"
#include "./command/command.hpp"
#include "./camera/camera.hpp"
#include "./simulation/simulation.hpp"

#include <imgui.h>
#include <vector>

class App {
public:
	App();
	//
	const glm::mat4& camera();
	//
	void initialize();
	void render(int w, int h);
	void update(double dtSec);
	//
	void onKeyDown(int key);
	void onKeyUp(int key);
	void onMouseBtnDown(int btn);
	void onMouseBtnUp(int btn);
	void onMouseWheel(double dx, double dy);
	void onResize(int w, int h);
	//
	void parse(const char* cmd);
	void execute(int type, CommandArgs* args);
	//
	void raw(const char* fmt, ...);
	void inf(const char* fmt, ...);
	void deb(const char* fmt, ...);
	void err(const char* fmt, ...);

private:
	void __log(ImU32 col, const char* fmt, va_list args);

private:
	bool _showUI;
	std::vector<Command*> _commands;
	Console _console;
	UsageDisplay _display;
	//
	int _cameraAngleX, _cameraAngleY;
	Camera _camera;
	Simulation _simulation;
};


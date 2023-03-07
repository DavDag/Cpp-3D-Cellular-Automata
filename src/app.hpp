#pragma once

#include "./utils/console.hpp"
#include "./utils/usagedisplay.hpp"
#include "./simulation/simulation.hpp"
#include "./command/command.hpp"

#include <vector>

class App {
public:
	App();

	void initialize();
	void render(int w, int h);
	void update(double dtSec);

	void onKeyDown(int key);
	void onKeyUp(int key);
	void onMouseBtnDown(int btn);
	void onMouseBtnUp(int btn);
	void onMouseWheel(double dx, double dy);
	void onResize(int w, int h);

	void executeCmd(const char* cmd);
	void execute(int type, CommandArgs* args);

private:
	bool _showUI;
	std::vector<Command*> _commands;
	Console _console;
	UsageDisplay _display;
	//
	Simulation _simulation;
};


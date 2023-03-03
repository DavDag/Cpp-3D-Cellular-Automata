#pragma once

#include "../utils/console.hpp"
#include "../utils/usagedisplay.hpp"

class App {
public:
	App();
	void render(int w, int h);
	void update(double dtSec);

	void onKeyDown(int key);
	void onKeyUp(int key);
	void onMouseBtnDown(int btn);
	void onMouseBtnUp(int btn);
	void onMouseWheel(double dx, double dy);
	void onResize(int w, int h);

	void executeCmd(const char* cmd);

private:
	bool _showUI;
	UsageDisplay _display;
	Console _console;
};


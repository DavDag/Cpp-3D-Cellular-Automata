#pragma once

#include "../utils/imgui_console.h"

class App {
public:
	App();
	void render(int w, int h);

	void onKeyDown(int key);
	void onKeyUp(int key);
	void onMouseBtnDown(int btn);
	void onMouseBtnUp(int btn);
	void onMouseWheel(double dx, double dy);
	void onResize(int w, int h);

	void executeCmd(const char* cmd);

private:
	double _lastFrameTimeSec;
	ImGuiConsole _console;
};


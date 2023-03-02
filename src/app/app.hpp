#pragma once

#include "../utils/imgui_console.h"

class App {
public:
	App();
	void render();

	void onKeyDown(int key);
	void onKeyUp(int key);
	void onMouseBtnDown(int btn);
	void onMouseBtnUp(int btn);
	void onMouseWheel(double dx, double dy);

private:
	ImGuiConsole _console;
};


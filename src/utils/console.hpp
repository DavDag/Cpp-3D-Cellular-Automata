#pragma once

#include <imgui.h>

class App;

struct LineData {
	char* buffer;
	ImU32 color;
};

class Console {
public:
	Console(App& app, int rowCount, int rowLenght, bool autowrap);
	//
	void initialize();
	void update(double dtSec);
	void render(int w, int h);
	//
	void log(ImU32 col, const char* line);

private:
	App& _app;
	bool _autowrap;
	int _rowCount, _rowLenght, _currentRow;
	LineData* _lines;
	char* _cmdBuffer;
};

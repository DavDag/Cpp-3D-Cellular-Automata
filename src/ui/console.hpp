#pragma once

#include <imgui.h>

class App;

struct LineData {
	enum Type { NONE = 0, INFO = 1, ERROR = 2, DEBUG = 3 } type = NONE;
	ImU32 color = IM_COL32_WHITE;
	char* buffer = nullptr;
	int repeatcount = 0;
};

class Console {
public:
	Console(App& app, int rowCount, int rowLenght, bool autowrap);
	//
	void initialize();
	void update(double dtSec);
	void render(int w, int h);
	//
	void log(LineData::Type type, ImU32 col, const char* line);
	//
	int getHistorySize();
	const char* getHistoryCmd(int delta);

private:
	App& _app;
	bool _autowrap;
	int _rowCount, _rowLenght, _currentRow;
	LineData* _lines;
	//
	char* _cmdBuffer;
	int _cmdHistoryIndexNextFree, _cmdHistorySize;
	char** _cmdHistory;
	int _cmdHistoryIndex;
};

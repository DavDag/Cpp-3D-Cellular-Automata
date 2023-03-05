#pragma once

class App;

class Console {
public:
	Console(App& app, int rowCount, int rowLenght, bool autowrap);
	void update(double dtSec);
	void render(int w, int h);
	void log(const char* fmt, ...);

private:
	App& _app;
	bool _autowrap;
	int _rowCount, _rowLenght, _currentRow;
	char** _lines;
	char* _cmdBuffer;
};

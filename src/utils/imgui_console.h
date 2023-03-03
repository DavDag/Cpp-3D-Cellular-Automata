#pragma once

class App;

class ImGuiConsole {
public:
	ImGuiConsole(App& app, int rowCount, int rowLenght, bool autowrap);
	void render(int w, int h);

	void toggle();
	void hide();
	void show();

	void log(const char* fmt, ...);

private:
	App& _app;
	bool _shown;
	bool _autowrap; // TODO
	int _rowCount, _rowLenght, _currentRow;
	char* _buffer;
	char* _cmdBuffer;
};

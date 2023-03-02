#pragma once

class ImGuiConsole {
public:
	ImGuiConsole(int rowCount, int rowLenght, bool autowrap);
	void render();

	void toggle();
	void hide();
	void show();

	void log(const char* line, bool inplace = false);

private:
	bool _shown;
	bool _autowrap; // TODO
	int _rowCount, _rowLenght, _currentRow;
	char* _buffer;
	char* _cmdBuffer;
};

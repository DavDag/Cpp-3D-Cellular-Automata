#include "imgui_console.h"

#include <stdio.h>
#include <imgui.h>

#define CMD_BUFFER_LEN 256

ImGuiConsole::ImGuiConsole(int rowCount, int rowLenght, bool autowrap) {
	this->_shown = true;
	this->_autowrap = autowrap;
	this->_rowCount = rowCount;
	this->_rowLenght = rowLenght;
	this->_currentRow = 0;
	this->_buffer = new char[_rowCount * _rowLenght];
	this->_cmdBuffer = new char[CMD_BUFFER_LEN];
	memset(this->_buffer, ' ', sizeof(char) * _rowCount * _rowLenght);
	memset(this->_cmdBuffer, '\0', sizeof(char) * CMD_BUFFER_LEN);
}

void ImGuiConsole::render() {
	//ImGui::ShowDemoWindow(nullptr);
	if (this->_shown) {
		ImGui::Begin("Console", &this->_shown, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysVerticalScrollbar);
		//
		ImGui::PushItemWidth(-1);
		if (ImGui::InputText("##cmdline", this->_cmdBuffer, CMD_BUFFER_LEN, ImGuiInputTextFlags_EnterReturnsTrue)) {
			this->log(this->_cmdBuffer);
			memset(this->_cmdBuffer, '\0', sizeof(char) * CMD_BUFFER_LEN);
		}
		ImGui::PopItemWidth();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		ImGuiListClipper clipper;
		clipper.Begin(this->_rowCount);
		while (clipper.Step())
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
				int j = (this->_currentRow - i + this->_rowCount - 1) % this->_rowCount;
				const char* rowBeg = &(this->_buffer[j * this->_rowLenght]);
				const char* rowEnd = rowBeg + this->_rowLenght;
				ImGui::TextUnformatted(rowBeg, rowEnd);
			}
		ImGui::PopStyleVar();
		//
		ImGui::End();
	}
}

void ImGuiConsole::toggle() {
	this->_shown = !this->_shown;
}

void ImGuiConsole::hide() {
	this->_shown = false;
}

void ImGuiConsole::show() {
	this->_shown = true;
}

void ImGuiConsole::log(const char* line, bool inplace /* = false */) {
	char* beg = &(this->_buffer[this->_currentRow * this->_rowLenght]);
	memset(beg, ' ', this->_rowLenght);
	int n = snprintf(beg, this->_rowLenght, "%d: %s", this->_currentRow, line);
	this->_currentRow = (this->_currentRow + 1) % this->_rowCount;
}

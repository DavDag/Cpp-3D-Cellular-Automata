#include "console.hpp"
#include "../app.hpp"

#include <stdio.h>
#include <imgui.h>

Console::Console(App& app, int rowCount, int rowLenght, bool autowrap):
	_app(app)
{
	this->_autowrap = autowrap;
	this->_rowCount = rowCount;
	this->_rowLenght = rowLenght;
	this->_currentRow = 0;
	this->_lines = new char*[_rowCount];
	this->_cmdBuffer = new char[_rowLenght];
	char* buffer = new char[_rowCount * _rowLenght];
	memset(buffer, '\0', sizeof(char) * _rowCount * _rowLenght);
	for (int i = 0; i < _rowCount; ++i)
		this->_lines[i] = &buffer[i * _rowLenght];
	memset(this->_cmdBuffer, '\0', sizeof(char) * _rowLenght);
}

void Console::initialize() {

}

void Console::update(double dtSec) {

}

void Console::render(int w, int h) {
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(w * 0.3f, (float) h), ImGuiCond_Always);
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoSavedSettings
		| ImGuiWindowFlags_AlwaysVerticalScrollbar;
	ImGui::Begin("Console", nullptr, windowFlags);
	//
	ImGui::Checkbox("Autowrap", &this->_autowrap);
	ImGui::PushItemWidth(w * 0.3f);
	ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue;
	//if (ImGui::IsWindowAppearing())
	//	ImGui::SetKeyboardFocusHere();
	if (ImGui::InputText("##cmdline", this->_cmdBuffer, this->_rowLenght, inputFlags)) {
		this->_app.executeCmd(this->_cmdBuffer);
		memset(this->_cmdBuffer, '\0', sizeof(char) * this->_rowLenght);
		ImGui::SetKeyboardFocusHere(-1);
	}
	ImGui::PopItemWidth();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 2));
	float wrapPos = (this->_autowrap) ? w * 0.3f : ImGui::GetFontSize() * this->_rowLenght;
	ImGui::PushTextWrapPos(wrapPos);
	ImGuiListClipper clipper;
	clipper.Begin(this->_rowCount, ImGui::GetFontSize());
	while (clipper.Step())
		for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
			int j = (this->_currentRow - i + this->_rowCount - 1) % this->_rowCount;
			const char* rowBeg = this->_lines[j];
			const char* rowEnd = rowBeg + strlen(this->_lines[j]);
			//
			ImGuiIO& io = ImGui::GetIO();
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			ImVec2 bgRect = ImGui::CalcTextSize(rowBeg, rowEnd, false, wrapPos);
			ImVec2 pos = ImGui::GetCursorScreenPos();
			draw_list->AddRectFilled(pos, ImVec2(pos.x + wrapPos, pos.y + bgRect.y), IM_COL32(128, 128, 128, 32 * (j % 2 + 1)));
			//
			ImGui::TextUnformatted(rowBeg, rowEnd);
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
				ImGui::SetTooltip("Right click to copy text");
			if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
				ImGui::SetClipboardText(rowBeg);
		}
	ImGui::PopTextWrapPos();
	ImGui::PopStyleVar();
	//
	ImGui::End();
}

void Console::log(const char* fmt, ...) {
	// TODO: Colors
	char* rowBeg = this->_lines[this->_currentRow];
	memset(rowBeg, '\0', this->_rowLenght);
	//
	va_list args;
	va_start(args, fmt);
	int n = vsnprintf(rowBeg, this->_rowLenght, fmt, args);
	va_end(args);
	//
	this->_currentRow = (this->_currentRow + 1) % this->_rowCount;
}

void Console::err(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	this->log(fmt, args);
	va_end(args);
}

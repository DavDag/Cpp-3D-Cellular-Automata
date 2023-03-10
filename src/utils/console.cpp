#include "console.hpp"
#include "../app.hpp"

#include <stdarg.h>
#include <stdio.h>
#include <imgui.h>

Console::Console(App& app, int rowCount, int rowLenght, bool autowrap):
	_app(app)
{
	this->_autowrap = autowrap;
	this->_rowCount = rowCount;
	this->_rowLenght = rowLenght;
	this->_currentRow = 0;
	this->_lines = new LineData[_rowCount];
	for (int i = 0; i < _rowCount; ++i) {
		LineData data = {
			.buffer = new char[_rowLenght],
			.color = IM_COL32(255, 255, 255, 255),
		};
		memset(data.buffer, '\0', sizeof(char) * _rowLenght);
		this->_lines[i] = data;
	}
	this->_cmdBuffer = new char[_rowLenght];
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
		this->_app.parse(this->_cmdBuffer);
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
			const char* rowBeg = this->_lines[j].buffer;
			const char* rowEnd = rowBeg + strlen(rowBeg);
			//
			ImGuiIO& io = ImGui::GetIO();
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			ImVec2 bgRect = ImGui::CalcTextSize(rowBeg, rowEnd, false, wrapPos);
			ImVec2 pos = ImGui::GetCursorScreenPos();
			draw_list->AddRectFilled(pos, ImVec2(pos.x + wrapPos, pos.y + bgRect.y), IM_COL32(128, 128, 128, 32 * (j % 2 + 1)));
			//
			ImGui::PushStyleColor(ImGuiCol_Text, this->_lines[j].color);
			ImGui::TextUnformatted(rowBeg, rowEnd);
			ImGui::PopStyleColor();
			//
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

void Console::log(ImU32 col, const char* line) {
	// TODO: Colors
	this->_lines[this->_currentRow].color = col;
	char* rowBeg = this->_lines[this->_currentRow].buffer;
	memset(rowBeg, '\0', this->_rowLenght);
	strcpy_s(rowBeg, this->_rowLenght, line);
	// next row (ring-buffer)
	this->_currentRow = (this->_currentRow + 1) % this->_rowCount;
}

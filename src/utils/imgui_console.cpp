#include "imgui_console.h"
#include "../app/app.hpp"

#include <stdio.h>
#include <imgui.h>

ImGuiConsole::ImGuiConsole(App& app, int rowCount, int rowLenght, bool autowrap)
	: _app(app)
{
	this->_shown = true;
	this->_autowrap = autowrap;
	this->_rowCount = rowCount;
	this->_rowLenght = rowLenght;
	this->_currentRow = 0;
	this->_buffer = new char[_rowCount * _rowLenght];
	this->_cmdBuffer = new char[_rowLenght];
	memset(this->_buffer, ' ', sizeof(char) * _rowCount * _rowLenght);
	memset(this->_cmdBuffer, '\0', sizeof(char) * _rowLenght);
}

void ImGuiConsole::render(int w, int h) {
	//ImGui::ShowDemoWindow(nullptr);
	if (this->_shown) {
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(w * 0.3, h), ImGuiCond_Always);
		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse
			| ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_AlwaysVerticalScrollbar
			| ImGuiWindowFlags_NoSavedSettings;
		ImGui::Begin("Console", &this->_shown, windowFlags);
		//
		ImGui::Checkbox("Autowrap", &this->_autowrap);
		ImGui::PushItemWidth(w * 0.3);
		ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue;
		if (ImGui::InputText("##cmdline", this->_cmdBuffer, this->_rowLenght, inputFlags)) {
			this->_app.executeCmd(this->_cmdBuffer);
			memset(this->_cmdBuffer, '\0', sizeof(char) * this->_rowLenght);
			ImGui::SetKeyboardFocusHere(-1);
		}
		ImGui::PopItemWidth();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 2));
		float wrapPos = (this->_autowrap) ? w * 0.3 : ImGui::GetFontSize() * this->_rowLenght;
		ImGui::PushTextWrapPos(wrapPos);
		ImGuiListClipper clipper;
		clipper.Begin(this->_rowCount, ImGui::GetFontSize());
		while (clipper.Step())
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
				int j = (this->_currentRow - i + this->_rowCount - 1) % this->_rowCount;
				const char* rowBeg = &(this->_buffer[j * this->_rowLenght]);
				const char* rowEnd = rowBeg + this->_rowLenght;
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

void ImGuiConsole::log(const char* fmt, ...) {
	char* beg = &(this->_buffer[this->_currentRow * this->_rowLenght]);
	memset(beg, ' ', this->_rowLenght);
	//
	va_list args;
	va_start(args, fmt);
	int n = vsnprintf(beg, this->_rowLenght, fmt, args);
	va_end(args);
	//
	this->_currentRow = (this->_currentRow + 1) % this->_rowCount;
}

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
			.type = LineData::Type::NONE,
			.color = IM_COL32(255, 255, 255, 255),
			.buffer = new char[_rowLenght],
			.repeatcount = 0,
		};
		memset(data.buffer, '\0', sizeof(char) * _rowLenght);
		this->_lines[i] = data;
	}
	//
	this->_cmdBuffer = new char[_rowLenght];
	memset(this->_cmdBuffer, '\0', sizeof(char) * _rowLenght);
	this->_cmdHistoryIndexNextFree = 0;
	this->_cmdHistoryIndex = 0;
	this->_cmdHistorySize = 16;
	this->_cmdHistory = new char* [this->_cmdHistorySize];
	for (int i = 0; i < this->_cmdHistorySize; ++i) {
		this->_cmdHistory[i] = new char[_rowLenght];
		memset(this->_cmdHistory[i], '\0', sizeof(char) * _rowLenght);
	}
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
	float windowContentWidth = ImGui::GetWindowContentRegionWidth();
	//
	ImGui::Checkbox("Autowrap", &this->_autowrap);
	ImGui::PushItemWidth(windowContentWidth);
	ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory;
	//if (ImGui::IsWindowAppearing())
	//	ImGui::SetKeyboardFocusHere();
	ImGuiInputTextCallback textcallback = [](ImGuiInputTextCallbackData* data) -> int {
		Console& console = *((Console*) data->UserData);
		if (console.getHistorySize() == 0) return 0;
		int delta = 0;
		switch (data->EventFlag) {
			case ImGuiInputTextFlags_CallbackHistory:
				if (data->EventKey == ImGuiKey_DownArrow) delta = -1;
				else if (data->EventKey == ImGuiKey_UpArrow) delta = 1;
				break;
			default:
				return 0;
		}
		const char* cmd = console.getHistoryCmd(delta);
		data->DeleteChars(0, data->BufTextLen);
		data->InsertChars(0, cmd);
		return 0;
	};
	if (ImGui::InputText("##cmdline", this->_cmdBuffer, this->_rowLenght, inputFlags, textcallback, this)) {
		// save command into history
		char* historyline = this->_cmdHistory[(this->_cmdHistoryIndexNextFree % this->_cmdHistorySize)];
		memcpy(historyline, this->_cmdBuffer, this->_rowLenght);
		this->_cmdHistoryIndex = 0;
		this->_cmdHistoryIndexNextFree++;
		// send command to app
		this->_app.parse(this->_cmdBuffer);
		memset(this->_cmdBuffer, '\0', sizeof(char) * this->_rowLenght);
		ImGui::SetKeyboardFocusHere(-1);
	}
	ImGui::PopItemWidth();
	int extraspacey = 2;
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 2 * extraspacey));
	float wrapPos = (this->_autowrap) ? windowContentWidth : ImGui::GetFontSize() * this->_rowLenght;
	ImGui::PushTextWrapPos(wrapPos);
	ImGuiListClipper clipper;
	clipper.Begin(this->_rowCount, ImGui::GetTextLineHeight());
	while (clipper.Step())
		for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
			// Line data
			int j = (this->_currentRow - i + this->_rowCount - 1) % this->_rowCount;
			LineData linedata = this->_lines[j];
			const char* rowBeg = linedata.buffer;
			const char* rowEnd = rowBeg + strlen(rowBeg);
			// Row background
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImVec2 bgRect = ImGui::CalcTextSize(rowBeg, rowEnd, false, wrapPos);
			bgRect.y += 2 * extraspacey;
			ImVec2 pos = ImGui::GetCursorScreenPos();
			pos.y -= extraspacey;
			drawList->AddRectFilled(pos, ImVec2(pos.x + windowContentWidth, pos.y + bgRect.y), IM_COL32(128, 128, 128, 32 * (j % 2 + 1)));
			// Text
			ImGui::PushStyleColor(ImGuiCol_Text, linedata.color);
			ImGui::TextUnformatted(rowBeg, rowEnd);
			ImGui::PopStyleColor();
			// Repeat badge
			if (linedata.repeatcount > 1) {
				char badge[16];
				sprintf_s(badge, "x%03d", linedata.repeatcount);
				float pd = 2.0f;
				ImVec2 textSize = ImGui::CalcTextSize("x000");
				ImVec2 textPos = ImVec2(pos.x + windowContentWidth - textSize.x - pd, pos.y + bgRect.y - textSize.y);
				drawList->AddRectFilled(
					textPos,
					ImVec2(textPos.x + textSize.x + 2 * pd, textPos.y + textSize.y),
					IM_COL32_BLACK
				);
				drawList->AddText(
					ImVec2(textPos.x + pd, textPos.y),
					IM_COL32_WHITE,
					badge
				);
			}
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

void Console::log(LineData::Type type, ImU32 col, const char* line) {
	LineData& prevline = this->_lines[(this->_rowCount + this->_currentRow - 1) % this->_rowCount];
	//
	if (strcmp(line, prevline.buffer) == 0) {
		prevline.repeatcount++;
		return;
	}
	//
	LineData& currentline = this->_lines[this->_currentRow];
	char* rowBeg = currentline.buffer;
	memset(rowBeg, '\0', this->_rowLenght);
	strcpy_s(rowBeg, this->_rowLenght, line);
	//
	currentline.type = type;
	currentline.color = col;
	currentline.repeatcount = 1;
	this->_currentRow = (this->_currentRow + 1) % this->_rowCount;
}

int Console::getHistorySize() {
	return glm::min(this->_cmdHistorySize, this->_cmdHistoryIndexNextFree);
}

const char* Console::getHistoryCmd(int delta) {
	this->_cmdHistoryIndex += delta;
	this->_cmdHistoryIndex = glm::min(getHistorySize(), glm::max(0, this->_cmdHistoryIndex));
	int i = (this->_cmdHistoryIndexNextFree - this->_cmdHistoryIndex) % this->_cmdHistorySize;
	return this->_cmdHistory[i];
}

#include "usagedisplay.hpp"
#include "../app/app.hpp"
#include "hwinfo.hpp"

#include <imgui.h>
#include <string>
#include <iostream>

UsageDisplay::UsageDisplay(App& app, float updatePerSec)
	: _app(app)
{
	this->_timeFromLastUpdateSec = 0.0;
	this->_updateIntervalSec = 1.0 / updatePerSec;
	//
	this->_fps = 0;
	this->_memUsagePercentage = 0;
	this->_memUsage = 0;
	this->_memAvailable = 0;
	this->_cpuUsagePercentageAllCores = 0;
	this->_gpuUsagePercentage = 0;
	this->_coreCount = hwinfo::cpu::threadCount();
	this->_cpuUsagePercentagePerCore = new double[_coreCount];
	memset(this->_cpuUsagePercentagePerCore, 0, sizeof(double) * _coreCount);
	//
	this->__initialize();
}

void UsageDisplay::render(int w, int h) {
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse
		| ImGuiWindowFlags_AlwaysAutoResize
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoSavedSettings
		| ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoFocusOnAppearing;
	ImGui::Begin("Usage", nullptr, windowFlags);
	ImGui::Text("Fps: %-6.2f", this->_fps);
	ImGui::Text("Mem: %-8.2f Mb", this->_memUsage);
	ImGui::Text("GPU: %5.2f%%", this->_gpuUsagePercentage);
	ImGui::Text("CPU: %5.2f%%", this->_cpuUsagePercentageAllCores);
	//
	const int ncores = this->_coreCount;
	const int ncols = (ncores > 8) ? 4 : 2;
	ImGui::BeginTable("cores", ncols);
	for (int core = 0; core < ncores; ++core) {
		float usage = this->_cpuUsagePercentagePerCore[core] / 100.0;
		ImGui::TableNextColumn();
		//
		ImGuiIO& io = ImGui::GetIO();
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 size = ImGui::CalcTextSize("100%");
		size.x = ImGui::GetColumnWidth();
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImColor col = IM_COL32(255, 255 * (1.0 - usage), 0, 128);
		draw_list->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), col);
		//
		ImGui::Text("%3.0f%%", this->_cpuUsagePercentagePerCore[core]);
	}
	ImGui::EndTable();
	ImGui::SetWindowPos(ImVec2(w - ImGui::GetWindowWidth(), 0), ImGuiCond_Always);
	ImGui::End();
}

void UsageDisplay::update(double dtSec) {
	this->_fpsAccumulator++;
	this->_timeFromLastUpdateSec += dtSec;
	if (this->_timeFromLastUpdateSec > this->_updateIntervalSec) {
		this->_timeFromLastUpdateSec -= this->_updateIntervalSec;
		//
		this->__update();
	}
}

void UsageDisplay::__initialize() {

}

void UsageDisplay::__update() {
	// Fps
	this->_fps = this->_fpsAccumulator / this->_updateIntervalSec;
	this->_fpsAccumulator = 0;

	// Memory
	this->_memUsage = hwinfo::mem::usageMb();
	this->_memAvailable = hwinfo::mem::availableMb();
	this->_memUsagePercentage = (this->_memUsage / this->_memAvailable) * 100.0;

	// CPU
	this->_cpuUsagePercentageAllCores = hwinfo::cpu::usage(this->_cpuUsagePercentagePerCore);

	double step = 100.0 / this->_coreCount;
	for (int core = 0; core < this->_coreCount; ++core)
		this->_cpuUsagePercentagePerCore[core] = core * step;

	// GPU
	this->_gpuUsagePercentage = hwinfo::gpu::usage();
}

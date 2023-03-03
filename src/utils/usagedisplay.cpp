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
	memset(this->_cpuUsagePercentagePerCore, 0, sizeof(float) * _coreCount);
	//
	this->__initialize();
}

void UsageDisplay::render(int w, int h) {
	ImGui::SetNextWindowPos(ImVec2(w * 0.8, 0), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(w * 0.2, h * 0.5), ImGuiCond_Always);
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoSavedSettings
		| ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoFocusOnAppearing;
	ImGui::Begin("Usage", nullptr, windowFlags);
	ImGui::Text("Fps: %.2f", this->_fps);
	ImGui::Text("Mem: %2.2fMb (%2.2f%%)", this->_memUsage, this->_memUsagePercentage);
	ImGui::Text("GPU: %2.2f%%", this->_gpuUsagePercentage);
	ImGui::Text("CPU: %2.2f%%", this->_cpuUsagePercentageAllCores);
	for (int core = 0; core < this->_coreCount; ++core)
		ImGui::Text("CPU #%02d: %2.2f%%", core, this->_cpuUsagePercentagePerCore[core]);
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

	// GPU
	this->_gpuUsagePercentage = hwinfo::gpu::usage();
}

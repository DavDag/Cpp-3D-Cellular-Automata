#pragma once

class App;

class UsageDisplay {
public:
	UsageDisplay(App& app, float updatePerSec);
	void render(int w, int h);
	void update(double dtSec);

private:
	void __initialize();
	void __update();

private:
	App& _app;
	double _timeFromLastUpdateSec;
	double _updateIntervalSec;
	int _fpsAccumulator;
	//
	double _fps;
	double _memUsagePercentage;
	double _memUsage;
	double _memAvailable;
	double _cpuUsagePercentageAllCores;
	int _coreCount;
	double* _cpuUsagePercentagePerCore;
	double _gpuUsagePercentage;
};

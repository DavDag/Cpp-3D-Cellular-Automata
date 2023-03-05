#pragma once

class App;

class Simulation {
public:
	Simulation(App& app);

	void update(double dtSec);
	void render(int w, int h);

	void reset();
	void pause();
	void resume();
	void step(int count);

private:
	void __tick();

private:
	App& _app;
	bool _paused;
	double _timeSinceLastTickSec;
	double _tickSpeedSec;
};

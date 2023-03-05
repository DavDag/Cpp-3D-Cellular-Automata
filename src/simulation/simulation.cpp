#include "simulation.hpp"

#include "../app/app.hpp"

Simulation::Simulation(App& app) :
	_app(app)
{
	this->_tickSpeedSec = 1.0 / 1; // 1 tick/sec
	this->_timeSinceLastTickSec = 0;
}

void Simulation::update(double dtSec) {
	this->_timeSinceLastTickSec += dtSec;
	while (this->_timeSinceLastTickSec >= this->_tickSpeedSec) {
		this->_timeSinceLastTickSec -= this->_tickSpeedSec;

		// Pausing stops "natural" ticks
		if (!this->_paused) this->__tick();
	}
}

void Simulation::render(int w, int h) {
	// TODO:
}

void Simulation::reset() {
	// TODO:
}

void Simulation::pause() {
	this->_paused = true;
}

void Simulation::resume() {
	this->_paused = false;
}

void Simulation::step(int count) {
	for (int i = 0; i < count; ++i)
		this->__tick();
}

void Simulation::__tick() {
	// TODO:
}

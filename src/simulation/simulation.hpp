#pragma once

#include "../utils/opengl/opengl.hpp"

class App;

struct SimulationCell;

class Simulation {
public:
	Simulation(App& app);
	//
	void initialize();
	void update(double dtSec);
	void render(int w, int h);
	//
	void pause();
	void resume();
	void reset();
	void step(int count);
	void speed(int tickPerSec);
	void size(int side);
	void seed(int seed);

private:
	void __tick();

private:
	App& _app;
	//
	bool _paused;
	double _timeSinceLastTickSec;
	double _tickSpeedSec;
	double _timeAccSec;
	//
	int _seed;
	int _side;
	SimulationCell*** _world;
	//
	GLProgram _gridProgram, _cubeInstProgram;
	GLuint _gridVAO, _cubeInstVAO;
	GLuint _gridVBO, _cubeInstVBO;
	GLuint _gridEBO, _cubeInstEBO;
	GLuint _gridMatLoc, _cubeInstMatLoc;
};

#pragma once

#include "world.hpp"
#include "../utils/opengl/opengl.hpp"

class App;

struct SimulationCellData {
	int status;
};

class Simulation {
public:
	Simulation(App& app);
	int size() const;
	//
	void initialize();
	void update(double dtSec);
	void render(int w, int h);
	//
	void pause();
	void resume();
	void reset();
	void step(int count);
	void setspeed(int tickPerSec);
	void setsize(int side);
	void setseed(int seed);

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
	World<SimulationCellData> _world;
	int _cellsToDrawCount;
	GLuint* _cellsToDrawList;
	//
	GLProgram _gridProgram, _cubeInstProgram;
	GLuint _gridVAO, _cubeInstVAO;
	GLuint _gridVBO, _cubeInstVBO;
	GLuint _cubeInstVBO2;
	GLuint _gridEBO, _cubeInstEBO;
	GLuint _gridMatLoc, _cubeInstMatLoc;
};

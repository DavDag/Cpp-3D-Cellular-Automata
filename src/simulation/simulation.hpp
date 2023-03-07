#pragma once

#include "../utils/opengl/opengl.hpp"

class App;

class Simulation {
public:
	Simulation(App& app);
	//
	void initialize();
	void update(double dtSec);
	void render(int w, int h);
	//
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
	double _timeAccSec;
	//
	GLProgram _gridProgram, _cubeInstProgram;
	GLuint _gridVAO, _cubeInstVAO;
	GLuint _gridVBO, _cubeInstVBO;
	GLuint _gridEBO, _cubeInstEBO;
	GLuint _gridMatLoc, _cubeInstMatLoc;
};

#pragma once

#include "world.hpp"
#include "../utils/opengl/opengl.hpp"

#include <unordered_set>

class App;

struct SimCellData {
	int status;
};

struct GLSimCellData {
	union {
		struct {
			GLubyte x;
			GLubyte y;
			GLubyte z;
			GLubyte _;
		};
		GLuint all;
	} coords;
	union {
		struct {
			GLubyte r;
			GLubyte g;
			GLubyte b;
			GLubyte _;
		};
		GLuint all;
	} color;
};

struct SimRule {
	std::unordered_set<int> aliveWith;
	std::unordered_set<int> bornWith;
	int stateCount;
	enum Method { NEUMANN = 1, MOORE = 2 } method;
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
	SimRule _rule;
	int _seed;
	World<SimCellData> _world;
	int _cellsToDrawCount;
	GLSimCellData* _cellsToDrawList;
	//
	GLProgram _gridProgram, _cubeInstProgram;
	GLuint _gridVAO, _cubeInstVAO;
	GLuint _cubeInstVBO2;
};

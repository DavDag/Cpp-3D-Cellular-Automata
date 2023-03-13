#pragma once

#include "world.hpp"
#include "../utils/opengl/opengl.hpp"

class App;
class Simulation;

struct GLCell {
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

class Renderer {
public:
	Renderer(App& app, Simulation& sim);
	//
	void initialize();
	void update(double dtSec);
	void render(const World& world, const glm::mat4& camera, int w, int h);
	void ui(int w, int h);
	//
	void setMaxCellCount(int count);

private:
	App& _app;
	Simulation& _sim;
	//
	int _maxCellCount;
	int _cellsToDrawCount;
	GLCell* _cellsToDrawList;
	//
	GLProgram _gridProgram, _cubeInstProgram;
	GLuint _gridVAO, _cubeInstVAO;
	GLuint _cubeInstVBO2;
};

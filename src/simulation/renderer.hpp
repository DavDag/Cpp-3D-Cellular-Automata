#pragma once

#include "world.hpp"
#include "../utils/opengl/opengl.hpp"

class App;
class Simulation;
class Camera;

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
	void render(const World& world, Camera& camera, int w, int h);
	void ui(int w, int h);
	//
	void setMaxCellCount(int count);

private:
	void _computeDrawList(const World& world);
	void _shadowPass(const World& world, Camera& camera);
	void _gridPass(const World& world, Camera& camera);
	void _cubesPass(const World& world, Camera& camera);

	glm::mat4 lightSpaceMat(const World& world) const;

private:
	App& _app;
	Simulation& _sim;
	// Grid
	GLProgram _gridProgram;
	GLuint _gridVAO;
	// Cubes
	int _maxCellCount;
	int _cellsToDrawCount;
	GLCell* _cellsToDrawList;
	GLProgram _cubeInstProgram;
	GLuint _cubeInstVAO;
	GLuint _cubeInstVBO2;
	glm::vec3 _lightDir, _lightCol;
	float _lightAmbCoeff, _lightDifCoeff, _lightSpeCoeff;
	// Shadow
	GLuint _shadowFBO;
	GLuint _shadowTex;
	GLProgram _shadowProgram;
};

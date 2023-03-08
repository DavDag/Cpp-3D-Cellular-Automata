#include "simulation.hpp"
#include "../app.hpp"

struct SimulationCell {

};

Simulation::Simulation(App& app) :
	_app(app)
{
	this->_paused = false;
	this->_tickSpeedSec = 1.0 / 1; // 1 tick/sec
	this->_timeSinceLastTickSec = 0;
	this->_timeAccSec = 0;
	//
	this->_seed = 0;
	this->_side = 8;
	this->_world = nullptr;
	//
	this->_gridVAO = 0;
	this->_gridVBO = 0;
	this->_gridEBO = 0;
	this->_gridMatLoc = 0;
	this->_cubeInstVAO = 0;
	this->_cubeInstVBO = 0;
	this->_cubeInstEBO = 0;
	this->_cubeInstMatLoc = 0;
	//
	this->reset();
}

void Simulation::initialize() {
	static float cubeVertices[] = {
		// Front
		 0.5, -0.5, -0.5, // 0: BL
		 0.5,  0.5, -0.5, // 1: TL
		-0.5,  0.5, -0.5, // 2: TR
		-0.5, -0.5, -0.5, // 3: BR
		// Back
		 0.5, -0.5,  0.5, // 4: BL
		 0.5,  0.5,  0.5, // 5: TL
		-0.5,  0.5,  0.5, // 6: TR
		-0.5, -0.5,  0.5, // 7: BR
	};
	static int gridIndices[] = {
		0, 1, 1, 2, 2, 3, 3, 0, // front
		4, 5, 5, 6, 6, 7, 7, 4, // back
		0, 4, 1, 5, 2, 6, 3, 7, // 4 connecting lines
	};
	static int cubeIndices[] = {
		0, 1, 2, 2, 3, 0, // front
		4, 5, 6, 6, 7, 4, // back
		2, 1, 5, 5, 6, 2, // top
		6, 7, 3, 3, 2, 6, // right
 		0, 4, 7, 7, 3, 0, // bottom
		0, 1, 5, 5, 4, 0, // left
	};

	// ================================
	// Grid
	const char* gridVShaderSrc = R"(#version 330 core
	layout (location = 0) in vec3 vPos;
	uniform mat4 uMat;
	void main() {
		gl_Position = uMat * vec4(vPos, 1.0f);
	}
	)";
	const char* gridFShaderSrc = R"(#version 330 core
	out vec4 oCol;
	void main() {
		oCol = vec4(1, 1, 1, 1);
	}
	)";
	//
	this->_gridProgram = GLProgram::fromShaders(
		"gridProgram",
		GLShader::fromSrc("gridVertShader", gridVShaderSrc, GL_VERTEX_SHADER),
		GLShader::fromSrc("gridFragShader", gridFShaderSrc, GL_FRAGMENT_SHADER)
	);
	GL_CALL(this->_gridMatLoc = glGetUniformLocation(this->_gridProgram.id(), "uMat"));
	GL_CALL(glGenVertexArrays(1, &this->_gridVAO));
	GL_CALL(glGenBuffers(1, &this->_gridVBO));
	GL_CALL(glGenBuffers(1, &this->_gridEBO));
	GL_CALL(glBindVertexArray(this->_gridVAO));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->_gridVBO));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->_gridEBO));
	GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gridIndices), gridIndices, GL_STATIC_DRAW));
	GL_CALL(glEnableVertexAttribArray(0));
	GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
	//GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL));
	//GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, NULL));
	GL_CALL(glBindVertexArray(NULL));

	// ================================
	// Cube
	const char* cubeInstVShaderSrc = R"(#version 330 core
	layout (location = 0) in vec3 vPos;
	uniform mat4 uMat;
	out vec3 fCol;
	void main() {
		fCol = vPos;
		gl_Position = uMat * vec4(vPos, 1.0f);
	}
	)";
	const char* cubeInstFShaderSrc = R"(#version 330 core
	in vec3 fCol;
	out vec4 oCol;
	void main() {
		oCol = vec4(fCol, 1);
	}
	)";
	//
	this->_cubeInstProgram = GLProgram::fromShaders(
		"cubeInstProgram",
		GLShader::fromSrc("cubeInstVertShader", cubeInstVShaderSrc, GL_VERTEX_SHADER),
		GLShader::fromSrc("cubeInstFragShader", cubeInstFShaderSrc, GL_FRAGMENT_SHADER)
	);
	GL_CALL(this->_cubeInstMatLoc = glGetUniformLocation(this->_cubeInstProgram.id(), "uMat"));
	GL_CALL(glGenVertexArrays(1, &this->_cubeInstVAO));
	GL_CALL(glGenBuffers(1, &this->_cubeInstVBO));
	GL_CALL(glGenBuffers(1, &this->_cubeInstEBO));
	GL_CALL(glBindVertexArray(this->_cubeInstVAO));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->_cubeInstVBO));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->_cubeInstEBO));
	GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW));
	GL_CALL(glEnableVertexAttribArray(0));
	GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
	//GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL));
	//GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, NULL));
	GL_CALL(glBindVertexArray(NULL));
}

void Simulation::update(double dtSec) {
	this->_timeAccSec += dtSec;
	this->_timeSinceLastTickSec += dtSec;
	while (this->_timeSinceLastTickSec >= this->_tickSpeedSec) {
		this->_timeSinceLastTickSec -= this->_tickSpeedSec;

		// Pausing stops "natural" ticks
		if (!this->_paused) this->__tick();
	}
}

void Simulation::render(int w, int h) {
	const glm::mat4& camera = this->_app.camera();
	// Grid
	{
		this->_gridProgram.bind();
		glm::mat4 grid(1.0f);
		grid = glm::scale(grid, glm::vec3(this->_side));
		glm::mat4 mat = camera * grid;
		GL_CALL(glBindVertexArray(this->_gridVAO));
		this->_gridProgram.uniformMat4f("uMat", mat);
		GL_CALL(glDrawElements(GL_LINES, 2 * (4 + 4 + 4), GL_UNSIGNED_INT, NULL));
		GL_CALL(glBindVertexArray(NULL));
		this->_gridProgram.unbind();
	}
	// Cubes
	{
		this->_cubeInstProgram.bind();
		glm::mat4 model(1.0f);
		glm::mat4 mat = camera * model;
		GL_CALL(glBindVertexArray(this->_cubeInstVAO));
		this->_cubeInstProgram.uniformMat4f("uMat", mat);
		GL_CALL(glDrawElements(GL_TRIANGLES, 3 * (2 * 6), GL_UNSIGNED_INT, NULL));
		GL_CALL(glBindVertexArray(NULL));
		this->_cubeInstProgram.unbind();
	}
}

void Simulation::pause() {
	this->_paused = true;
}

void Simulation::resume() {
	this->_paused = false;
}

void Simulation::reset() {
	// TODO:
}

void Simulation::step(int count) {
	for (int i = 0; i < count; ++i)
		this->__tick();
}

void Simulation::speed(int tickPerSec) {
	this->_tickSpeedSec = 1.0 / tickPerSec;
}

void Simulation::size(int side) {
	// TODO: inplace update ?
	// TODO: do not reset boolean parameter ?
	this->_side = side;
	this->reset();
}

void Simulation::seed(int seed) {
	// TODO: do not reset boolean parameter ?
	this->_seed = seed;
	this->reset();
}

void Simulation::__tick() {
	// TODO:
}

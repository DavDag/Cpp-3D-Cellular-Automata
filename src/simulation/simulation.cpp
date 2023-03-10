#include "simulation.hpp"
#include "../app.hpp"

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
	this->_world = World<SimulationCellData>(this->_side);
	this->_cellsToDrawCount = 0;
	this->_cellsToDrawList = new GLuint[this->_world.size()];
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

int Simulation::size() const {
	return this->_side;
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
	static GLuint gridIndices[] = {
		0, 1, 1, 2, 2, 3, 3, 0, // front
		4, 5, 5, 6, 6, 7, 7, 4, // back
		0, 4, 1, 5, 2, 6, 3, 7, // 4 connecting lines
	};
	static GLuint cubeIndices[] = {
		2, 1, 0, 0, 3, 2, // front
		4, 5, 6, 6, 7, 4, // back
		5, 1, 2, 2, 6, 5, // top
		3, 7, 6, 6, 2, 3, // right
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
	// 
	GL_CALL(glBindVertexArray(this->_gridVAO));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->_gridVBO));
	GL_CALL(glEnableVertexAttribArray(0)); // Associate attr <-> buffer (& store it in the VAO)
	GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, NULL)); // can be unbound
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->_gridEBO)); // Associate index buffer (& store it in the VAO)
	GL_CALL(glBindVertexArray(NULL));
	//
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->_gridVBO));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, NULL));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->_gridEBO));
	GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gridIndices), gridIndices, GL_STATIC_DRAW));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL));

	// ================================
	// Cube
	const char* cubeInstVShaderSrc = R"(#version 450 core
	layout (location = 0) in uint vIndex;
	layout (location = 1) in vec3 vPos;
	uniform uint uWorldSide;
	uniform mat4 uMat;
	out vec3 fCol;
	void main() {
		uint dz = (vIndex) % uWorldSide;
		uint dy = (vIndex / uWorldSide) % uWorldSide;
		uint dx = (vIndex / uWorldSide / uWorldSide) % uWorldSide;
		fCol = vec3(dx, dy, dz) / float(uWorldSide);
		gl_Position = uMat * vec4(vPos + vec3(dx, dy, dz), 1.0f);
	}
	)";
	const char* cubeInstFShaderSrc = R"(#version 450 core
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
	GL_CALL(glGenBuffers(1, &this->_cubeInstVBO2));
	GL_CALL(glGenBuffers(1, &this->_cubeInstEBO));
	//
	GL_CALL(glBindVertexArray(this->_cubeInstVAO));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->_cubeInstVBO2));
	GL_CALL(glEnableVertexAttribArray(0));  // Associate attr <-> buffer (& store it in the VAO)
	GL_CALL(glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, 0, (void*)0));
	GL_CALL(glVertexAttribDivisor(0, 1));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, NULL)); // can be unbound
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->_cubeInstVBO));
	GL_CALL(glEnableVertexAttribArray(1));  // Associate attr <-> buffer (& store it in the VAO)
	GL_CALL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, NULL)); // can be unbound
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->_cubeInstEBO)); // Associate index buffer (& store it in the VAO)
	GL_CALL(glBindVertexArray(NULL));
	//
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->_cubeInstVBO2));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, this->_world.size() * sizeof(GLuint), NULL, GL_DYNAMIC_DRAW));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, NULL));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->_cubeInstVBO));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, NULL));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->_cubeInstEBO));
	GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL));
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
		glm::mat4 grid(1.0f);
		grid = glm::scale(grid, glm::vec3(this->_side));
		glm::mat4 mat = camera * grid;
		//
		this->_gridProgram.bind();
		this->_gridProgram.uniformMat4f("uMat", mat);
		GL_CALL(glBindVertexArray(this->_gridVAO));
		GL_CALL(glDrawElements(GL_LINES, 2 * (4 + 4 + 4), GL_UNSIGNED_INT, NULL));
		GL_CALL(glBindVertexArray(NULL));
		this->_gridProgram.unbind();
	}
	// Cubes
	{
		this->_cellsToDrawCount = this->_world.size();
		for (GLuint i = 0; i < this->_cellsToDrawCount; ++i)
			this->_cellsToDrawList[i] = i;
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->_cubeInstVBO2));
		GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, this->_cellsToDrawCount * sizeof(GLuint), this->_cellsToDrawList));
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, NULL));
		//
		this->_cubeInstProgram.bind();
		glm::mat4 model(1.0f);
		model = glm::translate(model, glm::vec3(this->_world.side()) * -0.5f);
		model = glm::translate(model, glm::vec3(0.5f));
		glm::mat4 mat = camera * model;
		//
		GL_CALL(glBindVertexArray(this->_cubeInstVAO));
		this->_cubeInstProgram.uniform1u("uWorldSide", this->_world.side());
		this->_cubeInstProgram.uniformMat4f("uMat", mat);
		GL_CALL(glDrawElementsInstanced(GL_TRIANGLES, 3 * (2 * 6), GL_UNSIGNED_INT, NULL, this->_cellsToDrawCount));
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
	delete[] this->_cellsToDrawList;
	//
	this->_world = World<SimulationCellData>(this->_side);
	this->_cellsToDrawCount = 0;
	this->_cellsToDrawList = new GLuint[this->_world.size()];
}

void Simulation::step(int count) {
	for (int i = 0; i < count; ++i)
		this->__tick();
}

void Simulation::setspeed(int tickPerSec) {
	this->_tickSpeedSec = 1.0 / tickPerSec;
}

void Simulation::setsize(int side) {
	// TODO: inplace update ?
	// TODO: do not reset boolean parameter ?
	this->_side = side;
	this->reset();
}

void Simulation::setseed(int seed) {
	// TODO: do not reset boolean parameter ?
	this->_seed = seed;
	this->reset();
}

void Simulation::__tick() {
	// TODO:
}

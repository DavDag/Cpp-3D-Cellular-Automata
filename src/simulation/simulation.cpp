#include "simulation.hpp"
#include "../app.hpp"

#include <random>

Simulation::Simulation(App& app) :
	_app(app)
{
	this->_paused = false;
	this->_tickSpeedSec = 1.0 / 2; // tick/sec
	this->_timeSinceLastTickSec = 0;
	this->_timeAccSec = 0;
	//
	this->_rule = SimRule{
		.aliveWith = { 4 },
		.bornWith = { 4 },
		.stateCount = 5,
		.method = SimRule::Method::MOORE
	};
	this->_seed = 0;
	this->_world = World<SimCellData>(8, {.status=0});
	this->_cellsToDrawCount = 0;
	this->_cellsToDrawList = new GLSimCellData[this->_world.size()];
	//
	this->_gridVAO = 0;
	this->_cubeInstVAO = 0;
	this->_cubeInstVBO2 = 0;
	//
	this->reset();
}

int Simulation::size() const {
	return this->_world.side();
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
	GLuint tmpvbo, tmpebo;
	GL_CALL(glGenVertexArrays(1, &this->_gridVAO));
	GL_CALL(glGenBuffers(1, &tmpvbo));
	GL_CALL(glGenBuffers(1, &tmpebo));
	//
	GL_CALL(glBindVertexArray(this->_gridVAO));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, tmpvbo));
	GL_CALL(glEnableVertexAttribArray(0)); // Associate attr <-> buffer (& store it in the VAO)
	GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, NULL)); // can be unbound
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tmpebo)); // Associate index buffer (& store it in the VAO)
	GL_CALL(glBindVertexArray(NULL));
	//
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, tmpvbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, NULL));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tmpebo));
	GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gridIndices), gridIndices, GL_STATIC_DRAW));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL));

	// ================================
	// Cube
	const char* cubeInstVShaderSrc = R"(#version 450 core
	layout (location = 0) in vec3 vPos;
	layout (location = 1) in uvec3 vCoords;
	layout (location = 2) in vec3 vCol;
	uniform uint uWorldSide;
	uniform mat4 uMat;
	out vec3 fCol;
	void main() {
		fCol = vCol;
		gl_Position = uMat * vec4(vPos + vCoords, 1.0f);
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
	GL_CALL(glGenVertexArrays(1, &this->_cubeInstVAO));
	GL_CALL(glGenBuffers(1, &tmpvbo));
	GL_CALL(glGenBuffers(1, &this->_cubeInstVBO2));
	GL_CALL(glGenBuffers(1, &tmpebo));
	//
	GL_CALL(glBindVertexArray(this->_cubeInstVAO));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, tmpvbo));
	GL_CALL(glEnableVertexAttribArray(0));  // Associate attr <-> buffer (& store it in the VAO)
	GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, NULL)); // can be unbound
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->_cubeInstVBO2));
	GL_CALL(glEnableVertexAttribArray(1));  // Associate attr <-> buffer (& store it in the VAO)
	GL_CALL(glVertexAttribIPointer(1, 3, GL_UNSIGNED_BYTE, sizeof(GLSimCellData), (void*)offsetof(GLSimCellData, coords)));
	GL_CALL(glVertexAttribDivisor(1, 1));
	GL_CALL(glEnableVertexAttribArray(2));  // Associate attr <-> buffer (& store it in the VAO)
	GL_CALL(glVertexAttribPointer(2, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(GLSimCellData), (void*)offsetof(GLSimCellData, color)));
	GL_CALL(glVertexAttribDivisor(2, 1));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, NULL)); // can be unbound
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tmpebo)); // Associate index buffer (& store it in the VAO)
	GL_CALL(glBindVertexArray(NULL));
	//
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->_cubeInstVBO2));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, this->_world.size() * 4 * sizeof(GLubyte), NULL, GL_DYNAMIC_DRAW));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, NULL));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, tmpvbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, NULL));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tmpebo));
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
		grid = glm::scale(grid, glm::vec3(float(this->_world.side())));
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
		const int ws = this->_world.side();
		const int maxstatus = this->_rule.stateCount - 1;
		this->_cellsToDrawCount = 0;
		for (int ci = 0; ci < this->_world.size(); ++ci) {
			SimCellData worldcelldata = this->_world.get(ci);
			if (worldcelldata.status != 0) {
				float decay = worldcelldata.status / (float) maxstatus;
				GLSimCellData glcelldata = { 0 };
				// glcelldata.coords.all = ci; // reversed
				glcelldata.coords.x = (ci / ws / ws) % ws;
				glcelldata.coords.y = (ci / ws) % ws;
				glcelldata.coords.z = (ci) % ws;
				glcelldata.coords._ = 0;
				glcelldata.color.r = (GLubyte)(0xff * decay);
				glcelldata.color.g = 0;
				glcelldata.color.b = 0;
				glcelldata.color._ = 0;
				this->_cellsToDrawList[this->_cellsToDrawCount++] = glcelldata;
			}
		}
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->_cubeInstVBO2));
		GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, this->_cellsToDrawCount * sizeof(GLSimCellData), this->_cellsToDrawList));
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, NULL));
		//
		this->_cubeInstProgram.bind();
		glm::mat4 model(1.0f);
		model = glm::translate(model, glm::vec3(float(this->_world.side())) * -0.5f);
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
	this->_app.deb("simulation paused");
	this->_paused = true;
}

void Simulation::resume() {
	this->_app.deb("simulation resumed");
	this->_paused = false;
}

void Simulation::reset() {
	this->_app.deb("simulation resetted");
	SimCellData emptydata = {
		.status = 0
	};
	for (int i = 0; i < this->_world.size(); ++i)
		this->_world.set(emptydata, i);
	//
	std::mt19937 gen;
	std::uniform_int_distribution<int> distr(0, this->_rule.stateCount - 1);
	const float cen = this->_world.side() / 2.0f;
	const float off = 2.0f;
	for (float dx = -off; dx < +off; dx += 1.0f)
		for (float dy = -off; dy < +off; dy += 1.0f)
			for (float dz = -off; dz < +off; dz += 1.0f) {
				SimCellData data = {
					.status = distr(gen)
				};
				int x = (int) round(cen + dx);
				int y = (int) round(cen + dy);
				int z = (int) round(cen + dz);
				this->_world.set(data, x, y, z);
			}
}

void Simulation::step(int count) {
	for (int i = 0; i < count; ++i)
		this->__tick();
}

void Simulation::setspeed(int tickPerSec) {
	this->_app.deb("simulation speed updated");
	this->_tickSpeedSec = 1.0 / tickPerSec;
}

void Simulation::setsize(int side) {
	this->_app.deb("simulation size updated");
	// TODO: inplace update ?
	// TODO: do not reset boolean parameter ?
	this->_world = World<SimCellData>(this->_world.side(), {.status = 0});
	//
	delete[] this->_cellsToDrawList;
	this->_cellsToDrawCount = 0;
	this->_cellsToDrawList = new GLSimCellData[this->_world.size()];
	//
	this->reset();
}

void Simulation::setseed(int seed) {
	this->_app.deb("simulation seed updated");
	// TODO: do not reset boolean parameter ?
	this->_seed = seed;
	this->reset();
}

void Simulation::__tick() {
	this->_app.deb("simulation tick");
	const int ws = this->_world.side();
	const int initialstate = this->_rule.stateCount - 1;
	for (int x = 0; x < ws; ++x)
		for (int y = 0; y < ws; ++y)
			for (int z = 0; z < ws; ++z) {
				SimCellData data = this->_world.get(x, y, z);
				//
				int neighbours = (data.status == initialstate) ? -1 : 0;
				for (int dx = -1; dx <= 1; ++dx)
					for (int dy = -1; dy <= 1; ++dy)
						for (int dz = -1; dz <= 1; ++dz)
							if (this->_world.get(x + dx, y + dy, z + dz).status == initialstate)
								++neighbours;
				//
				if (data.status == initialstate) {
					// alive
					if (this->_rule.aliveWith.contains(neighbours))
						continue;
					data.status = initialstate - 1;
				}
				else if (data.status == 0) {
					// born
					if (this->_rule.bornWith.contains(neighbours))
						data.status = initialstate;
				}
				else {
					// decrement
					data.status = data.status - 1;
				}
				this->_world.set(data, x, y, z);
			}
}

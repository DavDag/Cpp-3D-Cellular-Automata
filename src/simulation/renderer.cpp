#include "renderer.hpp"
#include "world.hpp"
#include "simulation.hpp"
#include "../app.hpp"

Renderer::Renderer(App& app, Simulation& sim):
	_app(app),
	_sim(sim)
{
	this->_maxCellCount = 1;
	this->_cellsToDrawCount = 0;
	this->_cellsToDrawList = new GLCell[this->_maxCellCount];
	//
	this->_gridVAO = 0;
	this->_cubeInstVAO = 0;
	this->_cubeInstVBO2 = 0;
}

void Renderer::initialize() {
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
	GL_CALL(glVertexAttribIPointer(1, 3, GL_UNSIGNED_BYTE, sizeof(GLCell), (void*)offsetof(GLCell, coords)));
	GL_CALL(glVertexAttribDivisor(1, 1));
	GL_CALL(glEnableVertexAttribArray(2));  // Associate attr <-> buffer (& store it in the VAO)
	GL_CALL(glVertexAttribPointer(2, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(GLCell), (void*)offsetof(GLCell, color)));
	GL_CALL(glVertexAttribDivisor(2, 1));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, NULL)); // can be unbound
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tmpebo)); // Associate index buffer (& store it in the VAO)
	GL_CALL(glBindVertexArray(NULL));
	//
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->_cubeInstVBO2));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, this->_maxCellCount * sizeof(GLCell), NULL, GL_DYNAMIC_DRAW));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, NULL));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, tmpvbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, NULL));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tmpebo));
	GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL));
}

void Renderer::update(double dtSec) {

}

void Renderer::render(const World& world, const glm::mat4& camera, int w, int h) {
	// Grid
	{
		glm::mat4 grid(1.0f);
		grid = glm::scale(grid, glm::vec3(float(world.side())));
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
		const int ws = world.side();
		this->_cellsToDrawCount = 0;
		for (int ci = 0; ci < world.size(); ++ci) {
			WorldCell worldcell = world.get(ci);
			if (worldcell.status != 0) {
				GLCell glcell = { 0 };
				// glcell.coords.all = ci; // reversed
				glcell.coords.x = (ci / ws / ws) % ws;
				glcell.coords.y = (ci / ws) % ws;
				glcell.coords.z = (ci) % ws;
				glcell.coords._ = 0;
				this->_sim.colorrule(worldcell, glcell);
				this->_cellsToDrawList[this->_cellsToDrawCount++] = glcell;
			}
		}
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->_cubeInstVBO2));
		GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, this->_cellsToDrawCount * sizeof(GLCell), this->_cellsToDrawList));
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, NULL));
		//
		this->_cubeInstProgram.bind();
		glm::mat4 model(1.0f);
		model = glm::translate(model, glm::vec3(float(world.side())) * -0.5f);
		model = glm::translate(model, glm::vec3(0.5f));
		glm::mat4 mat = camera * model;
		//
		GL_CALL(glBindVertexArray(this->_cubeInstVAO));
		this->_cubeInstProgram.uniform1u("uWorldSide", world.side());
		this->_cubeInstProgram.uniformMat4f("uMat", mat);
		GL_CALL(glDrawElementsInstanced(GL_TRIANGLES, 3 * (2 * 6), GL_UNSIGNED_INT, NULL, this->_cellsToDrawCount));
		GL_CALL(glBindVertexArray(NULL));
		this->_cubeInstProgram.unbind();
	}
}

void Renderer::ui(int w, int h) {

}

void Renderer::setMaxCellCount(int count) {
	delete[] this->_cellsToDrawList;
	this->_maxCellCount = count + 1;
	this->_cellsToDrawCount = 0;
	this->_cellsToDrawList = new GLCell[this->_maxCellCount];
	//
	if (this->_cubeInstVBO2) {
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->_cubeInstVBO2));
		GL_CALL(glBufferData(GL_ARRAY_BUFFER, this->_maxCellCount * sizeof(GLCell), NULL, GL_DYNAMIC_DRAW));
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, NULL));
	}
}

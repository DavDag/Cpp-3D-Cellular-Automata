#include "renderer.hpp"
#include "world.hpp"
#include "simulation.hpp"
#include "../app.hpp"

#include <ppl.h>

#define SHADOW_WIDTH  4096
#define SHADOW_HEIGHT 4096

Renderer::Renderer(App& app, Simulation& sim):
	_app(app),
	_sim(sim)
{
	this->_parallel = false;
	// Grid
	this->_gridVAO = 0;
	// Cubes
	this->_maxCellCount = 1;
	this->_cellsToDrawCount = 0;
	this->_cellsToDrawList = new GLCell[this->_maxCellCount];
	this->_cubeInstVAO = 0;
	this->_cubeInstVBO2 = 0;
	this->_lightDir = glm::normalize(glm::vec3(0.0f, 0.5f, -1.0f));
	this->_lightCol = glm::vec3(1.0f);
	this->_lightAmbCoeff = 0.1f;
	this->_lightDifCoeff = 1.0f;
	this->_lightSpeCoeff = 0.5f;
	// Shadows
	this->_shadowFBO = 0;
	this->_shadowTex = 0;
}

void Renderer::initialize() {
	static float gridVertices[] = {
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

	// ================================
	// Grid
	{
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
		GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(gridVertices), gridVertices, GL_STATIC_DRAW));
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, NULL));
		GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tmpebo));
		GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gridIndices), gridIndices, GL_STATIC_DRAW));
		GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL));
	}

	// ================================
	static float cubeVertices[] = {
		// Front
		 0.5, -0.5,  0.5, 0, 0,  1, //  0: Bottom-Left
		 0.5,  0.5,  0.5, 0, 0,  1, //  1: Top-Left
		-0.5,  0.5,  0.5, 0, 0,  1, //  2: Top-Right
		-0.5, -0.5,  0.5, 0, 0,  1, //  3: Bottom-Right
		// Back
		 0.5, -0.5, -0.5, 0, 0, -1, //  4: Bottom-Left
		 0.5,  0.5, -0.5, 0, 0, -1, //  5: Top-Left
		-0.5,  0.5, -0.5, 0, 0, -1, //  6: Top-Right
		-0.5, -0.5, -0.5, 0, 0, -1, //  7: Bottom-Right
		// Left
		-0.5,  0.5, -0.5, -1, 0, 0, //  8: Front-Top
		-0.5,  0.5,  0.5, -1, 0, 0, //  9: Back-Top
		-0.5, -0.5, -0.5, -1, 0, 0, // 10: Front-Bottom
		-0.5, -0.5,  0.5, -1, 0, 0, // 11: Back-Bottom
		// Right
		 0.5,  0.5, -0.5,  1, 0, 0, // 12: Front-Top
		 0.5,  0.5,  0.5,  1, 0, 0, // 13: Back-Top
		 0.5, -0.5, -0.5,  1, 0, 0, // 14: Front-Bottom
		 0.5, -0.5,  0.5,  1, 0, 0, // 15: Back-Bottom
		// Top
		 0.5,  0.5,  0.5, 0,  1, 0, // 16: Front-Left
		 0.5,  0.5, -0.5, 0,  1, 0, // 17: Back-Left
		-0.5,  0.5, -0.5, 0,  1, 0, // 18: Back-Right
		-0.5,  0.5,  0.5, 0,  1, 0, // 19: Front-Right
		// Bottom
		 0.5, -0.5,  0.5, 0, -1, 0, // 20: Front-Left
		 0.5, -0.5, -0.5, 0, -1, 0, // 21: Back-Left
		-0.5, -0.5, -0.5, 0, -1, 0, // 22: Back-Right
		-0.5, -0.5,  0.5, 0, -1, 0, // 23: Front-Right
	};
	static GLuint cubeIndices[] = {
		 0,  1,  2,  2,  3,  0, // front
		 6,  5,  4,  4,  7,  6, // back
		10,  9,  8, 10, 11,  9, // left
		12, 13, 14, 13, 15, 14, // right
		16, 17, 18, 16, 18, 19, // top
		22, 21, 20, 23, 22, 20, // bottom
	};

	// ================================
	// Cube
	{
		const char* cubeInstVShaderSrc = R"(#version 450 core
		layout (location = 0) in vec3 vPos;
		layout (location = 1) in vec3 vNorm;
		layout (location = 2) in uvec3 vCoords;
		layout (location = 3) in vec3 vCol;
		uniform mat4 uViewProj;
		uniform mat4 uModelBase;
		uniform mat4 uLightSpace;
		out vec3 fPos;
		out vec3 fNorm;
		out vec3 fCol;
		out vec4 fPosLightSpace;
		mat4 fromTranslation(vec3 d) {
			return mat4(
				vec4(1.0f, 0.0f, 0.0f, 0.0f),
				vec4(0.0f, 1.0f, 0.0f, 0.0f),
				vec4(0.0f, 0.0f, 1.0f, 0.0f),
				vec4( d.x,  d.y,  d.z, 1.0f)
			);
		}
		void main() {
			mat4 model = uModelBase * fromTranslation(vCoords);
			fCol = vCol;
			fPos = vec3(model * vec4(vPos, 1.0f));
			fNorm = mat3(transpose(inverse(model))) * vNorm;
			fPosLightSpace = uLightSpace * vec4(fPos, 1.0f);
			gl_Position = uViewProj * vec4(fPos, 1.0f);
		}
		)";
		const char* cubeInstFShaderSrc = R"(#version 450 core
		in vec3 fPos;
		in vec3 fNorm;
		in vec3 fCol;
		in vec4 fPosLightSpace;
		uniform vec3 uViewPos;
		uniform vec3 uLightDir;
		uniform vec3 uLightCol;
		uniform float uAmbCoeff;
		uniform float uDifCoeff;
		uniform float uSpeCoeff;
		uniform sampler2D uShadowMap;
		out vec4 oCol;
		float ComputeLightFactor() {
			//
			// Blinn-Phong Lighting
			//
  	
			// Diffuse 
			vec3 norm = normalize(fNorm);
			vec3 lightDir = normalize(-uLightDir);
			float diff = max(dot(norm, lightDir), 0.0f);
			float diffuse = diff * uDifCoeff;
    
			// Specular
			vec3 viewDir = normalize(uViewPos - fPos);
			vec3 halfwayDir = normalize(lightDir + viewDir);
			float spec = pow(max(dot(norm, halfwayDir), 0.0f), 32);
			//vec3 reflectDir = reflect(-lightDir, norm);
			//float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
			float specular = spec * uSpeCoeff;
        
			return (diffuse + specular);
		}
		float ComputeShadowFactor() {
			//
			// PCF + Bias
			//

			// Coords in lightspace
			vec3 shadowProjCoords = fPosLightSpace.xyz/fPosLightSpace.w;
			shadowProjCoords = shadowProjCoords * 0.5f + 0.5f;
			float currentDepth = shadowProjCoords.z;
			if(currentDepth > 1.0f) return 0.0f;
			
			// Bias
			vec3 normal = normalize(fNorm);
			vec3 lightDir = normalize(-uLightDir);
			float bias = max(0.05f * (1.0f - dot(normal, lightDir)), 0.005f);
			
			/*
			// Shadow
			float shadow = 0.0f;
			vec2 texelSize = 1.0f / textureSize(uShadowMap, 0);
			for(int x = -1; x <= 1; ++x) {
				for(int y = -1; y <= 1; ++y) {
					float pcfDepth = texture(uShadowMap, shadowProjCoords.xy + vec2(x, y) * texelSize).r;
					shadow += (currentDepth - bias) > pcfDepth  ? 1.0f : 0.0f;
				}
			}
			shadow /= 9.0f;
			*/
	
			float closestDepth = texture(uShadowMap, shadowProjCoords.xy).r; 
			float shadow = (currentDepth - bias) > closestDepth  ? 1.0f : 0.0f;

			return shadow;
		}
		void main() {
			vec3 result = (uAmbCoeff + (1.0f - ComputeShadowFactor()) * ComputeLightFactor()) * uLightCol * fCol;
			oCol = vec4(result, 1.0f);
		}
		)";
		//
		this->_cubeInstProgram = GLProgram::fromShaders(
			"cubeInstProgram",
			GLShader::fromSrc("cubeInstVertShader", cubeInstVShaderSrc, GL_VERTEX_SHADER),
			GLShader::fromSrc("cubeInstFragShader", cubeInstFShaderSrc, GL_FRAGMENT_SHADER)
		);
		GLuint tmpvbo, tmpebo;
		GL_CALL(glGenVertexArrays(1, &this->_cubeInstVAO));
		GL_CALL(glGenBuffers(1, &tmpvbo));
		GL_CALL(glGenBuffers(1, &this->_cubeInstVBO2));
		GL_CALL(glGenBuffers(1, &tmpebo));
		//
		GL_CALL(glBindVertexArray(this->_cubeInstVAO));
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, tmpvbo));
		GL_CALL(glEnableVertexAttribArray(0));  // Associate attr <-> buffer (& store it in the VAO)
		GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(0 * sizeof(float))));
		GL_CALL(glEnableVertexAttribArray(1));  // Associate attr <-> buffer (& store it in the VAO)
		GL_CALL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))));
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, NULL)); // can be unbound
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->_cubeInstVBO2));
		GL_CALL(glEnableVertexAttribArray(2));  // Associate attr <-> buffer (& store it in the VAO)
		GL_CALL(glVertexAttribIPointer(2, 3, GL_UNSIGNED_BYTE, sizeof(GLCell), (void*)offsetof(GLCell, coords)));
		GL_CALL(glVertexAttribDivisor(2, 1));
		GL_CALL(glEnableVertexAttribArray(3));  // Associate attr <-> buffer (& store it in the VAO)
		GL_CALL(glVertexAttribPointer(3, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(GLCell), (void*)offsetof(GLCell, color)));
		GL_CALL(glVertexAttribDivisor(3, 1));
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

	// ================================
	// Shadows
	{
		const char* shadowVShaderSrc = R"(#version 450 core
			layout (location = 0) in vec3 vPos;
			layout (location = 2) in uvec3 vCoords;
			uniform mat4 uLightSpace;
			uniform mat4 uModelBase;
			mat4 fromTranslation(vec3 d) {
				return mat4(
					vec4(1.0, 0.0, 0.0, 0.0),
					vec4(0.0, 1.0, 0.0, 0.0),
					vec4(0.0, 0.0, 1.0, 0.0),
					vec4(d.x, d.y, d.z, 1.0)
				);
			}
			void main() {
				mat4 model = uModelBase * fromTranslation(vCoords);
				gl_Position = uLightSpace * model * vec4(vPos, 1.0);
			}
		)";
		const char* shadowFShaderSrc = R"(#version 450 core
			layout(location = 0) out float oDepth;
			void main() {
				oDepth = gl_FragCoord.z;
			}
		)";
		this->_shadowProgram = GLProgram::fromShaders(
			"shadowProgram",
			GLShader::fromSrc("shadowVShaderSrc", shadowVShaderSrc, GL_VERTEX_SHADER),
			GLShader::fromSrc("shadowFShaderSrc", shadowFShaderSrc, GL_FRAGMENT_SHADER)
		);
		GL_CALL(glCreateFramebuffers(1, &this->_shadowFBO));
		GL_CALL(glGenTextures(1, &this->_shadowTex));
		//
		GL_CALL(glBindTexture(GL_TEXTURE_2D, this->_shadowTex));
		GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
		float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
		GL_CALL(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor));
		GL_CALL(glBindTexture(GL_TEXTURE_2D, NULL));
		//
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, this->_shadowFBO));
		GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->_shadowTex, 0)); // attach to fbo
		GL_CALL(glDrawBuffer(GL_NONE)); // disable color data
		GL_CALL(glReadBuffer(GL_NONE)); // disable color data
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}
}

void Renderer::update(double dtSec) {

}

void Renderer::render(const World& world, Camera& camera, int w, int h) {
	this->_computeDrawList(world);
	// Preprocessing
	this->_shadowPass(world, camera);
	// Rendering
	GL_CALL(glViewport(0, 0, w, h));
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, NULL));
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	this->_gridPass(world, camera);
	this->_cubesPass(world, camera);
}

void Renderer::ui(int w, int h) {
	ImGui::SetNextWindowPos(ImVec2(0, h * 0.55f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(w * 0.25f, h * 0.3f), ImGuiCond_FirstUseEver);
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;
	ImGui::Begin("Rendering", nullptr, windowFlags);
	float windowContentWidth = ImGui::GetWindowContentRegionWidth();
	///////////////////////////////////////////////
	// Params
	ImGui::SeparatorText("");
	ImGui::Checkbox("Parallel", &this->_parallel);
	ImGui::SliderFloat3("LightDir", &this->_lightDir[0], -1.0f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	this->_lightDir = glm::normalize(this->_lightDir);
	ImGui::SliderFloat3("LightCol", &this->_lightCol[0], 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SliderFloat("Ambient", &this->_lightAmbCoeff, 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SliderFloat("Diffuse", &this->_lightDifCoeff, 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SliderFloat("Specular", &this->_lightSpeCoeff, 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	///////////////////////////////////////////////
	// Memory
	ImGui::SeparatorText("");
	int vboSize;
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->_cubeInstVBO2));
	GL_CALL(glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &vboSize));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	ImGui::Text("Current cell count: %8d ", this->_cellsToDrawCount);
	ImGui::Text("Max cell count: %12d ", this->_maxCellCount);
	ImGui::Text("Buffer Size: %15.2f Mb", vboSize / 1024.0f / 1024.0f);
	///////////////////////////////////////////////
	// Shadows
	//ImGui::GetForegroundDrawList()->AddImage((ImTextureID)this->_shadowTex, ImVec2(0, 0), ImVec2(SHADOW_WIDTH, SHADOW_HEIGHT));
	ImGui::Image((ImTextureID)this->_shadowTex, ImVec2(windowContentWidth, windowContentWidth));
	///////////////////////////////////////////////
	ImGui::End();
	//ImGui::ShowMetricsWindow();
}

void Renderer::setMaxCellCount(int count) {
	delete[] this->_cellsToDrawList;
	this->_maxCellCount = count;
	this->_cellsToDrawCount = 0;
	this->_cellsToDrawList = new GLCell[this->_maxCellCount];
	//
	if (this->_cubeInstVBO2) {
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->_cubeInstVBO2));
		GL_CALL(glBufferData(GL_ARRAY_BUFFER, this->_maxCellCount * sizeof(GLCell), NULL, GL_DYNAMIC_DRAW));
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, NULL));
	}
}

void Renderer::_computeDrawList(const World& world) {
	const int ws = world.side();
	if (this->_parallel) {
		auto partitioner = concurrency::auto_partitioner();
		concurrency::parallel_for(0, world.size(), [&](size_t ci) {
				WorldCell worldcell = world.get(ci);
				// reset flag
				this->_cellsToDrawList[ci].coords._ = 0;
				// 1st check: cell alive
				if (worldcell.status == 0) return;
				// 2nd check: 6 neumann neighbours are all != 0
				int x = (ci / ws / ws) % ws;
				int y = (ci / ws) % ws;
				int z = (ci) % ws;
				int neumannZeros = world.countNeumann(0, x, y, z);
				if (neumannZeros == 0) return;
				// Load cell
				GLCell glcell = { 0 };
				glcell.coords.x = x;
				glcell.coords.y = y;
				glcell.coords.z = z;
				glcell.coords._ = 1;
				this->_sim.applyColorRule(worldcell, glcell);
				this->_cellsToDrawList[ci] = glcell;
			},
			partitioner
		);
		//
		this->_cellsToDrawCount = 0;
		for (int ci = 0; ci < world.size(); ++ci) {
			GLCell glcell = this->_cellsToDrawList[ci];
			if (glcell.coords._ != 0)
				this->_cellsToDrawList[this->_cellsToDrawCount++] = glcell;
		}
	}
	else {
		this->_cellsToDrawCount = 0;
		for (int ci = 0; ci < world.size(); ++ci) {
			WorldCell worldcell = world.get(ci);
			if (worldcell.status != 0) {
				GLCell glcell = { 0 };
				glcell.coords.x = (ci / ws / ws) % ws;
				glcell.coords.y = (ci / ws) % ws;
				glcell.coords.z = (ci) % ws;
				glcell.coords._ = 0;
				this->_sim.applyColorRule(worldcell, glcell);
				this->_cellsToDrawList[this->_cellsToDrawCount++] = glcell;
			}
		}
	}
	//
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->_cubeInstVBO2));
	GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, this->_cellsToDrawCount * sizeof(GLCell), this->_cellsToDrawList));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, NULL));
}

void Renderer::_shadowPass(const World& world, Camera& camera) {
	GL_CALL(glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT));
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, this->_shadowFBO));
	GL_CALL(glClear(GL_DEPTH_BUFFER_BIT));
	//
	glm::mat4 model(1.0f);
	model = glm::translate(model, glm::vec3(float(world.side())) * -0.5f);
	model = glm::translate(model, glm::vec3(0.5f));
	this->_shadowProgram.bind();
	this->_shadowProgram.uniformMat4f("uModelBase", model);
	this->_shadowProgram.uniformMat4f("uLightSpace", this->lightSpaceMat(world));
	//
	GL_CALL(glEnable(GL_DEPTH_TEST));
	GL_CALL(glEnable(GL_CULL_FACE));
	GL_CALL(glCullFace(GL_FRONT));
	GL_CALL(glBindVertexArray(this->_cubeInstVAO));
	GL_CALL(glDrawElementsInstanced(GL_TRIANGLES, 6 * 6, GL_UNSIGNED_INT, NULL, this->_cellsToDrawCount));
	GL_CALL(glBindVertexArray(NULL));
	GL_CALL(glCullFace(GL_BACK));
	GL_CALL(glDisable(GL_CULL_FACE));
	GL_CALL(glDisable(GL_DEPTH_TEST));
	//
	this->_shadowProgram.unbind();
	//
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, NULL));
}

void Renderer::_gridPass(const World& world, Camera& camera) {
	glm::mat4 grid(1.0f);
	grid = glm::scale(grid, glm::vec3(float(world.side())));
	glm::mat4 mat = camera.matrix() * grid;
	//
	this->_gridProgram.bind();
	this->_gridProgram.uniformMat4f("uMat", mat);
	//
	GL_CALL(glBindVertexArray(this->_gridVAO));
	GL_CALL(glDrawElements(GL_LINES, 2 * (4 + 4 + 4), GL_UNSIGNED_INT, NULL));
	GL_CALL(glBindVertexArray(NULL));
	//
	this->_gridProgram.unbind();
}

void Renderer::_cubesPass(const World& world, Camera& camera) {
	glm::mat4 model(1.0f);
	model = glm::translate(model, glm::vec3(float(world.side())) * -0.5f);
	model = glm::translate(model, glm::vec3(0.5f));
	glm::mat4 viewproj = camera.matrix();
	//
	this->_cubeInstProgram.bind();
	this->_cubeInstProgram.uniformVec3f("uViewPos", camera.pos());
	this->_cubeInstProgram.uniformVec3f("uLightDir", this->_lightDir);
	this->_cubeInstProgram.uniformVec3f("uLightCol", this->_lightCol);
	this->_cubeInstProgram.uniform1f("uAmbCoeff", this->_lightAmbCoeff);
	this->_cubeInstProgram.uniform1f("uDifCoeff", this->_lightDifCoeff);
	this->_cubeInstProgram.uniform1f("uSpeCoeff", this->_lightSpeCoeff);
	this->_cubeInstProgram.uniformMat4f("uViewProj", viewproj);
	this->_cubeInstProgram.uniformMat4f("uModelBase", model);
	this->_cubeInstProgram.uniformMat4f("uLightSpace", this->lightSpaceMat(world));
	this->_shadowProgram.uniform1i("uShadowMap", 0);
	GL_CALL(glActiveTexture(GL_TEXTURE0));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, this->_shadowTex));
	//
	//GL_CALL(glEnable(GL_FRAMEBUFFER_SRGB));
	GL_CALL(glEnable(GL_DEPTH_TEST));
	GL_CALL(glEnable(GL_CULL_FACE));
	GL_CALL(glBindVertexArray(this->_cubeInstVAO));
	GL_CALL(glDrawElementsInstanced(GL_TRIANGLES, 6 * 6, GL_UNSIGNED_INT, NULL, this->_cellsToDrawCount));
	GL_CALL(glBindVertexArray(NULL));
	GL_CALL(glDisable(GL_CULL_FACE));
	GL_CALL(glDisable(GL_DEPTH_TEST));
	//GL_CALL(glDisable(GL_FRAMEBUFFER_SRGB));
	//
	this->_cubeInstProgram.unbind();
}

glm::mat4 Renderer::lightSpaceMat(const World& world) const {
	glm::mat4 lightProj(1.0f), lightView(1.0f);
	lightProj = glm::ortho<float>(-world.side(), world.side(), -world.side(), world.side(), 0.1f, 1000.0f);
	glm::vec3 lightPos = glm::vec3(-this->_lightDir);
	//lightPos *= world.side();
	lightView = glm::lookAt<float>(lightPos, glm::vec3(0), glm::vec3(0, 1, 0));
	return lightProj * lightView;
}

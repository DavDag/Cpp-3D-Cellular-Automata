#include "app.hpp"
#include "./utils/hwinfo.hpp"
#include "./utils/opengl/opengl.hpp"
#include "./command/command_hwinfo.hpp"
#include "./command/command_deps.hpp"
#include "./command/command_sim.hpp"
#include "./command/command_cam.hpp"

#include <imgui.h>
#include <GLFW/glfw3.h>
#include <stdarg.h>
#include <string>

App::App():
	_console(*this, 256, 4096, false),
	_display(*this, 1),
	_simulation(*this),
	_camera(*this, 45.0f, 0.1f, 1000.0f, glm::vec3(0, 1, 0), glm::vec3(0.0f))
{
	this->_showUI = false;
	this->_commands = std::vector<Command*>({
		new CommandHwInfo(),
		new CommandDeps(),
		new CommandSim(),
		new CommandCam(),
	});
	//
	this->_cameraAngleX = 0;
	this->_cameraAngleY = 0;
}

void App::initialize() {
	this->_console.initialize();
	this->_display.initialize();
	this->_simulation.initialize();
	this->_camera.locktarget(glm::vec3(0.0f));
	this->_camera.setzoomLimits(0.1f, 10.0f);
}

void App::update(double dtSec) {
	this->_console.update(dtSec);
	this->_display.update(dtSec);
	this->_simulation.update(dtSec);
}

void App::render(int w, int h) {
	///////////////////////////////////////
	// Camera update
	glm::vec4 pos(0.0f, 0.0f, -1.0f, 1.0f);
	glm::mat4 rotX(1.0f), rotY(1.0f);
	rotX = glm::rotate<float>(rotX, glm::radians((float)this->_cameraAngleX), glm::vec3(0, 1, 0));
	rotY = glm::rotate<float>(rotY, glm::radians((float)this->_cameraAngleY), glm::vec3(1, 0, 0));
	pos = (rotX * pos + (rotX * rotY) * pos);
	pos = glm::normalize(pos);
	pos *= (1.0f / this->_camera.zoom()) * this->_simulation.size() * 3.0f;
	this->_camera.setpos(pos);
	this->_camera.setviewport(w, h);

	///////////////////////////////////////
	// Simulation update
	GL_CALL(glEnable(GL_DEPTH_TEST));
	GL_CALL(glEnable(GL_CULL_FACE));
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	this->_simulation.render(w, h);
	GL_CALL(glDisable(GL_CULL_FACE));
	GL_CALL(glDisable(GL_DEPTH_TEST));

	///////////////////////////////////////
	// UI
	if (this->_showUI) {
		this->_console.render(w, h);
		this->_display.render(w, h);
	}

	//ImGui::ShowDemoWindow();
}

void App::onKeyDown(int key) {
	switch (key) {
		case GLFW_KEY_TAB: {
			this->_showUI = !this->_showUI;
			break;
		}

		case GLFW_KEY_A:
		case GLFW_KEY_LEFT: {
			this->_cameraAngleX -= 5;
			this->_cameraAngleX %= 360;
			break;
		}
		
		case GLFW_KEY_D:
		case GLFW_KEY_RIGHT: {
			this->_cameraAngleX += 5;
			this->_cameraAngleX %= 360;
			break;
		}

		case GLFW_KEY_S:
		case GLFW_KEY_DOWN: {
			this->_cameraAngleY -= 5;
			this->_cameraAngleY = glm::max(this->_cameraAngleY, -90);
			break;
		}

		case GLFW_KEY_W:
		case GLFW_KEY_UP: {
			this->_cameraAngleY += 5;
			this->_cameraAngleY = glm::min(this->_cameraAngleY, 90);
			break;
		}

		default:
			break;
	}
}

void App::onKeyUp(int key) {

}

void App::onMouseBtnDown(int btn) {

}

void App::onMouseBtnUp(int btn) {

}

void App::onMouseWheel(double dx, double dy) {
	this->_camera.movezoomPercentage(0.1f * (float) dy); // 10%
}

void App::onResize(int width, int height) {
	glViewport(0, 0, width, height);
	this->_camera.setviewport(width, height);
}

void App::parse(const char* cmd) {
	this->raw(">> %s", cmd);

	if (strcmp(cmd, "help") == 0) {
		char buffer[128];
		for (const auto& c : this->_commands) {
			c->description(buffer, 128);
			this->inf(buffer);
		}
		return;
	}

	// Parse cmd, substitute ' ' with '\0'
	// and "create" a string list.
	int cmdlen = (int) strlen(cmd);
	char buffer[512] = { '0' };
	strncpy_s(buffer, 512, cmd, cmdlen);
	const int maxArgc = 32;
	const char* args[maxArgc] = { nullptr };
	int argc = 0;
	args[argc++] = &buffer[0];
	for (int i = 0; i < cmdlen; ++i) {
		const char c = cmd[i];
		buffer[i] = c;
		if (c == ' ') {
			buffer[i] = '\0';
			args[argc++] = &buffer[i + 1];
		}
	}

	// Search for matching Command
	bool found = false;
	for (Command* command : this->_commands)
		if (command->test(args[0])) {
			CommandArgs* out;
			if (command->parse(argc, args, out)) {
				this->execute(command->type, out);
			}
			else {
				this->inf(command->help());
			}
			found = true;
			break;
		}
	if (!found) {
		this->err("Unrecognized command.\nType 'help' to get available commands");
	}
}

void App::execute(int type, CommandArgs* args) {
	switch (type) {
		case CommandHwInfo::TYPE: {
			CommandHwInfoArgs& nargs = *(CommandHwInfoArgs*)args;
			this->inf(
				"OpenGL: %s\nGPU: %s | %s\nCPU: %s | %s\nThreads: %d\nRAM: %.2f (GB)",
				hwinfo::opengl::version(),
				hwinfo::gpu::vendor(), hwinfo::gpu::renderer(),
				hwinfo::cpu::vendor(), hwinfo::cpu::brand(),
				hwinfo::cpu::threadCount(),
				hwinfo::mem::physicalTotMb() / 1024
			);
			break;
		}

		case CommandDeps::TYPE: {
			CommandDepsArgs& nargs = *(CommandDepsArgs*)args;
			this->inf(
				"GLFW: %s\nGLEW: %s\nIMGUI: %s\nGLM: %s",
				hwinfo::deps::glfwVersion(),
				hwinfo::deps::glewVersion(),
				hwinfo::deps::imguiVersion(),
				hwinfo::deps::glmVersion()
			);
			break;
		}

		case CommandSim::TYPE: {
			CommandSimArgs& nargs = *(CommandSimArgs*)args;
			switch (nargs.type) {
				case CommandSimArgs::Type::INFO:
					this->_simulation.info();
					break;

				case CommandSimArgs::Type::PAUSE:
					this->_simulation.pause();
					break;

				case CommandSimArgs::Type::RESUME:
					this->_simulation.resume();
					break;

				case CommandSimArgs::Type::RESET:
					if (nargs.data.newseed != -1)
						this->_simulation.setseed(nargs.data.seed);
					else
						this->_simulation.reset();
					break;

				case CommandSimArgs::Type::STEP:
					this->_simulation.step(nargs.data.step);
					break;

				case CommandSimArgs::Type::SPEED:
					this->_simulation.setspeed(nargs.data.speed);
					break;

				case CommandSimArgs::Type::SIZE:
					this->_simulation.setsize(nargs.data.size);
					break;

				case CommandSimArgs::Type::SEED:
					this->_simulation.setseed(nargs.data.seed);
					break;

				case CommandSimArgs::Type::RULE:
					this->_simulation.setrule(nargs.data.rule);
					break;

				case CommandSimArgs::Type::COLORRULE:
					this->_simulation.setcolorrule(nargs.data.colorrule);
					break;

				case CommandSimArgs::Type::NONE:
				default:
					// Should never happpen
					this->err("invalid cmd subtype!");
					break;
			}
			break;
		}

		case CommandCam::TYPE: {
			CommandArgsCam& nargs = *(CommandArgsCam*)args;
			switch (nargs.type) {
				case CamCmd::INFO:
					this->_camera.info();
					break;

				case CamCmd::NONE:
				default:
					// Should never happpen
					this->err("[error] invalid cmd subtype!");
					break;
			}
			break;
		}

		case 0:
		default:
			// Should never happpen
			this->err("[error] invalid cmd type!");
			break;
	}

	return;
}

const glm::mat4& App::camera() {
	return this->_camera.matrix();
}

void App::raw(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	this->__log(LineData::Type::NONE, IM_COL32(255, 255, 255, 255), fmt, args);
	va_end(args);
}

void App::inf(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	this->__log(LineData::Type::INFO, IM_COL32(192, 192, 192, 255), fmt, args);
	va_end(args);
}

void App::deb(const char* fmt, ...) {
#ifdef _DEBUG
	va_list args;
	va_start(args, fmt);
	this->__log(LineData::Type::DEBUG, IM_COL32(128, 128, 128, 255), fmt, args);
	va_end(args);
#endif
}

void App::err(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	this->__log(LineData::Type::ERROR, IM_COL32(255, 0, 0, 255), fmt, args);
	va_end(args);
}

void App::__log(LineData::Type type, ImU32 col, const char* fmt, va_list args) {
	static char buff[4096] = { {'\0'} };
	vsprintf_s(buff, fmt, args);
	this->_console.log(type, col, buff);
}

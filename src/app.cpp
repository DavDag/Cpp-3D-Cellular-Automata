#include "app.hpp"
#include "./utils/hwinfo.hpp"
#include "./utils/opengl/opengl.hpp"
#include "./command/command_hwinfo.hpp"
#include "./command/command_deps.hpp"
#include "./command/command_sim.hpp"
#include "./command/command_cam.hpp"

#include <GLFW/glfw3.h>
#include <string>

App::App():
	_console(*this, 128, 256, false),
	_display(*this, 1),
	_simulation(*this),
	_camera(45.0f, 0.1f, 100.0f, glm::vec3(10.0f, 10.0f, -20.0f), glm::vec3(0.0f))
{
	this->_showUI = false;
	this->_commands = std::vector<Command*>({
		new CommandHwInfo(),
		new CommandDeps(),
		new CommandSim(),
		new CommandCam(),
	});
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
	this->_camera.setviewport(w, h);
	GL_CALL(glEnable(GL_DEPTH_TEST));
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	this->_simulation.render(w, h);
	if (this->_showUI) {
		this->_console.render(w, h);
		this->_display.render(w, h);
	}
	GL_CALL(glDisable(GL_DEPTH_TEST));
}

void App::onKeyDown(int key) {
	if (key == GLFW_KEY_TAB) this->_showUI = !this->_showUI;
}

void App::onKeyUp(int key) {

}

void App::onMouseBtnDown(int btn) {

}

void App::onMouseBtnUp(int btn) {

}

void App::onMouseWheel(double dx, double dy) {
	this->_camera.movezoomPercentage(-0.1f * dy); // 10%
}

void App::onResize(int width, int height) {
	glViewport(0, 0, width, height);
	this->_camera.setviewport(width, height);
}

void App::executeCmd(const char* cmd) {
	this->_console.log(cmd);

	if (strcmp(cmd, "help") == 0) {
		char buffer[128];
		for (const auto& c : this->_commands) {
			c->description(buffer, 128);
			this->_console.log(buffer);
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
				this->_console.log(command->help());
			}
			found = true;
			break;
		}
	if (!found) {
		this->_console.log("Unrecognized command.\nType 'help' to get available commands");
	}
}

void App::execute(int type, CommandArgs* args) {
	switch (type) {
		case CommandHwInfo::TYPE: {
			CommandArgsHwInfo& nargs = *(CommandArgsHwInfo*)args;
			this->_console.log(
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
			CommandArgsDeps& nargs = *(CommandArgsDeps*)args;
			this->_console.log(
				"GLFW: %s\nGLEW: %s\nIMGUI: %s\nGLM: %s",
				hwinfo::deps::glfwVersion(),
				hwinfo::deps::glewVersion(),
				hwinfo::deps::imguiVersion(),
				hwinfo::deps::glmVersion()
			);
			break;
		}

		case CommandSim::TYPE: {
			CommandArgsSim& nargs = *(CommandArgsSim*)args;
			// TODO
			this->_console.log(
				"Simulation %d %d",
				nargs.type,
				nargs.stepCount
			);
			break;
		}

		case CommandCam::TYPE: {
			CommandArgsCam& nargs = *(CommandArgsCam*)args;
			switch (nargs.type) {
				case CamCmd::INFO: {
					char buffer[512];
					this->_camera.info(buffer, 512);
					this->_console.log("%s", buffer);
					break;
				}

				case CamCmd::NONE:
				default:
					this->_console.err("[error] invalid cmd subtype!");
					// Should never happpen
					break;
			}
			break;
		}

		case 0:
		default:
			this->_console.err("[error] invalid cmd type!");
			// Should never happpen
			break;
	}

	return;
}

const glm::mat4& App::camera() {
	return this->_camera.matrix();
}

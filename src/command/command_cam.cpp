#include "command_cam.hpp"

CommandCam::CommandCam() :
	Command(CommandCam::TYPE, "cam", 2, 2)
{
	this->_result = {
		.type = CamCmd::NONE
	};
	// TODO: add extra commands
}

const char* CommandCam::help() {
	return R"(<cam>
Shows informations about the camera.
-info
)";
}

bool CommandCam::__parse(int argc, const char* args[], CommandArgs*& out) {
	this->_result = {
		.type = CamCmd::NONE,
	};
	switch (argc) {
		case 2: {
			if (strcmp(args[1], "info") == 0)
				this->_result.type = CamCmd::INFO;
			break;
		}
	}
	if (this->_result.type == CamCmd::NONE)
		return false;
	out = &this->_result;
	return true;
}

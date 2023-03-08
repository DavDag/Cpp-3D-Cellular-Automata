#include "command_sim.hpp"

CommandSim::CommandSim():
	Command(CommandSim::TYPE, "sim", 2, 3)
{
	this->_result = {
		.type = SimCmd::NONE,
		.stepCount = 0,
	};
}

const char* CommandSim::help() {
	return R"(<sim>
Lets you control the simulation.
Valid choices are:
-pause
-resume
-reset
-step <count>
-speed <tickPerSec>
-size <side>
-seed <x>
)";
}

bool CommandSim::__parse(int argc, const char* args[], CommandArgs*& out) {
	this->_result = {
		.type = SimCmd::NONE,
		.stepCount = 0,
	};
	switch (argc) {
		case 2: {
			if (strcmp(args[1], "pause") == 0)
				this->_result.type = SimCmd::PAUSE;
			else if (strcmp(args[1], "resume") == 0)
				this->_result.type = SimCmd::RESUME;
			else if (strcmp(args[1], "reset") == 0)
				this->_result.type = SimCmd::RESET;
			break;
		}
		case 3: {
			if (strcmp(args[1], "step") == 0) {
				this->_result.type = SimCmd::STEP;
				if (!__parseIntOrFalse(args[2], this->_result.stepCount))
					return false;
			}
			break;
		}
	}
	if (this->_result.type == SimCmd::NONE)
		return false;
	out = &this->_result;
	return true;
}

#include "command_sim.hpp"

CommandSim::CommandSim():
	Command(CommandSim::TYPE, "sim", 2, 3)
{
	this->_result.type = CommandSimArgs::Type::NONE;
}

const char* CommandSim::help() {
	return R"(<sim>
Lets you control the simulation.
Valid choices are:
-info
-pause
-resume
-reset [--newseed]
-step <count>
-speed <tickPerSec>
-size <side>
-seed <x>
-rule <string>
-colorrule <pos3d|decay>
)";
}

bool CommandSim::__parse(int argc, const char* args[], CommandArgs*& out) {
	this->_result.type = CommandSimArgs::Type::NONE;
	switch (argc) {
		case 2: {
			if (strcmp(args[1], "info") == 0)
				this->_result.type = CommandSimArgs::Type::INFO;
			else if (strcmp(args[1], "pause") == 0)
				this->_result.type = CommandSimArgs::Type::PAUSE;
			else if (strcmp(args[1], "resume") == 0)
				this->_result.type = CommandSimArgs::Type::RESUME;
			else if (strcmp(args[1], "reset") == 0) {
				this->_result.type = CommandSimArgs::Type::RESET;
				this->_result.data.newseed = -1;
			}
			break;
		}
		case 3: {
			if (strcmp(args[1], "reset") == 0) {
				this->_result.type = CommandSimArgs::Type::RESET;
				if (strcmp(args[2], "--newseed") == 0)
					this->_result.data.newseed = rand();
				else
					return false;
			}
			else if (strcmp(args[1], "step") == 0) {
				this->_result.type = CommandSimArgs::Type::STEP;
				if (!__parseIntOrFalse(args[2], this->_result.data.step))
					return false;
			}
			else if (strcmp(args[1], "speed") == 0) {
				this->_result.type = CommandSimArgs::Type::SPEED;
				if (!__parseIntOrFalse(args[2], this->_result.data.speed))
					return false;
			}
			else if (strcmp(args[1], "size") == 0) {
				this->_result.type = CommandSimArgs::Type::SIZE;
				if (!__parseIntOrFalse(args[2], this->_result.data.size))
					return false;
			}
			else if (strcmp(args[1], "seed") == 0) {
				this->_result.type = CommandSimArgs::Type::SEED;
				if (!__parseIntOrFalse(args[2], this->_result.data.seed))
					return false;
			}
			else if (strcmp(args[1], "rule") == 0) {
				this->_result.type = CommandSimArgs::Type::RULE;
				// TODO
				return false;
			}
			else if (strcmp(args[1], "colorrule") == 0) {
				this->_result.type = CommandSimArgs::Type::COLORRULE;
				if (strcmp(args[2], "pos3d") == 0)
					this->_result.data.colorrule = ColorRule::POS3D;
				else if (strcmp(args[2], "decay") == 0)
					this->_result.data.colorrule = ColorRule::DECAY;
				else if (strcmp(args[2], "density") == 0)
					this->_result.data.colorrule = ColorRule::DENSITY;
				else {
					this->_result.data.colorrule = ColorRule::NONE;
					return false;
				}
			}
			break;
		}
	}
	if (this->_result.type == CommandSimArgs::Type::NONE)
		return false;
	out = &this->_result;
	return true;
}

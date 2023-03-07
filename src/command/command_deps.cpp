#include "command_deps.hpp"

CommandDeps::CommandDeps() :
	Command(CommandDeps::type, "deps", 1, 1)
{
}

const char* CommandDeps::help() {
	return "<deps>\nShows informations about installed libraries";
}

bool CommandDeps::__parse(int argc, const char* args[], CommandArgs*& out) {
	out = &this->_result;
	return true;
}

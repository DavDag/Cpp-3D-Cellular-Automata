#include "command_hwinfo.hpp"

CommandHwInfo::CommandHwInfo():
	Command(CommandHwInfo::type, "hwinfo", 1, 1)
{
}

const char* CommandHwInfo::help() {
	return "<hwinfo>\nShows informations about CPU, GPU & others";
}

bool CommandHwInfo::__parse(int argc, const char* args[], CommandArgs*& out) {
	out = &this->_result;
	return true;
}

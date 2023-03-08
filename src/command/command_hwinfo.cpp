#include "command_hwinfo.hpp"

CommandHwInfo::CommandHwInfo():
	Command(CommandHwInfo::TYPE, "hwinfo", 1, 1)
{
}

const char* CommandHwInfo::help() {
	return R"(<hwinfo>
Shows informations about: OpenGL, CPU, GPU & RAM
)";
}

bool CommandHwInfo::__parse(int argc, const char* args[], CommandArgs*& out) {
	out = &this->_result;
	return true;
}

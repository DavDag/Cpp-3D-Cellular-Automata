#include "command.hpp"

#include <string>

Command::Command(
	crc32 type,
	const char* name,
	int argcMin /* = 1*/,
	int argcMax /* = 1*/
):
	type(type),
	_name(name),
	_argcMin(argcMin),
	_argcMax(argcMax)
{
}

void Command::description(char* buffer, int size) {
	memset(buffer, '\0', size);
	snprintf(buffer, size,
		"Type:<%u>\nName:<%s>\nArgs:[%d,%d]",
		this->type, this->_name, this->_argcMin, this->_argcMax
	);
}

bool Command::test(const char* name) {
	return (strcmp(this->_name, name) == 0);
}

bool Command::parse(int argc, const char* args[], CommandArgs*& out) {
	// Check for invalid usage
	if (argc < this->_argcMin || argc > this->_argcMax)
		return false;
	// Check for "help"
	if (argc == 2 && strcmp(args[1], "help") == 0)
		return false;
	return this->__parse(argc, args, out);
}

bool Command::__parseIntOrFalse(const char* arg, int& out) {
	char* endptr;
	long parsed = strtol(arg, &endptr, 10);
	if (*endptr != '\0')
		return false;
	out = (int) parsed;
	return true;
}

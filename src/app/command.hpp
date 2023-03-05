#pragma once

#include "../utils/crc32.hpp"
#include <string>

#define COMMAND_UID() computeCRC32(__FILE__, sizeof(__FILE__)-1)
#define COMMAND_TYPE() static constexpr crc32 type = COMMAND_UID();

struct CommandArgs { };

class Command {
public:
	crc32 type;

public:
	Command(crc32 type, const char* name, int argMin = 1, int argMax = 1);

	void description(char* buffer, int size);
	bool test(const char* name);
	bool parse(int argc, const char* args[], CommandArgs*& out);
	virtual const char* help() = 0;

protected:
	virtual bool __parse(int argc, const char* args[], CommandArgs*& out) = 0;

	static bool __parseIntOrFalse(const char* arg, int& out);

private:
	const char* _name;
	const int _argcMax;
	const int _argcMin;
};

#pragma once

#include "./command.hpp"

struct CommandDepsArgs : CommandArgs {};

class CommandDeps : public Command {
public:
	static constexpr crc32 TYPE = COMMAND_UID();
public:
	CommandDeps();
	const char* help() override;
protected:
	bool __parse(int argc, const char* args[], CommandArgs*& out) override;
private:
	CommandDepsArgs _result;
};

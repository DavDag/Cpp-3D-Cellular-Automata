#pragma once

#include "./command.hpp"

struct CommandHwInfoArgs : CommandArgs {
};

class CommandHwInfo : public Command {
public:
	static constexpr crc32 TYPE = COMMAND_UID();
public:
	CommandHwInfo();
	const char* help() override;
protected:
	bool __parse(int argc, const char* args[], CommandArgs*& out) override;
private:
	CommandHwInfoArgs _result;
};

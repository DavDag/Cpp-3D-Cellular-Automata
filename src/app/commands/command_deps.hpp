#pragma once

#include "../command.hpp"

struct CommandArgsDeps: CommandArgs {
};

class CommandDeps : public Command {
public:
	COMMAND_TYPE()
public:
	CommandDeps();
	const char* help() override;
protected:
	bool __parse(int argc, const char* args[], CommandArgs*& out) override;
private:
	CommandArgsDeps _result;
};

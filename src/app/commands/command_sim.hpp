#pragma once

#include "../command.hpp"

enum class SimCmd { NONE = 0, PAUSE, RESUME, RESET, STEP };
struct CommandArgsSim: CommandArgs {
	SimCmd type;
	int stepCount;
};

class CommandSim : public Command {
public:
	COMMAND_TYPE()
public:
	CommandSim();
	const char* help() override;
protected:
	bool __parse(int argc, const char* args[], CommandArgs*& out) override;
private:
	CommandArgsSim _result;
};

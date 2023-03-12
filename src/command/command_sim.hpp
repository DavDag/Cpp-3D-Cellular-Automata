#pragma once

#include "./command.hpp"
#include "../simulation/simulation.hpp"

struct CommandSimArgs: CommandArgs {
	enum Type {
		NONE = 0,
		INFO,
		PAUSE,
		RESUME,
		RESET,
		STEP,
		SPEED,
		SIZE,
		SEED,
		RULE,
		COLORRULE,
	} type;
	union {
		// none, pause, resume
		int newseed; // reset
		int step; // step
		int speed; // speed
		int seed; // seed
		int size; // size
		SimRule rule; // rule
		ColorRule colorrule; // colorrule
	} data;
	CommandSimArgs() : type(Type::NONE), data{ 0 } {}
};

class CommandSim : public Command {
public:
	static constexpr crc32 TYPE = COMMAND_UID();
public:
	CommandSim();
	const char* help() override;
protected:
	bool __parse(int argc, const char* args[], CommandArgs*& out) override;
private:
	CommandSimArgs _result;
};

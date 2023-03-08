#pragma once

#include "./command.hpp"

enum class CamCmd { NONE = 0, INFO };
struct CommandArgsCam : CommandArgs {
	CamCmd type;
};

class CommandCam : public Command {
public:
	static constexpr crc32 TYPE = COMMAND_UID();
public:
	CommandCam();
	const char* help() override;
protected:
	bool __parse(int argc, const char* args[], CommandArgs*& out) override;
private:
	CommandArgsCam _result;
};

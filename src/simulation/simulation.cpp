#include "simulation.hpp"
#include "../app.hpp"

#include <random>

#define MAX_RULE_LENGHT 512

void SimRule::logIntoBufferAsString(char* buffer, int buffersize) const {
	auto sprintSetCompressed = [](std::unordered_set<int>& toprint, char* buffer, int buffersize) {
		int i = 0, beg = -1, end = -1;
		for (int ni = 0; ni < 27; ++ni)
			if (toprint.contains(ni)) {
				// Without range-begin
				if (beg == -1) {
					beg = ni; // assign range-begin
				}
				// Without range-end
				else if (end == -1) {
					// Distance 1
					if ((ni - beg) == 1) {
						end = ni; // assign range-end
					}
					else {
						i += sprintf_s(buffer + i, buffersize - i, (i == 0) ? "%d" : ",%d", beg); // log
						beg = ni; // assign range-begin
					}
				}
				// Inside range
				else {
					// Distance 1
					if ((ni - end) == 1) {
						end = ni; // update range-end
					}
					else {
						i += sprintf_s(buffer + i, buffersize - i, (i == 0) ? "%d-%d" : ",%d-%d", beg, end); // log
						beg = ni; // assign range-begin
						end = -1; // clear range-end
					}
				}
			}
		// Ended with range-alive
		if (end != -1) {
			i += sprintf_s(buffer + i, buffersize - i, (i == 0) ? "%d-%d" : ",%d-%d", beg, end); // log
		}
		else {
			i += sprintf_s(buffer + i, buffersize - i, (i == 0) ? "%d" : ",%d", beg); // log
		}
	};
	char tmp1[128], tmp2[128];
	sprintSetCompressed(*this->aliveWith, tmp1, 128);
	sprintSetCompressed(*this->bornWith, tmp2, 128);
	memset(buffer, '\0', buffersize);
	sprintf_s(buffer, buffersize, "%s/%s/%d/%c", tmp1, tmp2, this->stateCount, this->method);
};

const char* SimRule::updateFromString(const char* buffer, int buffersize) {
	char errorbuffer[256] = { {'\0'} };
	if (buffersize < 7) return "Invalid rule size: at least 7 characters are required";
	/////////////////////////////////////////////////////
	// Parse
	int localbufferindex = 0;
	char localbuffer[MAX_RULE_LENGHT * 2] = { {'\0'} };
	std::vector<const char*> tokens;
	int j = 0;
	for (int i = 0; i < buffersize - 1; ++i) {
		const char c = buffer[i];
		bool isComma = (c == ',');
		bool isDash = (c == '-');
		bool isSlash = (c == '/');
		bool isToken = isComma || isDash || isSlash;
		if (isToken) {
			int n = (i - j);
			//printf("Found token %c at %d. Parsing argument [%d, %d]\n", c, i, j, j + n);
			memcpy_s(
				localbuffer + localbufferindex,
				MAX_RULE_LENGHT * 2 - localbufferindex,
				buffer + j,
				n
			);
			// argument
			j = i + 1;
			tokens.push_back(localbuffer + localbufferindex);
			localbufferindex += n;
			localbuffer[localbufferindex++] = '\0';
			// separator
			tokens.push_back(localbuffer + localbufferindex);
			localbuffer[localbufferindex] = c;
			localbufferindex += 1;
			localbuffer[localbufferindex++] = '\0';
		}
		// advance
	}
	//for (auto t : tokens) printf("Token: %s\n", t);
	if (j != buffersize - 1) return "Invalid end. Must be a single character for method selection";
	// extract data
	std::vector<int> aliveWith;
	std::vector<int> bornWith;
	int statusCount = 0;
	int partIndex = 0;
	int last = -1;
	for (auto t : tokens) {
		if (strcmp(t, "/") == 0) {
			if (last == -1) return "Invalid '/' token";
			++partIndex;
			last = -1;
			if (partIndex > 3) return "Invalid '/' token";
			continue;
		}
		if (strcmp(t, ",") == 0) {
			if (last == -1) return "Invalid ',' token";
			continue;
		}
		if (strcmp(t, "-") == 0) {
			if (last == -1) return "Invalid '-' token";
			if (partIndex == 0) aliveWith.push_back(-1);
			if (partIndex == 1) bornWith.push_back(-1);
			if (partIndex >= 2) return "Invalid '-' token";
			continue;
		}
		if (partIndex == 3) return "Invalid format after third '/'";
		int len = strlen(t);
		char* end;
		long res = strtol(t, &end, 10);
		if (end == t || end != t + len) {
			sprintf_s(errorbuffer, "Invalid number: %s", t);
			return errorbuffer;
		}
		if (res < 0) return "Unacceptable negative number";
		if (res > 26) return "Unacceptable number greater than 26";
		if (partIndex == 2 && statusCount != 0) return "Multiple 'StatesCount' given";
		if (partIndex == 0) aliveWith.push_back(res);
		if (partIndex == 1) bornWith.push_back(res);
		if (partIndex == 2) statusCount = res;
		last = res;
	}
	/////////////////////////////////////////////////////
	// Method
	const char lastchar = buffer[buffersize - 1];
	Method method = Method::NONE;
	if (lastchar == Method::MOORE) method = Method::MOORE;
	if (lastchar == Method::NEUMANN) method = Method::NEUMANN;
	if (method == Method::NONE) return "Invalid method character. Use 'M' or 'N'";
	/////////////////////////////////////////////////////
	// Apply
	this->aliveWith->clear();
	this->aliveWith->insert(aliveWith.begin(), aliveWith.end());
	for (int i = 0; i < aliveWith.size(); ++i)
		if (aliveWith[i] == -1)
			for (int j = aliveWith[i - 1]; j < aliveWith[i + 1]; j++)
				this->aliveWith->insert(j);
	this->bornWith->clear();
	this->bornWith->insert(bornWith.begin(), bornWith.end());
	for (int i = 0; i < bornWith.size(); ++i)
		if (bornWith[i] == -1)
			for (int j = bornWith[i - 1]; j < bornWith[i + 1]; j++)
				this->bornWith->insert(j);
	this->stateCount = statusCount;
	this->method = method;
	/////////////////////////////////////////////////////
	// Ok
	return nullptr;
}

void ColorRule::logIntoBufferAsString(char* buffer, int buffersize) const {
	switch (_type) {
		case ColorRule::Type::DECAY:
			sprintf_s(buffer, buffersize, "decay");
			break;
		
		case ColorRule::Type::POS3D:
			sprintf_s(buffer, buffersize, "pos3d");
			break;

		case ColorRule::Type::DENSITY:
			sprintf_s(buffer, buffersize, "density");
			break;

		case ColorRule::Type::NONE:
		default:
			sprintf_s(buffer, buffersize, "unspecified");
			break;
	}
}

Simulation::Simulation(App& app) :
	_app(app),
	_renderer(app, *this)
{
	this->_paused = true; // false
	this->_tickPerSec = 8;
	this->_timeSinceLastTickSec = 0;
	this->_timeAccSec = 0;
	//
	this->_newRuleBuffer = new char[MAX_RULE_LENGHT];
	memset(this->_newRuleBuffer, '\0', MAX_RULE_LENGHT);
	this->_rule = SimRule{
		.aliveWith = new std::unordered_set<int>({ 4 }),
		.bornWith = new std::unordered_set<int>({ 4 }),
		.stateCount = 5,
		.method = SimRule::Method::MOORE
	};
	this->_rule.logIntoBufferAsString(this->_newRuleBuffer, MAX_RULE_LENGHT);
	this->_colorRule = ColorRule::DECAY;
	//
	this->_seed = rand();
	this->_genprob = 50;
	this->_world = World(32);
	this->_renderer.setMaxCellCount(this->_world.size());
	//
	this->reset();
}

void Simulation::initialize() {
	this->_renderer.initialize();
}

void Simulation::update(double dtSec) {
	this->_timeAccSec += dtSec;
	this->_timeSinceLastTickSec += dtSec;
	float tickSpeedSec = 1.0f / this->_tickPerSec;
	while (this->_timeSinceLastTickSec >= tickSpeedSec) {
		this->_timeSinceLastTickSec -= tickSpeedSec;
		// Pausing stops "natural" ticks
		if (!this->_paused) this->__tick();
	}
	this->_renderer.update(dtSec);
}

void Simulation::__tick() {
	this->_app.deb("simulation tick");
	const int ws = this->_world.side();
	const int initialstate = this->_rule.stateCount - 1;
	for (int x = 0; x < ws; ++x)
		for (int y = 0; y < ws; ++y)
			for (int z = 0; z < ws; ++z) {
				WorldCell data = this->_world.get(x, y, z);
				//
				int neighbours = 0;
				switch (this->_rule.method) {
				case SimRule::Method::MOORE: {
					neighbours = this->_world.countMoore(initialstate, x, y, z);
					break;
				}
				case SimRule::Method::NEUMANN: {
					neighbours = this->_world.countNeumann(initialstate, x, y, z);
					break;
				}
				}
				data.neighbours = neighbours;
				//
				if (data.status == initialstate) {
					// alive
					if (this->_rule.aliveWith->contains(neighbours))
						data.nextstatus = data.status;
					else
						data.nextstatus = initialstate - 1;
				}
				else if (data.status == 0) {
					// born
					if (this->_rule.bornWith->contains(neighbours))
						data.nextstatus = initialstate;
					else
						data.nextstatus = 0;
				}
				else {
					// decrement
					data.nextstatus = data.status - 1;
				}
				this->_world.set(data, x, y, z);
			}
	this->_world.flip();
}

void Simulation::render(int w, int h) {
	this->_renderer.render(
		this->_world,
		this->_app.camera(),
		w, h
	);
}

void Simulation::ui(int w, int h) {
	/////////////////////////////////////////
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(w * 0.25f, h * 0.5f), ImGuiCond_FirstUseEver);
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;
	ImGui::Begin("Simulation", nullptr, windowFlags);
	float windowContentWidth = ImGui::GetWindowContentRegionWidth();
	/////////////////////////////////////////
	// Rule
	ImGui::SeparatorText("");
	char tmp1[MAX_RULE_LENGHT];
	this->_rule.logIntoBufferAsString(tmp1, MAX_RULE_LENGHT);
	ImGui::Text("Rule: %s", tmp1);
	ImGui::PushItemWidth(windowContentWidth);
	if (ImGui::InputText(
		"###newrule",
		this->_newRuleBuffer, MAX_RULE_LENGHT,
		ImGuiInputTextFlags_EnterReturnsTrue)) {
		int len = strnlen(this->_newRuleBuffer, MAX_RULE_LENGHT);
		this->setruleFromString(this->_newRuleBuffer);
	}
	static std::pair<const char*, const char*> rules[] = {
		{"445 (Jason Rampe)", "4/4/5/M"},
		{"Amoeba (Jason Rampe)", "9-26/5-7,12-13,15/5/M"},
		{"Architecture (Jason Rampe)", "4-6/3/2/M"},
		{"Builder 1 (Jason Rampe)", "2,6,9/4,6,8-9/10/M"},
		{"Builder 2 (Jason Rampe)", "5-7/1/2/M"},
		{"Clouds 1 (Jason Rampe)", "13-26/13-14,17-19/2/M"},
		{"Clouds 2 (Jason Rampe)", "12-26/13-14/2/M"},
		{"Construction (Jason Rampe)", "0-2,4,6-11,13-17,21-26/9-10,16,23-24/2/M"},
		{"Coral (Jason Rampe)", "5-8/6-7,9,12/4/M"},
		{"Crystal Growth (Jason Rampe) 1", "0-6/1,3/2/N"},
		{"Crystal Growth (Jason Rampe) 2", "1-2/1,3/5/N"},
		{"Diamond Growth (Jason Rampe)", "5-6/1-3/7/N"},
		{"Expanding Shell (Jason Rampe)", "6,7-9,11,13,15-16,18/6-10,13-14,16,18-19,22-25/5/M"},
		{"More Structures (Jason Rampe)", "7-26/4/4/M"},
		{"Pulse Waves (Jason Rampe)", "3/1-3/10/M"},
		{"Pyroclastic (Jason Rampe)", "4-7/6-8/10/M"},
		{"Sample 1 (Jason Rampe)", "10-26/5,8-26/4/M"},
		{"Shells (Jason Rampe)", "3,5,7,9,11,15,17,19,21,23-24,26/3,6,8-9,11,14-17,19,24/7/M"},
		//{"Single Point Replication (Jason Rampe)", "/1/2/M"},
		{"Slow Decay 1 (Jason Rampe)", "13-26/10-26/3/M"},
		{"Slow Decay 2 (Jason Rampe)", "1,4,8,11,13-26/13-26/5/M"},
		{"Spiky Growth (Jason Rampe)", "0-3,7-9,11-13,18,21-22,24,26/13,17,20-26/4/M"},
		{"Stable Structures (Evan Wallace)", "13-26/14-19/2/M"},
		//{"Symmetry (Jason Rampe)", "/2/10/M"},
		{"von Neumann Builder (Jason Rampe)", "1-3/1,4-5/5/N"}
	};
	if (ImGui::BeginCombo("###ruledatabase", "Choose from DB")) {
		for (auto pair : rules) {
			const char* name = pair.first;
			const char* rule = pair.second;
			bool selected = (strcmp(rule, this->_newRuleBuffer) == 0);
			if (ImGui::Selectable(name, selected)) {
				this->_app.inf("Requested: %s %s", name, rule);
				this->setruleFromString(rule);
			}
			if (selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	ImGui::PopItemWidth();
	/////////////////////////////////////////
	// Status
	ImGui::SeparatorText("");
	ImGui::Text("Status: %8s", (this->_paused) ? "paused" : "running");
	if (ImGui::Button("Pause")) this->pause();
	ImGui::SameLine();
	if (ImGui::Button("Resume")) this->resume();
	ImGui::SameLine();
	if (ImGui::Button("Reset")) this->reset();
	ImGui::SameLine();
	if (ImGui::Button("#1")) this->step(1);
	ImGui::SameLine();
	if (ImGui::Button("#5")) this->step(5);
	/////////////////////////////////////////
	// Speed
	ImGui::SeparatorText("");
	ImGui::Text("Speed: %d t/s", this->_tickPerSec);
	for (int i = 0; i <= 6; ++i) {
		int v = 1 << (i + 0);
		char buff[4];
		sprintf_s(buff, "%2d", v);
		if (i > 0) ImGui::SameLine();
		if (ImGui::RadioButton(buff, this->_tickPerSec == v)) this->setspeed(v);
	}
	ImGui::Text("Tick time (avg): %d ms", 12);
	ImGui::Text("Tick time (min): %d ms", 8);
	ImGui::Text("Tick time (max): %d ms", 44);
	/////////////////////////////////////////
	// Size
	ImGui::SeparatorText("");
	ImGui::Text("Size: %d x %d x %d", this->size(), this->size(), this->size());
	for (int i = 0; i <= 3; ++i) {
		int v = 1 << (i + 4);
		char buff[4];
		sprintf_s(buff, "%3d", v);
		if (i > 0) ImGui::SameLine();
		if (ImGui::RadioButton(buff, this->size() == v)) this->setsize(v);
	}
	ImGui::Text("Cell count: %8d", this->_world.size());
	ImGui::Text("World Size: %8.2f Mb", this->_world.size() * sizeof(WorldCell) / 1024.0f / 1024.0f);
	/////////////////////////////////////////
	// Seed
	ImGui::SeparatorText("");
	ImGui::Text("Seed: %04x", this->_seed);
	if (ImGui::Button("New Seed")) this->setseed(rand());
	ImGui::Text("Gen Prob: %3d %%", this->_genprob);
	for (int i = 0; i < 3; ++i) {
		int v = 25 * (i + 1);
		char buff[8];
		sprintf_s(buff, "%3d %%", v);
		if (i > 0) ImGui::SameLine();
		if (ImGui::RadioButton(buff, this->_genprob == v)) this->setgenprob(v);
	}
	/////////////////////////////////////////
	// ColorRule
	ImGui::SeparatorText("");
	char tmp2[32];
	this->_colorRule.logIntoBufferAsString(tmp2, 32);
	ImGui::Text("ColorRule: %s", tmp2);
	static const char* colorRulesStr[] = { "None", "Decay", "Density", "Pos3D" };
	static ColorRule colorRules[] = { ColorRule::NONE, ColorRule::DECAY, ColorRule::DENSITY, ColorRule::POS3D };
	for (int i = 0; i < 4; ++i) {
		ColorRule v = colorRules[i];
		if (i > 0) ImGui::SameLine();
		if (ImGui::RadioButton(colorRulesStr[i], this->_colorRule == v)) this->setcolorrule(v);
	}
	/////////////////////////////////////////
	ImGui::End();
	this->_renderer.ui(w, h);
}

void Simulation::info() const {
	char rulestring[512];
	this->_rule.logIntoBufferAsString(rulestring, 512);
	char colorrulestring[32];
	this->_colorRule.logIntoBufferAsString(colorrulestring, 32);
	this->_app.inf(
		"Status: %s\n"
		"Speed: %d (t/s)\n"
		"Size: %d x %d x %d\n"
		"Seed: %d\n"
		"Rule: %s\n"
		"ColorRule: %s\n",
		(this->_paused) ? "paused" : "running",
		this->_tickPerSec,
		this->_world.side(), this->_world.side(), this->_world.side(),
		this->_seed,
		rulestring,
		colorrulestring
	);
}

void Simulation::pause() {
	this->_app.deb("simulation paused");
	this->_paused = true;
}

void Simulation::resume() {
	this->_app.deb("simulation resumed");
	this->_paused = false;
}

void Simulation::reset() {
	this->_app.deb("simulation resetted");
	WorldCell emptydata = { .status = 0, .neighbours = 0, .nextstatus = 0 };
	for (int i = 0; i < this->_world.size(); ++i)
		this->_world.set(emptydata, i);
	//
	std::mt19937 gen(this->_seed);
	std::uniform_int_distribution<int> distr(0, 99);
	float cen = this->_world.side() / 2.0f;
	float off = glm::max(2.0f, this->_world.side() * 0.1f);
	int initialstate = this->_rule.stateCount - 1;
	//
	for (float dx = -off; dx < +off; dx += 1.0f)
		for (float dy = -off; dy < +off; dy += 1.0f)
			for (float dz = -off; dz < +off; dz += 1.0f) {
				int rnd = distr(gen);
				WorldCell data = {
					.status = (rnd < this->_genprob) ? initialstate : 0,
					.neighbours = 0,
					.nextstatus = 0
				};
				int x = (int) round(cen + dx);
				int y = (int) round(cen + dy);
				int z = (int) round(cen + dz);
				this->_world.set(data, x, y, z);
			}
	//
	for (float dx = -off; dx < +off; dx += 1.0f)
		for (float dy = -off; dy < +off; dy += 1.0f)
			for (float dz = -off; dz < +off; dz += 1.0f) {
				int x = (int)round(cen + dx);
				int y = (int)round(cen + dy);
				int z = (int)round(cen + dz);
				WorldCell data = this->_world.get(x, y, z);
				switch (this->_rule.method) {
					case SimRule::Method::MOORE: {
						data.neighbours = this->_world.countMoore(initialstate, x, y, z);
						break;
					}
					case SimRule::Method::NEUMANN: {
						data.neighbours = this->_world.countNeumann(initialstate, x, y, z);
						break;
					}
				}
				this->_world.set(data, x, y, z);
			}
}

void Simulation::step(int count) {
	this->_app.deb("simulation step requested: %d", count);
	for (int i = 0; i < count; ++i)
		this->__tick();
}

void Simulation::setspeed(int tickPerSec) {
	this->_app.deb("simulation speed updated to %d", tickPerSec);
	this->_tickPerSec = tickPerSec;
}

void Simulation::setsize(int side) {
	this->_app.deb("simulation size updated to %d x %d x %d (%d)", side, side, side, side * side * side);
	this->_world = World(side);
	this->_renderer.setMaxCellCount(this->_world.size());
	this->reset();
}

void Simulation::setseed(int seed) {
	this->_app.deb("simulation seed updated to %d", seed);
	this->_seed = seed;
	this->reset();
}

void Simulation::setgenprob(int genprob) {
	this->_app.deb("simulation genprob updated to %d", genprob);
	this->_genprob = genprob;
	this->reset();
}

void Simulation::setruleFromString(const char* rule) {
	int len = strlen(rule);
	const char* errorStr = this->_rule.updateFromString(rule, len);
	if (errorStr != nullptr) {
		this->_app.err("Rule parsing error: %s", errorStr);
		return;
	}
	this->_rule.logIntoBufferAsString(this->_newRuleBuffer, MAX_RULE_LENGHT);
	this->_app.deb("simulation rule updated to %s", this->_newRuleBuffer);
	this->reset();
}

void Simulation::setcolorrule(ColorRule rule) {
	char colorrulestring[32];
	rule.logIntoBufferAsString(colorrulestring, 32);
	this->_app.deb("simulation color rule updated to %s", colorrulestring);
	this->_colorRule = rule;
}

int Simulation::size() const {
	return this->_world.side();
}

void Simulation::applyColorRule(WorldCell& worldcell, GLCell& glcell) const {
	switch (this->_colorRule) {
		case ColorRule::POS3D: {
			float r = ((float)glcell.coords.x + 1) / this->_world.side();
			float g = ((float)glcell.coords.y + 1) / this->_world.side();
			float b = ((float)glcell.coords.z + 1) / this->_world.side();
			glcell.color.r = (GLubyte)(0xff * r);
			glcell.color.g = (GLubyte)(0xff * g);
			glcell.color.b = (GLubyte)(0xff * b);
			glcell.color._ = 0;
			break;
		}

		case ColorRule::DENSITY: {
			float density = 1.0f;
			switch (this->_rule.method) {
				case SimRule::Method::MOORE:
					density = ((float)worldcell.neighbours / 26.0f);
					break;

				case SimRule::Method::NEUMANN:
					density = ((float)worldcell.neighbours / 6.0f);
					break;
			}
			glcell.color.r = (GLubyte)(0xff * density);
			glcell.color.g = 0;
			glcell.color.b = 0;
			glcell.color._ = 0;
			break;
		}	

		case ColorRule::DECAY:
		case ColorRule::NONE:
		default: {
			float decay = ((float) worldcell.status / (this->_rule.stateCount - 1));
			glcell.color.r = (GLubyte)(0xff * decay);
			glcell.color.g = 0;
			glcell.color.b = 0;
			glcell.color._ = 0;
			break;
		}
	}
}

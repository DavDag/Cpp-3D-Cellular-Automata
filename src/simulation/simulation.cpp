#include "simulation.hpp"
#include "../app.hpp"

#include <random>

void SimRule::logIntoBufferAsString(char* buffer, int buffersize) const {
	//
	char tmp1[256];
	int i = 0;
	for (int ni = 0; ni < 27; ++ni)
		if (this->aliveWith->contains(ni))
			i += sprintf_s(tmp1 + i, 256 - i, (i == 0) ? "%d" : ",%d", ni);
	//
	char tmp2[256];
	int j = 0;
	for (int ni = 0; ni < 27; ++ni)
		if (this->bornWith->contains(ni))
			j += sprintf_s(tmp2 + j, 256 - j, (j == 0) ? "%d" : ",%d", ni);
	//
	sprintf_s(
		buffer, buffersize,
		"%s/%s/%d/%s",
		tmp1,
		tmp2,
		this->stateCount,
		(this->method == SimRule::Method::MOORE) ? "M" : "N"
	);
};

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
	this->_paused = false;
	this->_tickPerSec = 8;
	this->_timeSinceLastTickSec = 0;
	this->_timeAccSec = 0;
	//
	this->_rule = SimRule{
		.aliveWith = new std::unordered_set<int>({ 4 }),
		.bornWith = new std::unordered_set<int>({ 4 }),
		.stateCount = 5,
		.method = SimRule::Method::MOORE
	};
	this->_colorRule = ColorRule::POS3D;
	this->_seed = rand();
	this->_genprob = 50;
	this->_world = World(32);
	this->_renderer.setMaxCellCount(this->_world.size());
	//
	this->reset();
}

int Simulation::size() const {
	return this->_world.side();
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
	char tmp1[64];
	this->_rule.logIntoBufferAsString(tmp1, 64);
	ImGui::Text("Rule: %s", tmp1);
	/////////////////////////////////////////
	// Status
	ImGui::SeparatorText("");
	ImGui::Text("Status: %8s", (this->_paused) ? "paused" : "running");
	if (ImGui::Button("Pause")) this->pause();
	ImGui::SameLine();
	if (ImGui::Button("Resume")) this->resume();
	ImGui::SameLine();
	if (ImGui::Button("Reset")) this->reset();
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

void Simulation::setrule(const SimRule& rule) {
	char rulestring[512];
	rule.logIntoBufferAsString(rulestring, 512);
	this->_app.deb("simulation rule updated to %s", rulestring);
	this->_rule = rule;
	this->reset();
}

void Simulation::setcolorrule(ColorRule rule) {
	char colorrulestring[32];
	this->_colorRule.logIntoBufferAsString(colorrulestring, 32);
	this->_app.deb("simulation color rule updated to %s", colorrulestring);
	this->_colorRule = rule;
}

void Simulation::colorrule(WorldCell& worldcell, GLCell& glcell) const {
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

#pragma once

#include "world.hpp"
#include "renderer.hpp"

#include <unordered_set>

class App;

struct SimRule {
	std::unordered_set<int>* aliveWith;
	std::unordered_set<int>* bornWith;
	int stateCount;
	enum Method { NONE = 0, NEUMANN = 'N', MOORE = 'M' } method;
	//
	void logIntoBufferAsString(char* buffer, int buffersize) const;
	const char* updateFromString(const char* buffer, int buffersize);
};

class ColorRule {
public:
	enum Type {
		NONE = 0,
		POS3D,
		DECAY,
		DENSITY,
	};
	//
	ColorRule() : ColorRule(Type::NONE) {};
	constexpr ColorRule(Type type) { _type = type; }
	constexpr operator Type() const { return _type; }
	//
	void logIntoBufferAsString(char* buffer, int buffersize) const;

private:
	Type _type;
};

class Simulation {
public:
	Simulation(App& app);
	//
	void initialize();
	void update(double dtSec);
	void render(int w, int h);
	void ui(int w, int h);
	//
	void info() const;
	void pause();
	void resume();
	void reset();
	void step(int count);
	void setspeed(int tickPerSec);
	void setsize(int side);
	void setseed(int seed);
	void setgenprob(int genprob);
	void setruleFromString(const char* rule);
	void setcolorrule(ColorRule rule);
	//
	int size() const;
	void applyColorRule(WorldCell& worldcell, GLCell& glcell) const;

private:
	void __tick();

private:
	App& _app;
	//
	bool _paused;
	double _timeSinceLastTickSec;
	int _tickPerSec;
	double _timeAccSec;
	//
	char* _newRuleBuffer;
	SimRule _rule;
	ColorRule _colorRule;
	int _seed, _genprob;
	World _world;
	Renderer _renderer;
};

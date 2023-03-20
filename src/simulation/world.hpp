#pragma once

#include <memory.h>

struct WorldCell {
	int status;
	int neighbours;
	int nextstatus;
};

class World {
public:
	World(World&&) = delete;
	World();
	World(int side, WorldCell out = { 0, 0, 0 });
	~World();
	World& operator=(World&& other) noexcept;
	//
	int side() const;
	int size() const;
	//
	WorldCell get(int i) const;
	void set(WorldCell value, int i) const;
	//
	WorldCell get(int x, int y, int z) const;
	void set(WorldCell value, int x, int y, int z) const;
	//
	int countMoore(int status, int x, int y, int z) const;
	int countNeumann(int status, int x, int y, int z) const;
	//
	void flip() const;

private:
	WorldCell _out;
	int _side, _side2, _side3;
	WorldCell* _data;
};


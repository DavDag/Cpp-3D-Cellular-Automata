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
	World(): World(0, {0, 0, 0}) {}
	World(int side, WorldCell out = { 0, 0, 0 }) {
		_out = out;
		_side = side;
		_side2 = side * side;
		_side3 = side * side * side;
		_data = new WorldCell[_side3];
		memset(this->_data, 0, sizeof(WorldCell) * this->size());
	}
	~World() { delete[] this->_data; }
	World& operator=(World&& other) noexcept {
		delete[] this->_data;
		this->_side = other._side;
		this->_side2 = other._side2;
		this->_side3 = other._side3;
		this->_data = new WorldCell[this->_side3];
		memcpy_s(this->_data, sizeof(WorldCell) * this->size(), other._data, sizeof(WorldCell) * other.size());
		return *this;
	}
	//
	int side() const { return this->_side; }
	int size() const { return this->_side3; }
	//
	WorldCell get(int i) const { return this->_data[i]; }
	void set(WorldCell value, int i) const { this->_data[i] = value; }
	//
	WorldCell get(int x, int y, int z) const {
		if (x == -1 || y == -1 || z == -1) return this->_out;
		if (x == this->_side || y == this->_side || z == this->_side) return this->_out;
		const int cx = x * this->_side2;
		const int cy = y * this->_side;
		const int cz = z;
		return this->_data[cx + cy + cz];
	}
	void set(WorldCell value, int x, int y, int z) const {
		const int cx = x * this->_side2;
		const int cy = y * this->_side;
		const int cz = z;
		this->_data[cx + cy + cz] = value;
	}
	//
	int countMoore(int status, int x, int y, int z) const {
		// TODO: Improve ?
		int result = 0;
		result -= (this->get(x, y, z).status == status) ? 1 : 0;
		for (int dx = -1; dx <= 1; ++dx)
			for (int dy = -1; dy <= 1; ++dy)
				for (int dz = -1; dz <= 1; ++dz)
					result += (this->get(x + dx, y + dy, z + dz).status == status) ? 1 : 0;
		return result;
	}
	int countNeumann(int status, int x, int y, int z) const {
		// TODO: Improve ?
		int result = 0;
		result += (this->get(x + 1, y + 0, z + 0).status == status) ? 1 : 0;
		result += (this->get(x - 1, y + 0, z + 0).status == status) ? 1 : 0;
		result += (this->get(x + 0, y + 1, z + 0).status == status) ? 1 : 0;
		result += (this->get(x + 0, y - 1, z + 0).status == status) ? 1 : 0;
		result += (this->get(x + 0, y + 0, z + 1).status == status) ? 1 : 0;
		result += (this->get(x + 0, y + 0, z - 1).status == status) ? 1 : 0;
		return result;
	}
	//
	void flip() const {
		for (int i = 0; i < this->size(); ++i)
			this->_data[i].status = this->_data[i].nextstatus;
	}

private:
	WorldCell _out;
	int _side, _side2, _side3;
	WorldCell* _data;
};


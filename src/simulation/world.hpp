#pragma once

#include <memory.h>

template<typename T>
class World {
public:
	World<T>(World<T>&&) = delete;
	World<T>(): World<T>(0, {0}) {}
	World<T>(int side, T out) {
		_out = out;
		_side = side;
		_side2 = side * side;
		_side3 = side * side * side;
		_data = new T[_side3];
		memset(this->_data, 0, sizeof(T) * this->size());
	}
	~World<T>() { delete[] this->_data; }
	constexpr World<T>& operator=(World<T>&& other) noexcept {
		delete[] this->_data;
		this->_side = other._side;
		this->_side2 = other._side2;
		this->_side3 = other._side3;
		this->_data = new T[this->_side3];
		memcpy_s(this->_data, sizeof(T) * this->size(), other._data, sizeof(T) * other.size());
		return *this;
	}
	//
	int side() const { return this->_side; }
	int size() const { return this->_side3; }
	//
	T get(int i) const { return this->_data[i]; }
	void set(T value, int i) const { this->_data[i] = value; }
	//
	T get(int x, int y, int z) const {
		if (x == -1 || y == -1 || z == -1) return this->_out;
		if (x == this->_side || y == this->_side || z == this->_side) return this->_out;
		const int cx = x * this->_side2;
		const int cy = y * this->_side;
		const int cz = z;
		return this->_data[cx + cy + cz];
	}
	void set(T value, int x, int y, int z) const {
		const int cx = x * this->_side2;
		const int cy = y * this->_side;
		const int cz = z;
		this->_data[cx + cy + cz] = value;
	}

private:
	T _out;
	int _side, _side2, _side3;
	T* _data;
};


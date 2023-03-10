#pragma once

#include <glm/glm.hpp>

class App;

class Camera {
public:
	Camera(App& app, float fovy, float near, float far, const glm::vec3& pos, const glm::vec3& dir);
	//
	const glm::mat4& matrix();
	void info() const;
	float zoom() const;
	//
	void movepos(const glm::vec3& delta);
	void setpos(const glm::vec3& pos);
	void movedir(const glm::vec3& delta);
	void setdir(const glm::vec3& dir);
	void setfovy(float fovy);
	void setviewport(int w, int h);
	void setplanes(float near, float far);
	//
	void movezoom(float delta);
	void movezoomPercentage(float percentage);
	void setzoom(float zoom);
	void setzoomLimits(float min, float max);
	//
	void locktarget(const glm::vec3& targetpos);
	void unlocktarget();

private:
	App& _app;
	//
	float _fovy;
	int _w, _h;
	float _near, _far;
	glm::vec3 _pos;
	glm::vec3 _dir;
	//
	float _zoom, _zoomMin, _zoomMax;
	//
	bool _lockTarget;
	glm::vec3 _target;
	//
	bool _dirty;
	glm::mat4 _matrix;
};

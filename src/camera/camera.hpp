#pragma once

#include <glm/glm.hpp>

class Camera {
public:
	Camera();

	const glm::mat4& viewmatrix();

private:
	glm::mat4 a;
};

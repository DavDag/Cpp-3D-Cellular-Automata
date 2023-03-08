#include "camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

static glm::vec3 v3fZero = glm::vec3(0.0f);

Camera::Camera(float fovy, float near, float far, const glm::vec3& pos, const glm::vec3& dir) {
	this->_fovy = fovy;
	this->_w = 0;
	this->_h = 0;
	this->_near = near;
	this->_far = far;
	this->_pos = pos;
	this->_dir = dir;
	//
	this->_zoom = 1.0;
	this->_zoomMin = 1.0;
	this->_zoomMax = 1.0;
	//
	this->_lockTarget = false;
	this->_target = glm::vec3(0.0f);
	//
	this->_dirty = true;
	this->_matrix = glm::mat4(1.0f);
}

const glm::mat4& Camera::matrix() {
	if (this->_dirty) {
		// To prevent "reverse" effect on camera
		float fovyWithZoom = glm::min(175.0f, this->_fovy * this->_zoom);
		// perspective
		glm::mat4 persp =
			glm::perspective<float>(
				glm::radians(fovyWithZoom),
				this->_w / (float)this->_h,
				this->_near, this->_far
			);
		// view
		glm::mat4 view =
			(this->_lockTarget)
			? glm::lookAt<float>(
				this->_pos,
				this->_target,
				glm::vec3(0, 1, 0)
			)
			: glm::lookAt<float>(
				this->_pos,
				this->_pos + this->_dir,
				glm::vec3(0, 1, 0)
			);
		// combine
		this->_matrix = persp * view;
		this->_dirty = false;
	}
	return this->_matrix;
}

void Camera::info(char* buff, int buffsize) const {
	sprintf_s(buff, buffsize,
		"FOVY: %.2f deg\n"
		"Viewport: [%dx%d] px\n"
		"Planes: %.2f (near) %.2f (far)\n"
		"Zoom: %.1f %%\n"
		"ZoomLimits: [%.1f%%, %.1f%%]\n"
		"Pos: [%.2f,%.2f,%.2f]\n"
		"Dir: [%.2f,%.2f,%.2f]\n"
		"LockTarget: %s\n"
		"Target: [%.2f,%.2f,%.2f]\n",
		this->_fovy,
		this->_w, this->_h,
		this->_near, this->_far,
		this->_zoom * 100.0,
		this->_zoomMin * 100.0, this->_zoomMax * 100.0,
		this->_pos.x, this->_pos.y, this->_pos.z,
		this->_dir.x, this->_dir.y, this->_dir.z,
		(this->_lockTarget) ? "yes" : "no",
		this->_target.x, this->_target.y, this->_target.z
	);
}

void Camera::movepos(const glm::vec3& delta) {
	this->_dirty |= (delta != v3fZero);
	this->_pos += delta;
}

void Camera::setpos(const glm::vec3& pos) {
	this->_dirty |= (pos != this->_pos);
	this->_pos = pos;
}

void Camera::movedir(const glm::vec3& delta) {
	this->_dirty |= (delta != v3fZero);
	this->_dir += delta;
}

void Camera::setdir(const glm::vec3& dir) {
	this->_dirty |= (dir != this->_dir);
	this->_dir = dir;
}

void Camera::setfovy(float fovy) {
	this->_dirty |= (this->_fovy != fovy);
	this->_fovy = fovy;
}

void Camera::setviewport(int w, int h) {
	this->_dirty |= (this->_w != w) || (this->_h != h);
	this->_w = w;
	this->_h = h;
}

void Camera::setplanes(float near, float far) {
	this->_dirty |= (this->_near != near) || (this->_far != far);
	this->_near = near;
	this->_far = far;
}

void Camera::movezoom(float delta) {
	this->_dirty |= (delta != 0.0f);
	this->_zoom += delta;
	this->_zoom = glm::max(this->_zoom, this->_zoomMin);
	this->_zoom = glm::min(this->_zoom, this->_zoomMax);
}

void Camera::movezoomPercentage(float percentage) {
	this->movezoom(this->_zoom * percentage);
}

void Camera::setzoom(float zoom) {
	this->_dirty |= (this->_zoom != zoom);
	this->_zoom = zoom;
	this->_zoom = glm::max(this->_zoom, this->_zoomMin);
	this->_zoom = glm::min(this->_zoom, this->_zoomMax);
}

void Camera::setzoomLimits(float min, float max) {
	this->_dirty |= (this->_zoomMin != min) || (this->_zoomMax != max);
	this->_zoomMin = min;
	this->_zoomMax = max;
	this->_zoom = glm::max(this->_zoom, this->_zoomMin);
	this->_zoom = glm::min(this->_zoom, this->_zoomMax);
}

void Camera::locktarget(const glm::vec3& targetpos) {
	this->_dirty |= (this->_lockTarget != true) || (this->_target != targetpos);
	this->_lockTarget = true;
	this->_target = targetpos;
}

void Camera::unlocktarget() {
	this->_dirty |= (this->_lockTarget != false) || (this->_target != v3fZero);
	this->_lockTarget = false;
	this->_target = v3fZero;
}

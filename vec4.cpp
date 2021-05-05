#include "vec4.h"

#include <cmath>

vec4::vec4() {}

vec4::vec4(float x, float y, float z) : x(x), y(y), z(z) {}

vec4::vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

float vec4::length() {
	return sqrt(this->dot(*this));
}

vec4 vec4::normalize() {
	float l = this->length();
	return { this->x / l, this->y / l, this->z / l };
}

float vec4::dot(vec4& v) {
	return this->x * v.x + this->y * v.y + this->z * v.z;
}

vec4 vec4::cross(vec4& v) {
	return {
		this->y * v.z - this->z * v.y,
		this->z * v.x - this->x * v.z,
		this->x * v.y - this->y * v.x
	};
}

vec4 vec4::operator+(const vec4& v) {
	return { this->x + v.x, this->y + v.y, this->z + v.z };
}

vec4 vec4::operator-(const vec4& v) {
	return { this->x - v.x, this->y - v.y, this->z - v.z };
}

vec4 vec4::operator*(float k) {
	return { this->x * k, this->y * k, this->z * k };
}

vec4 vec4::operator/(float k) {
	return { this->x / k, this->y / k, this->z / k };
}

vec4 vec4::planeIntersect(vec4& plane_p, vec4& plane_n, vec4& lineStart, vec4& lineEnd) {
	plane_n = plane_n.normalize();
	float plane_d = -plane_n.dot(plane_p);
	float ad = lineStart.dot(plane_n);
	float bd = lineEnd.dot(plane_n);
	float t = (-plane_d - ad) / (bd - ad);
	vec4 lineStartToEnd = lineEnd - lineStart;
	vec4 lineToIntersect = lineStartToEnd * t;
	return lineStart + lineToIntersect;
}
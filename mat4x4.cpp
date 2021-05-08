/* Created: 29.04.2021
 * Author: Makar Ivashko
 * Description: Class for managing 4x4 transformation matrices.
 * Functionality: vector and matrix multiplication, creation of
 * rotation, translation, and projection matrices
 */

#include "mat4x4.h"

#include <cmath>

mat4x4::mat4x4() {
	this->m[0][0] = 1.0f;
	this->m[1][1] = 1.0f;
	this->m[2][2] = 1.0f;
	this->m[3][3] = 1.0f;
}

mat4x4 mat4x4::makeRotationX(float angleRad)
{
	mat4x4 matrix;
	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = cosf(angleRad);
	matrix.m[1][2] = sinf(angleRad);
	matrix.m[2][1] = -sinf(angleRad);
	matrix.m[2][2] = cosf(angleRad);
	matrix.m[3][3] = 1.0f;
	return matrix;
}

mat4x4 mat4x4::makeRotationY(float angleRad)
{
	mat4x4 matrix;
	matrix.m[0][0] = cosf(angleRad);
	matrix.m[0][2] = sinf(angleRad);
	matrix.m[2][0] = -sinf(angleRad);
	matrix.m[1][1] = 1.0f;
	matrix.m[2][2] = cosf(angleRad);
	matrix.m[3][3] = 1.0f;
	return matrix;
}

mat4x4 mat4x4::makeRotationZ(float angleRad)
{
	mat4x4 matrix;
	matrix.m[0][0] = cosf(angleRad);
	matrix.m[0][1] = sinf(angleRad);
	matrix.m[1][0] = -sinf(angleRad);
	matrix.m[1][1] = cosf(angleRad);
	matrix.m[2][2] = 1.0f;
	matrix.m[3][3] = 1.0f;
	return matrix;
}

mat4x4 mat4x4::makeTranslation(float x, float y, float z)
{
	mat4x4 matrix;
	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = 1.0f;
	matrix.m[2][2] = 1.0f;
	matrix.m[3][3] = 1.0f;
	matrix.m[3][0] = x;
	matrix.m[3][1] = y;
	matrix.m[3][2] = z;
	return matrix;
}

mat4x4 mat4x4::cameraTransform(vec4& pos, vec4& target, vec4& up) {
	// new forward direction
	vec4 newForward = target - pos;
	newForward = newForward.normalize();

	// new up direction
	vec4 newUp = (up - newForward * up.dot(newForward)).normalize();

	// new right direction
	vec4 newRight = newUp.cross(newForward);

	// transform matrix
	mat4x4 matrix;
	matrix.m[0][0] = newRight.x;	matrix.m[0][1] = newRight.y;	matrix.m[0][2] = newRight.z;	matrix.m[0][3] = 0.0f;
	matrix.m[1][0] = newUp.x;		matrix.m[1][1] = newUp.y;		matrix.m[1][2] = newUp.z;		matrix.m[1][3] = 0.0f;
	matrix.m[2][0] = newForward.x;	matrix.m[2][1] = newForward.y;	matrix.m[2][2] = newForward.z;	matrix.m[2][3] = 0.0f;
	matrix.m[3][0] = pos.x;			matrix.m[3][1] = pos.y;			matrix.m[3][2] = pos.z;			matrix.m[3][3] = 1.0f;
	return matrix;
}

mat4x4 mat4x4::createProjection(float fovDeg, float aspectRatio, float distNear, float distFar)
{
	float fFovRad = 1.0f / tanf(fovDeg * 0.5f / 180.0f * 3.14159f);
	mat4x4 matrix;
	matrix.m[0][0] = aspectRatio * fFovRad;
	matrix.m[1][1] = fFovRad;
	matrix.m[2][2] = distFar / (distFar - distNear);
	matrix.m[3][2] = (-distFar * distNear) / (distFar - distNear);
	matrix.m[2][3] = 1.0f;
	matrix.m[3][3] = 0.0f;
	return matrix;
}

mat4x4 mat4x4::quickInverse() 
{
	mat4x4 matrix;
	matrix.m[0][0] = this->m[0][0]; matrix.m[0][1] = this->m[1][0]; matrix.m[0][2] = this->m[2][0]; matrix.m[0][3] = 0.0f;
	matrix.m[1][0] = this->m[0][1]; matrix.m[1][1] = this->m[1][1]; matrix.m[1][2] = this->m[2][1]; matrix.m[1][3] = 0.0f;
	matrix.m[2][0] = this->m[0][2]; matrix.m[2][1] = this->m[1][2]; matrix.m[2][2] = this->m[2][2]; matrix.m[2][3] = 0.0f;
	matrix.m[3][0] = -(this->m[3][0] * matrix.m[0][0] + this->m[3][1] * matrix.m[1][0] + this->m[3][2] * matrix.m[2][0]);
	matrix.m[3][1] = -(this->m[3][0] * matrix.m[0][1] + this->m[3][1] * matrix.m[1][1] + this->m[3][2] * matrix.m[2][1]);
	matrix.m[3][2] = -(this->m[3][0] * matrix.m[0][2] + this->m[3][1] * matrix.m[1][2] + this->m[3][2] * matrix.m[2][2]);
	matrix.m[3][3] = 1.0f;
	return matrix;
}

vec4 mat4x4::operator* (const vec4& i) const  {
	vec4 v;
	v.x = i.x * this->m[0][0] + i.y * this->m[1][0] + i.z * this->m[2][0] + i.w * this->m[3][0];
	v.y = i.x * this->m[0][1] + i.y * this->m[1][1] + i.z * this->m[2][1] + i.w * this->m[3][1];
	v.z = i.x * this->m[0][2] + i.y * this->m[1][2] + i.z * this->m[2][2] + i.w * this->m[3][2];
	v.w = i.x * this->m[0][3] + i.y * this->m[1][3] + i.z * this->m[2][3] + i.w * this->m[3][3];
	return v;
}

mat4x4 mat4x4::operator* (const mat4x4& m) const {
	mat4x4 matrix;
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++)
			matrix.m[r][c] = this->m[r][0] * m.m[0][c] + this->m[r][1] * m.m[1][c] + this->m[r][2] * m.m[2][c] + this->m[r][3] * m.m[3][c];
	return matrix;
}
/* Created: 29.04.2021
 * Author: Makar Ivashko
 * Description: Class for managing 4x4 transformation matrices.
 * Functionality: vector and matrix multiplication, creation of
 * rotation, translation, and projection matrices
 */

#pragma once

#include "vec4.h"

class mat4x4
{
public:
	float m[4][4] = { 0 };

	// identity matrix is created by default
	mat4x4();
	
	// matrix inverse
	mat4x4 quickInverse();

	// matrix multiplication by vector
	vec4 operator* (vec4& i);
	
	// matrix multiplication by matrix
	mat4x4 operator* (mat4x4& m);

	// creation of rotational matrix around X axis
	static mat4x4 makeRotationX(float angleRad);

	// creation of rotational matrix around Y axis
	static mat4x4 makeRotationY(float angleRad);

	// creation of rotational matrix around Z axis
	static mat4x4 makeRotationZ(float angleRad);

	// creation of tranlational matrix
	static mat4x4 makeTranslation(float x, float y, float z);

	// camera transformation
	static mat4x4 cameraTransform(vec4& pos, vec4& target, vec4& up);

	// creation of matrix for polygon projection
	static mat4x4 createProjection(float fovDeg, float aspectRatio, float distNear, float distFar);
};
/* Created: 29.04.2021
 * Author: Makar Ivashko
 * Description: Class for managing 3d vectors, containing 3-dimensional 
 * coordinates and 1 axillary variable for matrix transformations. Can do 
 * basic vector operations: multiplication, addition, cross, and dot product
 */

#pragma once

class vec4 {
public:
	// 3 dimensional coordinates
	float x = 0;
	float y = 0;
	float z = 0;
	// Addidional term to perform matrix vector multiplication
	float w = 1;

	// default constractor
	vec4();

	// basic constractor of dimensional variables
	vec4(float x, float y, float z);

	// constractor with additional variable as parameter
	vec4(float x, float y, float z, float w);

	// get vector length
	float length();

	// vector normalization 
	vec4 normalize();

	// dot product of two vectors
	float dot(vec4& v);

	// cross product of two vectors
	vec4 cross(vec4& v);

	// addition of two vectors
	vec4 operator+(const vec4& v);

	// substraction of two vectors
	vec4 operator-(const vec4& v);

	// multiplication by constant
	vec4 operator*(float k);

	// division by constant 
	vec4 operator/(float k);
	static vec4 planeIntersect(vec4& plane_p, vec4& plane_n, vec4& lineStart, vec4& lineEnd);
};

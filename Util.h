/* Created: 29.04.2021
 * Author: Makar Ivashko
 * Short description: polygon and mesh classes, with some utility functions
 */

#pragma once

#include "vec4.h"
#include "mat4x4.h"

#include <vector>
#include <string>

class polygon
{
public:
	vec4 p[3];
	unsigned int color = 0;

	static int clipAgainstPlane(vec4 plane_p, vec4 plane_n, polygon& in_tri, polygon& out_tri1, polygon& out_tri2);
};

class mesh
{
public:
	std::vector<polygon> polys;

	bool loadObjectFile(std::string sFilename);
};
